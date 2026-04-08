# Others - 单目与双目视觉测量

本文档详细介绍 others 模块中实现的视觉测量算法，包括单目视觉测量和双目视觉基础矩阵估计（八点法）。

## 目录

- [1. 单目视觉测量](#1-单目视觉测量)
- [2. 双目视觉测量（八点法）](#2-双目视觉测量八点法)

---

## 1. 单目视觉测量

单目视觉测量利用相机内参和已知深度信息，将 2D 图像坐标转换为 3D 世界坐标。

### 1.1 相机针孔模型

#### 数学模型

相机针孔模型描述了 3D 世界坐标到 2D 图像坐标的投影关系：

$$
s \begin{bmatrix}
u \\
v \\
1
\end{bmatrix}
=
\begin{bmatrix}
f_x & 0 & c_x \\
0 & f_y & c_y \\
0 & 0 & 1
\end{bmatrix}
\begin{bmatrix}
X \\
Y \\
Z
\end{bmatrix}
$$

其中：
- $(u, v)$ 是图像坐标（像素）
- $(X, Y, Z)$ 是世界坐标（毫米）
- $s = Z$ 是深度（尺度因子）
- $f_x, f_y$ 是焦距（像素单位）
- $(c_x, c_y)$ 是主点（图像中心，像素单位）

#### 相机内参矩阵

$$
K = \begin{bmatrix}
f_x & 0 & c_x \\
0 & f_y & c_y \\
0 & 0 & 1
\end{bmatrix}
$$

**老太太理解版**：
想象相机像一个针孔，光线通过针孔投影到后面的平面（图像传感器）上。针孔到平面的距离就是焦距。焦距越大，看得越远；主点是图像的中心位置。

---

### 1.2 从图像到世界坐标

#### 逆投影公式

已知图像坐标 $(u, v)$ 和深度 $Z$，求世界坐标 $(X, Y, Z)$：

$$
X = \frac{(u - c_x) \cdot Z}{f_x}
$$

$$
Y = \frac{(v - c_y) \cdot Z}{f_y}
$$

$$
Z = Z
$$

**推导**：

从投影公式：
$$
u = f_x \frac{X}{Z} + c_x
$$

移项得：
$$
X = \frac{(u - c_x) \cdot Z}{f_x}
$$

同理可得 $Y$ 的公式。

**老太太理解版**：
你在照片上看到一个点，如果你知道这个点离相机有多远（深度），就能算出这个点在现实世界中的位置。就像看地图：知道比例尺（焦距）和距离，就能算出真实距离。

---

### 1.3 代码实现

#### 数据结构

**文件位置**: `single_measure.h`

```cpp
struct CameraIntrinsics {
  double fx;  // 焦距 X
  double fy;  // 焦距 Y
  double cx;  // 主点 X
  double cy;  // 主点 Y
};

struct Point3D {
  double x, y, z;
};

struct MeasurePoint {
  cv::Point2f image;  // 图像坐标
  Point3D world;      // 世界坐标
  std::string label;  // 标签
};
```

#### 图像到世界坐标转换

**文件位置**: `single_measure.h`

```cpp
inline Point3D imageToWorld(const cv::Point2f& img, double depth,
                            const CameraIntrinsics& cam) {
  return Point3D{
      .x = (img.x - cam.cx) * depth / cam.fx,
      .y = (img.y - cam.cy) * depth / cam.fy,
      .z = depth
  };
}
```

**公式对应**：
- `.x`：对应公式 $X = \frac{(u - c_x) \cdot Z}{f_x}$
- `.y`：对应公式 $Y = \frac{(v - c_y) \cdot Z}{f_y}$
- `.z`：直接等于深度 $Z$

#### 3D 点距离计算

**文件位置**: `single_measure.h`

```cpp
inline double distance(const Point3D& p1, const Point3D& p2) {
  double dx = p1.x - p2.x;
  double dy = p1.y - p2.y;
  double dz = p1.z - p2.z;
  return std::sqrt(dx * dx + dy * dy + dz * dz);
}
```

**欧几里得距离公式**：

$$
d = \sqrt{(x_1 - x_2)^2 + (y_1 - y_2)^2 + (z_1 - z_2)^2}
$$

---

### 1.4 测量流程

**函数位置**: `single_measure.cpp` 第 12-72 行

```cpp
void runSingleMeasure(const cv::Mat& img) {
  const CameraIntrinsics cam{
      .fx = 800.0, .fy = 800.0, .cx = img.cols / 2.0, .cy = img.rows / 2.0};

  const auto [cx, cy] = std::pair{img.cols / 2, img.rows / 2};

  const std::vector pts = {
      cv::Point2f(cx - 100, cy - 100), cv::Point2f(cx + 100, cy - 100),
      cv::Point2f(cx + 100, cy + 100), cv::Point2f(cx - 100, cy + 100)};

  constexpr double depth = 1000.0;

  std::vector<MeasurePoint> points;
  int idx = 0;
  rng::transform(pts, std::back_inserter(points), [&](auto p) {
    return MeasurePoint{.image = p,
                        .world = imageToWorld(p, depth, cam),
                        .label = "P" + std::to_string(++idx)};
  });

  std::vector<double> dists;
  for (size_t i = 0; i + 1 < points.size(); ++i) {
    dists.push_back(distance(points[i].world, points[i + 1].world));
  }

  const double closing = distance(points.back().world, points.front().world);
  const double perimeter =
      std::accumulate(dists.begin(), dists.end(), 0.0) + closing;
  
  // ... 输出结果 ...
}
```

**步骤说明**：
1. **第 13-14 行**：定义相机内参
   - 焦距 $f_x = f_y = 800$ 像素
   - 主点在图像中心
2. **第 18-20 行**：定义 4 个测量点（矩形四角）
3. **第 22 行**：设定深度 $Z = 1000$ mm（1 米）
4. **第 26-29 行**：将图像坐标转换为世界坐标
5. **第 32-34 行**：计算相邻点之间的距离
6. **第 37 行**：计算闭合距离（最后一点到第一点）
7. **第 38-39 行**：计算周长

**示例输出**：
```
Single-View Measurement
Camera: fx=800.0 fy=800.0 cx=320.0 cy=240.0
Depth: 1000.0mm

3D Coordinates:
  P1 ( 220, 140) → (-125.0, -125.0, 1000.0)mm
  P2 ( 420, 140) → ( 125.0, -125.0, 1000.0)mm
  P3 ( 420, 340) → ( 125.0,  125.0, 1000.0)mm
  P4 ( 220, 340) → (-125.0,  125.0, 1000.0)mm

Distances:
  P1 → P2: 250.0mm
  P2 → P3: 250.0mm
  P3 → P4: 250.0mm
  P4 → P1: 250.0mm (closing)

Summary:
  Perimeter: 1000.0mm (100.0cm)
  Area: 62500.0mm² (625.00cm²)
```

**老太太理解版**：
单目测量就像用尺子量照片上的东西。但你需要知道两个信息：相机的参数（就像尺子的刻度）和物体离相机有多远（深度）。知道了这些，就能把照片上的点换算成真实世界的位置和距离。

---

## 2. 双目视觉测量（八点法）

双目视觉利用两个相机从不同角度拍摄同一场景，通过对应点的视差来恢复 3D 信息。**基础矩阵**是描述两个视图几何关系的核心。

### 2.1 对极几何

#### 基本概念

- **对极点** (Epipole)：一个相机中心在另一个相机图像上的投影
- **对极线** (Epipolar Line)：空间中一条射线在图像上的投影
- **对极约束**：对应点必须位于各自图像的对极线上

**老太太理解版**：
想象你左右眼同时看一个东西。左眼看到的点，右眼也能看到，但位置不同。右眼看到的这个点，一定在左眼视线的某条特定线上（对极线），这就是对极约束。

---

### 2.2 基础矩阵

#### 数学定义

基础矩阵 $F$ 是一个 $3 \times 3$ 的矩阵，描述左右图像对应点的关系：

$$
\mathbf{x}_r^T \cdot F \cdot \mathbf{x}_l = 0
$$

其中：
- $\mathbf{x}_l = [u_l, v_l, 1]^T$ 是左图像点（齐次坐标）
- $\mathbf{x}_r = [u_r, v_r, 1]^T$ 是右图像点（齐次坐标）

这个等式称为**对极约束**。

#### 基础矩阵的性质

1. **秩为 2**：$\text{rank}(F) = 2$
2. **7 个自由度**：$F$ 有 9 个元素，但有约束：
   - $\det(F) = 0$（秩为 2）
   - 尺度不确定性（$F$ 和 $kF$ 等价）
   - 所以实际自由度 = 9 - 1 - 1 = 7

**老太太理解版**：
基础矩阵就像一本"字典"，告诉你左图的一个点对应右图的哪条线。这个字典需要至少 8 对对应点来"学习"（八点法）。

---

### 2.3 八点法（Eight-Point Algorithm）

#### 算法原理

利用 8 对（或更多）对应点来估计基础矩阵 $F$。

#### 对极约束展开

将对极约束展开：

$$
[u_r, v_r, 1] \begin{bmatrix}
f_{11} & f_{12} & f_{13} \\
f_{21} & f_{22} & f_{23} \\
f_{31} & f_{32} & f_{33}
\end{bmatrix}
\begin{bmatrix}
u_l \\
v_l \\
1
\end{bmatrix} = 0
$$

展开得：

$$
u_r u_l f_{11} + u_r v_l f_{12} + u_r f_{13} + v_r u_l f_{21} + v_r v_l f_{22} + v_r f_{23} + u_l f_{31} + v_l f_{32} + f_{33} = 0
$$

整理成矩阵形式：

$$
\begin{bmatrix}
u_r u_l & u_r v_l & u_r & v_r u_l & v_r v_l & v_r & u_l & v_l & 1
\end{bmatrix}
\begin{bmatrix}
f_{11} \\ f_{12} \\ f_{13} \\ f_{21} \\ f_{22} \\ f_{23} \\ f_{31} \\ f_{32} \\ f_{33}
\end{bmatrix}
= 0
$$

#### 线性方程组

对于 $n$ 对点，构建系数矩阵 $A$ ($n \times 9$)：

$$
A = \begin{bmatrix}
u_r^1 u_l^1 & u_r^1 v_l^1 & u_r^1 & v_r^1 u_l^1 & v_r^1 v_l^1 & v_r^1 & u_l^1 & v_l^1 & 1 \\
u_r^2 u_l^2 & u_r^2 v_l^2 & u_r^2 & v_r^2 u_l^2 & v_r^2 v_l^2 & v_r^2 & u_l^2 & v_l^2 & 1 \\
\vdots & \vdots & \vdots & \vdots & \vdots & \vdots & \vdots & \vdots & \vdots \\
u_r^n u_l^n & u_r^n v_l^n & u_r^n & v_r^n u_l^n & v_r^n v_l^n & v_r^n & u_l^n & v_l^n & 1
\end{bmatrix}
$$

目标：求解 $A \cdot \mathbf{f} = \mathbf{0}$

#### SVD 求解

使用奇异值分解（SVD）：

$$
A = U \Sigma V^T
$$

解是 $V$ 的最后一列（对应最小奇异值）。

#### 秩 2 约束

重构的 $F$ 可能不满足 $\text{rank}(F) = 2$，需要强制：

1. 对 $F$ 进行 SVD：$F = U \Sigma V^T$
2. 将最小奇异值设为 0：$\Sigma' = \text{diag}(\sigma_1, \sigma_2, 0)$
3. 重构：$F' = U \Sigma' V^T$

**老太太理解版**：
八点法就像解谜游戏。你有 8 对对应点（左图的点和右图的对应点），要找出它们之间的关系（基础矩阵）。用数学方法（SVD）找到最符合这 8 对点的那个"字典"。

---

### 2.4 点归一化（提高数值稳定性）

#### 为什么需要归一化？

直接使用像素坐标（如 0-1920）计算时，数值范围很大，容易导致数值不稳定。

#### 归一化步骤

1. **计算质心**：

$$
\bar{x} = \frac{1}{n} \sum_{i=1}^{n} x_i, \quad \bar{y} = \frac{1}{n} \sum_{i=1}^{n} y_i
$$

2. **计算平均距离**：

$$
d_{avg} = \frac{1}{n} \sum_{i=1}^{n} \sqrt{(x_i - \bar{x})^2 + (y_i - \bar{y})^2}
$$

3. **计算缩放因子**：

$$
s = \frac{\sqrt{2}}{d_{avg}}
$$

目标：让归一化后的点到原点的平均距离为 $\sqrt{2}$。

4. **归一化变换矩阵**：

$$
T = \begin{bmatrix}
s & 0 & -s\bar{x} \\
0 & s & -s\bar{y} \\
0 & 0 & 1
\end{bmatrix}
$$

5. **归一化点**：

$$
\mathbf{x}' = T \mathbf{x}
$$

#### 反归一化基础矩阵

使用归一化点计算得到 $F'$，需要反归一化：

$$
F = T_r^T F' T_l
$$

其中 $T_l$ 和 $T_r$ 分别是左右图像的归一化矩阵。

**老太太理解版**：
归一化就像把所有点都移到一个标准的小房间里（中心在原点，平均距离固定），在小房间里做计算更准确，算完再搬回原来的大房间。

---

### 2.5 代码实现

#### 点归一化

**函数位置**: `stereo_measure.cpp` 第 102-136 行

```cpp
void normalizePoints(const std::vector<cv::Point2f>& points,
                     std::vector<cv::Point2f>& normalized_points,
                     cv::Mat& transform) {
  // 计算质心
  cv::Point2f centroid(0, 0);
  for (const auto& p : points) {
    centroid += p;
  }
  centroid.x /= static_cast<float>(points.size());
  centroid.y /= static_cast<float>(points.size());

  // 计算平均距离
  float avg_dist = 0;
  for (const auto& p : points) {
    float dx = p.x - centroid.x;
    float dy = p.y - centroid.y;
    avg_dist += std::sqrt(dx * dx + dy * dy);
  }
  avg_dist /= static_cast<float>(points.size());

  // 缩放因子
  float scale = std::sqrt(2.0f) / avg_dist;

  // 构建变换矩阵
  transform = (cv::Mat_<double>(3, 3) << scale, 0, -scale * centroid.x, 0,
               scale, -scale * centroid.y, 0, 0, 1);

  // 归一化点
  normalized_points.clear();
  for (const auto& p : points) {
    float nx = scale * (p.x - centroid.x);
    float ny = scale * (p.y - centroid.y);
    normalized_points.emplace_back(nx, ny);
  }
}
```

**步骤说明**：
1. **第 106-110 行**：计算质心（所有点的平均位置）
2. **第 113-119 行**：计算点到质心的平均距离
3. **第 122 行**：计算缩放因子 $s = \frac{\sqrt{2}}{d_{avg}}$
4. **第 125-126 行**：构建归一化变换矩阵 $T$
5. **第 130-134 行**：对每个点应用归一化变换

---

#### 计算基础矩阵

**函数位置**: `stereo_measure.cpp` 第 138-216 行

```cpp
cv::Mat computeFundamentalMatrix(const std::vector<PointPair>& points) {
  if (points.size() < 8) {
    std::cerr << "错误: 需要至少8对点来计算基础矩阵" << std::endl;
    return cv::Mat();
  }

  // 分离左右点
  std::vector<cv::Point2f> left_points, right_points;
  for (const auto& pair : points) {
    left_points.push_back(pair.left);
    right_points.push_back(pair.right);
  }

  // 归一化点（提高数值稳定性）
  std::vector<cv::Point2f> left_normalized, right_normalized;
  cv::Mat T_left, T_right;
  normalizePoints(left_points, left_normalized, T_left);
  normalizePoints(right_points, right_normalized, T_right);

  // 构建系数矩阵 A (n x 9)
  Eigen::MatrixXd A(points.size(), 9);

  for (size_t i = 0; i < points.size(); ++i) {
    const auto& pl = left_normalized[i];
    const auto& pr = right_normalized[i];

    A(i, 0) = pr.x * pl.x;  // x' * x
    A(i, 1) = pr.x * pl.y;  // x' * y
    A(i, 2) = pr.x;         // x'
    A(i, 3) = pr.y * pl.x;  // y' * x
    A(i, 4) = pr.y * pl.y;  // y' * y
    A(i, 5) = pr.y;         // y'
    A(i, 6) = pl.x;         // x
    A(i, 7) = pl.y;         // y
    A(i, 8) = 1.0;          // 1
  }

  // 使用 SVD 求解 A * f = 0
  Eigen::JacobiSVD<Eigen::MatrixXd> svd(
      A, Eigen::ComputeFullU | Eigen::ComputeFullV);
  Eigen::VectorXd f = svd.matrixV().col(8);  // 最后一列

  // 重构 F 矩阵 (3x3)
  Eigen::Matrix3d F_normalized;
  F_normalized << f(0), f(1), f(2), f(3), f(4), f(5), f(6), f(7), f(8);

  // 强制 F 的秩为 2
  Eigen::JacobiSVD<Eigen::Matrix3d> svd_F(
      F_normalized, Eigen::ComputeFullU | Eigen::ComputeFullV);
  Eigen::Vector3d singular_values = svd_F.singularValues();
  singular_values(2) = 0;  // 将最小奇异值设为0

  Eigen::Matrix3d F_rank2 = svd_F.matrixU() * singular_values.asDiagonal() *
                            svd_F.matrixV().transpose();

  // 反归一化: F = T_right^T * F_normalized * T_left
  // ... (转换代码省略) ...

  return F;
}
```

**步骤说明**：
1. **第 145-150 行**：分离左右图像的点
2. **第 152-156 行**：归一化点（提高数值稳定性）
3. **第 159-174 行**：构建系数矩阵 $A$
   - 每一行对应一对点的对极约束
4. **第 177-179 行**：SVD 求解 $A \mathbf{f} = 0$
   - 解是 $V$ 的最后一列
5. **第 182-183 行**：将解重构为 $3 \times 3$ 矩阵
6. **第 186-191 行**：强制秩为 2
   - 对 $F$ 进行 SVD
   - 将最小奇异值设为 0
   - 重构矩阵
7. **第 197-205 行**：反归一化 $F = T_r^T F' T_l$

---

#### 验证基础矩阵

**函数位置**: `stereo_measure.cpp` 第 218-239 行

```cpp
double validateFundamentalMatrix(const cv::Mat& F,
                                 const std::vector<PointPair>& points) {
  if (F.empty() || points.empty()) {
    return -1.0;
  }

  double total_error = 0.0;

  for (const auto& pair : points) {
    // 齐次坐标
    cv::Mat x_left = (cv::Mat_<double>(3, 1) << pair.left.x, pair.left.y, 1.0);
    cv::Mat x_right =
        (cv::Mat_<double>(3, 1) << pair.right.x, pair.right.y, 1.0);

    // 对极约束: x_right^T * F * x_left = 0
    cv::Mat result = x_right.t() * F * x_left;
    double error = std::abs(result.at<double>(0, 0));
    total_error += error;
  }

  return total_error / static_cast<double>(points.size());
}
```

**验证公式**：

$$
e_i = |\mathbf{x}_r^T F \mathbf{x}_l|
$$

平均误差：

$$
e_{avg} = \frac{1}{n} \sum_{i=1}^{n} e_i
$$

理想情况下，$e_i = 0$（完美满足对极约束）。

---

### 2.6 对极线绘制

**函数位置**: `stereo_measure.cpp` 第 302-346 行

对于右图像的点 $\mathbf{x}_r$，其在左图像中的对极线为：

$$
\mathbf{l}_l = F \mathbf{x}_r
$$

对极线方程：$ax + by + c = 0$，其中 $[a, b, c]^T = \mathbf{l}_l$

```cpp
// 右图像点的齐次坐标
cv::Mat x_right =
    (cv::Mat_<double>(3, 1) << pair.right.x, pair.right.y, 1.0);

// 对极线: l = F * x_right
cv::Mat epiline_left = F * x_right;
double a = epiline_left.at<double>(0, 0);
double b = epiline_left.at<double>(1, 0);
double c = epiline_left.at<double>(2, 0);

// 绘制对极线 ax + by + c = 0
cv::Point2f pt1, pt2;
pt1.x = 0;
pt1.y = static_cast<float>(-c / b);
pt2.x = static_cast<float>(left_img.cols);
pt2.y = static_cast<float>(-(a * pt2.x + c) / b);

cv::line(left_with_epilines, pt1, pt2, color, 2);
```

**老太太理解版**：
对极线就是"搜索线"。右图的一个点，在左图上一定在某条线上（对极线）。知道基础矩阵后，就能画出这条线。如果对应点真的在这条线上，说明基础矩阵算得准。

---

## 3. 总结

### 3.1 单目视觉测量

| 组件 | 数学模型 | 代码位置 |
|------|----------|----------|
| 针孔模型 | $s[u,v,1]^T = K[X,Y,Z]^T$ | `single_measure.h` |
| 逆投影 | $X = \frac{(u-c_x)Z}{f_x}$ | `imageToWorld()` |
| 距离计算 | $d = \sqrt{\Delta x^2 + \Delta y^2 + \Delta z^2}$ | `distance()` |
| 测量流程 | 图像坐标 → 3D 坐标 → 距离/面积 | `runSingleMeasure()` |

**限制**：需要已知深度信息。

---

### 3.2 双目视觉测量（八点法）

| 组件 | 数学模型 | 代码位置 |
|------|----------|----------|
| 对极约束 | $\mathbf{x}_r^T F \mathbf{x}_l = 0$ | 第 233 行 |
| 点归一化 | $\mathbf{x}' = T\mathbf{x}$ | 第 102-136 行 |
| SVD 求解 | $A\mathbf{f} = 0 \Rightarrow \mathbf{f} = V_{:,9}$ | 第 177-179 行 |
| 秩 2 约束 | $\Sigma' = \text{diag}(\sigma_1, \sigma_2, 0)$ | 第 186-191 行 |
| 反归一化 | $F = T_r^T F' T_l$ | 第 197-205 行 |
| 对极线 | $\mathbf{l} = F\mathbf{x}$ | 第 316 行 |
| 验证 | $e = |\mathbf{x}_r^T F \mathbf{x}_l|$ | 第 218-239 行 |

**优势**：不需要已知深度，通过两个视图恢复 3D 结构。

---

### 3.3 应用场景

- **单目测量**：已知深度的场景（如激光测距辅助、结构化光）
- **双目测量**：未知深度的场景，通过视差恢复深度

---

## 4. 关键数学公式

### 相机投影

$$
\begin{bmatrix} u \\ v \\ 1 \end{bmatrix} = K \begin{bmatrix} X/Z \\ Y/Z \\ 1 \end{bmatrix}
$$

### 对极约束

$$
\mathbf{x}_r^T F \mathbf{x}_l = 0
$$

### SVD 分解

$$
A = U \Sigma V^T
$$

### 归一化

$$
T = \begin{bmatrix}
s & 0 & -s\bar{x} \\
0 & s & -s\bar{y} \\
0 & 0 & 1
\end{bmatrix}, \quad s = \frac{\sqrt{2}}{d_{avg}}
$$

---

这两个模块展示了从 2D 到 3D 的视觉测量技术，是计算机视觉中深度估计和三维重建的基础。
