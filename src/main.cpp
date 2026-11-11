// main.cpp                                           Owner: Person 1
// Entry point. Reads the command line and calls the other modules.
// It contains no barcode logic.
//
//   barcode                        starts the interactive menu
//   barcode encode 123456 out.bmp  creates a barcode image
//   barcode decode out.bmp         reads a barcode image

#include <iostream>
#include <string>

#include "barcode/decoder.hpp"
#include "barcode/encoder.hpp"
#include "barcode/error.hpp"
#include "barcode/menu.hpp"

namespace {

    void printUsage() {
        std::cout << "Barcode Encoder and Decoder (EAN-13, 6-digit codes)\n\n"
            << "Usage:\n"
            << "  barcode                        Start the interactive menu\n"
            << "  barcode encode <code> <file>   Create a barcode image (e.g. barcode encode 123456 out.bmp)\n"
            << "  barcode decode <file>          Read a barcode image (e.g. barcode decode out.bmp)\n"
            << "  barcode help                   Show this message\n";
    }

    int runEncode(const std::string& code, const std::string& path) {
        generateToFile(code, path);
        std::cout << "Barcode for " << code << " saved to " << path << "\n";
        return 0;
    }

    int runDecode(const std::string& path) {
        std::cout << decodeFromFile(path) << "\n";
        return 0;
    }

}  // namespace

int main(int argc, char* argv[]) {
    try {
        // No arguments: start the interactive menu.
        if (argc == 1) {
            return runMenu();
        }

        const std::string command = argv[1];

        if (command == "help" || command == "--help" || command == "-h") {
            printUsage();
            return 0;
        }

        if (command == "encode") {
            if (argc != 4) {
                std::cerr << "Error: encode needs a code and an output file.\n\n";
                printUsage();
                return 1;
            }
            return runEncode(argv[2], argv[3]);
        }

        if (command == "decode") {
            if (argc != 3) {
                std::cerr << "Error: decode needs an image file.\n\n";
                printUsage();
                return 1;
            }
            return runDecode(argv[2]);
        }

        std::cerr << "Error: unknown command '" << command << "'.\n\n";
        printUsage();
        return 1;

    }
    catch (const BarcodeError& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << "\n";
        return 2;
    }
}