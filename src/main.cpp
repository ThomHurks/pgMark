#include "main.h"
#include "graph_generator.h"
#include "node_attribute_generator.h"
#include <fstream>
#include <getopt.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    std::ios_base::sync_with_stdio(false);
    std::ostream::sync_with_stdio(false); // On some platforms, stdout flushes on \n.
    std::string graph_file;

    while (true) {
        int option_index = 0;
        static struct option long_options[] = {
                {"output", required_argument, nullptr, 'o'},
                {"help",   no_argument,       nullptr, 'h'},
                {nullptr,  0,                 nullptr, 0}
        };

        int c = getopt_long_only(argc, argv, "o:h",
                             long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
            case 'o':
                graph_file = std::string(optarg);
                break;
            case 'h':
                std::cout << "Usage: pgMark [OPTION]... SCHEMA_FILE.\n";
                std::cout << "Generate a graph according to a specified SCHEMA_FILE.\n";
                std::cout << "\n";
                std::cout << "Mandatory arguments to long options are mandatory for short options too.\n";
                std::cout << "-o, --output=FILE  the optional output file.\n";
                std::cout << "-h, --help         display this help and exit.\n";
                exit(EXIT_SUCCESS);
            default:
                exit(EXIT_FAILURE);
        }
    }

    if (optind >= argc) {
        std::cout << "Missing schema!\n";
        exit(EXIT_FAILURE);
    }
    std::string conf_file(argv[optind++]);

    if (optind >= argc) {
        std::cout << "Missing graph size!\n";
        exit(EXIT_FAILURE);
    }
    int graphSize = 0;
    try {
        graphSize = std::stoi(argv[optind++]);
    }
    catch (std::invalid_argument &) {
        std::cout << "Please input a valid number for the graph size.\n";
        exit(EXIT_FAILURE);
    }
    catch (std::out_of_range &) {
        std::cout << "Please input a graph size within a 32-bit integer range.\n";
        exit(EXIT_FAILURE);
    }
    if (graphSize <= 0) {

        std::cout << "Please input a graph size that is > 0.\n";
        exit(EXIT_FAILURE);
    }

    if (optind < argc) {
        std::cout << "Encountered extraneous values that are not options: ";
        while (optind < argc) {
            std::cout << argv[optind++] << " ";
        }
        std::cout << "\n";
        exit(EXIT_FAILURE);
    }

    if (!checkFileExists(conf_file)) {
        std::cout << "The given schema file does not exist.\n";
        exit(EXIT_FAILURE);
    }

    Configuration config(conf_file, graphSize);

    std::streambuf *buf;
    std::ofstream output_file;
    if (graph_file.empty()) {
        buf = std::cout.rdbuf();
    } else {
        output_file.open(graph_file);
        buf = output_file.rdbuf();
    }

    std::ostream graph_stream(buf);
    buf = nullptr;

    GraphGenerator generator(config);
    generator.generateGraph(graph_stream);

    NodeAttributeGenerator attributeGenerator(config);
    attributeGenerator.generateAttributes(graph_stream);
}

bool checkFileExists(const std::string &a_Name) {
    struct stat buffer{};
    return stat(a_Name.c_str(), &buffer) == 0;
}
