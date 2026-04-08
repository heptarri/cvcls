#include "chapt5.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <print>
#include <ranges>

namespace views = std::views;

// Kernel 类型别名模板，更清晰的类型表达
template <int H, int W>
using Kernel = std::array<std::array<float, W>, H>;

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

// 均值平滑
cv::Mat imgMean(const cv::Mat& src) {
  constexpr std::array<std::array<float, 3>, 3> kernel = {
      {{1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}}};

  return imgFilter<3, 3>(src, kernel);
}

// 高斯平滑
cv::Mat imgGaussian(const cv::Mat& src) {
  constexpr std::array<std::array<float, 3>, 3> kernel = {
      {{1.0f, 2.0f, 1.0f}, {2.0f, 4.0f, 2.0f}, {1.0f, 2.0f, 1.0f}}};

  return imgFilter<3, 3>(src, kernel);
}

// Robert 轮廓提取
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

// Sobel 轮廓提取
cv::Mat imgSobel(const cv::Mat& src) {
  cv::Mat dst(src.rows, src.cols, CV_8UC1);

  constexpr std::array<std::array<float, 3>, 3> gx = {
      {{-1.0f, 0.0f, 1.0f}, {-2.0f, 0.0f, 2.0f}, {-1.0f, 0.0f, 1.0f}}};
  constexpr std::array<std::array<float, 3>, 3> gy = {
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

// 拉普拉斯算子轮廓提取
cv::Mat imgLaplacian(const cv::Mat& src) {
  cv::Mat dst(src.rows, src.cols, CV_8UC1);

  constexpr std::array<std::array<float, 3>, 3> kernel = {
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

// 中值滤波（3x3）
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

// 运行 Chapter 5 的图像处理流程，并保存输出文件
void runChapt5(const cv::Mat& src, std::string_view outputRoot) {
  const std::string outDir(outputRoot);
  cv::Mat meant = imgMean(src);
  cv::imwrite(outDir + "/chapt5_meant.png", meant);
  std::cout << "[TASK] mean img task completed!" << std::endl;

  cv::Mat gaussian = imgGaussian(src);
  cv::imwrite(outDir + "/chapt5_gauss.png", gaussian);
  std::cout << "[TASK] gaussian img task completed!" << std::endl;

  cv::Mat median = imgMedian(src);
  cv::imwrite(outDir + "/chapt5_median.png", median);
  std::cout << "[TASK] median img task completed!" << std::endl;

  cv::Mat robert = imgRobert(src);
  cv::imwrite(outDir + "/chapt5_robert.png", robert);
  std::cout << "[TASK] robert img task completed!" << std::endl;

  cv::Mat sobel = imgSobel(src);
  cv::imwrite(outDir + "/chapt5_sobel.png", sobel);
  std::cout << "[TASK] sobel edge img task completed!" << std::endl;

  cv::Mat laplacian = imgLaplacian(src);
  cv::imwrite(outDir + "/chapt5_laplacian.png", laplacian);
  std::cout << "[TASK] laplacian edge img task completed!" << std::endl;
}
