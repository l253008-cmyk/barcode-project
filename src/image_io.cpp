
#include "barcode/image_io.hpp"
#include "barcode/error.hpp"

#include <fstream>
#include <string>
#include <vector>

// Check file extension without caring about capital letters.
static bool endsWith(const std::string& text, const std::string& ending)
{
    if (text.size() < ending.size())
        return false;

    for (std::size_t i = 0; i < ending.size(); i++)
    {
        char a = text[text.size() - ending.size() + i];
        char b = ending[i];

        if (a >= 'A' && a <= 'Z') a = a - 'A' + 'a';
        if (b >= 'A' && b <= 'Z') b = b - 'A' + 'a';

        if (a != b)
            return false;
    }

    return true;
}

// Check whitespace.
static bool isSpace(int c)
{
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

// Read one word from a PGM header.
static std::string readToken(std::ifstream& in)
{
    int c = in.peek();

    while (c != EOF)
    {
        if (isSpace(c))
        {
            in.get();
        }
        else if (c == '#')
        {
            while (c != EOF && c != '\n')
                c = in.get();
        }
        else
        {
            break;
        }

        c = in.peek();
    }

    std::string token;

    while (c != EOF && !isSpace(c) && c != '#')
    {
        token += static_cast<char>(in.get());
        c = in.peek();
    }

    return token;
}

// Convert a string into an integer.
static int toNumber(const std::string& token)
{
    if (token.empty() || token.size() > 9)
        throw BarcodeError("Bad number in image header");

    int value = 0;

    for (std::size_t i = 0; i < token.size(); i++)
    {
        if (token[i] < '0' || token[i] > '9')
            throw BarcodeError("Bad number in image header");

        value = value * 10 + (token[i] - '0');
    }

    return value;
}

// Read a number stored in little-endian format.
static long readLE(const std::vector<unsigned char>& bytes,
    std::size_t position, int count)
{
    unsigned long value = 0;

    for (int i = count - 1; i >= 0; i--)
        value = (value << 8) | bytes[position + i];

    return static_cast<long>(value);
}

// Read a signed 4-byte number.
static long readSigned32(const std::vector<unsigned char>& bytes,
    std::size_t position)
{
    long value = readLE(bytes, position, 4);

    if (value >= 2147483648L)
        value -= 4294967296L;

    return value;
}

// Write a number in little-endian format.
static void putLE(std::vector<unsigned char>& bytes,
    unsigned long value, int count)
{
    for (int i = 0; i < count; i++)
    {
        bytes.push_back(
            static_cast<unsigned char>((value >> (8 * i)) & 255)
        );
    }
}

// Load PGM images: P2 and P5.
static Image loadPgm(std::ifstream& in, const std::string& magic)
{
    int width = toNumber(readToken(in));
    int height = toNumber(readToken(in));
    int maxValue = toNumber(readToken(in));

    if (width <= 0 || height <= 0)
        throw BarcodeError("Image has an invalid size");

    if (maxValue <= 0 || maxValue > 255)
        throw BarcodeError("Only 8-bit PGM images are supported");

    Image img;
    img.width = width;
    img.height = height;

    std::size_t total = static_cast<std::size_t>(width) * height;
    img.pixels.assign(total, 0);

    if (magic == "P5")
    {
        in.get();

        std::vector<char> raw(total);

        in.read(raw.data(), static_cast<std::streamsize>(total));

        if (static_cast<std::size_t>(in.gcount()) != total)
            throw BarcodeError("PGM file is too short");

        for (std::size_t i = 0; i < total; i++)
        {
            int value = static_cast<unsigned char>(raw[i]);

            if (value > maxValue)
                value = maxValue;

            img.pixels[i] =
                static_cast<unsigned char>(value * 255 / maxValue);
        }
    }
    else
    {
        // P2 stores pixels as numbers.
        for (std::size_t i = 0; i < total; i++)
        {
            int value = toNumber(readToken(in));

            if (value > maxValue)
                value = maxValue;

            img.pixels[i] =
                static_cast<unsigned char>(value * 255 / maxValue);
        }
    }

    return img;
}

// Load uncompressed BMP images: 8-bit and 24-bit.
static Image loadBmp(std::ifstream& in)
{
    in.clear();
    in.seekg(0, std::ios::end);

    std::streamoff size = in.tellg();

    in.seekg(0, std::ios::beg);

    if (size < 54)
        throw BarcodeError("BMP file is too short");

    std::vector<unsigned char> file(static_cast<std::size_t>(size));

    in.read(reinterpret_cast<char*>(file.data()), size);

    if (in.gcount() != size)
        throw BarcodeError("Could not read BMP file");

    long dataOffset = readLE(file, 10, 4);
    long headerSize = readLE(file, 14, 4);
    long width = readSigned32(file, 18);
    long rawHeight = readSigned32(file, 22);
    long bits = readLE(file, 28, 2);
    long compression = readLE(file, 30, 4);

    if (headerSize < 40)
        throw BarcodeError("Unsupported BMP header");

    if (compression != 0)
        throw BarcodeError("Compressed BMP is not supported");

    if (bits != 8 && bits != 24)
        throw BarcodeError("Only 8-bit and 24-bit BMP are supported");

    bool topDown = rawHeight < 0;
    long height = topDown ? -rawHeight : rawHeight;

    if (width <= 0 || height <= 0 ||
        width > 100000 || height > 100000)
        throw BarcodeError("Image has an invalid size");

    // BMP rows are padded to a multiple of 4 bytes.
    long rowSize = ((bits * width + 31) / 32) * 4;
    long needed = dataOffset + rowSize * height;

    if (dataOffset < 54 || needed > static_cast<long>(file.size()))
        throw BarcodeError("BMP file is too short");

    // Default grayscale palette for 8-bit BMP.
    unsigned char palette[256];

    for (int i = 0; i < 256; i++)
        palette[i] = static_cast<unsigned char>(i);

    if (bits == 8)
    {
        std::size_t paletteStart =
            static_cast<std::size_t>(14 + headerSize);

        for (int i = 0; i < 256; i++)
        {
            std::size_t p = paletteStart +
                static_cast<std::size_t>(i) * 4;

            if (p + 3 >= static_cast<std::size_t>(dataOffset))
                break;

            int blue = file[p];
            int green = file[p + 1];
            int red = file[p + 2];

            palette[i] = static_cast<unsigned char>(
                (299 * red + 587 * green + 114 * blue) / 1000
                );
        }
    }

    Image img;
    img.width = static_cast<int>(width);
    img.height = static_cast<int>(height);

    img.pixels.assign(
        static_cast<std::size_t>(width) * height, 0
    );

    for (long y = 0; y < height; y++)
    {
        long sourceRow;

        if (topDown)
            sourceRow = y;
        else
            sourceRow = height - 1 - y;

        std::size_t rowStart =
            static_cast<std::size_t>(dataOffset + sourceRow * rowSize);

        for (long x = 0; x < width; x++)
        {
            unsigned char gray;

            if (bits == 24)
            {
                std::size_t p = rowStart +
                    static_cast<std::size_t>(x) * 3;

                int blue = file[p];
                int green = file[p + 1];
                int red = file[p + 2];

                gray = static_cast<unsigned char>(
                    (299 * red + 587 * green + 114 * blue) / 1000
                    );
            }
            else
            {
                gray = palette[
                    file[rowStart + static_cast<std::size_t>(x)]
                ];
            }

            img.at(static_cast<int>(x), static_cast<int>(y)) = gray;
        }
    }

    return img;
}

// Open an image and detect its format.
Image loadImage(const std::string& path)
{
    std::ifstream in(path, std::ios::binary);

    if (!in)
        throw BarcodeError("Cannot open file: " + path);

    char first = 0;
    char second = 0;

    in.get(first);
    in.get(second);

    if (!in)
        throw BarcodeError("File is empty or too short");

    std::string magic;
    magic += first;
    magic += second;

    if (magic == "P2" || magic == "P5")
        return loadPgm(in, magic);

    if (magic == "BM")
        return loadBmp(in);

    throw BarcodeError("Unsupported image format: " + path);
}

// Save an image as PGM P5.
static void savePgm(const Image& img, std::ofstream& out)
{
    out << "P5\n";
    out << img.width << " " << img.height << "\n";
    out << "255\n";

    out.write(
        reinterpret_cast<const char*>(img.pixels.data()),
        static_cast<std::streamsize>(img.pixels.size())
    );
}

// Save an image as uncompressed 24-bit BMP.
static void saveBmp(const Image& img, std::ofstream& out)
{
    long rowSize = ((24L * img.width + 31) / 32) * 4;
    long pixelBytes = rowSize * img.height;
    long fileSize = 54 + pixelBytes;

    std::vector<unsigned char> header;

    header.push_back('B');
    header.push_back('M');

    putLE(header, fileSize, 4);
    putLE(header, 0, 4);
    putLE(header, 54, 4);
    putLE(header, 40, 4);
    putLE(header, img.width, 4);
    putLE(header, img.height, 4);
    putLE(header, 1, 2);
    putLE(header, 24, 2);
    putLE(header, 0, 4);
    putLE(header, pixelBytes, 4);
    putLE(header, 2835, 4);
    putLE(header, 2835, 4);
    putLE(header, 0, 4);
    putLE(header, 0, 4);

    out.write(
        reinterpret_cast<const char*>(header.data()),
        static_cast<std::streamsize>(header.size())
    );

    std::vector<unsigned char> row(
        static_cast<std::size_t>(rowSize), 0
    );

    // BMP saves the bottom row first.
    for (int y = img.height - 1; y >= 0; y--)
    {
        for (int x = 0; x < img.width; x++)
        {
            unsigned char gray = img.at(x, y);

            row[static_cast<std::size_t>(x) * 3] = gray;
            row[static_cast<std::size_t>(x) * 3 + 1] = gray;
            row[static_cast<std::size_t>(x) * 3 + 2] = gray;
        }

        out.write(
            reinterpret_cast<const char*>(row.data()),
            static_cast<std::streamsize>(row.size())
        );
    }
}

// Save an image according to its file extension.
void saveImage(const Image& img, const std::string& path)
{
    if (img.width <= 0 || img.height <= 0 ||
        img.pixels.size() !=
        static_cast<std::size_t>(img.width) * img.height)
    {
        throw BarcodeError("Cannot save an invalid image");
    }

    bool wantBmp = endsWith(path, ".bmp");
    bool wantPgm = endsWith(path, ".pgm");

    if (!wantBmp && !wantPgm)
        throw BarcodeError("Use .bmp or .pgm file extension");

    std::ofstream out(path, std::ios::binary);

    if (!out)
        throw BarcodeError("Cannot write file: " + path);

    if (wantBmp)
        saveBmp(img, out);
    else
        savePgm(img, out);

    out.close();

    if (!out)
        throw BarcodeError("Error while writing file: " + path);
}
