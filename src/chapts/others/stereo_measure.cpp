#include "stereo_measure.h"

#include <Eigen/Dense>
#include <iostream>

namespace StereoMeasure {

// 全局变量用于鼠标回调
struct MouseCallbackData {
  cv::Mat display_img;
  std::vector<cv::Point2f>* points;
  int max_points;
  std::string window_name;
};

// 鼠标回调函数
static void mouseCallback(int event, int x, int y, int flags, void* userdata) {
  auto* data = static_cast<MouseCallbackData*>(userdata);

  if (event == cv::EVENT_LBUTTONDOWN) {
    if (data->points->size() < static_cast<size_t>(data->max_points)) {
      data->points->emplace_back(static_cast<float>(x), static_cast<float>(y));

      // 在图像上绘制点
      cv::circle(data->display_img, cv::Point(x, y), 5, cv::Scalar(0, 0, 255),
                 -1);
      cv::putText(data->display_img, std::to_string(data->points->size()),
                  cv::Point(x + 10, y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.5,
                  cv::Scalar(0, 255, 0), 2);
      cv::imshow(data->window_name, data->display_img);

      std::cout << "点 " << data->points->size() << ": (" << x << ", " << y
                << ")" << std::endl;
    }
  }
}

std::vector<PointPair> selectCorrespondingPoints(const cv::Mat& left_img,
                                                 const cv::Mat& right_img) {
  std::vector<PointPair> pairs;
  std::vector<cv::Point2f> left_points;
  std::vector<cv::Point2f> right_points;

  // 创建显示窗口
  const std::string left_window = "Left Image - Click 8 points";
  const std::string right_window = "Right Image - Click corresponding 8 points";

  cv::namedWindow(left_window, cv::WINDOW_NORMAL);
  cv::namedWindow(right_window, cv::WINDOW_NORMAL);

  // 调整窗口大小以适应屏幕
  cv::resizeWindow(left_window, 800, 600);
  cv::resizeWindow(right_window, 800, 600);

  // 准备左图像选择
  cv::Mat left_display = left_img.clone();
  MouseCallbackData left_data{left_display, &left_points, 8, left_window};

  std::cout << "\n=== 选择左图像的8个点 ===" << std::endl;
  std::cout << "请在左图像窗口中用鼠标左键点击8个特征点" << std::endl;
  std::cout << "点击完成后按任意键继续..." << std::endl;

  cv::imshow(left_window, left_display);
  cv::setMouseCallback(left_window, mouseCallback, &left_data);

  // 等待用户选择8个点
  while (left_points.size() < 8) {
    int key = cv::waitKey(100);
    if (key >= 0 && left_points.size() >= 8) break;
  }
  cv::waitKey(0);

  // 准备右图像选择
  cv::Mat right_display = right_img.clone();
  MouseCallbackData right_data{right_display, &right_points, 8, right_window};

  std::cout << "\n=== 选择右图像的对应点 ===" << std::endl;
  std::cout << "请在右图像窗口中按相同顺序点击对应的8个点" << std::endl;
  std::cout << "点击完成后按任意键继续..." << std::endl;

  cv::imshow(right_window, right_display);
  cv::setMouseCallback(right_window, mouseCallback, &right_data);

  // 等待用户选择8个对应点
  while (right_points.size() < 8) {
    int key = cv::waitKey(100);
    if (key >= 0 && right_points.size() >= 8) break;
  }
  cv::waitKey(0);

  cv::destroyWindow(left_window);
  cv::destroyWindow(right_window);

  // 组合成点对
  for (size_t i = 0; i < 8; ++i) {
    pairs.push_back({left_points[i], right_points[i]});
  }

  return pairs;
}

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
  // 对极约束: x'_i^T * F * x_i = 0
  // 展开为: [x'*x, x'*y, x', y'*x, y'*y, y', x, y, 1] * f = 0
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
  // f 是最小奇异值对应的右奇异向量
  Eigen::JacobiSVD<Eigen::MatrixXd> svd(
      A, Eigen::ComputeFullU | Eigen::ComputeFullV);
  Eigen::VectorXd f = svd.matrixV().col(8);  // 最后一列

  // 重构 F 矩阵 (3x3)
  Eigen::Matrix3d F_normalized;
  F_normalized << f(0), f(1), f(2), f(3), f(4), f(5), f(6), f(7), f(8);

  // 强制 F 的秩为 2（基础矩阵的约束）
  Eigen::JacobiSVD<Eigen::Matrix3d> svd_F(
      F_normalized, Eigen::ComputeFullU | Eigen::ComputeFullV);
  Eigen::Vector3d singular_values = svd_F.singularValues();
  singular_values(2) = 0;  // 将最小奇异值设为0

  Eigen::Matrix3d F_rank2 = svd_F.matrixU() * singular_values.asDiagonal() *
                            svd_F.matrixV().transpose();

  // 反归一化: F = T_right^T * F_normalized * T_left
  Eigen::Matrix3d T_left_eigen, T_right_eigen;
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      T_left_eigen(i, j) = T_left.at<double>(i, j);
      T_right_eigen(i, j) = T_right.at<double>(i, j);
    }
  }

  Eigen::Matrix3d F_final = T_right_eigen.transpose() * F_rank2 * T_left_eigen;

  // 转换为 OpenCV Mat
  cv::Mat F(3, 3, CV_64F);
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      F.at<double>(i, j) = F_final(i, j);
    }
  }

  return F;
}

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

