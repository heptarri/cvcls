#include "common.h"

#include <algorithm>
#include <print>
#include <ranges>

// 检查图像是否有效：宽度、高度大于 0
bool checkValid(const cv::Mat& img) noexcept {
  return !img.empty() && img.cols > 0 && img.rows > 0;
}

// 输出图像基本信息，包括宽、高和通道数
void getInfo(const cv::Mat& img) {
  std::println("Basic information:");
  std::println("Width: {} px", img.cols);
  std::println("Height: {} px", img.rows);
  std::println("Channels: {}", img.channels());
}

// 获取指定像素位置的值，返回一个 RGB 三元组（使用 optional 更安全）
std::optional<cv::Vec3b> getPixelValue(const cv::Mat& img, int row,
                                       int col) noexcept {
  if (row < 0 || row >= img.rows || col < 0 || col >= img.cols) [[unlikely]] {
    return std::nullopt;
  }
  return img.at<cv::Vec3b>(row, col);
}

// 在指定位置写入像素值
bool setPixelValue(cv::Mat& img, int row, int col,
                   const cv::Vec3b& value) noexcept {
  if (row < 0 || row >= img.rows || col < 0 || col >= img.cols) [[unlikely]] {
    return false;
  }
  img.at<cv::Vec3b>(row, col) = value;
  return true;
}

// 最近邻缩放：将图像缩放到指定宽高
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

// 创建指定尺寸的纯色图像
cv::Mat createBlankImage(int width, int height, const cv::Vec3b& color) {
  cv::Mat img(height, width, CV_8UC3, cv::Scalar(color[0], color[1], color[2]));
  return img;
}

// 判断彩色图像是否为灰度图（R == G == B 对所有像素成立）
bool isGrayscale(const cv::Mat& img) {
  if (img.channels() != 3) [[unlikely]] {
    return false;
  }

  for (int y : std::views::iota(0, img.rows)) {
    for (int x : std::views::iota(0, img.cols)) {
      const auto pixel = img.at<cv::Vec3b>(y, x);
      if (pixel[0] != pixel[1] || pixel[1] != pixel[2]) [[unlikely]] {
        return false;
      }
    }
  }
  return true;
}

// 判断灰度图像是否为二值图像（仅包含 0 和 255 两种灰度值）
bool isBinary(const cv::Mat& img) {
  if (img.channels() != 1) [[unlikely]] {
    return false;
  }

  for (int y : std::views::iota(0, img.rows)) {
    for (int x : std::views::iota(0, img.cols)) {
      const auto v = img.at<uchar>(y, x);
      if (v != 0 && v != 255) [[unlikely]] {
        return false;
      }
    }
  }
  return true;
}

// 将彩色图像转换为灰度图像，使用加权平均法
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
