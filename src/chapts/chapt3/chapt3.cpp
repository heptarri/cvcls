#include "chapt3.h"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <print>
#include <ranges>

#include "common.h"

namespace views = std::views;

// 计算灰度直方图，并返回 256 个灰度级的频次
std::array<uint32_t, 256> calculateHistogram(const cv::Mat& gray) {
  std::array<uint32_t, 256> hist{};  // 值初始化为0

  for (int y : views::iota(0, gray.rows)) {
    for (int x : views::iota(0, gray.cols)) {
      hist[gray.at<uchar>(y, x)]++;
    }
  }
  return hist;
}

// 将直方图绘制为图像，白色柱状图，黑色背景
cv::Mat drawHistogram(const std::array<uint32_t, 256>& hist) {
  constexpr int histH = 400;
  constexpr int histW = 512;
  constexpr int binW = histW / 256;

  cv::Mat img(histH, histW, CV_8UC3, cv::Scalar(0, 0, 0));

  const auto maxCount = std::ranges::max(hist);
  if (maxCount == 0) [[unlikely]] {
    return img;
  }

  for (int i : views::iota(0, 256)) {
    const auto normalized = static_cast<uint32_t>(
        std::round((hist[i] / static_cast<float>(maxCount)) * (histH - 1.0f)));
    const int x0 = i * binW;
    const int x1 = std::min((i + 1) * binW, histW);

    for (int x : views::iota(x0, x1)) {
      for (int y : views::iota(static_cast<int>(histH - normalized), histH)) {
        img.at<cv::Vec3b>(y, x) = cv::Vec3b(255, 255, 255);
      }
    }
  }
  return img;
}

// 线性灰度变换：y = alpha * x + beta
cv::Mat genGrayLinTrans(const cv::Mat& src, float alpha, float beta) {
  const cv::Mat gray = toGrayscale(src);
  cv::Mat dst(gray.rows, gray.cols, CV_8UC1);

  for (int y : views::iota(0, gray.rows)) {
    for (int x : views::iota(0, gray.cols)) {
      const float value = gray.at<uchar>(y, x);
      const float mapped = std::round(alpha * value + beta);
      dst.at<uchar>(y, x) =
          static_cast<uchar>(std::clamp(mapped, 0.0f, 255.0f));
    }
  }
  return dst;
}

// 对数灰度变换：y = log(1 + r) * (255 / log(256))
cv::Mat genGrayLogTrans(const cv::Mat& src) {
  const cv::Mat gray = toGrayscale(src);
  float scale = 255.0f / std::log(256.0f);
  cv::Mat dst(gray.rows, gray.cols, CV_8UC1);

  for (int y : views::iota(0, gray.rows)) {
    for (int x : views::iota(0, gray.cols)) {
      const float value = gray.at<uchar>(y, x);
      const float mapped = std::log(value + 1.0f) * scale;
      dst.at<uchar>(y, x) =
          static_cast<uchar>(std::clamp(mapped, 0.0f, 255.0f));
    }
  }
  return dst;
}

// Gamma 变换：s = 255 * (r / 255)^gamma
cv::Mat genGammaTrans(const cv::Mat& src, float gamma) {
  cv::Mat dst(src.rows, src.cols, src.type());
  constexpr float inv255 = 1.0f / 255.0f;

  for (int y : views::iota(0, src.rows)) {
    for (int x : views::iota(0, src.cols)) {
      auto pixel = src.at<cv::Vec3b>(y, x);
      for (int c : views::iota(0, 3)) {
        const float normalized = pixel[c] * inv255;
        const float mapped = std::pow(normalized, gamma) * 255.0f;
        pixel[c] = static_cast<uchar>(std::clamp(mapped, 0.0f, 255.0f));
      }
      dst.at<cv::Vec3b>(y, x) = pixel;
    }
  }
  return dst;
}

/**
 * 手动计算 Otsu 阈值
 *
 * wF: 前景像素占比
 * wB: 背景像素占比
 * mF: 前景像素平均灰度
 * mB: 背景像素平均灰度
 * varBetween: 类间方差 = wF * wB * (mF - mB)^2
 * 目的是让 varBetween 最大化，从而找到最佳的分割阈值
 */
uint8_t otsuThreshold(const cv::Mat& gray) {
  const auto hist = calculateHistogram(gray);
  uint32_t total = 0;
  float sum = 0.0f;

  for (int i : views::iota(0, 256)) {
    total += hist[i];
    sum += i * hist[i];
  }

  float sumB = 0.0f;
  uint32_t wB = 0;
  float maxVar = 0.0f;
  uint8_t threshold = 0;

  for (int t : views::iota(0, 256)) {
    wB += hist[t];
    if (wB == 0) [[unlikely]]
      continue;

    const uint32_t wF = total - wB;
    if (wF == 0) [[unlikely]]
      break;

    sumB += t * hist[t];
    const float mB = sumB / wB;
    const float mF = (sum - sumB) / wF;

    const float varBetween = wB * wF * (mB - mF) * (mB - mF);
    if (varBetween > maxVar) {
      maxVar = varBetween;
      threshold = static_cast<uint8_t>(t);
    }
  }

  return threshold;
}

