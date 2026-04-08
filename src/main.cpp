#include <array>
#include <filesystem>
#include <functional>
#include <opencv2/opencv.hpp>
#include <print>
#include <string_view>
#include <unordered_map>

#include "chapts/chapt2/chapt2.h"
#include "chapts/chapt3/chapt3.h"
#include "chapts/chapt4/chapt4.h"
#include "chapts/chapt5/chapt5.h"
#include "chapts/others/single_measure.h"
#include "chapts/others/stereo_measure.h"
#include "common/common.h"

namespace fs = std::filesystem;

void printUsage(std::string_view programName) {
  std::println("Usage: {} [chapter_number]", programName);
  std::println(
      "  chapter_number: 2, 3, 4, 5, measure, stereo, stereo-json, or 'all' "
      "(default: all)");
  std::println("");
  std::println("Examples:");
  std::println("  {}             # Run all chapters", programName);
  std::println("  {} 2           # Run only Chapter 2", programName);
  std::println("  {} measure     # Run single-view measurement", programName);
  std::println("  {} stereo      # Run stereo measurement (8-point algorithm)",
               programName);
  std::println(
      "  {} stereo-json # Run stereo using points.json (bypass GUI issues)",
      programName);
  std::println("  {} all         # Run all chapters", programName);
}

int main(int argc, char* argv[]) {
  // 解析命令行参数
  const std::string_view chapterArg = (argc > 1) ? argv[1] : "all";

  if (chapterArg == "-h" || chapterArg == "--help") {
    printUsage(argv[0]);
    return 0;
  }

  // 设置路径
  const fs::path imagePath = "/home/hepcl/workspace/cvcls/static/image.png";
  const fs::path outputRoot = "/home/hepcl/workspace/cvcls/output";

  // 创建输出目录（使用 std::filesystem）
  std::error_code ec;
  fs::create_directories(outputRoot, ec);
  if (ec) {
    std::println(stderr, "[Error] Failed to create output directory: {}",
                 ec.message());
    return 1;
  }

  // 读取图像
  const cv::Mat dynImage = cv::imread(imagePath.string(), cv::IMREAD_COLOR);
  if (dynImage.empty()) {
    std::println(stderr, "[Error] Cannot read the image from {}",
                 imagePath.string());
    return 1;
  }

  // 转换为 RGB
  cv::Mat rgbImage;
  cv::cvtColor(dynImage, rgbImage, cv::COLOR_BGR2RGB);

  if (!checkValid(rgbImage)) {
    std::println(stderr, "[Error] Invalid image.");
    return 1;
  }

  // 使用 lambda 和 unordered_map 来管理章节调度
  const auto outputRootStr = outputRoot.string();

  const std::unordered_map<std::string_view, std::function<void()>> chapters = {
      {"2",
       [&rgbImage, &outputRootStr]() {
         std::println("\n========== Running Chapter 2 ==========");
         runChapt2(rgbImage, outputRootStr);
         std::println("[CHPT] Chapter 2 Generated!");
       }},
      {"3",
       [&rgbImage, &outputRootStr]() {
         std::println("\n========== Running Chapter 3 ==========");
         runChapt3(rgbImage, outputRootStr);
         std::println("[CHPT] Chapter 3 Generated!");
       }},
      {"4",
       [&rgbImage, &outputRootStr]() {
         std::println("\n========== Running Chapter 4 ==========");
         runChapt4(rgbImage, outputRootStr);
         std::println("[CHPT] Chapter 4 Generated!");
       }},
      {"5",
       [&rgbImage, &outputRootStr]() {
         std::println("\n========== Running Chapter 5 ==========");
         const cv::Mat grayImage = toGrayscale(rgbImage);
         runChapt5(grayImage, outputRootStr);
         std::println("[CHPT] Chapter 5 Generated!");
       }},
      {"measure",
       [&rgbImage]() {
         std::println(
             "\n========== Running Single-View Measurement ==========");
         SingleMeasure::runSingleMeasure(rgbImage);
         std::println("[CHPT] Measurement Complete!");
       }},
      {"stereo",
       []() {
         std::println(
             "\n========== Running Stereo Measurement (8-Point Algorithm) "
             "==========");
         const std::string leftPath =
             "/home/hepcl/workspace/cvcls/static/left.png";
         const std::string rightPath =
             "/home/hepcl/workspace/cvcls/static/right.png";
         StereoMeasure::runStereoMeasure(leftPath, rightPath);
         std::println("[CHPT] Stereo Measurement Complete!");
       }},
  };

  // 运行相应的章节
  const bool runAll = (chapterArg == "all");

  if (runAll) {
    // 运行所有章节（按顺序）
    const std::array<std::string_view, 6> order = {"2", "3",       "4",
                                                   "5", "measure", "stereo"};
    for (const auto& chapter : order) {
      if (auto it = chapters.find(chapter); it != chapters.end()) {
        it->second();
      }
    }
  } else {
    // 运行指定章节
    if (const auto it = chapters.find(chapterArg); it != chapters.end()) {
      it->second();
    } else {
      std::println(stderr, "[Error] Invalid chapter number: {}", chapterArg);
      printUsage(argv[0]);
      return 1;
    }
  }

  std::println("\nDone. Files saved to {}.", outputRoot.string());
  return 0;
}
