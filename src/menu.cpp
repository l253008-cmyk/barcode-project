#include <iostream>
#include <string>
#include "menu.hpp"
#include "generator.hpp"
#include "display.hpp"
#include "decoder.hpp"
#include "image_io.hpp"

using namespace std;

void runMenu()
{
    int choice;
    string code;
    string path;
    string savepath;

    do
    {
        cout << "\n1. Generate Barcode" << endl;
        cout << "2. Read Barcode" << endl;
        cout << "3. Exit" << endl;
        cout << "Enter choice: ";
        cin >> choice;

        if (choice == 1)
        {
            cout << "Enter code: ";
            cin >> code;

            Image img = generateBarcode(code);

            showImage(img);

            cout << "Enter file name to save: ";
            cin >> savepath;

            saveImage(img, savepath);

            cout << "Barcode saved successfully." << endl;
        }
        else if (choice == 2)
        {
            cout << "Enter image path: ";
            cin >> path;

            string result = decodeFromFile(path);

            cout << "Decoded Barcode: " << result << endl;
        }
        else if (choice == 3)
        {
            cout << "Exiting..." << endl;
        }
        else
        {
            cout << "Invalid choice." << endl;
        }

    } while (choice != 3);
}