#include <iostream>
#include <string>
#include "decoder.hpp"

using namespace std;

int main()
{
    string clean = decodeFromFile("samples/clean.bmp");
    string noisy = decodeFromFile("samples/noisy.bmp");
    string upside = decodeFromFile("samples/upside_down.bmp");

    string expectedClean = "EXPECTED_CODE_1";
    string expectedNoisy = "EXPECTED_CODE_2";
    string expectedUpside = "EXPECTED_CODE_3";

    if (clean == expectedClean)
        cout << "Clean image: PASS" << endl;
    else
        cout << "Clean image: FAIL" << endl;

    if (noisy == expectedNoisy)
        cout << "Noisy image: PASS" << endl;
    else
        cout << "Noisy image: FAIL" << endl;

    if (upside == expectedUpside)
        cout << "Upside down image: PASS" << endl;
    else
        cout << "Upside down image: FAIL" << endl;

    return 0;
}