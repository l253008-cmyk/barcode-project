#include "barcode/menu.hpp"

#include <iostream>
#include <string>

#include "barcode/decoder.hpp"
#include "barcode/display.hpp"
#include "barcode/encoder.hpp"
#include "barcode/error.hpp"
#include "barcode/image.hpp"
#include "barcode/image_io.hpp"
#include "barcode/validator.hpp"

using namespace std;

static bool isTrimChar(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '"';
}


static string trim(const string& s) {
    int start = 0;
    int end = static_cast<int>(s.size());
    while (start < end && isTrimChar(s[start])) {
        ++start;
    }
    while (end > start && isTrimChar(s[end - 1])) {
        --end;
    }
    return s.substr(start, end - start);
}

static bool readLine(const string& prompt, string& out) {
    cout << prompt;
    if (!getline(cin, out)) {
        return false;
    }
    out = trim(out);
    return true;
}

static void printMenu() {
    cout << "\n=== Barcode Encoder and Decoder ===\n"
         << "  1  Generate barcode\n"
         << "  2  Read barcode\n"
         << "  3  Exit\n";
}

static void handleGenerate() {
    string code;
    if (!readLine("Enter a 6-digit number (first digit must not be 0): ", code)) {
        return;
    }
    requireValidCode(code);

    const Image img = generateBarcode(code);

    const string defaultPath = "barcode_" + code + ".bmp";
    string path;
    if (!readLine("Save as [" + defaultPath + "]: ", path) || path.empty()) {
        path = defaultPath;
    }

    saveImage(img, path);
    cout << "Barcode saved to: " << path << '\n';

    showImage(img);
    cout << "Barcode for " << code << " is shown on the screen.\n";
}

static void handleRead() {
    string path;
    if (!readLine("Enter the image file path (BMP or PGM): ", path)) {
        return;
    }
    if (path.empty()) {
        throw BarcodeError("No file path was entered.");
    }

    const string code = decodeFromFile(path);
    cout << "Decoded number: " << code << '\n';
}

int runMenu() {
    string choice;

    while (true) {
        printMenu();
        if (!readLine("Choose an option (1-3): ", choice)) {
            cout << '\n';
            return 0;
        }

        if (choice == "3") {
            cout << "Goodbye.\n";
            return 0;
        }

        try {
            if (choice == "1") {
                handleGenerate();
            } else if (choice == "2") {
                handleRead();
            } else {
                cout << "Invalid choice. Please enter 1, 2 or 3.\n";
            }
        } catch (const BarcodeError& e) {
            cout << "Error: " << e.what() << '\n';
        }
    }
}
