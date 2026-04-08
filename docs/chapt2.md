# Chapter 2 - 基本图像操作

本文档介绍 Chapter 2 中实现的基本图像操作。这一章主要使用 common 模块中的函数，展示如何进行像素级别的图像操作。

## 目录

- [1. 概述](#1-概述)
- [2. 像素读取](#2-像素读取)
- [3. 像素修改](#3-像素修改)
- [4. 图像缩放](#4-图像缩放)
- [5. 创建空白图像](#5-创建空白图像)
- [6. 灰度转换](#6-灰度转换)

---

## 1. 概述

Chapter 2 的主要目的是演示基本的图像操作，包括：
- 读取和显示图像基本信息
- 访问和修改单个像素
- 图像尺寸变换
- 颜色空间转换

所有操作都通过 common 模块提供的基础函数实现。

---

## 2. 像素读取

### 功能说明

读取图像中指定位置的像素值，并输出其 RGB 分量。

### 代码实现

**函数位置**: `chapt2.cpp` 第 12-17 行

```cpp
auto pixel = getPixelValue(img, 297, 260);
if (pixel) {
  std::cout << "Pixel value at (297,260): [" << static_cast<int>((*pixel)[0])
            << ", " << static_cast<int>((*pixel)[1]) << ", "
            << static_cast<int>((*pixel)[2]) << "]" << std::endl;
}
```

**步骤说明**：
1. **第 12 行**：调用 `getPixelValue()` 读取坐标 (297, 260) 处的像素
2. **第 13 行**：检查返回值是否有效（坐标可能越界）
3. **第 14-16 行**：输出 RGB 三个通道的值
   - `(*pixel)[0]` = B（蓝色）
   - `(*pixel)[1]` = G（绿色）
   - `(*pixel)[2]` = R（红色）

**老太太理解版**：就像在方格纸上找到第 297 行、第 260 列的那个小格子，看看它是什么颜色。颜色用三个数字表示：红色有多少、绿色有多少、蓝色有多少。

---

## 3. 像素修改

### 功能说明

将图像左上角 100×100 区域的所有像素设置为黄色（RGB: 255, 255, 0）。

### 数学原理

对于图像 $I$，修改指定区域的像素值：

$$
I(y, x) = 
\begin{cases}
(255, 255, 0) & \text{if } 0 \leq x < 100 \text{ and } 0 \leq y < 100 \\
I(y, x) & \text{otherwise}
\end{cases}
$$

### 代码实现

**函数位置**: `chapt2.cpp` 第 20-29 行

```cpp
cv::Mat modified = img.clone();
int maxY = std::min(100, modified.rows);
int maxX = std::min(100, modified.cols);
for (int y = 0; y < maxY; ++y) {
  for (int x = 0; x < maxX; ++x) {
    [[maybe_unused]] bool success =
        setPixelValue(modified, y, x, cv::Vec3b(255, 255, 0));  // BGR: yellow
  }
}
cv::imwrite(outputDir + "/chapt2_set_pixel.png", modified);
```

**步骤说明**：
1. **第 20 行**：克隆原图像，避免修改原始数据
2. **第 21-22 行**：确定修改区域的边界
   - 使用 `std::min` 防止越界（如果图像小于 100×100）
3. **第 23-28 行**：双重循环遍历区域内每个像素
4. **第 26 行**：设置为黄色 (B=255, G=255, R=0)
   - 注意：OpenCV 使用 BGR 顺序
5. **第 29 行**：保存修改后的图像

**老太太理解版**：想象用黄色笔在照片左上角涂一个 100×100 的正方形区域，把这个区域内所有的点都涂成黄色。

---

## 4. 图像缩放

### 功能说明

将图像缩放到 320×240 像素的尺寸。

### 数学原理

使用**最近邻插值**方法进行缩放（详见 common.md 第 2 节）：

$$
I_{dst}(y, x) = I_{src}\left(\left\lfloor y \cdot \frac{H_{src}}{H_{dst}} \right\rfloor, \left\lfloor x \cdot \frac{W_{src}}{W_{dst}} \right\rfloor\right)
$$

### 代码实现

**函数位置**: `chapt2.cpp` 第 32-35 行

```cpp
cv::Mat resized = resizeImage(img, 320, 240);
cv::imwrite(outputDir + "/chapt2_resized.png", resized);
std::cout << "[TASK] Resize task completed!" << std::endl;
```

**步骤说明**：
1. **第 33 行**：调用 `resizeImage()` 函数缩放到指定尺寸
   - 目标宽度：320 像素
   - 目标高度：240 像素
2. **第 34 行**：保存缩放后的图像

**效果**：
- 如果原图更大：图像被缩小
- 如果原图更小：图像被放大
- 长宽比可能改变（可能出现拉伸或压缩）

**老太太理解版**：就像把一张大照片缩印成小照片，或者把小照片放大。我们用最简单的方法：每个新位置找最近的老位置，把颜色抄过来。

---

## 5. 创建空白图像

### 功能说明

创建一个 320×240 的纯蓝色图像。

### 数学原理

创建一个矩阵，所有像素值都相同：

$$
I(y, x) = (255, 0, 0) \quad \forall (y, x) \in [0, H) \times [0, W)
$$

这里 (255, 0, 0) 在 BGR 顺序中表示纯蓝色。

### 代码实现

**函数位置**: `chapt2.cpp` 第 38-41 行

```cpp
cv::Mat blank =
    createBlankImage(320, 240, cv::Vec3b(255, 0, 0));  // BGR: blue
cv::imwrite(outputDir + "/chapt2_blank.png", blank);
std::cout << "[TASK] Create blank task completed!" << std::endl;
```

**步骤说明**：
1. **第 38-39 行**：调用 `createBlankImage()` 创建空白图像
   - 宽度：320 像素
   - 高度：240 像素
   - 颜色：BGR(255, 0, 0) = 蓝色
2. **第 40 行**：保存图像

**Common 模块实现** (`common.cpp` 第 59-62 行)：
```cpp
cv::Mat createBlankImage(int width, int height, const cv::Vec3b& color) {
  cv::Mat img(height, width, CV_8UC3, cv::Scalar(color[0], color[1], color[2]));
  return img;
}
```

**老太太理解版**：创建一张空白的画布，整张都涂成一种颜色（这里是蓝色）。就像买了一块纯色的布。

---

## 6. 灰度转换

### 功能说明

将彩色图像转换为灰度图像。

### 数学原理

使用**加权平均法**（详见 common.md 第 1 节）：

$$
\text{Gray}(y, x) = 0.299 \times R(y,x) + 0.587 \times G(y,x) + 0.114 \times B(y,x)
$$

这个公式基于人眼视觉特性：
- 对绿光最敏感（权重 58.7%）
- 对红光次之（权重 29.9%）
- 对蓝光最不敏感（权重 11.4%）

### 代码实现

**函数位置**: `chapt2.cpp` 第 44-48 行

```cpp
std::cout << "Is grayscale: " << (isGrayscale(img) ? "true" : "false")
          << std::endl;
cv::Mat gray = toGrayscale(img);
cv::imwrite(outputDir + "/chapt2_gray.png", gray);
std::cout << "[TASK] Change to gray task completed!" << std::endl;
```

**步骤说明**：
1. **第 44-45 行**：检查原图是否已经是灰度图
2. **第 46 行**：调用 `toGrayscale()` 进行转换
3. **第 47 行**：保存灰度图像

**视觉效果**：
- 彩色信息丢失
- 仅保留亮度信息
- 从 3 通道（RGB）变为 1 通道（灰度）

**老太太理解版**：就像把彩色照片变成黑白照片。我们不是简单地把三种颜色平均，而是按照人眼的感受来计算：绿色看起来更亮，所以它的比重更大。

---

## 7. 图像属性检查

### 7.1 灰度图判断

**函数位置**: `chapt2.cpp` 第 44-45 行

检查图像是否为灰度图（所有像素的 R = G = B）。

### 7.2 二值图判断

**函数位置**: `chapt2.cpp` 第 50-51 行

```cpp
std::cout << "Is binary: " << (isBinary(gray) ? "true" : "false")
          << std::endl;
```

检查灰度图是否为二值图（所有像素值只有 0 或 255）。

**数学条件**：

$$
\text{isBinary} = \begin{cases}
\text{true} & \text{if } \forall (y,x): I(y,x) \in \{0, 255\} \\
\text{false} & \text{otherwise}
\end{cases}
$$

**老太太理解版**：检查这张黑白照片是不是只有纯黑和纯白两种颜色，没有灰色。就像检查是不是剪影图。

---

## 8. 执行流程

Chapter 2 的 `runChapt2()` 函数按以下顺序执行所有操作：

```
1. 获取图像信息 (宽度、高度、通道数)
     ↓
2. 读取指定位置的像素值
     ↓
3. 修改图像区域（左上角涂黄色）
     ↓
4. 缩放图像 (320×240)
     ↓
5. 创建空白蓝色图像
     ↓
6. 转换为灰度图
     ↓
7. 检查是否为二值图
```

每个操作都会生成对应的输出文件：
- `chapt2_set_pixel.png` - 像素修改结果
- `chapt2_resized.png` - 缩放结果
- `chapt2_blank.png` - 空白图像
- `chapt2_gray.png` - 灰度转换结果

---

## 总结

Chapter 2 是图像处理的入门章节，涵盖的操作：

| 操作 | 函数 | 核心原理 | 代码行数 |
|------|------|----------|----------|
| 像素读取 | `getPixelValue()` | 坐标索引 | 12-17 |
| 像素修改 | `setPixelValue()` | 坐标赋值 | 20-29 |
| 图像缩放 | `resizeImage()` | 最近邻插值 | 32-35 |
| 创建空白 | `createBlankImage()` | 常量填充 | 38-41 |
| 灰度转换 | `toGrayscale()` | 加权平均 | 44-48 |

这些基础操作为后续章节的高级算法（直方图、几何变换、滤波等）奠定了基础。
