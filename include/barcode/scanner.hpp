#pragma once

#include <string>
#include <vector>

#include "barcode/image.hpp"

// Records the run lengths (pixels in a row of the same colour) of one pixel row.
// The list always starts with a white run (0 if the row starts with black),
// so even positions are white runs and odd positions are black runs.
std::vector<int> scanRow(const Image& binaryImg, int row);

// Finds the start guard (black, white, black, each one module wide).
// startIndex = position of its first black run, moduleWidth = pixels per module.
bool findStartGuard(const std::vector<int>& runs, int& startIndex, double& moduleWidth);

// Turns the runs after the start guard into 13 digits ("" if it fails).
std::string decodeRuns(const std::vector<int>& runs, int startIndex, double moduleWidth);