# Chapter 4 - 几何变换

本文档详细介绍 Chapter 4 中实现的几何变换算法的数学原理，包括平移、翻转、旋转和多种插值方法。

## 目录

- [1. 图像平移](#1-图像平移)
- [2. 图像翻转](#2-图像翻转)
- [3. 图像转置](#3-图像转置)
- [4. 图像缩放（三种插值）](#4-图像缩放三种插值)
- [5. 图像旋转（三种插值）](#5-图像旋转三种插值)

---

## 1. 图像平移

### 1.1 数学原理

图像平移是将图像沿 x 和 y 方向移动指定的距离。

**平移变换公式**：

$$
\begin{bmatrix}
x' \\
y'
\end{bmatrix}
=
\begin{bmatrix}
x \\
y
\end{bmatrix}
+
\begin{bmatrix}
t_x \\
t_y
\end{bmatrix}
$$

其中：
- $(x, y)$ 是原始坐标
- $(x', y')$ 是平移后的坐标
- $(t_x, t_y)$ 是平移向量

**逆映射**（从目标到源）：

$$
\begin{bmatrix}
x_{src} \\
y_{src}
\end{bmatrix}
=
\begin{bmatrix}
x_{dst} \\
y_{dst}
\end{bmatrix}
-
\begin{bmatrix}
t_x \\
t_y
\end{bmatrix}
$$

但实际实现中，我们使用**正向映射**（从源到目标）。

### 1.2 代码实现

**函数位置**: `chapt4.cpp` 第 8-22 行

```cpp
cv::Mat posTrans(const cv::Mat& src, int tx, int ty) {
  cv::Mat dst(src.rows, src.cols, src.type(), cv::Scalar(0, 0, 0));

  for (int y = 0; y < src.rows; ++y) {
    for (int x = 0; x < src.cols; ++x) {
      int targetX = x + tx;
      int targetY = y + ty;
      if (targetX >= 0 && targetX < src.cols && targetY >= 0 &&
          targetY < src.rows) {
        dst.at<cv::Vec3b>(targetY, targetX) = src.at<cv::Vec3b>(y, x);
      }
    }
  }
  return dst;
}
```

**步骤说明**：
1. **第 9 行**：创建黑色背景的目标图像
2. **第 11-12 行**：遍历源图像的每个像素
3. **第 13-14 行**：计算目标坐标
   - `targetX = x + tx`
   - `targetY = y + ty`
4. **第 15-16 行**：边界检查（确保目标坐标在图像范围内）
5. **第 17 行**：将源像素复制到目标位置

**老太太理解版**：
想象你在桌子上移动一张照片。向右移动 100 像素，就是把每个点都往右挪 100 步。移出桌子边缘的部分就看不见了（变成黑色）。

**示例** (`chapt4.cpp` 第 321 行)：
```cpp
cv::Mat translated = posTrans(img, 100, 100);
```
- 向右平移 100 像素
- 向下平移 100 像素

---

## 2. 图像翻转

### 2.1 数学原理

图像翻转分为三种模式：

#### 垂直翻转（mode = 0）

$$
\begin{bmatrix}
x' \\
y'
\end{bmatrix}
=
\begin{bmatrix}
x \\
H - 1 - y
\end{bmatrix}
$$

上下颠倒，$y$ 坐标反转。

#### 水平翻转（mode = 1）

$$
\begin{bmatrix}
x' \\
y'
\end{bmatrix}
=
\begin{bmatrix}
W - 1 - x \\
y
\end{bmatrix}
$$

左右镜像，$x$ 坐标反转。

#### 水平+垂直翻转（mode = -1）

$$
\begin{bmatrix}
x' \\
y'
\end{bmatrix}
=
\begin{bmatrix}
W - 1 - x \\
H - 1 - y
\end{bmatrix}
$$

旋转 180°，$x$ 和 $y$ 都反转。

### 2.2 代码实现

**函数位置**: `chapt4.cpp` 第 25-45 行

```cpp
cv::Mat imgFlip(const cv::Mat& src, int mode) {
  cv::Mat dst(src.rows, src.cols, src.type());

  for (int y = 0; y < src.rows; ++y) {
    for (int x = 0; x < src.cols; ++x) {
      int targetX, targetY;
      if (mode == 0) {
        targetX = x;
        targetY = src.rows - 1 - y;
      } else if (mode == -1) {
        targetX = src.cols - 1 - x;
        targetY = src.rows - 1 - y;
      } else {  // mode == 1
        targetX = src.cols - 1 - x;
        targetY = y;
      }
      dst.at<cv::Vec3b>(targetY, targetX) = src.at<cv::Vec3b>(y, x);
    }
  }
  return dst;
}
```

**步骤说明**：
1. **第 28-29 行**：遍历源图像每个像素
2. **第 31-39 行**：根据 mode 计算目标坐标
   - **mode = 0**：垂直翻转
   - **mode = 1**：水平翻转
   - **mode = -1**：水平+垂直翻转
3. **第 41 行**：复制像素到目标位置

**老太太理解版**：
- **垂直翻转**：像把照片倒过来看，上面的变到下面
- **水平翻转**：像照镜子，左边的变到右边
- **两个都翻**：像把照片旋转 180 度

---

## 3. 图像转置

### 3.1 数学原理

图像转置是将行列互换，相当于沿主对角线翻转。

**转置变换**：

$$
\begin{bmatrix}
x' \\
y'
\end{bmatrix}
=
\begin{bmatrix}
y \\
x
\end{bmatrix}
$$

**结果**：
- 原图尺寸 $W \times H$ → 转置后 $H \times W$
- 相当于顺时针旋转 90° + 垂直翻转

### 3.2 代码实现

**函数位置**: `chapt4.cpp` 第 48-57 行

```cpp
cv::Mat imgTranspose(const cv::Mat& src) {
  cv::Mat dst(src.cols, src.rows, src.type());

  for (int y = 0; y < src.rows; ++y) {
    for (int x = 0; x < src.cols; ++x) {
      dst.at<cv::Vec3b>(x, y) = src.at<cv::Vec3b>(y, x);
    }
  }
  return dst;
}
```

**步骤说明**：
1. **第 49 行**：创建目标图像，注意尺寸互换（cols ↔ rows）
2. **第 51-54 行**：遍历源图像
3. **第 53 行**：将 $(y, x)$ 处的像素放到 $(x, y)$

**老太太理解版**：
想象一个 3×5 的照片（宽 3 高 5），转置后变成 5×3（宽 5 高 3）。就像把照片顺时针转 90 度再翻个面。

---

## 4. 图像缩放（三种插值）

图像缩放需要插值方法来计算新像素的值。本项目实现了三种插值算法。

### 4.1 最近邻插值（Nearest Neighbor）

#### 数学原理

对于目标图像中的每个像素，找到源图像中最接近的像素，直接使用其值。

**坐标映射**：

$$
x_{src} = \left\lfloor \frac{x_{dst}}{W_{dst}} \times W_{src} \right\rfloor
$$

$$
y_{src} = \left\lfloor \frac{y_{dst}}{H_{dst}} \times H_{src} \right\rfloor
$$

**特点**：
- ✅ 计算最快
- ❌ 质量最差，有明显锯齿

#### 代码实现

**函数位置**: `chapt4.cpp` 第 60-75 行

```cpp
cv::Mat imgResizeNearest(const cv::Mat& src, int w, int h) {
  cv::Mat dst(h, w, src.type());

  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      int srcX =
          static_cast<int>(std::floor((x / static_cast<float>(w)) * src.cols));
      int srcY =
          static_cast<int>(std::floor((y / static_cast<float>(h)) * src.rows));
      srcX = std::min(srcX, src.cols - 1);
      srcY = std::min(srcY, src.rows - 1);
      dst.at<cv::Vec3b>(y, x) = src.at<cv::Vec3b>(srcY, srcX);
    }
  }
  return dst;
}
```

**步骤说明**：
1. **第 64-68 行**：计算源图像坐标
   - 使用比例缩放
   - 向下取整
2. **第 69-70 行**：边界限制
3. **第 71 行**：直接复制最近像素的值

**老太太理解版**：
最简单的方法，就像用放大镜看照片，每个新位置找最近的老位置，把颜色抄过来。放大后会看到明显的小方块（马赛克效果）。

---

### 4.2 双线性插值（Bilinear Interpolation）

#### 数学原理

使用**周围 4 个像素**的加权平均来计算新像素值。

**插值公式**：

设源图像坐标为 $(f_x, f_y)$，其整数部分和小数部分为：
- $(x_0, y_0) = (\lfloor f_x \rfloor, \lfloor f_y \rfloor)$
- $(dx, dy) = (f_x - x_0, f_y - y_0)$

四个邻近像素：
- $P_{00} = I(y_0, x_0)$ （左上）
- $P_{10} = I(y_0, x_1)$ （右上）
- $P_{01} = I(y_1, x_0)$ （左下）
- $P_{11} = I(y_1, x_1)$ （右下）

**两次线性插值**：

1. 水平方向插值：
$$
P_0 = P_{00} + (P_{10} - P_{00}) \times dx
$$
$$
P_1 = P_{01} + (P_{11} - P_{01}) \times dx
$$

2. 垂直方向插值：
$$
P = P_0 + (P_1 - P_0) \times dy
$$

**合并公式**：
$$
P = (1-dx)(1-dy)P_{00} + dx(1-dy)P_{10} + (1-dx)dy P_{01} + dx \cdot dy \cdot P_{11}
$$

#### 代码实现

**函数位置**: `chapt4.cpp` 第 78-118 行

```cpp
cv::Mat imgResizeBilinear(const cv::Mat& src, int w, int h) {
  cv::Mat dst(h, w, src.type());
  float sw = static_cast<float>(src.cols);
  float sh = static_cast<float>(src.rows);
  int maxX = src.cols - 1;
  int maxY = src.rows - 1;

  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      float fx = (x / static_cast<float>(w)) * sw;
      float fy = (y / static_cast<float>(h)) * sh;
      int x0 = static_cast<int>(std::floor(fx));
      int y0 = static_cast<int>(std::floor(fy));
      int x1 = std::min(x0 + 1, maxX);
      int y1 = std::min(y0 + 1, maxY);
      x0 = std::max(x0, 0);
      y0 = std::max(y0, 0);
      float dx = fx - x0;
      float dy = fy - y0;

      cv::Vec3b p00 = src.at<cv::Vec3b>(y0, x0);
      cv::Vec3b p10 = src.at<cv::Vec3b>(y0, x1);
      cv::Vec3b p01 = src.at<cv::Vec3b>(y1, x0);
      cv::Vec3b p11 = src.at<cv::Vec3b>(y1, x1);

      cv::Vec3b pixel;
      for (int c = 0; c < 3; ++c) {
        float v00 = p00[c];
        float v10 = p10[c];
        float v01 = p01[c];
        float v11 = p11[c];
        float v0 = v00 + (v10 - v00) * dx;
        float v1 = v01 + (v11 - v01) * dx;
        float v = v0 + (v1 - v0) * dy;
        pixel[c] = static_cast<uchar>(std::clamp(std::round(v), 0.0f, 255.0f));
      }
      dst.at<cv::Vec3b>(y, x) = pixel;
    }
  }
  return dst;
}
```

**步骤说明**：
1. **第 87-88 行**：计算源图像浮点坐标
2. **第 89-96 行**：确定 4 个邻近像素的坐标
3. **第 98 行**：计算小数部分 $(dx, dy)$
4. **第 100-103 行**：读取 4 个邻近像素
5. **第 106-113 行**：对每个颜色通道进行双线性插值
   - **第 111 行**：水平插值
   - **第 113 行**：垂直插值

**老太太理解版**：
不只看最近的一个点，而是看周围 4 个点，按照距离远近加权平均。离得近的点权重大，离得远的权重小。就像问路时综合几个人的意见。结果比最近邻平滑多了。

---

### 4.3 三次插值（Cubic Interpolation - Catmull-Rom）

#### 数学原理

使用**周围 16 个像素**（4×4 邻域）的加权平均，权重由三次函数决定。

**Catmull-Rom 权重函数**：

$$
w(t) = \begin{cases}
(a+2)|t|^3 - (a+3)|t|^2 + 1 & \text{if } |t| \leq 1 \\
a|t|^3 - 5a|t|^2 + 8a|t| - 4a & \text{if } 1 < |t| < 2 \\
0 & \text{if } |t| \geq 2
\end{cases}
$$

其中 $a = -0.5$（常用值）。

**插值公式**：

对于坐标 $(f_x, f_y)$：

$$
P = \sum_{j=-1}^{2} \sum_{i=-1}^{2} I(y_0+j, x_0+i) \cdot w(dx - i) \cdot w(dy - j)
$$

其中 $(x_0, y_0)$ 是整数部分，$(dx, dy)$ 是小数部分。

**特点**：
- ✅ 质量最高，最平滑
- ❌ 计算量最大（16 个像素）

#### 权重函数实现

**函数位置**: `chapt4.cpp` 第 121-131 行

```cpp
float cubicWeight(float t) {
  const float a = -0.5f;
  t = std::abs(t);
  if (t <= 1.0f) {
    return (a + 2.0f) * t * t * t - (a + 3.0f) * t * t + 1.0f;
  } else if (t < 2.0f) {
    return a * t * t * t - 5.0f * a * t * t + 8.0f * a * t - 4.0f * a;
  } else {
    return 0.0f;
  }
}
```

**步骤说明**：
1. **第 123 行**：取绝对值
2. **第 124-125 行**：$|t| \leq 1$ 时的公式
3. **第 126-127 行**：$1 < |t| < 2$ 时的公式
4. **第 128-129 行**：$|t| \geq 2$ 时权重为 0

#### 三次插值实现

**函数位置**: `chapt4.cpp` 第 134-173 行

```cpp
cv::Mat imgResizeCubic(const cv::Mat& src, int w, int h) {
  cv::Mat dst(h, w, src.type());
  float sw = static_cast<float>(src.cols);
  float sh = static_cast<float>(src.rows);
  int maxX = src.cols - 1;
  int maxY = src.rows - 1;

  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      float fx = (x / static_cast<float>(w)) * sw;
      float fy = (y / static_cast<float>(h)) * sh;
      int ix = static_cast<int>(std::floor(fx));
      int iy = static_cast<int>(std::floor(fy));
      float dx = fx - ix;
      float dy = fy - iy;

      float rgb[3] = {0.0f, 0.0f, 0.0f};
      for (int j = -1; j <= 2; ++j) {
        float wy = cubicWeight(dy - j);
        int sy = std::clamp(iy + j, 0, maxY);
        for (int i = -1; i <= 2; ++i) {
          float wx = cubicWeight(dx - i);
          int sx = std::clamp(ix + i, 0, maxX);
          cv::Vec3b pixel = src.at<cv::Vec3b>(sy, sx);
          for (int c = 0; c < 3; ++c) {
            rgb[c] += pixel[c] * wx * wy;
          }
        }
      }

      cv::Vec3b result;
      for (int c = 0; c < 3; ++c) {
        result[c] =
            static_cast<uchar>(std::clamp(std::round(rgb[c]), 0.0f, 255.0f));
      }
      dst.at<cv::Vec3b>(y, x) = result;
    }
  }
  return dst;
}
```

**步骤说明**：
1. **第 143-147 行**：计算浮点坐标和小数部分
2. **第 150-161 行**：双重循环遍历 4×4 邻域
   - **第 151 行**：计算垂直方向权重 $w_y$
   - **第 154 行**：计算水平方向权重 $w_x$
   - **第 158 行**：累加加权像素值：$\sum w_x \cdot w_y \cdot pixel$
3. **第 164-168 行**：限制到 [0, 255] 并写入结果

**老太太理解版**：
最精细的方法，看周围 16 个点（4×4 的格子）。每个点根据距离用特殊公式计算权重，离得远的点权重快速衰减。就像听一群专家意见，但离问题越近的专家意见越重要。结果最平滑，但计算也最慢。

---

## 5. 图像旋转（三种插值）

图像旋转需要三角变换和插值方法。

### 5.1 旋转数学原理

#### 旋转矩阵

绕原点逆时针旋转角度 $\theta$：

$$
\begin{bmatrix}
x' \\
y'
\end{bmatrix}
=
\begin{bmatrix}
\cos\theta & -\sin\theta \\
\sin\theta & \cos\theta
\end{bmatrix}
\begin{bmatrix}
x \\
y
\end{bmatrix}
$$

#### 绕图像中心旋转

为了绕图像中心旋转，需要三步变换：

1. **平移到原点**：$(x, y) \rightarrow (x - c_x, y - c_y)$
2. **旋转**：应用旋转矩阵
3. **平移回去**：$(x', y') \rightarrow (x' + c_x, y' + c_y)$

其中 $(c_x, c_y) = (\frac{W}{2}, \frac{H}{2})$ 是图像中心。

#### 逆变换（用于插值）

为了填充目标图像，我们对每个目标像素计算其在源图像中的位置（**逆映射**）：

$$
\begin{bmatrix}
x_{src} \\
y_{src}
\end{bmatrix}
=
\begin{bmatrix}
\cos\theta & \sin\theta \\
-\sin\theta & \cos\theta
\end{bmatrix}
\begin{bmatrix}
x_{dst} - c_x \\
y_{dst} - c_y
\end{bmatrix}
+
\begin{bmatrix}
c_x \\
c_y
\end{bmatrix}
$$

**老太太理解版**：
想象一张照片钉在中心点上旋转。但编程时我们反着来：对于新图片上的每个点，问"你是从老图片哪里转过来的？"找到那个位置，把颜色取过来。

---

### 5.2 最近邻插值旋转

**函数位置**: `chapt4.cpp` 第 181-206 行

```cpp
cv::Mat imgRotateNearest(const cv::Mat& src, float angleDeg) {
  float angle = angleDeg * CV_PI / 180.0f;
  float cosA = std::cos(angle);
  float sinA = std::sin(angle);
  int width = src.cols;
  int height = src.rows;
  float cx = width / 2.0f;
  float cy = height / 2.0f;

  cv::Mat dst(src.rows, src.cols, src.type(), cv::Scalar(0, 0, 0));

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      float xf = x - cx;
      float yf = y - cy;
      float srcX = cosA * xf + sinA * yf + cx;
      float srcY = -sinA * xf + cosA * yf + cy;
      int sx = static_cast<int>(std::round(srcX));
      int sy = static_cast<int>(std::round(srcY));
      if (sx >= 0 && sx < width && sy >= 0 && sy < height) {
        dst.at<cv::Vec3b>(y, x) = src.at<cv::Vec3b>(sy, sx);
      }
    }
  }
  return dst;
}
```

**步骤说明**：
1. **第 182 行**：角度转弧度：$\theta_{rad} = \theta_{deg} \times \frac{\pi}{180}$
2. **第 183-184 行**：预计算三角函数值
3. **第 187-188 行**：计算图像中心
4. **第 194-195 行**：平移到原点
5. **第 196-197 行**：应用逆旋转矩阵
   - $x_{src} = \cos\theta \cdot x_f + \sin\theta \cdot y_f + c_x$
   - $y_{src} = -\sin\theta \cdot x_f + \cos\theta \cdot y_f + c_y$
6. **第 198-199 行**：四舍五入到最近整数
7. **第 200-202 行**：边界检查并复制像素

---

### 5.3 双线性插值旋转

**函数位置**: `chapt4.cpp` 第 209-259 行

原理与缩放的双线性插值相同，只是坐标计算使用旋转矩阵。

**关键代码**（第 226-227 行）：
```cpp
float srcX = cosA * xf + sinA * yf + cx;
float srcY = -sinA * xf + cosA * yf + cy;
```

然后对 $(srcX, srcY)$ 周围的 4 个像素进行双线性插值（第 230-254 行）。

---

### 5.4 三次插值旋转

**函数位置**: `chapt4.cpp` 第 262-312 行

使用 Catmull-Rom 三次插值，原理与缩放相同。

**关键代码**（第 279-280 行）：
```cpp
float srcX = cosA * xf + sinA * yf + cx;
float srcY = -sinA * xf + cosA * yf + cy;
```

然后对 $(srcX, srcY)$ 周围的 16 个像素进行三次插值（第 288-300 行）。

---

## 6. 插值方法对比

### 6.1 质量对比

| 方法 | 邻域大小 | 计算量 | 质量 | 适用场景 |
|------|----------|--------|------|----------|
| 最近邻 | 1 像素 | 最低 | 低（锯齿） | 快速预览 |
| 双线性 | 2×2 = 4 | 中等 | 中等（平滑） | 一般应用 |
| 三次 | 4×4 = 16 | 最高 | 高（最平滑） | 高质量要求 |

### 6.2 缩放效果

- **最近邻**：放大后有明显马赛克，缩小后有锯齿
- **双线性**：较平滑，但边缘可能模糊
- **三次**：最平滑，边缘保留较好

### 6.3 旋转效果

- **最近邻**：旋转后边缘锯齿明显
- **双线性**：边缘较平滑
- **三次**：边缘最平滑，细节保留最好

---

## 7. 执行流程

Chapter 4 的 `runChapt4()` 函数执行顺序：

```
1. 平移变换 (tx=100, ty=100)
     ↓
2. 水平翻转 (mode=1)
     ↓
3. 转置
     ↓
4. 缩放到 100×100 (使用三次插值)
     ↓
5. 旋转 30 度 (使用三次插值)
```

输出文件：
- `chapt4_translated.png` - 平移结果
- `chapt4_flipped.png` - 翻转结果
- `chapt4_transposed.png` - 转置结果
- `chapt4_resized.png` - 缩放结果
- `chapt4_rotated.png` - 旋转结果

---

## 8. 总结

Chapter 4 涵盖的几何变换：

| 变换 | 数学模型 | 插值方法 | 代码行数 |
|------|----------|----------|----------|
| 平移 | 向量加法 | 无需插值 | 8-22 |
| 翻转 | 坐标反转 | 无需插值 | 25-45 |
| 转置 | 坐标互换 | 无需插值 | 48-57 |
| 缩放（最近邻） | 比例映射 | 最近邻 | 60-75 |
| 缩放（双线性） | 比例映射 | 4 点双线性 | 78-118 |
| 缩放（三次） | 比例映射 | 16 点三次 | 134-173 |
| 旋转（最近邻） | 旋转矩阵 | 最近邻 | 181-206 |
| 旋转（双线性） | 旋转矩阵 | 4 点双线性 | 209-259 |
| 旋转（三次） | 旋转矩阵 | 16 点三次 | 262-312 |

**关键公式**：
- 旋转矩阵：$R(\theta) = \begin{bmatrix} \cos\theta & -\sin\theta \\ \sin\theta & \cos\theta \end{bmatrix}$
- 双线性插值：$P = (1-dx)(1-dy)P_{00} + dx(1-dy)P_{10} + (1-dx)dy P_{01} + dx \cdot dy \cdot P_{11}$
- Catmull-Rom 权重：$w(t) = (a+2)|t|^3 - (a+3)|t|^2 + 1$ （$|t| \leq 1$）

这些几何变换是图像处理的基础操作，广泛应用于图像校正、增强和数据增广。
