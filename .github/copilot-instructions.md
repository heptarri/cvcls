# Copilot Instructions for CVCLS

## Build System

This project uses **CMake** (>= 3.10) as the build system with C++17 standard.

### Build Commands

```bash
# Configure project
mkdir build && cd build
cmake ..

# Build all targets
make

# Build specific chapter
make chapt2
make chapt3
make chapt4
make chapt5

# Run from build/bin directory
cd build/bin
./chapt2
./chapt3
./chapt4
./chapt5

# Clean build
cd build && make clean
# Or completely remove build directory
rm -rf build
```

### Available Targets

- **chapt2**: Basic image operations executable
- **chapt3**: Histogram and transformations executable
- **chapt4**: Geometric transformations executable
- **chapt5**: Filtering operations executable
- **common**: Static library with shared utility functions

### Output

- Build artifacts: `./build/` directory
- Executables: `./build/bin/` directory
- Runtime output: `./build/bin/output/` directory (created by program at runtime)
- Each chapter reads from `static/image.png` (relative path: `../../static/image.png` from bin directory)
- Each chapter generates processed images in the output directory

## Architecture

### Project Structure

```
chapts/
├── common/              # Shared utility library
│   ├── common.h
│   └── common.cpp
├── chapt2/              # Chapter 2: Basic operations
│   ├── chapt2.h
│   ├── chapt2.cpp
│   └── main.cpp
├── chapt3/              # Chapter 3: Histogram & transformations
│   ├── chapt3.h
│   ├── chapt3.cpp
│   └── main.cpp
├── chapt4/              # Chapter 4: Geometric transformations
│   ├── chapt4.h
│   ├── chapt4.cpp
│   └── main.cpp
└── chapt5/              # Chapter 5: Filtering operations
    ├── chapt5.h
    ├── chapt5.cpp
    └── main.cpp
```

### Architecture Overview

This is a **Computer Vision learning project** structured as chapters, each implementing different image processing algorithms from scratch using OpenCV's Mat type but with custom implementations.

**Execution Flow:**
1. Each chapter's `main.cpp` loads RGB image from `static/image.png`
2. Converts BGR (OpenCV default) → RGB
3. Creates grayscale version via custom `toGrayscale()` if needed (e.g., Chapter 5)
4. Runs the chapter's processing pipeline (`runChaptN`)
5. Saves output images to `./output/` (relative to executable location in build/bin/)

**Common Utilities** (`chapts/common/`):
- Manual pixel access/manipulation
- Custom nearest-neighbor resizing (not using OpenCV's resize)
- Grayscale conversion using weighted RGB formula (0.299R + 0.587G + 0.114B)
- Image validation and type checking (grayscale, binary)

**Chapter Modules:**
- **Chapter 2** (`chapts/chapt2/`): Basic operations (using common utilities)
- **Chapter 3** (`chapts/chapt3/`): Histogram analysis, grayscale transformations (linear, log, gamma), Otsu thresholding, histogram equalization
- **Chapter 4** (`chapts/chapt4/`): Geometric transformations with multiple interpolation methods (nearest, bilinear, cubic Catmull-Rom)
- **Chapter 5** (`chapts/chapt5/`): Convolution filters using custom `imgFilter` template, edge detection (Robert, Sobel, Laplacian), smoothing (mean, gaussian, median)

### Key Implementation Pattern

All image processing functions follow this pattern:
- Accept `const cv::Mat&` as input (RGB or grayscale)
- Return `cv::Mat` as output (new image, not in-place modification)
- Use manual pixel iteration (not OpenCV built-ins for core algorithms)
- Each chapter has a `runChaptN(img, outputRoot)` function that generates multiple output images

## Conventions

### Image Format Assumptions

- **Input images**: Expected as RGB 3-channel (`CV_8UC3`) unless specified as grayscale
- **Grayscale images**: Single channel (`CV_8UC1`) with values 0-255
- **Pixel access**: Use `cv::Vec3b` for RGB, `uchar` for grayscale
- **Color order**: RGB (not BGR) after the initial conversion in main.cpp

### Manual Implementation Philosophy

This codebase implements algorithms **manually** rather than using OpenCV's built-in functions:
- Custom interpolation methods (nearest, bilinear, cubic)
- Custom Otsu threshold calculation
- Custom histogram equalization
- Manual convolution with template-based kernel application

**Do not replace manual implementations with OpenCV equivalents** (e.g., don't use `cv::resize()` in place of custom `resizeImage()`). The purpose is educational.

### Coordinate System

- **Pixel coordinates**: `(row, col)` or `(y, x)` indexing (standard matrix notation)
- **Image dimensions**: `img.rows` = height, `img.cols` = width
- **Rotation**: Performed around image center, with out-of-bounds pixels filled black

### Naming Conventions

- Functions: camelCase (`toGrayscale`, `imgFlip`, `runChapt3`)
- Chapter runner functions: `runChaptN()`
- Image transformation functions: prefix with `img` or operation type (`genGrayLinTrans`, `imgRotate`)
- Filter operations: prefix with `img` (`imgMean`, `imgSobel`)

### Adding New Algorithms

When adding new image processing functions to a chapter:

1. **Declare in chapter header** (e.g., `chapts/chapt3/chapt3.h`)
2. **Implement in chapter .cpp** following manual implementation philosophy
3. **Add to `runChaptN()`** to generate output file
4. **Update chapter's main.cpp** if generating specific statistics
5. Use descriptive output filenames: `outputRoot + "/chapt3_algorithm_name.png"`
6. Rebuild: `cd build && make chaptN`

### Dependencies

- **OpenCV**: Used for `cv::Mat` container and file I/O (`imread`, `imwrite`), not for processing algorithms
- **C++ Standard**: Uses C++17 features (including `std::clamp`)
- **CMake**: Build system (>= 3.10)
- No additional dependencies beyond OpenCV

## Common Tasks

### Adding a New Chapter

1. Create `chapts/chaptN/` directory
2. Create `chapts/chaptN/chaptN.h` and `chapts/chaptN/chaptN.cpp`
3. Implement `void runChaptN(const cv::Mat& img, const std::string& outputRoot)`
4. Create `chapts/chaptN/main.cpp` based on existing chapter main files
5. Add new target to `CMakeLists.txt`:
   ```cmake
   add_executable(chaptN
       chapts/chaptN/main.cpp
       chapts/chaptN/chaptN.cpp
       chapts/chaptN/chaptN.h
   )
   target_include_directories(chaptN PRIVATE chapts/chaptN)
   target_link_libraries(chaptN common ${OpenCV_LIBS})
   ```
6. Rebuild: `cd build && make`

### Modifying Output

- All image outputs use `cv::imwrite()` with PNG format
- Output directory is created in main: `mkdir(outputRoot.c_str(), 0755)`
- Use consistent naming: `chapt{N}_{operation}.png`

### Working with Filters (Chapter 5)

The `imgFilter<H, W>()` template accepts a 2D array kernel:
```cpp
const float kernel[3][3] = {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}};
cv::Mat result = imgFilter<3, 3>(src, kernel);
```
- Template parameters are kernel height and width
- Returns filtered grayscale image
- Handles boundary conditions by clamping coordinates
