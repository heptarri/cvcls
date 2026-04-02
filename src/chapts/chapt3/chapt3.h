#ifndef CHAPT3_H
#define CHAPT3_H

#include <array>
#include <opencv2/opencv.hpp>
#include <string_view>

// 计算灰度直方图，并返回 256 个灰度级的频次
[[nodiscard]] std::array<uint32_t, 256> calculateHistogram(const cv::Mat& gray);

// 将直方图绘制为图像，白色柱状图，黑色背景
[[nodiscard]] cv::Mat drawHistogram(const std::array<uint32_t, 256>& hist);

// 线性灰度变换：y = alpha * x + beta
[[nodiscard]] cv::Mat genGrayLinTrans(const cv::Mat& src, float alpha, float beta);

// 对数灰度变换：y = log(1 + r) * (255 / log(256))
[[nodiscard]] cv::Mat genGrayLogTrans(const cv::Mat& src);

// Gamma 变换：s = 255 * (r / 255)^gamma
[[nodiscard]] cv::Mat genGammaTrans(const cv::Mat& src, float gamma);

// 手动计算 Otsu 阈值
[[nodiscard]] uint8_t otsuThreshold(const cv::Mat& gray);

// 二值化转换。typeFlag：0=正向二值，1=反向二值，2=Otsu 自动阈值
[[nodiscard]] cv::Mat genThreshold(const cv::Mat& src, uint8_t thresh, uint8_t typeFlag);

// 分段线性灰度映射
[[nodiscard]] cv::Mat genPiecewiseLin(const cv::Mat& src, uint8_t r1, uint8_t s1, 
                                      uint8_t r2, uint8_t s2);

// 直方图均衡化
[[nodiscard]] cv::Mat genEqualizeHist(const cv::Mat& src);

// 运行 Chapter 3 的图像处理流程，并保存输出文件
void runChapt3(const cv::Mat& img, std::string_view outputRoot);

#endif  // CHAPT3_H
