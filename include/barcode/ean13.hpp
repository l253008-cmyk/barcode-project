#pragma once
#include <string>
using namespace std;

int computeCheckDigit(const string& first12);
string toEan13(const string& code);
string fromEan13(const string& ean13);
string buildModules(const string& ean13);