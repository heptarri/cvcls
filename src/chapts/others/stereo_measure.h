#pragma once

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

namespace StereoMeasure {

// 对应点对结构
struct PointPair {
  cv::Point2f left;   // 左图像点
  cv::Point2f right;  // 右图像点
};

// 八点法计算基础矩阵F
// points: 至少8对对应点
// 返回: 3x3 基础矩阵
cv::Mat computeFundamentalMatrix(const std::vector<PointPair>& points);

// 归一化点坐标（用于提高数值稳定性）
void normalizePoints(const std::vector<cv::Point2f>& points,
                     std::vector<cv::Point2f>& normalized_points,
                     cv::Mat& transform);

// 使用UI交互式选择对应点
// left_img: 左图像
// right_img: 右图像
// 返回: 选中的对应点对（至少8对）
std::vector<PointPair> selectCorrespondingPoints(const cv::Mat& left_img,
                                                 const cv::Mat& right_img);

// 验证基础矩阵（计算对极约束误差）
double validateFundamentalMatrix(const cv::Mat& F,
                                 const std::vector<PointPair>& points);

// 主运行函数
void runStereoMeasure(const std::string& left_path,
                      const std::string& right_path);

}  // namespace StereoMeasure
