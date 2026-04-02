#include "chapt2.h"

#include <iostream>

#include "common.h"

// 运行 Chapter 2 的图像处理流程，并保存输出文件
void runChapt2(const cv::Mat& img, const std::string& outputRoot) {
  getInfo(img);

  auto pixel = getPixelValue(img, 297, 260);
  if (pixel) {
    std::cout << "Pixel value at (297,260): [" << static_cast<int>((*pixel)[0])
              << ", " << static_cast<int>((*pixel)[1]) << ", "
              << static_cast<int>((*pixel)[2]) << "]" << std::endl;
  }

  // 修改像素
  cv::Mat modified = img.clone();
  int maxY = std::min(100, modified.rows);
  int maxX = std::min(100, modified.cols);
  for (int y = 0; y < maxY; ++y) {
    for (int x = 0; x < maxX; ++x) {
      [[maybe_unused]] bool success = setPixelValue(modified, y, x, cv::Vec3b(255, 255, 0));  // BGR: yellow
    }
  }
  cv::imwrite(outputRoot + "/chapt2_set_pixel.png", modified);
  std::cout << "[TASK] Set pixel task completed!" << std::endl;

  // 缩放
  cv::Mat resized = resizeImage(img, 320, 240);
  cv::imwrite(outputRoot + "/chapt2_resized.png", resized);
  std::cout << "[TASK] Resize task completed!" << std::endl;

  // 创建空白图像
  cv::Mat blank =
      createBlankImage(320, 240, cv::Vec3b(255, 0, 0));  // BGR: blue
  cv::imwrite(outputRoot + "/chapt2_blank.png", blank);
  std::cout << "[TASK] Create blank task completed!" << std::endl;

  // 转换为灰度图
  std::cout << "Is grayscale: " << (isGrayscale(img) ? "true" : "false")
            << std::endl;
  cv::Mat gray = toGrayscale(img);
  cv::imwrite(outputRoot + "/chapt2_gray.png", gray);
  std::cout << "[TASK] Change to gray task completed!" << std::endl;

  std::cout << "Is binary: " << (isBinary(gray) ? "true" : "false")
            << std::endl;
}
