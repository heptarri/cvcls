# CVCLS - Computer Vision Learning Project

Assignment code for the Image Processing and Machine Vision course, Department of Robotics Engineering, Chang'an University.

## Architecture

```
src/
├── main.cpp         
├── common/          
│   ├── common.h
│   └── common.cpp
└── chapts/
    ├── chapt2/      
    │   ├── chapt2.h
    │   ├── chapt2.cpp
    │   └── main.cpp
    ├── chapt3/    
    │   ├── chapt3.h
    │   ├── chapt3.cpp
    │   └── main.cpp
    ├── chapt4/    
    │   ├── chapt4.h
    │   ├── chapt4.cpp
    │   └── main.cpp
    └── chapt5/    
        ├── chapt5.h
        ├── chapt5.cpp
        └── main.cpp
```

## Usage

### REquirements

- CMake >= 3.10
- C++23 (G++ 14)
- OpenCV 4.x

### Build

```bash
mkdir build && cd build

cmake ..

make cvcls

make

make chapt2
make chapt3
make chapt4
make chapt5
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

./cvcls --help
```
