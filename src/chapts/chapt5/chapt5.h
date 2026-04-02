#ifndef CHAPT5_H
#define CHAPT5_H

#include <opencv2/opencv.hpp>
#include <array>
#include <string>
#include <string_view>

// Kernel 类型别名模板
template<int H, int W>
using Kernel = std::array<std::array<float, W>, H>;

// 通用图像滤波器（支持 std::array）
template <int H, int W>
cv::Mat imgFilter(const cv::Mat& src, const Kernel<H, W>& mask);

// 通用图像滤波器（兼容 C 数组）
template <int H, int W>
cv::Mat imgFilter(const cv::Mat& src, const float (&mask)[H][W]);

// 均值平滑
[[nodiscard]] cv::Mat imgMean(const cv::Mat& src);

// 高斯平滑
[[nodiscard]] cv::Mat imgGaussian(const cv::Mat& src);

// Robert 轮廓提取
[[nodiscard]] cv::Mat imgRobert(const cv::Mat& src);

// Sobel 轮廓提取
[[nodiscard]] cv::Mat imgSobel(const cv::Mat& src);

// 拉普拉斯算子轮廓提取
[[nodiscard]] cv::Mat imgLaplacian(const cv::Mat& src);

// 中值滤波（3x3）
[[nodiscard]] cv::Mat imgMedian(const cv::Mat& src);

// 运行 Chapter 5 的图像处理流程，并保存输出文件
void runChapt5(const cv::Mat& src, std::string_view outputRoot);

#endif  // CHAPT5_H
