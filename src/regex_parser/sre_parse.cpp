#include "sre_parse.h"
#include "branch.h"
#include "character_sets.h"
#include "group_ref.h"
#include "group_ref_exists.h"
#include "max_repeat.h"
#include "min_repeat.h"
#include "subpattern_opcode.h"

std::shared_ptr<Subpattern> sre_parse::parse(Tokenizer &a_Source, State &a_State, bool a_Verbose, int a_Nested, bool a_First) {
    // Parse a simple pattern.
    auto subpattern = std::make_shared<Subpattern>(a_State);
    while (true) {
        std::wstring that = a_Source.readNext();
        if (that.empty()) {
            break; // End of pattern.
        }
        if (that == L"|" || that == L")") {
            break; // End of subpattern.
        }
        a_Source.get();

        // TODO(thom): implement verbose code?

        if (that[0] == '\\') {
            auto code = escape(a_Source, that, a_State);
            subpattern->append(code);
        } else if (CharacterSets::SPECIAL_CHARS.find(that) == CharacterSets::SPECIAL_CHARS.end()) {
            subpattern->append(std::make_shared<Literal>(that));
        } else if (that == L"[") {
            // Character set.
            std::vector<std::shared_ptr<const Opcode>> set;
            bool negate = a_Source.match(L"^");
            // Check remaining characters.
            while (true) {
                that = a_Source.get();
                if (that.empty()) {
                    throw std::invalid_argument("Unterminated character set");
                }
                if (that == L"]" && !set.empty()) {
                    break;
                }
                std::shared_ptr<const Opcode> code1 =
                        (that[0] == '\\') ? classEscape(a_Source, that) : std::make_shared<Literal>(that);
                if (a_Source.match(L"-")) {
                    // Potential range.
                    that = a_Source.get();
                    if (that.empty()) {
                        throw std::invalid_argument("Unterminated character set");
                    }
                    if (that == L"]") {
                        if (code1->getName() == "IN") {
                            code1 = std::dynamic_pointer_cast<const In>(code1)->getItem(0);
                        }
                        set.emplace_back(code1);
                        set.emplace_back(std::make_shared<Literal>('-'));
                        break;
                    }
                    const std::shared_ptr<const Opcode> code2 = (that[0] == '\\') ? classEscape(a_Source, that) : std::make_shared<const Literal>(that);
                    if (code1->getName() != "LITERAL" || code2->getName() != "LITERAL") {
                        throw std::invalid_argument("Bad character range");
                    }
                    wchar_t lo = std::dynamic_pointer_cast<const Literal>(code1)->getLiteral();
                    wchar_t hi = std::dynamic_pointer_cast<const Literal>(code2)->getLiteral();
                    if (hi < lo) {
                        throw std::invalid_argument("Bad character range");
                    }
                    set.emplace_back(std::make_shared<Range>(lo, hi));
                } else {
                    if (code1->getName() == "IN") {
                        code1 = std::dynamic_pointer_cast<const In>(code1)->getItem(0);
                    }
                    set.emplace_back(code1);
                }
            }
            set = makeUnique(set);
            if (set.size() == 1 && set[0]->getName() == "LITERAL") {
                // Optimization.
                if (negate) {
                    subpattern->append(std::make_shared<NotLiteral>(std::dynamic_pointer_cast<const Literal>(set[0])->getLiteral()));
                } else {
                    subpattern->append(set[0]);
                }
            } else {
                if (negate) {
                    set.insert(set.begin(), std::make_shared<Negate>());
                }
                // Charmap optimization can't be added here because global flags are still not known.
                subpattern->append(std::make_shared<In>(set));
            }
        } else if (CharacterSets::REPEAT_CHARS.find(that) != CharacterSets::REPEAT_CHARS.end()) {
            // Repeat previous item.
            int here = a_Source.tell();
            int min = 0;
            int max = 0;
            if (that == L"?") {
                min = 0;
                max = 1;
            } else if (that == L"*") {
                min = 0;
                max = MAXREPEAT;
            } else if (that == L"+") {
                min = 0;
                max = MAXREPEAT;
            } else if (that == L"{") {
                if (a_Source.readNext() == L"}") {
                    subpattern->append(std::make_shared<Literal>(that));
                    continue;
                }
                min = 0;
                max = MAXREPEAT;
                std::wstring lo;
                std::wstring hi;
                while (a_Source.readNext().length() == 1 && CharacterSets::DIGITS.find(a_Source.readNext()[0]) != CharacterSets::DIGITS.end()) {
                    lo += a_Source.get();
                }
                if (a_Source.match(L",")) {
                    while (a_Source.readNext().length() == 1 && CharacterSets::DIGITS.find(a_Source.readNext()[0]) != CharacterSets::DIGITS.end()) {
                        hi += a_Source.get();
                    }
                } else {
                    hi = lo;
                }
                if (!a_Source.match(L"}")) {
                    subpattern->append(std::make_shared<Literal>(that));
                    a_Source.seek(here);
                    continue;
                }
                if (!lo.empty()) {
                    min = std::stoi(lo);
                    if (min >= MAXREPEAT) {
                        throw std::invalid_argument("The repetition number is too large.");
                    }
                }
                if (!hi.empty()) {
                    max = std::stoi(hi);
                    if (max >= MAXREPEAT) {
                        throw std::invalid_argument("The repetition number is too large");
                    }
                    if (max < min) {
                        throw std::invalid_argument("Min repeat greater than max repeat.");
                    }
                }
            } else {
                throw std::invalid_argument("Unsupported quantifier.");
            }
            // Figure out which item to repeat.
            std::shared_ptr<const Subpattern> item;
            if (subpattern->length() > 0) {
                item = std::make_shared<const Subpattern>(a_State, subpattern->getItem(subpattern->length() - 1));
            }
            if (!item || item->getItem(0)->getName() == "AT") {
                throw std::invalid_argument("Nothing to repeat.");
            }
            if (CharacterSets::REPEATCODES.find(item->getItem(0)->getName()) != CharacterSets::REPEATCODES.end()) {
                throw std::invalid_argument("Multiple repeat.");
            }
            if (item->getItem(0)->getName() == "SUBPATTERN") {
                const auto& sub = std::dynamic_pointer_cast<const SubpatternOpcode>(item->getItem(0));
                if (sub->getGroup() == -1 && sub->getAddFlags() == 0 && sub->getDelFlags() == 0) {
                    item = sub->getSubpattern();
                }
            }
            if (a_Source.match(L"?")) {
                subpattern->setItem(subpattern->length() - 1, std::make_shared<MinRepeat>(min, max, item));
            } else {
                subpattern->setItem(subpattern->length() - 1, std::make_shared<MaxRepeat>(min, max, item));
            }
        } else if (that == L".") {
            subpattern->append(std::make_shared<Any>());
        } else if (that == L"(") {
            int group = 0; // TODO(thom): originally was True.
            std::wstring name;
            int add_flags = 0;
            int del_flags = 0;
            if (a_Source.match(L"?")) {
                // Options.
                std::wstring character = a_Source.get();
                if (character.empty()) {
                    throw std::invalid_argument("Unexpected end of pattern.");
                }
                if (character == L"P") {
                    // Python extensions.
                    if (a_Source.match(L"<")) {
                        // Named group: skip forward to end of name.
                        name = a_Source.getUntil(L">", "group name");
                        // TODO(thom): use regex to verify if it is an identifier?
                    } else if (a_Source.match(L"=")) {
                        // Named backreference.
                        name = a_Source.getUntil(L")", "group name");
                        // TODO(thom): use regex to verify if it is an identifier?
                        int gid;
                        try {
                            gid = a_State.getGroup(name);
                        } catch (std::out_of_range &) {
                            throw std::invalid_argument("Unknown group name.");
                        }
                        if (!a_State.checkGroup(gid)) {
                            throw std::invalid_argument("Cannot refer to an open group.");
                        }
                        a_State.checkLookBehindGroup(gid);
                        subpattern->append(std::make_shared<GroupRef>(gid, a_State));
                        continue;
                    } else {
                        character = a_Source.get();
                        if (character.empty()) {
                            throw std::invalid_argument("Unexpected end of pattern.");
                        }
                        throw std::invalid_argument("Unknown extension ?P");
                    }
                } else if (character == L":") {
                    // Non-capturing group.
                    group = -1;
                } else if (character == L"#") {
                    // Comment.
                    while (true) {
                        if (a_Source.readNext().empty()) {
                            throw std::invalid_argument("Missing ), unterminated comment");
                        }
                        if (a_Source.get() == L")") {
                            break;
                        }
                    }
                    continue;
                } else if (character == L"=" || character == L"!" || character == L"<") {
                    // Lookahead assertions.
                    int dir = 1;
                    int lookBehindGroups = -1;
                    if (character == L"<") {
                        character = a_Source.get();
                        if (character.empty()) {
                            throw std::invalid_argument("Unexpected end of pattern.");
                        }
                        if (character != L"=" && character != L"!") {
                            throw std::invalid_argument("Unknown extension ?<");
                        }
                        dir = -1; // Lookbehind.
                        lookBehindGroups = a_State.getLookBehindGroups();
                        if (lookBehindGroups == -1) { // is None
                            a_State.setLookBehindGroups(a_State.groups());
                        }
                    }
                    auto p = parse_sub(a_Source, a_State, a_Verbose, a_Nested + 1);
                    if (dir < 0) {
                        if (lookBehindGroups == -1) {
                            a_State.setLookBehindGroups(-1);
                        }
                    }
                    if (!a_Source.match(L")")) {
                        throw std::invalid_argument("Missing ), unterminated subpattern.");
                    }
                    if (character == L"=") {
                        subpattern->append(std::make_shared<Assert>(dir, *p));
                    } else {
                        subpattern->append(std::make_shared<AssertNot>(dir, *p));
                    }
                    continue;
                } else if (character == L"(") {
                    // Conditional backreference group.
                    std::wstring cond_name = a_Source.getUntil(L")", "group name");
                    a_State.getGroup(cond_name);
                    int cond_group;
                    bool is_integer = false;
                    try {
                        size_t after;
                        cond_group = std::stoi(cond_name, &after);
                        if (after < cond_name.size()) {
                            throw std::invalid_argument("Group is not a valid number");
                        }
                        is_integer = true;
                        if (cond_group < 0) {
                            throw std::out_of_range("Group number must be larger than 0.");
                        }
                        if (cond_group == 0) {
                            throw std::invalid_argument("Bad group number.");
                        }
                        if (cond_group >= MAXGROUPS) {
                            throw std::invalid_argument("Invalid group reference.");
                        }
                    } catch (std::exception&) {
                        if (is_integer) {
                            throw;
                        }
                        try {
                            cond_group = a_State.getGroup(cond_name);
                        } catch (std::out_of_range &) {
                            throw std::invalid_argument("Unknown group name.");
                            //throw std::invalid_argument("Bad character in group name.");
                        }
                    }
                    a_State.checkLookBehindGroup(cond_group);
                    auto item_yes = parse(a_Source, a_State, a_Verbose, a_Nested + 1);
                    std::shared_ptr<const Subpattern> item_no;
                    if (a_Source.match(L"|")) {
                        item_no = parse(a_Source, a_State, a_Verbose, a_Nested + 1);
                        if (a_Source.readNext() == L"|") {
                            throw std::invalid_argument("Conditional backref with more than two branches.");
                        }
                    } else {
                        item_no = nullptr;
                    }
                    if (!a_Source.match(L")")) {
                        throw std::invalid_argument("Missing ), unterminated subpattern.");
                    }
                    subpattern->append(std::make_shared<GroupRefExists>(cond_group, item_yes, item_no));
                    continue;
                } else if (CharacterSets::FLAGS.find(character) != CharacterSets::FLAGS.end() || character == L"-") {
                    // Flags.
                    std::pair<int, int> flags = parseFlags(a_Source, a_State, character);
                    add_flags = flags.first;
                    del_flags = flags.second;
                    group = -1;
                } else {
                    throw std::invalid_argument("Unknown extension ?");
                }
            }
            // Parse group contents.
            if (group != -1) {
                group = a_State.openGroup(name);
            }
            bool sub_verbose = false; // TODO(thom): get actual var.
            auto p = parse_sub(a_Source, a_State, sub_verbose, a_Nested + 1);
            if (!a_Source.match(L")")) {
                throw std::invalid_argument("Missing ), unterminated subpattern");
            }
            if (group != -1) {
                a_State.closeGroup(group, *p);
            }
            subpattern->append(std::make_shared<SubpatternOpcode>(group, add_flags, del_flags, p));
        } else if (that == L"^") {
            subpattern->append(std::make_shared<At>(E_CATEGORY_TYPE::AT_BEGINNING));
        } else if (that == L"$") {
            subpattern->append(std::make_shared<At>(E_CATEGORY_TYPE::AT_END));
        } else {
            throw std::invalid_argument("Unsupported special character.");
        }
    }
    // Unpack non-capturing groups.
    for (int i = subpattern->length() - 1; i >= 0; --i) {
        const auto& opcode = subpattern->getItem(i);
        if (opcode->getName() == "SUBPATTTERN") {
            const auto& sub = std::dynamic_pointer_cast<const SubpatternOpcode>(opcode);
            if (sub->getGroup() == -1 && !sub->getAddFlags() && !sub->getDelFlags()) {
                subpattern->deleteItem(i);
                for (int k = sub->getSubpattern()->length() - 1; k >= 0; --k) {
                    subpattern->insertItem(i, sub->getSubpattern()->getItem(k));
                }
            }
        }
    }
    return subpattern;
}