void runStereoMeasure(const std::string& left_path,
                      const std::string& right_path) {
  std::cout << "=== 立体视觉基础矩阵估计（八点法） ===" << std::endl;
  std::cout << "加载图像..." << std::endl;

  // 读取左右图像
  cv::Mat left_img = cv::imread(left_path);
  cv::Mat right_img = cv::imread(right_path);

  if (left_img.empty() || right_img.empty()) {
    std::cerr << "错误: 无法读取图像" << std::endl;
    std::cerr << "左图像路径: " << left_path << std::endl;
    std::cerr << "右图像路径: " << right_path << std::endl;
    return;
  }

  std::cout << "左图像尺寸: " << left_img.cols << "x" << left_img.rows
            << std::endl;
  std::cout << "右图像尺寸: " << right_img.cols << "x" << right_img.rows
            << std::endl;

  // 交互式选择对应点
  std::vector<PointPair> point_pairs =
      selectCorrespondingPoints(left_img, right_img);

  std::cout << "\n已选择 " << point_pairs.size() << " 对对应点" << std::endl;

  // 打印选中的点
  std::cout << "\n对应点对:" << std::endl;
  for (size_t i = 0; i < point_pairs.size(); ++i) {
    std::cout << "点对 " << (i + 1) << ": 左(" << point_pairs[i].left.x << ", "
              << point_pairs[i].left.y << ") -> 右(" << point_pairs[i].right.x
              << ", " << point_pairs[i].right.y << ")" << std::endl;
  }

  // 使用八点法计算基础矩阵
  std::cout << "\n计算基础矩阵..." << std::endl;
  cv::Mat F = computeFundamentalMatrix(point_pairs);

  if (F.empty()) {
    std::cerr << "错误: 基础矩阵计算失败" << std::endl;
    return;
  }

  // 打印基础矩阵
  std::cout << "\n基础矩阵 F:" << std::endl;
  for (int i = 0; i < 3; ++i) {
    std::cout << "[";
    for (int j = 0; j < 3; ++j) {
      std::cout << std::setw(12) << std::setprecision(6) << std::scientific
                << F.at<double>(i, j);
      if (j < 2) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
  }

  // 验证基础矩阵
  double avg_error = validateFundamentalMatrix(F, point_pairs);
  std::cout << "\n对极约束平均误差: " << avg_error << std::endl;

  // 可视化对极线
  std::cout << "\n绘制对极线..." << std::endl;

  cv::Mat left_with_epilines = left_img.clone();
  cv::Mat right_with_epilines = right_img.clone();

  // 对于每个右图像点，计算其在左图像中的对极线
  for (size_t i = 0; i < point_pairs.size(); ++i) {
    const auto& pair = point_pairs[i];

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

    cv::Scalar color(rand() % 256, rand() % 256, rand() % 256);
    cv::line(left_with_epilines, pt1, pt2, color, 2);
    cv::circle(left_with_epilines, pair.left, 5, color, -1);

    // 对于左图像点，计算其在右图像中的对极线
    cv::Mat x_left = (cv::Mat_<double>(3, 1) << pair.left.x, pair.left.y, 1.0);
    cv::Mat epiline_right = F.t() * x_left;
    a = epiline_right.at<double>(0, 0);
    b = epiline_right.at<double>(1, 0);
    c = epiline_right.at<double>(2, 0);

    pt1.x = 0;
    pt1.y = static_cast<float>(-c / b);
    pt2.x = static_cast<float>(right_img.cols);
    pt2.y = static_cast<float>(-(a * pt2.x + c) / b);

    cv::line(right_with_epilines, pt1, pt2, color, 2);
    cv::circle(right_with_epilines, pair.right, 5, color, -1);
  }

  // 显示结果
  cv::namedWindow("Left with Epilines", cv::WINDOW_NORMAL);
  cv::namedWindow("Right with Epilines", cv::WINDOW_NORMAL);
  cv::resizeWindow("Left with Epilines", 800, 600);
  cv::resizeWindow("Right with Epilines", 800, 600);

  cv::imshow("Left with Epilines", left_with_epilines);
  cv::imshow("Right with Epilines", right_with_epilines);

  std::cout << "\n按任意键退出..." << std::endl;
  cv::waitKey(0);
  cv::destroyAllWindows();

  std::cout << "\n完成!" << std::endl;
}

}  // namespace StereoMeasure
