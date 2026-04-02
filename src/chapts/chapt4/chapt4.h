#ifndef CHAPT4_H
#define CHAPT4_H

#include <opencv2/opencv.hpp>
#include <string>

// 图像平移变换，超出边界处填充黑色
cv::Mat posTrans(const cv::Mat& src, int tx, int ty);

// 对图像进行翻转：0=垂直，1=水平，-1=水平+垂直
cv::Mat imgFlip(const cv::Mat& src, int mode);

// 转置图像，行列互换
cv::Mat imgTranspose(const cv::Mat& src);

// 使用最近邻插值缩放图像
cv::Mat imgResize(const cv::Mat& src, int w, int h);

// 最近邻插值缩放
cv::Mat imgResizeNearest(const cv::Mat& src, int w, int h);

// 双线性插值缩放
cv::Mat imgResizeBilinear(const cv::Mat& src, int w, int h);

// 三次插值缩放（Catmull-Rom）
cv::Mat imgResizeCubic(const cv::Mat& src, int w, int h);

// 绕图像中心旋转指定角度，超出边界部分填充黑色
cv::Mat imgRotate(const cv::Mat& src, float angleDeg);

// 最近邻插值旋转
cv::Mat imgRotateNearest(const cv::Mat& src, float angleDeg);

// 双线性插值旋转
cv::Mat imgRotateBilinear(const cv::Mat& src, float angleDeg);

// 三次插值旋转（Catmull-Rom）
cv::Mat imgRotateCubic(const cv::Mat& src, float angleDeg);

// 运行 Chapter 4 的图像处理流程，并保存输出文件
void runChapt4(const cv::Mat& img, const std::string& outputRoot);

#endif  // CHAPT4_H
