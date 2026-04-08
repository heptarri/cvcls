# Chapter 5 - 图像滤波与边缘检测

本文档详细介绍 Chapter 5 中实现的图像滤波和边缘检测算法的数学原理。

## 目录

- [1. 卷积基础](#1-卷积基础)
- [2. 均值滤波](#2-均值滤波)
- [3. 高斯滤波](#3-高斯滤波)
- [4. Robert 算子](#4-robert-算子)
- [5. Sobel 算子](#5-sobel-算子)
- [6. 拉普拉斯算子](#6-拉普拉斯算子)
- [7. 中值滤波](#7-中值滤波)

---

## 1. 卷积基础

### 1.1 什么是卷积

卷积是图像处理中最重要的操作之一。它将一个小矩阵（称为**卷积核**或**滤波器**）滑动到图像上，对每个位置进行加权求和。

### 1.2 数学定义

对于图像 $I$ 和卷积核 $K$（大小为 $H \times W$），卷积操作定义为：

$$
O(y, x) = \sum_{j=0}^{H-1} \sum_{i=0}^{W-1} I(y + j - \lfloor H/2 \rfloor, x + i - \lfloor W/2 \rfloor) \cdot K(j, i)
$$

**简单说**：对于输出图像的每个像素，用卷积核覆盖对应的输入区域，逐元素相乘后求和。

### 1.3 归一化卷积

对于平滑滤波器，通常需要归一化以保持图像亮度：

$$
O(y, x) = \frac{\sum_{j,i} I(y+j, x+i) \cdot K(j, i)}{\sum_{j,i} K(j, i)}
$$

**老太太理解版**：
想象你有一块小印章（卷积核），上面每个格子有不同的数字（权重）。你把印章盖在图片的每个位置，把对应格子的数字乘以图片那个位置的颜色，全部加起来，就得到新图片那个位置的颜色。

### 1.4 通用卷积实现

**函数位置**: `chapt5.cpp` 第 28-66 行

```cpp
template <int H, int W>
cv::Mat imgFilter(const cv::Mat& src, const Kernel<H, W>& mask) {
  cv::Mat dst(src.rows, src.cols, CV_8UC1);

  constexpr int halfW = W / 2;
  constexpr int halfH = H / 2;

  // 计算权重和
  float weightSum = 0.0f;
  for (const auto& row : mask) {
    for (const auto val : row) {
      weightSum += val;
    }
  }

  const bool normalize = (weightSum != 0.0f);

  for (int y : views::iota(0, src.rows)) {
    for (int x : views::iota(0, src.cols)) {
      float accum = 0.0f;

      for (int my : views::iota(0, H)) {
        for (int mx : views::iota(0, W)) {
          const int px = x + mx - halfW;
          const int py = y + my - halfH;
          const int sx = std::clamp(px, 0, src.cols - 1);
          const int sy = std::clamp(py, 0, src.rows - 1);

          accum += src.at<uchar>(sy, sx) * mask[my][mx];
        }
      }

      const float value = normalize ? (accum / weightSum) : accum;
      dst.at<uchar>(y, x) =
          static_cast<uchar>(std::clamp(std::round(value), 0.0f, 255.0f));
    }
  }
  return dst;
}
```

**步骤说明**：
1. **第 32-34 行**：定义卷积核的半径
2. **第 36-42 行**：计算卷积核权重和（用于归一化）
3. **第 44 行**：判断是否需要归一化（权重和不为 0）
4. **第 46-47 行**：遍历目标图像每个像素
5. **第 50-59 行**：对当前像素的邻域进行卷积
   - **第 51-54 行**：计算卷积核对应的源图像坐标
   - **第 55-56 行**：边界处理（clamp 到有效范围）
   - **第 58 行**：累加加权像素值
6. **第 62 行**：归一化（如果需要）
7. **第 63-64 行**：限制到 [0, 255] 并写入结果

**边界处理**：使用 `std::clamp` 将越界坐标限制到图像边界，相当于边缘像素延拓。

---

## 2. 均值滤波

### 2.1 数学原理

均值滤波（也叫**平均滤波**或**盒式滤波**）用邻域像素的平均值替换中心像素，可以平滑图像和去除噪声。

#### 3×3 均值滤波卷积核

$$
K_{mean} = \frac{1}{9} \begin{bmatrix}
1 & 1 & 1 \\
1 & 1 & 1 \\
1 & 1 & 1
\end{bmatrix}
$$

#### 计算公式

$$
O(y, x) = \frac{1}{9} \sum_{j=-1}^{1} \sum_{i=-1}^{1} I(y+j, x+i)
$$

简单说：用周围 9 个像素（包括自己）的平均值。

### 2.2 效果

- ✅ 去除椒盐噪声（随机黑白点）
- ✅ 图像平滑
- ❌ 模糊边缘（丢失细节）

### 2.3 代码实现

**函数位置**: `chapt5.cpp` 第 81-85 行

```cpp
cv::Mat imgMean(const cv::Mat& src) {
  constexpr Kernel<3, 3> kernel = {
      {{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}}};
  return imgFilter<3, 3>(src, kernel);
}
```

**步骤说明**：
1. **第 82-83 行**：定义 3×3 的全 1 卷积核
2. **第 84 行**：调用通用滤波函数
   - 自动计算权重和（9）
   - 自动归一化（除以 9）

**老太太理解版**：
想象你和周围 8 个邻居一起投票决定你家墙的颜色。每个人的意见权重相同，最后取平均。这样极端的颜色（噪声）就被平均掉了，但也让画面变模糊了。

---

## 3. 高斯滤波

### 3.1 数学原理

高斯滤波使用**高斯函数**作为权重，距离中心越近的像素权重越大。

#### 高斯函数（二维）

$$
G(x, y) = \frac{1}{2\pi\sigma^2} e^{-\frac{x^2 + y^2}{2\sigma^2}}
$$

其中 $\sigma$ 是标准差，控制高斯分布的"宽度"。

#### 离散化的 3×3 高斯核（近似）

$$
K_{gaussian} = \frac{1}{16} \begin{bmatrix}
1 & 2 & 1 \\
2 & 4 & 2 \\
1 & 2 & 1
\end{bmatrix}
$$

这是一个近似的高斯核（$\sigma \approx 1$）：
- 中心权重最大（4）
- 上下左右权重次之（2）
- 四个角权重最小（1）

### 3.2 效果

相比均值滤波：
- ✅ 更好地保留边缘（中心权重大）
- ✅ 去噪效果更自然
- ✅ 对高斯噪声特别有效

### 3.3 代码实现

**函数位置**: `chapt5.cpp` 第 88-92 行

```cpp
cv::Mat imgGaussian(const cv::Mat& src) {
  constexpr Kernel<3, 3> kernel = {
      {{1.0f, 2.0f, 1.0f}, {2.0f, 4.0f, 2.0f}, {1.0f, 2.0f, 1.0f}}};
  return imgFilter<3, 3>(src, kernel);
}
```

**步骤说明**：
1. **第 89-90 行**：定义高斯核（权重和为 16）
2. **第 91 行**：调用通用滤波函数（自动除以 16）

**老太太理解版**：
高斯滤波像是"智能版"的均值滤波。投票时，离你近的邻居（中心）的意见更重要，离得远的邻居意见权重小。这样既能平滑噪声，又不会把边缘模糊得太厉害。

---

## 4. Robert 算子

### 4.1 数学原理

**Robert 算子**（Robert Cross Operator）是最简单的边缘检测算子，使用 2×2 的卷积核检测对角线方向的边缘。

#### 两个方向的卷积核

$$
G_x = \begin{bmatrix}
1 & 0 \\
0 & -1
\end{bmatrix}
\quad
G_y = \begin{bmatrix}
0 & 1 \\
-1 & 0
\end{bmatrix}
$$

- $G_x$：检测主对角线方向的边缘
- $G_y$：检测副对角线方向的边缘

#### 梯度幅值

$$
M(y, x) = |G_x| + |G_y|
$$

或者更精确地：

$$
M(y, x) = \sqrt{G_x^2 + G_y^2}
$$

但本实现使用绝对值之和（计算更快）。

### 4.2 代码实现

**函数位置**: `chapt5.cpp` 第 95-122 行

```cpp
cv::Mat imgRobert(const cv::Mat& src) {
  cv::Mat dst(src.rows, src.cols, CV_8UC1);

  constexpr std::array<std::array<float, 2>, 2> gx = {
      {{1.0f, 0.0f}, {0.0f, -1.0f}}};
  constexpr std::array<std::array<float, 2>, 2> gy = {
      {{0.0f, 1.0f}, {-1.0f, 0.0f}}};

  for (int y : views::iota(0, src.rows)) {
    for (int x : views::iota(0, src.cols)) {
      float accX = 0.0f;
      float accY = 0.0f;

      for (int dy : views::iota(0, 2)) {
        for (int dx : views::iota(0, 2)) {
          const int sx = std::clamp(x + dx - 1, 0, src.cols - 1);
          const int sy = std::clamp(y + dy - 1, 0, src.rows - 1);
          const float pixel = src.at<uchar>(sy, sx);
          accX += pixel * gx[dy][dx];
          accY += pixel * gy[dy][dx];
        }
      }
      float edge = std::abs(accX) + std::abs(accY);
      dst.at<uchar>(y, x) = static_cast<uchar>(std::clamp(edge, 0.0f, 255.0f));
    }
  }
  return dst;
}
```

**步骤说明**：
1. **第 98-101 行**：定义两个 2×2 的 Robert 核
2. **第 103-104 行**：遍历每个像素
3. **第 108-116 行**：计算两个方向的卷积
   - **第 113 行**：$G_x$ 方向的累加
   - **第 114 行**：$G_y$ 方向的累加
4. **第 118 行**：计算边缘强度：$M = |G_x| + |G_y|$
5. **第 119 行**：限制到 [0, 255] 并写入

**老太太理解版**：
Robert 算子像是用两把斜着的尺子量图片的变化。一把尺子从左上到右下量，一把从右上到左下量。哪里颜色变化大（有边缘），哪里的值就大，最后显示出来就是白色的线（边缘）。

---

## 5. Sobel 算子

### 5.1 数学原理

**Sobel 算子**是经典的边缘检测算子，使用 3×3 的卷积核检测水平和垂直方向的边缘。

#### 两个方向的卷积核

$$
G_x = \begin{bmatrix}
-1 & 0 & 1 \\
-2 & 0 & 2 \\
-1 & 0 & 1
\end{bmatrix}
\quad
G_y = \begin{bmatrix}
-1 & -2 & -1 \\
0 & 0 & 0 \\
1 & 2 & 1
\end{bmatrix}
$$

**特点**：
- $G_x$：检测垂直边缘（水平方向梯度）
- $G_y$：检测水平边缘（垂直方向梯度）
- 中间行/列的权重加倍（2），增强中心响应

#### 梯度幅值

$$
M(y, x) = \sqrt{G_x^2 + G_y^2}
$$

本实现使用精确的欧几里得距离（而非绝对值之和）。

#### 梯度方向（可选）

$$
\theta(y, x) = \arctan\left(\frac{G_y}{G_x}\right)
$$

### 5.2 代码实现

**函数位置**: `chapt5.cpp` 第 125-154 行

```cpp
cv::Mat imgSobel(const cv::Mat& src) {
  cv::Mat dst(src.rows, src.cols, CV_8UC1);

  constexpr Kernel<3, 3> gx = {
      {{-1.0f, 0.0f, 1.0f}, {-2.0f, 0.0f, 2.0f}, {-1.0f, 0.0f, 1.0f}}};
  constexpr Kernel<3, 3> gy = {
      {{-1.0f, -2.0f, -1.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 2.0f, 1.0f}}};

  for (int y : views::iota(0, src.rows)) {
    for (int x : views::iota(0, src.cols)) {
      float accX = 0.0f;
      float accY = 0.0f;

      for (int dy : views::iota(0, 3)) {
        for (int dx : views::iota(0, 3)) {
          const int sx = std::clamp(x + dx - 1, 0, src.cols - 1);
          const int sy = std::clamp(y + dy - 1, 0, src.rows - 1);
          const float pixel = src.at<uchar>(sy, sx);
          accX += pixel * gx[dy][dx];
          accY += pixel * gy[dy][dx];
        }
      }

      const float magnitude = std::sqrt(accX * accX + accY * accY);
      dst.at<uchar>(y, x) =
          static_cast<uchar>(std::clamp(magnitude, 0.0f, 255.0f));
    }
  }
  return dst;
}
```

**步骤说明**：
1. **第 128-131 行**：定义 Sobel 核（水平和垂直）
2. **第 133-134 行**：遍历每个像素
3. **第 138-146 行**：计算两个方向的卷积
4. **第 148 行**：计算梯度幅值：$M = \sqrt{G_x^2 + G_y^2}$
5. **第 149-150 行**：限制到 [0, 255] 并写入

**老太太理解版**：
Sobel 算子像是用两把更精确的尺子量图片。一把横着量（检测上下的边缘），一把竖着量（检测左右的边缘）。中间的点权重更大，所以测得更准。最后用勾股定理算出总的边缘强度。

---

## 6. 拉普拉斯算子

### 6.1 数学原理

**拉普拉斯算子**（Laplacian Operator）是**二阶微分算子**，对图像的二阶导数进行检测，对细节和噪声都很敏感。

#### 连续拉普拉斯算子

$$
\nabla^2 f = \frac{\partial^2 f}{\partial x^2} + \frac{\partial^2 f}{\partial y^2}
$$

#### 离散近似（3×3 核）

$$
K_{Laplacian} = \begin{bmatrix}
0 & 1 & 0 \\
1 & -4 & 1 \\
0 & 1 & 0
\end{bmatrix}
$$

**解释**：
- 中心权重 $-4$
- 上下左右权重各 $+1$
- 权重和为 $0$（不改变平均亮度）

#### 公式推导

离散二阶导数可近似为：

$$
\frac{\partial^2 f}{\partial x^2} \approx f(x-1) - 2f(x) + f(x+1)
$$

$$
\frac{\partial^2 f}{\partial y^2} \approx f(y-1) - 2f(y) + f(y+1)
$$

相加得：

$$
\nabla^2 f \approx [f(x-1) + f(x+1) + f(y-1) + f(y+1)] - 4f(x,y)
$$

正好对应卷积核！

### 6.2 特性

- **零交叉点**：拉普拉斯值从正变负或从负变正的位置对应边缘
- **对噪声敏感**：二阶导数会放大噪声，通常先高斯平滑
- **各向同性**：对所有方向的边缘响应相同

### 6.3 代码实现

**函数位置**: `chapt5.cpp` 第 157-180 行

```cpp
cv::Mat imgLaplacian(const cv::Mat& src) {
  cv::Mat dst(src.rows, src.cols, CV_8UC1);

  constexpr Kernel<3, 3> kernel = {
      {{0.0f, 1.0f, 0.0f}, {1.0f, -4.0f, 1.0f}, {0.0f, 1.0f, 0.0f}}};

  for (int y : views::iota(0, src.rows)) {
    for (int x : views::iota(0, src.cols)) {
      float acc = 0.0f;

      for (int dy : views::iota(0, 3)) {
        for (int dx : views::iota(0, 3)) {
          const int sx = std::clamp(x + dx - 1, 0, src.cols - 1);
          const int sy = std::clamp(y + dy - 1, 0, src.rows - 1);
          acc += src.at<uchar>(sy, sx) * kernel[dy][dx];
        }
      }

      dst.at<uchar>(y, x) = static_cast<uchar>(
          std::clamp(std::round(std::abs(acc)), 0.0f, 255.0f));
    }
  }
  return dst;
}
```

**步骤说明**：
1. **第 160-161 行**：定义拉普拉斯核
2. **第 163-164 行**：遍历每个像素
3. **第 167-173 行**：计算卷积
4. **第 175-176 行**：取绝对值（因为我们只关心边缘强度，不关心方向）
5. **第 176 行**：限制到 [0, 255] 并写入

**注意**：第 176 行使用了 `std::abs(acc)`，这是因为拉普拉斯算子会产生正负值，但我们只需要边缘的强度。

**老太太理解版**：
拉普拉斯算子像是测量图片的"起伏"。如果一个点和周围差别很大（有边缘），拉普拉斯值就大。它特别敏感，连小的细节和噪声都能检测到。就像用手指感受桌面的凹凸，哪里不平哪里就是边缘。

---

## 7. 中值滤波

### 7.1 数学原理

**中值滤波**是一种**非线性滤波**，不使用卷积，而是用邻域像素的**中位数**替换中心像素。

#### 算法步骤

对于 3×3 邻域：

1. 提取 9 个像素值：$\{p_0, p_1, \ldots, p_8\}$
2. 对这 9 个值排序
3. 取中间值（第 5 个）作为输出

$$
O(y, x) = \text{median}\{I(y+j, x+i) : -1 \leq j, i \leq 1\}
$$

### 7.2 效果

- ✅ **去除椒盐噪声极佳**（比均值滤波好得多）
- ✅ **保留边缘**（不像均值滤波那样模糊边缘）
- ❌ 对高斯噪声效果不如均值/高斯滤波
- ❌ 计算量较大（需要排序）

### 7.3 为什么对椒盐噪声有效？

椒盐噪声是随机的黑点（0）和白点（255），这些是**极值**。在排序后，极值会被排到两端，中位数不会是极值，从而有效去除噪声。

**示例**：
- 原始邻域：`[120, 255, 118, 119, 255, 121, 122, 123, 0]`（有两个白点噪声 255，一个黑点噪声 0）
- 排序后：`[0, 118, 119, 120, 121, 122, 123, 255, 255]`
- 中位数：`121`（第 5 个值，噪声被忽略）

### 7.4 代码实现

**函数位置**: `chapt5.cpp` 第 183-204 行

```cpp
cv::Mat imgMedian(const cv::Mat& src) {
  cv::Mat dst(src.rows, src.cols, CV_8UC1);

  for (int y : views::iota(0, src.rows)) {
    for (int x : views::iota(0, src.cols)) {
      std::array<uchar, 9> window;
      int idx = 0;

      for (int dy : views::iota(-1, 2)) {
        for (int dx : views::iota(-1, 2)) {
          const int sx = std::clamp(x + dx, 0, src.cols - 1);
          const int sy = std::clamp(y + dy, 0, src.rows - 1);
          window[idx++] = src.at<uchar>(sy, sx);
        }
      }

      std::ranges::sort(window);
      dst.at<uchar>(y, x) = window[4];
    }
  }
  return dst;
}
```

**步骤说明**：
1. **第 186-187 行**：遍历每个像素
2. **第 188-189 行**：创建 9 元素的窗口数组
3. **第 191-196 行**：提取 3×3 邻域的 9 个像素
   - **第 193-194 行**：边界限制
   - **第 195 行**：存入窗口数组
4. **第 199 行**：对窗口数组排序
5. **第 200 行**：取中间值（索引 4）作为输出

**时间复杂度**：
- 排序：$O(9 \log 9) = O(1)$（常数时间，因为窗口大小固定）
- 总体：$O(W \times H)$（与图像大小成正比）

**老太太理解版**：
中值滤波像是"投票选代表"。看周围 9 个人（包括自己），把大家的意见排个队，选中间那个人的意见（不是平均）。这样极端意见（噪声）就被忽略了，而且边缘不会像求平均那样被模糊掉。

---

## 8. 滤波器对比

### 8.1 平滑滤波对比

| 滤波器 | 原理 | 去噪效果 | 边缘保留 | 计算量 |
|--------|------|----------|----------|--------|
| 均值 | 算术平均 | 中等 | 差（模糊） | 低 |
| 高斯 | 加权平均 | 好 | 中等 | 低 |
| 中值 | 取中位数 | 对椒盐噪声极佳 | 好 | 中等 |

### 8.2 边缘检测算子对比

| 算子 | 核大小 | 计算量 | 噪声鲁棒性 | 边缘定位 | 特点 |
|------|--------|--------|------------|----------|------|
| Robert | 2×2 | 最低 | 差 | 一般 | 对角线边缘 |
| Sobel | 3×3 | 中等 | 好 | 好 | 水平/垂直边缘 |
| Laplacian | 3×3 | 中等 | 差（二阶） | 精确 | 各向同性，对噪声敏感 |

---

## 9. 执行流程

Chapter 5 的 `runChapt5()` 函数执行顺序：

```
输入：灰度图像
     ↓
1. 均值滤波 (3×3)
     ↓
2. 高斯滤波 (3×3)
     ↓
3. 中值滤波 (3×3)
     ↓
4. Robert 边缘检测 (2×2)
     ↓
5. Sobel 边缘检测 (3×3)
     ↓
6. Laplacian 边缘检测 (3×3)
```

输出文件：
- `chapt5_meant.png` - 均值滤波结果
- `chapt5_gauss.png` - 高斯滤波结果
- `chapt5_median.png` - 中值滤波结果
- `chapt5_robert.png` - Robert 边缘检测结果
- `chapt5_sobel.png` - Sobel 边缘检测结果
- `chapt5_laplacian.png` - Laplacian 边缘检测结果

---

## 10. 数学公式总结

### 卷积

$$
(I * K)(y, x) = \sum_{j} \sum_{i} I(y+j, x+i) \cdot K(j, i)
$$

### 梯度幅值（Sobel）

$$
M = \sqrt{G_x^2 + G_y^2}
$$

### 拉普拉斯

$$
\nabla^2 I = \frac{\partial^2 I}{\partial x^2} + \frac{\partial^2 I}{\partial y^2}
$$

### 中位数

$$
\text{median}(X) = X_{\lfloor n/2 \rfloor} \quad \text{(排序后)}
$$

---

## 11. 总结

Chapter 5 涵盖的算法：

| 算法 | 类型 | 卷积核 | 用途 | 代码行数 |
|------|------|--------|------|----------|
| 通用卷积 | 框架 | 模板化 | 所有卷积操作 | 28-66 |
| 均值滤波 | 平滑 | 3×3 全1 | 去噪、平滑 | 81-85 |
| 高斯滤波 | 平滑 | 3×3 高斯 | 去噪、保边缘 | 88-92 |
| Robert 算子 | 边缘 | 2×2 对角 | 快速边缘检测 | 95-122 |
| Sobel 算子 | 边缘 | 3×3 梯度 | 标准边缘检测 | 125-154 |
| Laplacian | 边缘 | 3×3 二阶 | 精确边缘定位 | 157-180 |
| 中值滤波 | 平滑 | 无（排序） | 去椒盐噪声 | 183-204 |

**关键设计**：
- 使用模板编程实现通用卷积框架（第 28-66 行）
- 自动归一化（权重和不为 0 时）
- 边界处理使用 clamp（边缘延拓）
- 边缘检测使用绝对值或梯度幅值

这些滤波器是图像预处理和特征提取的基础工具，广泛应用于降噪、边缘检测、图像分割等领域。