// 二值化转换。typeFlag：0=正向二值，1=反向二值，2=Otsu 自动阈值
cv::Mat genThreshold(const cv::Mat& src, uint8_t thresh, uint8_t typeFlag) {
  const cv::Mat gray = toGrayscale(src);
  const uint8_t threshold = (typeFlag == 2) ? otsuThreshold(gray) : thresh;
  cv::Mat dst(gray.rows, gray.cols, CV_8UC1);

  for (int y : views::iota(0, gray.rows)) {
    for (int x : views::iota(0, gray.cols)) {
      const uchar value = gray.at<uchar>(y, x);
      const uchar out = (typeFlag == 1) ? ((value > threshold) ? 0 : 255)
                                        : ((value > threshold) ? 255 : 0);
      dst.at<uchar>(y, x) = out;
    }
  }
  return dst;
}

/**
 * 分段线性灰度映射
 *
 * s={
 *  (s1/r1)*r, r<r1
 *  s1+((s2-s1)/(r2-r1))*(r-r1), r1<=r<r2
 *  s2+((255-s2)/(255-r2))*(r-r2), r>=r2
 * }
 *
 * LUT 查表
 **/
cv::Mat genPiecewiseLin(const cv::Mat& src, uint8_t r1, uint8_t s1, uint8_t r2,
                        uint8_t s2) {
  std::array<uint8_t, 256> lut;

  for (int i = 0; i < 256; ++i) {
    if (i < r1) {
      lut[i] = static_cast<uint8_t>(
          std::round((s1 / static_cast<float>(std::max(r1, (uint8_t)1))) * i));
    } else if (i < r2) {
      float slope = (s2 - s1) / static_cast<float>(r2 - r1);
      lut[i] = static_cast<uint8_t>(std::round(slope * (i - r1) + s1));
    } else {
      float slope = (255.0f - s2) / (255.0f - r2);
      lut[i] = static_cast<uint8_t>(std::round(slope * (i - r2) + s2));
    }
  }

  cv::Mat dst(src.rows, src.cols, src.type());
  for (int y = 0; y < src.rows; ++y) {
    for (int x = 0; x < src.cols; ++x) {
      cv::Vec3b pixel = src.at<cv::Vec3b>(y, x);
      dst.at<cv::Vec3b>(y, x) =
          cv::Vec3b(lut[pixel[0]], lut[pixel[1]], lut[pixel[2]]);
    }
  }
  return dst;
}

/**
 * 直方图均衡化
 *
 * cdf = \sigma hist[i]
 * s = round((cdf - cdf_min) / (total_px - cdf_min) * 255)
 */
cv::Mat genEqualizeHist(const cv::Mat& src) {
  cv::Mat gray = toGrayscale(src);
  auto hist = calculateHistogram(gray);
  uint32_t total = gray.rows * gray.cols;

  std::array<uint32_t, 256> cdf;
  uint32_t cumulative = 0;
  for (int i = 0; i < 256; ++i) {
    cumulative += hist[i];
    cdf[i] = cumulative;
  }

  uint32_t cdfMin = 0;
  for (int i = 0; i < 256; ++i) {
    if (cdf[i] > 0) {
      cdfMin = cdf[i];
      break;
    }
  }

  std::array<uint8_t, 256> lut;
  float totalF = static_cast<float>(total);
  float cdfMinF = static_cast<float>(cdfMin);

  for (int i = 0; i < 256; ++i) {
    float numerator = cdf[i] - cdfMinF;
    float mapped = std::round((numerator / (totalF - cdfMinF)) * 255.0f);
    lut[i] = static_cast<uint8_t>(std::clamp(mapped, 0.0f, 255.0f));
  }

  cv::Mat dst(gray.rows, gray.cols, CV_8UC1);
  for (int y = 0; y < gray.rows; ++y) {
    for (int x = 0; x < gray.cols; ++x) {
      uchar value = gray.at<uchar>(y, x);
      dst.at<uchar>(y, x) = lut[value];
    }
  }
  return dst;
}

// 运行 Chapter 3 的图像处理流程，并保存输出文件
void runChapt3(const cv::Mat& img, std::string_view outputRoot) {
  const std::string outDir(outputRoot);
  cv::Mat gray = toGrayscale(img);
  auto hist = calculateHistogram(gray);
  cv::Mat histImg = drawHistogram(hist);
  cv::imwrite(outDir + "/chapt3_histogram.png", histImg);
  std::cout << "[TASK] histogram task completed!" << std::endl;

  cv::Mat linear = genGrayLinTrans(img, 1.2f, 20.0f);
  cv::imwrite(outDir + "/chapt3_linear.png", linear);
  std::cout << "[TASK] linear task completed!" << std::endl;

  cv::Mat log = genGrayLogTrans(img);
  cv::imwrite(outDir + "/chapt3_log.png", log);
  std::cout << "[TASK] log task completed!" << std::endl;

  cv::Mat gamma = genGammaTrans(img, 2.5f);
  cv::imwrite(outDir + "/chapt3_gamma.png", gamma);
  std::cout << "[TASK] gamma task completed!" << std::endl;

  cv::Mat threshold = genThreshold(img, 1, 2);
  cv::imwrite(outDir + "/chapt3_threshold.png", threshold);
  std::cout << "[TASK] threshold task completed!" << std::endl;

  cv::Mat piecewise = genPiecewiseLin(img, 50, 20, 200, 240);
  cv::imwrite(outDir + "/chapt3_piecewise.png", piecewise);
  std::cout << "[TASK] piecewise task completed!" << std::endl;

  cv::Mat equalized = genEqualizeHist(img);
  cv::imwrite(outDir + "/chapt3_equalized.png", equalized);
  std::cout << "[TASK] equalize task completed!" << std::endl;
}
