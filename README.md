# Barcode Encoder and Decoder

**A C++17 application for generating and decoding EAN-13 barcodes.**

![Language](https://img.shields.io/badge/Language-C%2B%2B17-blue)
![Build](https://img.shields.io/badge/Build-CMake-green)
![Standard](https://img.shields.io/badge/Barcode-EAN--13-orange)
![Libraries](https://img.shields.io/badge/External%20Libraries-None-success)

## Overview

The **Barcode Encoder and Decoder** is a C++ application developed as Increment 1 of a Super Store Management System for a Software Engineering course.

The application provides two main functionalities:

- **Barcode Encoding:** Accepts a valid six-digit numeric code, converts it into an EAN-13 barcode, generates a barcode image, and saves it to the local system.
- **Barcode Decoding:** Reads an existing barcode image, analyzes its black-and-white patterns, and retrieves the original six-digit code.

The system implements barcode generation, image processing, and decoding logic from scratch using C++17 and the standard library, without relying on external barcode or image-processing libraries.

## Features

### 1. Barcode Generation
- Accepts six-digit numeric codes.
- Validates input before processing.
- Converts the input into a valid EAN-13 barcode.
- Generates barcode patterns using EAN-13 encoding rules.
- Renders the barcode as a black-and-white image.
- Saves the generated barcode as a BMP image.
- Displays a text-based barcode preview in the console.

### 2. Barcode Decoding
- Reads barcode images from the local system.
- Converts images into binary black-and-white representations.
- Detects barcode patterns using run-length analysis.
- Identifies individual digits using EAN-13 pattern tables.
- Verifies the check digit.
- Recovers the original six-digit code.

### 3. Input Validation
- Ensures the input contains exactly six digits.
- Rejects non-numeric characters.
- Rejects codes beginning with zero.
- Provides meaningful error messages for invalid input.

### 4. Image File Support
- BMP image loading and saving.
- PGM image loading and saving.
- Grayscale conversion for supported colour BMP images.
- Image thresholding for barcode detection.

### 5. Interactive Interface
- Command-line interface.
- Interactive menu with barcode generation and decoding options.
- Console-based barcode preview.
- Error handling that allows the application to continue running.

### 6. Automated Testing
- Unit tests for individual modules.
- Validation and image-processing tests.
- Barcode encoding and decoding tests.
- Round-trip verification to ensure encoding and decoding produce consistent results.

## How It Works

The application follows two main processing pipelines.

### Barcode Encoding

```text
Six-Digit Input
      |
      v
Input Validation
      |
      v
EAN-13 Conversion
(Padding + Check Digit)
      |
      v
Binary Pattern Generation
      |
      v
Barcode Image Rendering
      |
      v
Save as BMP Image
      |
      v
Console Preview
```

### Barcode Decoding

```text
Barcode Image
      |
      v
Load Image
      |
      v
Image Preprocessing
      |
      v
Binary Thresholding
      |
      v
Run-Length Analysis
      |
      v
EAN-13 Pattern Recognition
      |
      v
Check Digit Verification
      |
      v
Recover Original 6-Digit Code
```

## EAN-13 Implementation

The project follows the EAN-13 barcode standard.

Since the application works with six-digit codes, six leading zeros are added to the original input to create a twelve-digit number. A thirteenth check digit is then calculated using the EAN-13 checksum algorithm.

**Example:**

| Description | Value |
|---|---|
| Original Code | `123456` |
| Padded Code | `000000123456` |
| EAN-13 Code | `0000001234565` |
| Check Digit | `5` |

The complete EAN-13 representation consists of:

- Start guard pattern: `101`
- Six left-side digits
- Centre guard pattern: `01010`
- Six right-side digits
- End guard pattern: `101`

Each digit is represented using a seven-module binary pattern, producing a total of 95 barcode modules, excluding the quiet zones.

During decoding, the check digit is verified and the six leading padding digits are removed to recover the original code.

## Technologies Used

| Technology | Purpose |
|---|---|
| C++17 | Core application development |
| CMake | Build configuration and compilation |
| Git | Version control |
| GitHub | Repository management and collaboration |
| C++ Standard Library | Data structures and file handling |
| CTest | Automated test execution |

**External Libraries:** None. Barcode generation, image handling, and decoding functionality are implemented from scratch.

## Project Structure

```text
barcode-project/
│
├── include/
│   └── barcode/
│       ├── config.hpp
│       ├── error.hpp
│       ├── image.hpp
│       ├── validator.hpp
│       ├── patterns.hpp
│       ├── ean13.hpp
│       ├── encoder.hpp
│       ├── scanner.hpp
│       ├── preprocess.hpp
│       ├── decoder.hpp
│       ├── image_io.hpp
│       ├── renderer.hpp
│       ├── display.hpp
│       └── menu.hpp
│
├── src/
│   ├── main.cpp
│   ├── validator.cpp
│   ├── patterns.cpp
│   ├── ean13.cpp
│   ├── encoder.cpp
│   ├── scanner.cpp
│   ├── preprocess.cpp
│   ├── decoder.cpp
│   ├── image_io.cpp
│   ├── renderer.cpp
│   ├── display.cpp
│   └── menu.cpp
│
├── tests/
│   ├── test_scanner.cpp
│   ├── test_ean13.cpp
│   ├── test_encoder.cpp
│   ├── test_validator.cpp
│   ├── test_image_io.cpp
│   ├── test_renderer.cpp
│   ├── test_preprocess.cpp
│   ├── test_decoder.cpp
│   ├── test_roundtrip.cpp
│   └── test_samples.cpp
│
├── samples/
│   └── Sample barcode images
│
├── docs/
│   └── Project documentation
│
├── CMakeLists.txt
├── .gitignore
└── README.md
```

## Getting Started

### Prerequisites

Before running the project, make sure you have:

- A C++17-compatible compiler (GCC, Clang or MSVC).
- CMake installed.
- Git installed.

### Installation

**1. Clone the repository**

```bash
git clone <repository-url>
```

**2. Navigate to the project directory**

```bash
cd barcode-project
```

**3. Configure the project**

```bash
cmake -B build
```

**4. Build the project**

```bash
cmake --build build
```

**5. Run the application**

Run the generated executable from the build directory. The exact executable path depends on your operating system and CMake generator.

## Usage

The application supports both an interactive menu and command-line operations.

### Interactive Menu

Launch the application without command-line arguments.

The menu provides three options:

```text
========== BARCODE ENCODER & DECODER ==========

1. Generate Barcode
2. Read Barcode
3. Exit

Enter your choice:
```

**Option 1: Generate Barcode**

Enter a valid six-digit code. The application generates the corresponding EAN-13 barcode, displays a text-based preview in the console, and saves the actual barcode image as a BMP file.

**Option 2: Read Barcode**

Enter the path of an existing BMP or PGM barcode image. The application processes the image and displays the decoded six-digit number.

### Command-Line Operations

The application also supports direct commands:

**Generate a barcode:**

```bash
barcode encode 123456 out.bmp
```

**Decode a barcode:**

```bash
barcode decode out.bmp
```

## Running Tests

The project uses CTest to execute automated tests.

After building the project, run:

```bash
ctest --test-dir build --output-on-failure
```

The test suite covers:

- Six-digit input validation.
- EAN-13 check digit calculation.
- Barcode pattern generation.
- Image loading and saving.
- Image preprocessing and thresholding.
- Run-length scanning and pattern recognition.
- Barcode decoding.
- Round-trip encoding and decoding.

### Round-Trip Verification

One of the most important tests is round-trip verification.

A valid six-digit code is encoded into a barcode image and then decoded back into its original value.

```text
Input Code:       123456
       |
       v
Encode Barcode
       |
       v
Generated Image
       |
       v
Decode Barcode
       |
       v
Output Code:      123456
```

The test passes when the decoded output matches the original input.

## Software Engineering Principles

The project applies the following software engineering principles:

| Principle | Application |
|---|---|
| Modularity | Functionality is divided into independent modules. |
| Separation of Concerns | Each module handles a specific responsibility. |
| High Cohesion | Related functionality is grouped together. |
| Low Coupling | Modules interact through clearly defined interfaces. |
| Abstraction | Complex encoding and decoding operations are hidden behind simple functions. |
| Information Hiding | Internal implementation details remain private to their respective modules. |
| Reusability | Shared validation, EAN-13 logic and image operations are reused. |
| Generality | Configurable parameters are maintained centrally. |
| Anticipation of Change | Image handling and decoding operations are designed to be replaceable. |
| Incrementality | Development is divided into manageable stages. |
| Verifiability | Automated tests validate individual modules and the complete system. |

## Scope and Limitations

The project focuses on barcode generation and decoding using uploaded image files.

**Supported:**
- Six-digit numeric codes beginning with a non-zero digit.
- EAN-13 barcode generation.
- BMP and PGM image formats.
- Decoding of clean barcode images.
- Console-based barcode preview.
- Basic tolerance for image noise and upside-down images.

**Not Supported:**
- Direct barcode scanning through a laptop camera.
- Direct PNG or JPG image processing.
- Heavily blurred barcode images.
- Rotated or perspective-distorted barcode photographs.

These limitations keep the implementation focused on the EAN-13 standard, image-processing fundamentals, and software engineering principles without external libraries.

## Team Members

This project is developed collaboratively by five Software Engineering students.

| Member | Name | Responsibilities |
|---|---|---|
| Person 1 | **Mahnoor Aslam** | Barcode Scanner, Command-Line Interface and README |
| Person 2 | **Hadia Razzaq** | Pattern Tables, EAN-13 Logic and Encoder |
| Person 3 | **Farda Fatima** | Shared Core, Validation, Image I/O, Renderer and Repository Setup |
| Person 4 | **Hadia Kashif** | Image Preprocessing, Decoder Pipeline, Build System and Integration Testing |
| Person 5 | **Manahil Butt** | On-Screen Display, Interactive Menu, Sample Images and Documentation |

## Collaboration and Version Control

Git and GitHub are used to manage collaborative development.

The team follows a structured workflow:

- Each member works on an assigned feature branch.
- Changes are submitted through Pull Requests.
- Team members review each other's code.
- The main branch is protected.
- GitHub Issues are used to assign and track tasks.
- Individual contributions are maintained through commits and pull requests.

## Future Improvements

Potential extensions for future increments include:

- Integration with the complete Super Store Management System.
- Support for additional barcode formats.
- Camera-based barcode scanning.
- Improved image preprocessing for difficult barcode images.
- Support for additional image formats.
- Integration with product inventory and billing operations.

## License

This project is developed for academic purposes as part of a Software Engineering course.

---

**Developed using C++17 | Software Engineering Project | 2026**