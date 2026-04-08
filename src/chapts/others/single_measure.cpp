#include "single_measure.h"

#include <algorithm>
#include <numeric>
#include <print>

namespace SingleMeasure {

namespace rng = std::ranges;
namespace vw = std::views;

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

  std::println("====================================");
  std::println("Single-View Measurement");
  std::println("====================================");
  std::println("Camera: fx={:.1f} fy={:.1f} cx={:.1f} cy={:.1f}", cam.fx,
               cam.fy, cam.cx, cam.cy);
  std::println("Depth: {:.1f}mm\n", depth);

  std::println("3D Coordinates:");
  for (const auto& [img, world, label] : points) {
    std::println("  {} ({:4.0f},{:4.0f}) → ({:6.1f}, {:6.1f}, {:6.1f})mm",
                 label, img.x, img.y, world.x, world.y, world.z);
  }

  std::println("\nDistances:");
  for (size_t i = 0; i < dists.size(); ++i) {
    std::println("  {} → {}: {:.1f}mm", points[i].label, points[i + 1].label,
                 dists[i]);
  }
  std::println("  {} → {}: {:.1f}mm (closing)", points.back().label,
               points.front().label, closing);

  std::println("\nSummary:");
  std::println("  Points: {}", points.size());
  std::println("  Perimeter: {:.1f}mm ({:.1f}cm)", perimeter, perimeter / 10);

  if (points.size() == 4) {
    const double area = dists[0] * dists[1];
    std::println("  Area: {:.1f}mm² ({:.2f}cm²)", area, area / 100);
  }

  std::println("====================================\n");
}

}  // namespace SingleMeasure
