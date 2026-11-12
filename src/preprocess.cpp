#include "barcode/preprocess.hpp"
#include "barcode/error.hpp"

// Adaptive threshold.
// Every pixel is compared with the average of the pixels around it in the
// same row, so shadows and uneven light do not break the barcode.
Image toBinary(const Image& gray) {
    if (gray.width <= 0 || gray.height <= 0 ||
        gray.pixels.size() != static_cast<size_t>(gray.width) * gray.height) {
        throw BarcodeError("Invalid or empty image");
    }

    // Window size: about 1/5 of the image width (at least 5 pixels each side),
    // so it works for small and big images and is wider than any single bar.
    int radius = gray.width / 10;
    if (radius < 5) {
        radius = 5;
    }

    // A pixel is black only if it is this much darker than its neighbours.
    // This keeps plain white or plain grey areas white.
    const int offset = 10;

    Image binary;
    binary.width = gray.width;
    binary.height = gray.height;
    binary.pixels.resize(gray.pixels.size());

    for (int y = 0; y < gray.height; y++) {
        // The window slides along the row. We keep its running total, so for
        // each new pixel we only add the pixel that enters on the right and
        // remove the pixel that leaves on the left (no extra list needed).
        long long windowSum = 0;
        int left = 0;      // first pixel inside the window
        int right = -1;    // last pixel inside the window (none yet)

        for (int x = 0; x < gray.width; x++) {
            int newLeft = x - radius;
            if (newLeft < 0) newLeft = 0;
            int newRight = x + radius;
            if (newRight > gray.width - 1) newRight = gray.width - 1;

            while (right < newRight) {      // pixel enters on the right
                right++;
                windowSum += gray.at(right, y);
            }
            while (left < newLeft) {        // pixel leaves on the left
                windowSum -= gray.at(left, y);
                left++;
            }

            long long average = windowSum / (right - left + 1);

            if (gray.at(x, y) < average - offset) {
                binary.at(x, y) = 0;
            }
            else {
                binary.at(x, y) = 255;
            }
        }
    }
    return binary;

}