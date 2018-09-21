#include <iostream>
#include <string>
#include <algorithm>
#include "command_handler.h"


int main(int argc, char** argv) {
    if (argc == 1 || argc > 2) {
        std::cerr << "Wrong number of arguments. Aborting" << std::endl;
        return 1;
    }

    std::string bulkSize(argv[1]);
    if (std::all_of(bulkSize.begin(), bulkSize.end(), ::isdigit)) {
        CommandHandler(std::stoul(argv[1]), 2, std::cin);
        return 0;
    } else {
        std::cerr << "Argument must be a positive integer. Aborting" << std::endl;
        return 2;
    }
}