#ifndef COMMON_H
#define COMMON_H

#include <opencv2/opencv.hpp>
#include <optional>
#include <string>
#include <string_view>

// RGB 转灰度权重常量 (ITU-R BT.601 标准)
inline constexpr float RGB_TO_GRAY_R = 0.299f;
inline constexpr float RGB_TO_GRAY_G = 0.587f;
inline constexpr float RGB_TO_GRAY_B = 0.114f;

// 检查图像是否有效：宽度、高度大于 0
[[nodiscard]] bool checkValid(const cv::Mat& img) noexcept;

// 输出图像基本信息，包括宽、高和通道数
void getInfo(const cv::Mat& img);

// 获取指定像素位置的值，返回一个 RGB 三元组（使用 optional 更安全）
[[nodiscard]] std::optional<cv::Vec3b> getPixelValue(const cv::Mat& img, int row, int col) noexcept;

// 在指定位置写入像素值
[[nodiscard]] bool setPixelValue(cv::Mat& img, int row, int col, const cv::Vec3b& value) noexcept;

// 最近邻缩放：将图像缩放到指定宽高
[[nodiscard]] cv::Mat resizeImage(const cv::Mat& src, int width, int height);

// 创建指定尺寸的纯色图像
[[nodiscard]] cv::Mat createBlankImage(int width, int height, const cv::Vec3b& color);

// 判断彩色图像是否为灰度图（R == G == B 对所有像素成立）
[[nodiscard]] bool isGrayscale(const cv::Mat& img);

// 判断灰度图像是否为二值图像（仅包含 0 和 255 两种灰度值）
[[nodiscard]] bool isBinary(const cv::Mat& img);

// 将彩色图像转换为灰度图像，使用加权平均法
[[nodiscard]] cv::Mat toGrayscale(const cv::Mat& src);

#endif  // COMMON_H