std::shared_ptr<Subpattern> sre_parse::parse_sub(Tokenizer &a_Source, State &a_State, bool a_Verbose, int a_Nested) {
    // Parse an alternation: a|b|c
    std::vector<std::shared_ptr<Subpattern>> items;
    while (true) {
        items.emplace_back(parse(a_Source, a_State, a_Verbose, a_Nested + 1, !a_Nested && items.empty()));
        if (!a_Source.match(L"|")) {
            break;
        }
    }

    if (items.size() == 1) {
        return items[0];
    }

    auto subpattern = std::make_shared<Subpattern>(a_State);

    // Check if all items share a common prefix.
    while (true) {
        bool invalidPrefix = false;
        std::shared_ptr<const Opcode> prefix;
        for (const auto &item : items) {
            // TODO(thom): check if item is truthy?
            if (!prefix) {
                prefix = item->getItem(0);
            } else if (item->getItem(0) != prefix) {
                invalidPrefix = true;
                break;
            }
        }
        if (!invalidPrefix) {
            // All sub-items start with a common prefix; move it out of the branch.
            for (auto &item : items) {
                item->deleteItem(0);
            }
            subpattern->append(prefix);
            continue; // Check the next one.
        }
        break;
    }

    // Check if the branch can be replaced by a character set.
    std::vector<std::shared_ptr<const Opcode>> orderedSet;
    bool validCharset = true;
    for (const auto &item : items) {
        if (item->length() != 1) {
            validCharset = false;
            break;
        }
        auto av = item->getItem(0);

        if (av->getName() == "LITERAL") {
            if (std::find(orderedSet.begin(), orderedSet.end(), av) == orderedSet.end()) {
                orderedSet.emplace_back(av);
            }
        } else if (av->getName() == "IN") {
            const auto in = std::dynamic_pointer_cast<const In>(av);
            if (in->getItem(0)->getName() != "NEGATE") {
                for (auto &arg : in->getArgs()) {
                    if (std::find(orderedSet.begin(), orderedSet.end(), arg) == orderedSet.end()) {
                        orderedSet.emplace_back(arg);
                    }
                }
            } else {
                validCharset = false;
                break;
            }
        } else {
            validCharset = false;
            break;
        }
    }
    if (validCharset) {
        subpattern->append(std::make_shared<In>(orderedSet));
        return subpattern;
    }

    subpattern->append(std::make_shared<const Branch>(items));

    return subpattern;
}

