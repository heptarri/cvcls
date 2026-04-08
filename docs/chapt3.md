# Chapter 3 - 直方图与灰度变换

本文档详细介绍 Chapter 3 中实现的直方图分析和各种灰度变换算法的数学原理。

## 目录

- [1. 直方图](#1-直方图)
- [2. 线性灰度变换](#2-线性灰度变换)
- [3. 对数灰度变换](#3-对数灰度变换)
- [4. Gamma 变换](#4-gamma-变换)
- [5. 二值化与 Otsu 阈值](#5-二值化与-otsu-阈值)
- [6. 分段线性变换](#6-分段线性变换)
- [7. 直方图均衡化](#7-直方图均衡化)

---

## 1. 直方图

### 1.1 什么是直方图

图像直方图是图像灰度级分布的统计图，横轴表示灰度级（0-255），纵轴表示该灰度级出现的频次。

### 1.2 数学定义

对于灰度图像 $I$（大小为 $H \times W$），灰度级 $k$ 的直方图值为：

$$
h(k) = \sum_{y=0}^{H-1} \sum_{x=0}^{W-1} \delta(I(y,x), k)
$$

其中：
$$
\delta(a, b) = \begin{cases}
1 & \text{if } a = b \\
0 & \text{otherwise}
\end{cases}
$$

简单说：$h(k)$ 就是图像中灰度值等于 $k$ 的像素个数。

### 1.3 计算直方图

**函数位置**: `chapt3.cpp` 第 14-23 行

```cpp
std::array<uint32_t, 256> calculateHistogram(const cv::Mat& gray) {
  std::array<uint32_t, 256> hist{};  // 值初始化为0

  for (int y : views::iota(0, gray.rows)) {
    for (int x : views::iota(0, gray.cols)) {
      hist[gray.at<uchar>(y, x)]++;
    }
  }
  return hist;
}
```

**步骤说明**：
1. **第 15 行**：创建 256 个桶的计数器数组，初始化为 0
2. **第 17-20 行**：遍历图像每个像素
3. **第 19 行**：读取像素灰度值，对应桶的计数加 1
4. **第 22 行**：返回完整的直方图

**老太太理解版**：想象你有 256 个盒子，标号从 0 到 255。看图片上每个点的灰度值，比如是 100，就往 100 号盒子里放一个小球。最后数数每个盒子里有多少球，就是直方图。

### 1.4 绘制直方图

**函数位置**: `chapt3.cpp` 第 26-51 行

```cpp
cv::Mat drawHistogram(const std::array<uint32_t, 256>& hist) {
  constexpr int histH = 400;
  constexpr int histW = 512;
  constexpr int binW = histW / 256;

  cv::Mat img(histH, histW, CV_8UC3, cv::Scalar(0, 0, 0));

  const auto maxCount = std::ranges::max(hist);
  if (maxCount == 0) [[unlikely]] {
    return img;
  }

  for (int i : views::iota(0, 256)) {
    const auto normalized = static_cast<uint32_t>(
        std::round((hist[i] / static_cast<float>(maxCount)) * (histH - 1.0f)));
    const int x0 = i * binW;
    const int x1 = std::min((i + 1) * binW, histW);

    for (int x : views::iota(x0, x1)) {
      for (int y : views::iota(static_cast<int>(histH - normalized), histH)) {
        img.at<cv::Vec3b>(y, x) = cv::Vec3b(255, 255, 255);
      }
    }
  }
  return img;
}
```

**归一化公式**：

$$
h_{norm}(k) = \frac{h(k)}{\max(h)} \times (H_{display} - 1)
$$

**步骤说明**：
1. **第 27-29 行**：设置画布尺寸（400×512）
2. **第 31 行**：创建黑色背景图像
3. **第 33 行**：找到直方图的最大值（用于归一化）
4. **第 38-40 行**：对每个灰度级，计算归一化后的高度
5. **第 44-47 行**：绘制白色柱状图

**老太太理解版**：把刚才数出来的结果画成柱状图。最高的柱子顶到天花板（画布顶部），其他柱子按比例缩放，这样方便观察分布。

---

## 2. 线性灰度变换

### 2.1 数学原理

线性变换是最简单的灰度变换，公式为：

$$
s = \alpha \cdot r + \beta
$$

其中：
- $r$ 是输入灰度值（0-255）
- $s$ 是输出灰度值（0-255）
- $\alpha$ 是斜率（对比度系数）
- $\beta$ 是截距（亮度调整）

**参数效果**：
- $\alpha > 1$：增强对比度（图像变锐利）
- $0 < \alpha < 1$：降低对比度（图像变柔和）
- $\beta > 0$：增加亮度（图像变亮）
- $\beta < 0$：降低亮度（图像变暗）

### 2.2 代码实现

**函数位置**: `chapt3.cpp` 第 54-67 行

```cpp
cv::Mat genGrayLinTrans(const cv::Mat& src, float alpha, float beta) {
  const cv::Mat gray = toGrayscale(src);
  cv::Mat dst(gray.rows, gray.cols, CV_8UC1);

  for (int y : views::iota(0, gray.rows)) {
    for (int x : views::iota(0, gray.cols)) {
      const float value = gray.at<uchar>(y, x);
      const float mapped = std::round(alpha * value + beta);
      dst.at<uchar>(y, x) =
          static_cast<uchar>(std::clamp(mapped, 0.0f, 255.0f));
    }
  }
  return dst;
}
```

**步骤说明**：
1. **第 55 行**：转换为灰度图
2. **第 56 行**：创建输出图像
3. **第 58-62 行**：遍历每个像素
4. **第 61 行**：应用线性变换公式 $s = \alpha \times r + \beta$
5. **第 62-63 行**：限制在 [0, 255] 范围内

**使用示例** (`chapt3.cpp` 第 240 行)：
```cpp
cv::Mat linear = genGrayLinTrans(img, 1.2f, 20.0f);
```
- $\alpha = 1.2$：对比度增强 20%
- $\beta = 20$：整体亮度增加 20

**老太太理解版**：线性变换就像调节电视的亮度和对比度。$\alpha$ 是对比度旋钮（转大了画面更清晰），$\beta$ 是亮度旋钮（调高了画面更亮）。

---

## 3. 对数灰度变换

### 3.1 数学原理

对数变换用于扩展图像中较暗的灰度级，压缩较亮的灰度级：

$$
s = c \cdot \log(1 + r)
$$

其中：
- $r$ 是输入灰度值（0-255）
- $s$ 是输出灰度值（0-255）
- $c$ 是缩放常数

**归一化缩放常数**：

为了让输出范围也是 [0, 255]，当 $r = 255$ 时，$s$ 应该等于 255：

$$
c = \frac{255}{\log(256)}
$$

### 3.2 对数函数特性

$$
\frac{d}{dr}\log(1+r) = \frac{1}{1+r}
$$

- 当 $r$ 较小时，导数较大 → 暗部细节被拉伸
- 当 $r$ 较大时，导数较小 → 亮部被压缩

**应用场景**：增强图像中暗部细节，如医学图像、遥感图像。

### 3.3 代码实现

**函数位置**: `chapt3.cpp` 第 70-84 行

```cpp
cv::Mat genGrayLogTrans(const cv::Mat& src) {
  const cv::Mat gray = toGrayscale(src);
  float scale = 255.0f / std::log(256.0f);
  cv::Mat dst(gray.rows, gray.cols, CV_8UC1);

  for (int y : views::iota(0, gray.rows)) {
    for (int x : views::iota(0, gray.cols)) {
      const float value = gray.at<uchar>(y, x);
      const float mapped = std::log(value + 1.0f) * scale;
      dst.at<uchar>(y, x) =
          static_cast<uchar>(std::clamp(mapped, 0.0f, 255.0f));
    }
  }
  return dst;
}
```

**步骤说明**：
1. **第 72 行**：计算缩放系数 $c = \frac{255}{\log(256)}$
2. **第 78 行**：应用对数变换 $s = c \times \log(r + 1)$
   - 注意：加 1 防止 $\log(0)$ 未定义

**老太太理解版**：想象你在暗室里看照片，看不清暗处的细节。对数变换就像打开一盏灯，把暗的地方照亮（拉伸），但不让亮的地方过曝（压缩）。

---

## 4. Gamma 变换

### 4.1 数学原理

Gamma 变换（也叫幂次变换）是一种非线性变换：

$$
s = 255 \times \left(\frac{r}{255}\right)^\gamma
$$

其中：
- $r$ 是输入灰度值（0-255）
- $s$ 是输出灰度值（0-255）
- $\gamma$ 是 Gamma 系数

**Gamma 值的影响**：
- $\gamma > 1$：图像变暗，高光被压缩
- $\gamma < 1$：图像变亮，阴影被拉伸
- $\gamma = 1$：无变化（恒等变换）

### 4.2 曲线形状

$$
s = 255 \times \left(\frac{r}{255}\right)^\gamma
$$

- $\gamma = 0.5$：凹曲线（提亮）
- $\gamma = 1.0$：直线（不变）
- $\gamma = 2.0$：凸曲线（压暗）

### 4.3 代码实现

**函数位置**: `chapt3.cpp` 第 87-103 行

```cpp
cv::Mat genGammaTrans(const cv::Mat& src, float gamma) {
  cv::Mat dst(src.rows, src.cols, src.type());
  constexpr float inv255 = 1.0f / 255.0f;

  for (int y : views::iota(0, src.rows)) {
    for (int x : views::iota(0, src.cols)) {
      auto pixel = src.at<cv::Vec3b>(y, x);
      for (int c : views::iota(0, 3)) {
        const float normalized = pixel[c] * inv255;
        const float mapped = std::pow(normalized, gamma) * 255.0f;
        pixel[c] = static_cast<uchar>(std::clamp(mapped, 0.0f, 255.0f));
      }
      dst.at<cv::Vec3b>(y, x) = pixel;
    }
  }
  return dst;
}
```

**步骤说明**：
1. **第 89 行**：预计算 $\frac{1}{255}$
2. **第 93 行**：读取 RGB 像素
3. **第 94-98 行**：对每个颜色通道：
   - **第 95 行**：归一化到 [0, 1]：$r' = \frac{r}{255}$
   - **第 96 行**：应用幂次：$s' = (r')^\gamma$
   - **第 96 行**：恢复到 [0, 255]：$s = s' \times 255$
4. **第 99 行**：写入输出图像

**使用示例** (`chapt3.cpp` 第 249 行)：
```cpp
cv::Mat gamma = genGammaTrans(img, 2.5f);
```
- $\gamma = 2.5$：图像明显变暗，适合校正过亮的图像

**老太太理解版**：Gamma 变换就像给照片加滤镜。$\gamma$ 大于 1 像戴墨镜（变暗），$\gamma$ 小于 1 像打闪光灯（变亮）。和线性变换不同，它对不同亮度的地方调整程度不同。

---

## 5. 二值化与 Otsu 阈值

### 5.1 二值化原理

二值化将灰度图像转换为只有黑白两色的图像：

$$
I_{binary}(y, x) = \begin{cases}
255 & \text{if } I(y,x) > T \\
0 & \text{otherwise}
\end{cases}
$$

其中 $T$ 是阈值。

### 5.2 Otsu 自动阈值法

**核心思想**：选择一个阈值 $T$，使得前景和背景的类间方差最大。

#### 数学推导

设：
- $w_0$ = 背景像素比例（$I \leq T$）
- $w_1$ = 前景像素比例（$I > T$）
- $\mu_0$ = 背景平均灰度
- $\mu_1$ = 前景平均灰度
- $\mu_T$ = 整体平均灰度

**类间方差**：

$$
\sigma_B^2 = w_0 \cdot w_1 \cdot (\mu_0 - \mu_1)^2
$$

**目标**：找到使 $\sigma_B^2$ 最大的阈值 $T^*$：

$$
T^* = \arg\max_{T} \sigma_B^2(T)
$$

### 5.3 Otsu 算法实现

**函数位置**: `chapt3.cpp` 第 106-142 行

```cpp
uint8_t otsuThreshold(const cv::Mat& gray) {
  const auto hist = calculateHistogram(gray);
  uint32_t total = 0;
  float sum = 0.0f;

  for (int i : views::iota(0, 256)) {
    total += hist[i];
    sum += i * hist[i];
  }

  float sumB = 0.0f;
  uint32_t wB = 0;
  float maxVar = 0.0f;
  uint8_t threshold = 0;

  for (int t : views::iota(0, 256)) {
    wB += hist[t];
    if (wB == 0) [[unlikely]]
      continue;

    const uint32_t wF = total - wB;
    if (wF == 0) [[unlikely]]
      break;

    sumB += t * hist[t];
    const float mB = sumB / wB;
    const float mF = (sum - sumB) / wF;

    const float varBetween = wB * wF * (mB - mF) * (mB - mF);
    if (varBetween > maxVar) {
      maxVar = varBetween;
      threshold = static_cast<uint8_t>(t);
    }
  }

  return threshold;
}
```

**步骤说明**：
1. **第 107 行**：计算直方图
2. **第 111-114 行**：计算总像素数和加权和
   - $total = \sum_{i=0}^{255} h(i)$
   - $sum = \sum_{i=0}^{255} i \times h(i)$
3. **第 121 行**：遍历所有可能的阈值（0-255）
4. **第 122 行**：累加背景像素数 $w_B$
5. **第 126 行**：计算前景像素数 $w_F = total - w_B$
6. **第 130-132 行**：计算背景和前景的平均灰度
   - $\mu_B = \frac{sumB}{w_B}$
   - $\mu_F = \frac{sum - sumB}{w_F}$
7. **第 134 行**：计算类间方差
   $$varBetween = w_B \times w_F \times (\mu_B - \mu_F)^2$$
8. **第 135-137 行**：记录最大方差对应的阈值

### 5.4 二值化实现

**函数位置**: `chapt3.cpp` 第 145-159 行

```cpp
cv::Mat genThreshold(const cv::Mat& src, uint8_t thresh, uint8_t typeFlag) {
  const cv::Mat gray = toGrayscale(src);
  const uint8_t threshold = (typeFlag == 2) ? otsuThreshold(gray) : thresh;
  cv::Mat dst(gray.rows, gray.cols, CV_8UC1);

  for (int y : views::iota(0, gray.rows)) {
    for (int x : views::iota(0, gray.cols)) {
      const uchar value = gray.at<uchar>(y, x);
      const uchar out = (typeFlag == 1) ? ((value > threshold) ? 0 : 255)
                                        : ((value > threshold) ? 255 : 0);
      dst.at<uchar>(y, x) = out;
    }
  }
  return dst;
}
```

**typeFlag 参数**：
- `0`：正向二值化（$I > T$ → 255，否则 → 0）
- `1`：反向二值化（$I > T$ → 0，否则 → 255）
- `2`：使用 Otsu 自动计算阈值

**老太太理解版**：
想象你要把照片变成剪影（只有黑白）。需要定一个"分界线"（阈值）：比这亮的变白，比这暗的变黑。Otsu 算法会自动找到最佳分界线，让前景和背景分得最清楚。

---

## 6. 分段线性变换

### 6.1 数学原理

分段线性变换由三段直线组成，通过两个控制点 $(r_1, s_1)$ 和 $(r_2, s_2)$ 定义：

$$
s = \begin{cases}
\frac{s_1}{r_1} \times r & \text{if } r < r_1 \\
\frac{s_2 - s_1}{r_2 - r_1} \times (r - r_1) + s_1 & \text{if } r_1 \leq r < r_2 \\
\frac{255 - s_2}{255 - r_2} \times (r - r_2) + s_2 & \text{if } r \geq r_2
\end{cases}
$$

**应用**：可以实现灰度拉伸、压缩特定区域的对比度。

### 6.2 代码实现

**函数位置**: `chapt3.cpp` 第 162-188 行

```cpp
cv::Mat genPiecewiseLin(const cv::Mat& src, uint8_t r1, uint8_t s1, uint8_t r2,
                        uint8_t s2) {
  std::array<uint8_t, 256> lut;

  for (int i = 0; i < 256; ++i) {
    if (i < r1) {
      lut[i] = static_cast<uint8_t>(
          std::round((s1 / static_cast<float>(std::max(r1, (uint8_t)1))) * i));
    } else if (i < r2) {
      float slope = (s2 - s1) / static_cast<float>(r2 - r1);
      lut[i] = static_cast<uint8_t>(std::round(slope * (i - r1) + s1));
    } else {
      float slope = (255.0f - s2) / (255.0f - r2);
      lut[i] = static_cast<uint8_t>(std::round(slope * (i - r2) + s2));
    }
  }

  cv::Mat dst(src.rows, src.cols, src.type());
  for (int y = 0; y < src.rows; ++y) {
    for (int x = 0; x < src.cols; ++x) {
      cv::Vec3b pixel = src.at<cv::Vec3b>(y, x);
      dst.at<cv::Vec3b>(y, x) =
          cv::Vec3b(lut[pixel[0]], lut[pixel[1]], lut[pixel[2]]);
    }
  }
  return dst;
}
```

**步骤说明**：
1. **第 164-176 行**：构建查找表（LUT），计算所有可能输入值的输出
   - **第 167-169 行**：第一段（$r < r_1$）
   - **第 170-172 行**：第二段（$r_1 \leq r < r_2$）
   - **第 173-175 行**：第三段（$r \geq r_2$）
2. **第 179-185 行**：对每个像素的每个通道应用 LUT

**使用示例** (`chapt3.cpp` 第 256 行)：
```cpp
cv::Mat piecewise = genPiecewiseLin(img, 50, 20, 200, 240);
```
- $(r_1, s_1) = (50, 20)$：暗部压缩
- $(r_2, s_2) = (200, 240)$：亮部压缩
- 中间段拉伸（增强对比度）

**老太太理解版**：
想象你有一个亮度调节杆，但不是整体调，而是分段调：暗的地方往下压一点，亮的地方往下压一点，中间的地方往上拉，这样中间部分的细节就更清楚了。

---

## 7. 直方图均衡化

### 7.1 数学原理

直方图均衡化的目的是**使图像的直方图分布更均匀**，从而增强对比度。

#### 累积分布函数（CDF）

定义累积分布函数：

$$
CDF(k) = \sum_{i=0}^{k} h(i)
$$

其中 $h(i)$ 是灰度级 $i$ 的像素个数。

#### 均衡化公式

新的灰度值为：

$$
s_k = \left\lfloor \frac{CDF(k) - CDF_{min}}{total - CDF_{min}} \times 255 \right\rfloor
$$

其中：
- $CDF(k)$ 是灰度级 $k$ 的累积分布值
- $CDF_{min}$ 是最小的非零 CDF 值
- $total$ 是总像素数

#### 为什么有效？

均衡化后的直方图近似均匀分布，即每个灰度级的像素数接近 $\frac{total}{256}$。

### 7.2 代码实现

**函数位置**: `chapt3.cpp` 第 191-229 行

```cpp
cv::Mat genEqualizeHist(const cv::Mat& src) {
  cv::Mat gray = toGrayscale(src);
  auto hist = calculateHistogram(gray);
  uint32_t total = gray.rows * gray.cols;

  std::array<uint32_t, 256> cdf;
  uint32_t cumulative = 0;
  for (int i = 0; i < 256; ++i) {
    cumulative += hist[i];
    cdf[i] = cumulative;
  }

  uint32_t cdfMin = 0;
  for (int i = 0; i < 256; ++i) {
    if (cdf[i] > 0) {
      cdfMin = cdf[i];
      break;
    }
  }

  std::array<uint8_t, 256> lut;
  float totalF = static_cast<float>(total);
  float cdfMinF = static_cast<float>(cdfMin);

  for (int i = 0; i < 256; ++i) {
    float numerator = cdf[i] - cdfMinF;
    float mapped = std::round((numerator / (totalF - cdfMinF)) * 255.0f);
    lut[i] = static_cast<uint8_t>(std::clamp(mapped, 0.0f, 255.0f));
  }

  cv::Mat dst(gray.rows, gray.cols, CV_8UC1);
  for (int y = 0; y < gray.rows; ++y) {
    for (int x = 0; x < gray.cols; ++x) {
      uchar value = gray.at<uchar>(y, x);
      dst.at<uchar>(y, x) = lut[value];
    }
  }
  return dst;
}
```

**步骤说明**：
1. **第 193 行**：计算直方图
2. **第 196-200 行**：计算累积分布函数（CDF）
   $$CDF(k) = \sum_{i=0}^{k} h(i)$$
3. **第 203-208 行**：找到最小的非零 CDF 值
4. **第 210-219 行**：构建映射表（LUT）
   - **第 216 行**：应用均衡化公式
     $$s = \frac{CDF(r) - CDF_{min}}{total - CDF_{min}} \times 255$$
5. **第 221-227 行**：对每个像素应用 LUT

**老太太理解版**：
想象你有一堆硬币要放进盒子。原来的分布很不均：有的盒子塞得满满的，有的盒子空空的。直方图均衡化就是重新分配，让每个盒子里的硬币数量差不多，这样图像的亮度分布更均匀，看起来更清楚。

### 7.3 效果

- **对比度增强**：暗部和亮部的差异更明显
- **细节显现**：原本不明显的细节变得清晰
- **适用场景**：医学图像、曝光不足的照片

---

## 8. 执行流程

Chapter 3 的 `runChapt3()` 函数执行顺序：

```
1. 计算并绘制直方图
     ↓
2. 线性灰度变换 (α=1.2, β=20)
     ↓
3. 对数灰度变换
     ↓
4. Gamma 变换 (γ=2.5)
     ↓
5. Otsu 自动阈值二值化
     ↓
6. 分段线性变换
     ↓
7. 直方图均衡化
```

每个操作生成对应的输出文件：
- `chapt3_histogram.png` - 原图直方图
- `chapt3_linear.png` - 线性变换结果
- `chapt3_log.png` - 对数变换结果
- `chapt3_gamma.png` - Gamma 变换结果
- `chapt3_threshold.png` - 二值化结果
- `chapt3_piecewise.png` - 分段线性变换结果
- `chapt3_equalized.png` - 直方图均衡化结果

---

## 总结

Chapter 3 涵盖的算法：

| 算法 | 公式 | 用途 | 代码行数 |
|------|------|------|----------|
| 直方图计算 | $h(k) = \sum \delta(I, k)$ | 统计灰度分布 | 14-23 |
| 线性变换 | $s = \alpha r + \beta$ | 调节亮度对比度 | 54-67 |
| 对数变换 | $s = c \log(1+r)$ | 增强暗部细节 | 70-84 |
| Gamma 变换 | $s = 255(r/255)^\gamma$ | 非线性调节 | 87-103 |
| Otsu 阈值 | $\arg\max \sigma_B^2$ | 自动二值化 | 106-142 |
| 分段线性 | 三段式映射 | 局部对比度调节 | 162-188 |
| 直方图均衡化 | CDF 映射 | 增强整体对比度 | 191-229 |

这些变换是图像增强的基础工具，广泛应用于预处理阶段。
