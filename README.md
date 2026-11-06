# \# Barcode Encoder and Decoder

# 

# EAN-13 barcode encoder and decoder for 6-digit codes, written in C++17 with no external libraries.

# 

# \- \*\*Part 1 (Encoder):\*\* turns a 6-digit number into a barcode image.

# \- \*\*Part 2 (Decoder):\*\* reads a barcode image and outputs the 6-digit number.

# 

# \## Team

# 

# | Member | Responsibility |

# |---|---|

# | Mahnoor Aslam (Team Lead) | Barcode scanner, command-line interface, README |

# | Hadia Razzaq | Pattern tables, EAN-13 logic, encoder |

# | Farda Fatima | Shared core, validation, image file I/O, renderer |

# | Hadia Kashif | Image preprocessing, decoder pipeline, build system, integration tests |

# 

# \## Build

# 

# cmake -B build

# cmake --build build

# 

# \## Usage

# 

# barcode encode 123456 out.pgm

# barcode decode out.pgm

# 

# \## Project structure

# 

# \- `include/barcode/` header files (the agreed interfaces)

# \- `src/` source files

# \- `tests/` unit and round-trip tests

# \- `samples/` sample barcode images

# \- `docs/` architecture diagram and report sections