const std::shared_ptr<Subpattern> sre_parse::parse(const std::string &a_String, int a_Flags) {
    Tokenizer source(a_String);
    State state(a_String, a_Flags);
    bool verbose = (a_Flags & SRE_FLAG_VERBOSE) != 0;
    auto p = parse_sub(source, state, verbose, 0);

    state.fixFlags();

    if (!source.readNext().empty()) {
        assert(source.readNext() == L")");
        throw std::invalid_argument("Unbalanced parenthesis.");
    }

    return p;
}

std::shared_ptr<const Opcode> sre_parse::escape(Tokenizer &a_Source, std::wstring a_Escape, State &a_State) {
    // Handle escape code in expression.
    try {
        return CharacterSets::CATEGORIES.at(a_Escape);
    } catch (std::out_of_range &) {
        // Just continue.
    }
    try {
        return CharacterSets::ESCAPES.at(a_Escape);
    } catch (std::out_of_range &) {
        // Just continue.
    }
    assert(a_Escape.size() == 2);
    auto c = a_Escape.at(1);
    if (c == 'x') {
        // Hexadecimal escape.
        a_Escape += a_Source.getWhile(2, CharacterSets::HEXDIGITS);
        if (a_Escape.size() != 4) {
            throw std::invalid_argument("Incomplete escape ");
        }
        size_t idx;
        int character = std::stoi(a_Escape.substr(2), &idx, 16);
        if (idx < a_Escape.length() - 2) {
            throw std::invalid_argument("Invalid hexadecimal escape ");
        }
        return std::make_shared<Literal>(character);
    }
    if (c == 'u') { // TODO(thom): check for istext?
        // Unicode escape (exactly four digits).
        a_Escape += a_Source.getWhile(4, CharacterSets::HEXDIGITS);
        if (a_Escape.size() != 6) {
            throw std::invalid_argument("Incomplete escape ");
        }
        size_t idx;
        int character = std::stoi(a_Escape.substr(2), &idx, 16);
        if (idx < a_Escape.length() - 2) {
            throw std::invalid_argument("Invalid unicode escape ");
        }
        return std::make_shared<Literal>(character);
    }
    if (c == 'U') {
        // Unicode escape (exactly eight digits).
        a_Escape += a_Source.getWhile(8, CharacterSets::HEXDIGITS);
        if (a_Escape.size() != 10) {
            throw std::invalid_argument("Incomplete escape ");
        }
        size_t idx;
        int character = std::stoi(a_Escape.substr(2), &idx, 16);
        if (idx < a_Escape.length() - 2) {
            throw std::invalid_argument("Invalid unicode escape ");
        }
        // TODO(thom): raise error for invalid code with chr(c).
        return std::make_shared<Literal>(character);
    }
    if (c == 'N') {
        // Named unicode escape e.g. \N{EM DASH}.
        if (!a_Source.match(L"{")) {
            throw std::invalid_argument("Missing {");
        }
        std::wstring charName = a_Source.getUntil(L"}", "character name");
        //try {
        //    // TODO: lookup and convert to ordinal.
        //    int character = ord(unicodedata.lookup(charName));
        //    return new Literal(character);
        //} catch (std::exception&) {
        //    throw std::invalid_argument("Undefined character name " + charName);
        //}
        throw std::invalid_argument("Named unicode escapes not yet supported.");
    }
    if (c == '0') {
        // Octal escape.
        a_Escape += a_Source.getWhile(2, CharacterSets::OCTDIGITS);
        size_t idx;
        int character = std::stoi(a_Escape.substr(1), &idx, 8);
        if (idx < a_Escape.length() - 1) {
            throw std::invalid_argument("Invalid octal escape ");
        }
        // TODO(thom): this check is not present in the original code. Needed?
        if (character > 0377) {
            throw std::invalid_argument("Octal escape value outside of range 0-0377.");
        }
        return std::make_shared<Literal>(character);
    }
    if (CharacterSets::DIGITS.find(c) != CharacterSets::DIGITS.end()) {
        // Octal escape *or* decimal group reference.
        if (CharacterSets::DIGITS.find(a_Source.readNext()[0]) != CharacterSets::DIGITS.end()) {
            a_Escape += a_Source.get();
            if (CharacterSets::OCTDIGITS.find(a_Escape[1]) != CharacterSets::OCTDIGITS.end() &&
                CharacterSets::OCTDIGITS.find(a_Escape[2]) != CharacterSets::OCTDIGITS.end() &&
                CharacterSets::OCTDIGITS.find(a_Source.readNext()[0]) != CharacterSets::OCTDIGITS.end()) {
                // Got three octal digits; this is an octal escape.
                a_Escape += a_Source.get();
                size_t idx;
                int character = std::stoi(a_Escape.substr(1), &idx, 8);
                if (idx < a_Escape.length() - 1) {
                    throw std::invalid_argument("Invalid octal escape ");
                }
                if (character > 0377) {
                    throw std::invalid_argument("Octal escape value outside of range 0-0377.");
                }
                return std::make_shared<Literal>(character);
            }
        }
        // Not an octal escape, so this is a group reference.
        size_t idx;
        int group = std::stoi(a_Escape.substr(1), &idx);
        if (idx < a_Escape.length() - 1) {
            throw std::invalid_argument("Invalid group reference ");
        }
        if (group < a_State.groups()) {
            if (!a_State.checkGroup(group)) {
                throw std::invalid_argument("Cannot refer to an open group.");
            }
            a_State.checkLookBehindGroup(group);
            return std::make_shared<GroupRef>(group, a_State);
        }
        throw std::invalid_argument("Invalid group reference.");
    }
    if (a_Escape.length() == 2) {
        if (CharacterSets::ASCIILETTERS.find(c) != CharacterSets::ASCIILETTERS.end()) {
            throw std::invalid_argument("Bad escape");
        }
        return std::make_shared<Literal>(c); // Originally passed as int.
    }
    throw std::invalid_argument("Bad escape.");
}

