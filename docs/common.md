# Common 模块 - 基础图像处理函数数学原理

本文档详细介绍 common 模块中实现的基础图像处理算法的数学原理。

## 目录

- [1. 灰度转换](#1-灰度转换)
- [2. 图像缩放（最近邻插值）](#2-图像缩放最近邻插值)
- [3. 像素访问与验证](#3-像素访问与验证)

---

## 1. 灰度转换

### 数学原理

将彩色图像（RGB）转换为灰度图像的过程叫做**灰度化**。最常用的方法是**加权平均法**，因为人眼对不同颜色的敏感度不同。

#### 公式

$$
\text{Gray} = 0.299 \times R + 0.587 \times G + 0.114 \times B
$$

其中：
- $R$ 是红色通道的像素值（0-255）
- $G$ 是绿色通道的像素值（0-255）
- $B$ 是蓝色通道的像素值（0-255）

#### 为什么是这些权重？

这些权重来自于人眼的视觉特性：
- 人眼对**绿色最敏感**（权重 0.587，占比最大）
- 对**红色次之**（权重 0.299）
- 对**蓝色最不敏感**（权重 0.114，占比最小）

这个公式被称为**亮度加权公式**，符合 ITU-R BT.601 标准。

### 实现方式

**老太太理解版**：想象你有一张彩色照片，上面有红、绿、蓝三种颜色。要把它变成黑白照片，我们不是简单地把三个颜色加起来除以 3，而是要考虑人眼的感受。绿色看起来最亮，所以给它最大的比重；蓝色看起来最暗，所以给它最小的比重。

#### 代码实现（common.cpp）

**函数位置**: `common.cpp` 第 99-112 行

```cpp
cv::Mat toGrayscale(const cv::Mat& src) {
  cv::Mat gray(src.rows, src.cols, CV_8UC1);
  
  for (int y : std::views::iota(0, src.rows)) {
    for (int x : std::views::iota(0, src.cols)) {
      const auto pixel = src.at<cv::Vec3b>(y, x);
      const float value = RGB_TO_GRAY_R * pixel[2] + RGB_TO_GRAY_G * pixel[1] +
                          RGB_TO_GRAY_B * pixel[0];
      gray.at<uchar>(y, x) =
          static_cast<uchar>(std::clamp(std::round(value), 0.0f, 255.0f));
    }
  }
  return gray;
}
```

**步骤说明**：
1. **第 100 行**：创建单通道灰度图像矩阵
2. **第 102-103 行**：遍历每个像素位置 $(y, x)$
3. **第 104 行**：读取该位置的 RGB 值（注意：OpenCV 存储顺序是 BGR）
   - `pixel[0]` = B（蓝色）
   - `pixel[1]` = G（绿色）
   - `pixel[2]` = R（红色）
4. **第 105-106 行**：应用加权公式计算灰度值
5. **第 108 行**：四舍五入并限制在 0-255 范围内
6. **第 109 行**：将计算出的灰度值写入输出图像

**常量定义**（common.h）：
```cpp
constexpr float RGB_TO_GRAY_R = 0.299f;
constexpr float RGB_TO_GRAY_G = 0.587f;
constexpr float RGB_TO_GRAY_B = 0.114f;
```

---

## 2. 图像缩放（最近邻插值）

### 数学原理

图像缩放是将图像从一个尺寸改变到另一个尺寸。**最近邻插值**是最简单的插值方法。

#### 基本思想

对于输出图像中的每个像素，找到它在原图像中对应的最近的像素，直接使用该像素的值。

#### 坐标映射公式

假设：
- 原图尺寸：$W_{src} \times H_{src}$
- 目标图尺寸：$W_{dst} \times H_{dst}$
- 目标图中的像素坐标：$(x_{dst}, y_{dst})$

则对应的原图坐标为：

$$
x_{src} = \left\lfloor x_{dst} \times \frac{W_{src}}{W_{dst}} \right\rfloor
$$

$$
y_{src} = \left\lfloor y_{dst} \times \frac{H_{src}}{H_{dst}} \right\rfloor
$$

其中 $\lfloor \cdot \rfloor$ 表示向下取整。

### 实现方式

**老太太理解版**：想象你有一张小照片要放大成大照片。对于大照片上的每一个点，我们要问：你对应小照片上的哪个点？找到最近的那个点，把它的颜色抄过来就行了。虽然简单，但放大后可能会有锯齿。

#### 代码实现（common.cpp）

**函数位置**: `common.cpp` 第 40-56 行

```cpp
cv::Mat resizeImage(const cv::Mat& src, int width, int height) {
  cv::Mat dst(height, width, src.type());

  const auto scaleX = static_cast<float>(src.cols) / width;
  const auto scaleY = static_cast<float>(src.rows) / height;

  for (int y : std::views::iota(0, height)) {
    for (int x : std::views::iota(0, width)) {
      const int srcX =
          std::clamp(static_cast<int>(x * scaleX), 0, src.cols - 1);
      const int srcY =
          std::clamp(static_cast<int>(y * scaleY), 0, src.rows - 1);
      dst.at<cv::Vec3b>(y, x) = src.at<cv::Vec3b>(srcY, srcX);
    }
  }
  return dst;
}
```

**步骤说明**：
1. **第 41 行**：创建目标尺寸的输出图像
2. **第 43-44 行**：计算缩放比例
   - `scaleX` = $\frac{W_{src}}{W_{dst}}$（水平缩放因子）
   - `scaleY` = $\frac{H_{src}}{H_{dst}}$（垂直缩放因子）
3. **第 46-47 行**：遍历目标图像的每个像素
4. **第 48-51 行**：计算源图像对应坐标
   - 将坐标乘以缩放因子
   - 使用 `clamp` 确保坐标不越界
5. **第 52 行**：直接复制源像素值到目标位置

**示例**：
- 原图 200×200，目标 100×100（缩小一半）
- 缩放因子：scaleX = scaleY = 2.0
- 目标图 (50, 50) → 源图 (100, 100)

---

## 3. 像素访问与验证

### 3.1 获取像素值

**函数位置**: `common.cpp` 第 21-27 行

```cpp
std::optional<cv::Vec3b> getPixelValue(const cv::Mat& img, int row,
                                       int col) noexcept {
  if (row < 0 || row >= img.rows || col < 0 || col >= img.cols) [[unlikely]] {
    return std::nullopt;
  }
  return img.at<cv::Vec3b>(row, col);
}
```

**原理**：
- 检查坐标是否在有效范围内：$0 \leq row < H$ 且 $0 \leq col < W$
- 如果有效，返回该位置的 RGB 值
- 如果越界，返回空值（`std::nullopt`）

### 3.2 设置像素值

**函数位置**: `common.cpp` 第 30-37 行

```cpp
bool setPixelValue(cv::Mat& img, int row, int col,
                   const cv::Vec3b& value) noexcept {
  if (row < 0 || row >= img.rows || col < 0 || col >= img.cols) [[unlikely]] {
    return false;
  }
  img.at<cv::Vec3b>(row, col) = value;
  return true;
}
```

**原理**：
- 同样检查边界条件
- 如果有效，写入新的像素值并返回 `true`
- 如果越界，不做任何操作并返回 `false`

### 3.3 灰度图判断

**函数位置**: `common.cpp` 第 65-79 行

**数学判据**：

如果对于所有像素都满足 $R = G = B$，则该图像为灰度图。

$$
\forall (x, y): R(x,y) = G(x,y) = B(x,y)
$$

### 3.4 二值图判断

**函数位置**: `common.cpp` 第 82-96 行

**数学判据**：

如果对于所有像素都满足 $I(x,y) \in \{0, 255\}$，则该图像为二值图。

$$
\forall (x, y): I(x,y) \in \{0, 255\}
$$

**老太太理解版**：二值图就像黑白棋，只有纯黑（0）和纯白（255）两种颜色，没有中间的灰色。

---

## 4. 图像信息获取

**函数位置**: `common.cpp` 第 13-18 行

```cpp
void getInfo(const cv::Mat& img) {
  std::println("Basic information:");
  std::println("Width: {} px", img.cols);
  std::println("Height: {} px", img.rows);
  std::println("Channels: {}", img.channels());
}
```

输出图像的基本属性：
- **宽度** (Width): 列数，单位像素
- **高度** (Height): 行数，单位像素
- **通道数** (Channels): 
  - 1 = 灰度图
  - 3 = RGB 彩色图
  - 4 = RGBA（带透明度）

---

## 总结

common 模块提供了最基础的图像处理工具：

| 功能 | 函数 | 核心算法 | 代码行数 |
|------|------|----------|----------|
| 灰度转换 | `toGrayscale()` | 加权平均法 | 99-112 |
| 图像缩放 | `resizeImage()` | 最近邻插值 | 40-56 |
| 像素读取 | `getPixelValue()` | 边界检查 | 21-27 |
| 像素写入 | `setPixelValue()` | 边界检查 | 30-37 |
| 灰度判断 | `isGrayscale()` | RGB 相等性检查 | 65-79 |
| 二值判断 | `isBinary()` | 值域检查 | 82-96 |

这些函数是后续章节所有高级算法的基础。
