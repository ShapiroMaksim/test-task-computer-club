//Author: Maksim Shapiro
#include <iostream>
#include <string>
#include <exception>

#include "event-handler/eventHandler.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Invalid set of arguments." << std::endl;
        return 1;
    }
    try {
        std::string fileName = std::string(argv[1]);
        EventHandler handler;
        handler.ParseFile(fileName);
        handler.processEvents();
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
    return 0;
}
