#include <iostream>
#include <string>

#include "display.hpp"
#include "image_io.hpp"

using namespace std;

void showImage(const Image& img)
{
    if (img.width <= 0 || img.height <= 0)
    {
        cout << "Invalid image." << endl;
        return;
    }

    string path = "temp_barcode.bmp";

    saveImage(img, path);

    cout << "\nBarcode Preview:\n";

    int row = img.height / 2;

    for (int x = 0; x < img.width; x++)
    {
        if (img.at(x, row) == 0)
            cout << "#";
        else
            cout << " ";
    }

    cout << endl;
}