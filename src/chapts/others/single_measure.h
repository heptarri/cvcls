#pragma once

#include <concepts>
#include <numbers>
#include <opencv2/opencv.hpp>
#include <ranges>
#include <string_view>
#include <vector>

namespace SingleMeasure {

// 相机内参
struct CameraIntrinsics {
  double fx{800.0}, fy{800.0};
  double cx{320.0}, cy{240.0};
};

template <typename T>
concept Point3D = requires(T p) {
  { p.x } -> std::convertible_to<double>;
  { p.y } -> std::convertible_to<double>;
  { p.z } -> std::convertible_to<double>;
};

// 测量点
struct MeasurePoint {
  cv::Point2f image;
  cv::Point3f world;
  std::string label;
};

// 图像点转世界坐标
[[nodiscard]] inline cv::Point3f imageToWorld(
    cv::Point2f img_pt, double depth, const CameraIntrinsics& cam) noexcept {
  return {static_cast<float>((img_pt.x - cam.cx) * depth / cam.fx),
          static_cast<float>((img_pt.y - cam.cy) * depth / cam.fy),
          static_cast<float>(depth)};
}

// 3D距离计算
[[nodiscard]] inline double distance(Point3D auto p1,
                                     Point3D auto p2) noexcept {
  constexpr auto sq = [](auto x) constexpr { return x * x; };
  return std::sqrt(sq(p2.x - p1.x) + sq(p2.y - p1.y) + sq(p2.z - p1.z));
}

void runSingleMeasure(const cv::Mat& img);

}  // namespace SingleMeasure
