# CVCLS - Computer Vision Learning Project

Assignment code for the Image Processing and Machine Vision course, Department of Robotics Engineering, Chang'an University.

## Source architecture

```
.
├── chapts
│   ├── chapt2
│   │   ├── chapt2.cpp
│   │   └── chapt2.h
│   ├── chapt3
│   │   ├── chapt3.cpp
│   │   └── chapt3.h
│   ├── chapt4
│   │   ├── chapt4.cpp
│   │   └── chapt4.h
│   ├── chapt5
│   │   ├── chapt5.cpp
│   │   └── chapt5.h
│   └── others
│       ├── single_measure.cpp
│       ├── single_measure.h
│       ├── stereo_measure.cpp
│       └── stereo_measure.h
├── common
│   ├── common.cpp
│   └── common.h
└── main.cpp
```

## Usage

### Requirements

- CMake >= 3.10
- C++23 (G++ 14)
- OpenCV 4.x

### Build

```bash
cmake -S . -B build
cmake --build build
```

### Running


```bash
cd build/bin

./cvcls
# 或
./cvcls all

./cvcls 2    
./cvcls 3    
./cvcls 4    
./cvcls 5    
./cvcls measure
./cvcls stereo

./cvcls --help
```

> Path of images should be changed to your real address.