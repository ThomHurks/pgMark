#ifndef REGEX_SRE_PARSE_H
#define REGEX_SRE_PARSE_H

#include <string>
#include <functional>
#include <memory>
#include "tokenizer.h"
#include "state.h"
#include "subpattern.h"
#include "in.h"
#include "category_types.h"

class sre_parse {
private:
    const int SRE_FLAG_VERBOSE = 1;
    // TODO: find actual value and deduplicate.
    const int MAXREPEAT = 999;
    const int MAXGROUPS = 999;

    std::shared_ptr<Subpattern> parse(Tokenizer &a_Source, State &a_State, bool a_Verbose, int a_Nested, bool a_First = false);

    std::shared_ptr<Subpattern> parse_sub(Tokenizer &a_Source, State &a_State, bool a_Verbose, int a_Nested);

    std::shared_ptr<const Opcode> escape(Tokenizer &a_Source, std::wstring a_Escape, State &a_State);

    std::shared_ptr<const Opcode>  classEscape(Tokenizer &a_Source, std::wstring a_Escape);

    const std::vector<std::shared_ptr<const Opcode>> makeUnique(const std::vector<std::shared_ptr<const Opcode>> &a_Vector);

    std::pair<int, int> parseFlags(Tokenizer &a_Source, State &a_State, std::wstring &a_Character);

public:
    sre_parse() = default;

    const std::shared_ptr<Subpattern> parse(const std::string &a_String, int a_Flags = 0);
};


#endif //REGEX_SRE_PARSE_H