std::shared_ptr<const Opcode> sre_parse::classEscape(Tokenizer &a_Source, std::wstring a_Escape) {
    // Handle escape code inside character class.
    try {
        return CharacterSets::ESCAPES.at(a_Escape);
    } catch (std::out_of_range &) {
        // Just continue.
    }
    try {
        auto code = CharacterSets::CATEGORIES.at(a_Escape);
        if (code->getName() == "IN") {
            return std::move(code);
        }
    } catch (std::out_of_range &) {
        // Just continue.
    }
    assert(a_Escape.size() == 2);
    auto c = a_Escape.at(1);
    if (c == 'x') {
        // Hexadecimal escape (exactly two digits).
        a_Escape += a_Source.getWhile(2, CharacterSets::HEXDIGITS);
        if (a_Escape.size() != 4) {
            throw std::invalid_argument("Incomplete escape ");
        }
        size_t idx;
        int character = std::stoi(a_Escape.substr(2), &idx, 16);
        if (idx < a_Escape.length() - 2) {
            throw std::invalid_argument("Invalid hexadecimal escape ");
        }
        return std::make_shared<Literal>(character);
    }
    if (c == 'u') { // TODO(thom): check for istext?
        // Unicode escape (exactly four digits).
        a_Escape += a_Source.getWhile(4, CharacterSets::HEXDIGITS);
        if (a_Escape.size() != 6) {
            throw std::invalid_argument("Incomplete escape ");
        }
        size_t idx;
        int character = std::stoi(a_Escape.substr(2), &idx, 16);
        if (idx < a_Escape.length() - 2) {
            throw std::invalid_argument("Invalid unicode escape ");
        }
        return std::make_shared<Literal>(character);
    }
    if (c == 'U') {
        // Unicode escape (exactly eight digits).
        a_Escape += a_Source.getWhile(8, CharacterSets::HEXDIGITS);
        if (a_Escape.size() != 10) {
            throw std::invalid_argument("Incomplete escape ");
        }
        size_t idx;
        int character = std::stoi(a_Escape.substr(2), &idx, 16);
        if (idx < a_Escape.length() - 2) {
            throw std::invalid_argument("Invalid unicode escape ");
        }
        // TODO(thom): raise error for invalid code with chr(c).
        return std::make_shared<Literal>(character);
    }
    if (c == 'N') {
        // Named unicode escape e.g. \N{EM DASH}.
        if (!a_Source.match(L"{")) {
            throw std::invalid_argument("Missing {");
        }
        std::wstring charName = a_Source.getUntil(L"}", "character name");
        //try {
        //    // TODO: lookup and convert to ordinal.
        //    int character = ord(unicodedata.lookup(charName));
        //    return new Literal(character);
        //} catch (std::exception&) {
        //    throw std::invalid_argument("Undefined character name " + charName);
        //}
        throw std::invalid_argument("Named unicode escapes not yet supported.");
    }
    if (CharacterSets::OCTDIGITS.find(c) != CharacterSets::OCTDIGITS.end()) {
        // Octal escape (up to three digits).
        a_Escape += a_Source.getWhile(2, CharacterSets::OCTDIGITS);
        size_t idx;
        int character = std::stoi(a_Escape.substr(1), &idx, 8);
        if (idx < a_Escape.length() - 1) {
            throw std::invalid_argument("Invalid octal escape ");
        }
        if (character > 0377) {
            throw std::invalid_argument("Octal escape value outside of range 0-0377.");
        }
        return std::make_shared<Literal>(character);
    }
    if (CharacterSets::DIGITS.find(c) != CharacterSets::DIGITS.end()) {
        throw std::invalid_argument("Bad escape; starts with digit.");
    }
    if (a_Escape.length() == 2) {
        if (CharacterSets::ASCIILETTERS.find(c) != CharacterSets::ASCIILETTERS.end()) {
            throw std::invalid_argument("Bad escape");
        }
        return std::make_shared<Literal>(c);
    }
    throw std::invalid_argument("Bad escape.");
}

const std::vector<std::shared_ptr<const Opcode>> sre_parse::makeUnique(const std::vector<std::shared_ptr<const Opcode>> &a_Vector) {
    std::unordered_set<std::shared_ptr<const Opcode>> set(a_Vector.begin(), a_Vector.end());
    if (set.size() == a_Vector.size()) {
        return a_Vector;
    }
    std::vector<std::shared_ptr<const Opcode>> uniqueVector(set.size());
    for (const auto &opcode : a_Vector) {
        if (set.erase(opcode) != 0) {
            uniqueVector.emplace_back(opcode);
        }
    }
    return uniqueVector;
}

std::pair<int, int> sre_parse::parseFlags(Tokenizer &a_Source, State &a_State, std::wstring &a_Character) {
    // TODO(thom): implement.
    return std::pair<int, int>(0, 0);
}
