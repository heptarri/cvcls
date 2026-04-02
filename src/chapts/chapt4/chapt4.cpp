#include "chapt4.h"

#include <algorithm>
#include <cmath>
#include <iostream>

// 图像平移变换，超出边界处填充黑色
cv::Mat posTrans(const cv::Mat& src, int tx, int ty) {
  cv::Mat dst(src.rows, src.cols, src.type(), cv::Scalar(0, 0, 0));

  for (int y = 0; y < src.rows; ++y) {
    for (int x = 0; x < src.cols; ++x) {
      int targetX = x + tx;
      int targetY = y + ty;
      if (targetX >= 0 && targetX < src.cols && targetY >= 0 &&
          targetY < src.rows) {
        dst.at<cv::Vec3b>(targetY, targetX) = src.at<cv::Vec3b>(y, x);
      }
    }
  }
  return dst;
}

// 对图像进行翻转：0=垂直，1=水平，-1=水平+垂直
cv::Mat imgFlip(const cv::Mat& src, int mode) {
  cv::Mat dst(src.rows, src.cols, src.type());

  for (int y = 0; y < src.rows; ++y) {
    for (int x = 0; x < src.cols; ++x) {
      int targetX, targetY;
      if (mode == 0) {
        targetX = x;
        targetY = src.rows - 1 - y;
      } else if (mode == -1) {
        targetX = src.cols - 1 - x;
        targetY = src.rows - 1 - y;
      } else {  // mode == 1
        targetX = src.cols - 1 - x;
        targetY = y;
      }
      dst.at<cv::Vec3b>(targetY, targetX) = src.at<cv::Vec3b>(y, x);
    }
  }
  return dst;
}

// 转置图像，行列互换
cv::Mat imgTranspose(const cv::Mat& src) {
  cv::Mat dst(src.cols, src.rows, src.type());

  for (int y = 0; y < src.rows; ++y) {
    for (int x = 0; x < src.cols; ++x) {
      dst.at<cv::Vec3b>(x, y) = src.at<cv::Vec3b>(y, x);
    }
  }
  return dst;
}

// 最近邻插值缩放
cv::Mat imgResizeNearest(const cv::Mat& src, int w, int h) {
  cv::Mat dst(h, w, src.type());

  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      int srcX =
          static_cast<int>(std::floor((x / static_cast<float>(w)) * src.cols));
      int srcY =
          static_cast<int>(std::floor((y / static_cast<float>(h)) * src.rows));
      srcX = std::min(srcX, src.cols - 1);
      srcY = std::min(srcY, src.rows - 1);
      dst.at<cv::Vec3b>(y, x) = src.at<cv::Vec3b>(srcY, srcX);
    }
  }
  return dst;
}

// 双线性插值缩放
cv::Mat imgResizeBilinear(const cv::Mat& src, int w, int h) {
  cv::Mat dst(h, w, src.type());
  float sw = static_cast<float>(src.cols);
  float sh = static_cast<float>(src.rows);
  int maxX = src.cols - 1;
  int maxY = src.rows - 1;

  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      float fx = (x / static_cast<float>(w)) * sw;
      float fy = (y / static_cast<float>(h)) * sh;
      int x0 = static_cast<int>(std::floor(fx));
      int y0 = static_cast<int>(std::floor(fy));
      int x1 = std::min(x0 + 1, maxX);
      int y1 = std::min(y0 + 1, maxY);
      x0 = std::max(x0, 0);
      y0 = std::max(y0, 0);
      float dx = fx - x0;
      float dy = fy - y0;

      cv::Vec3b p00 = src.at<cv::Vec3b>(y0, x0);
      cv::Vec3b p10 = src.at<cv::Vec3b>(y0, x1);
      cv::Vec3b p01 = src.at<cv::Vec3b>(y1, x0);
      cv::Vec3b p11 = src.at<cv::Vec3b>(y1, x1);

      cv::Vec3b pixel;
      for (int c = 0; c < 3; ++c) {
        float v00 = p00[c];
        float v10 = p10[c];
        float v01 = p01[c];
        float v11 = p11[c];
        float v0 = v00 + (v10 - v00) * dx;
        float v1 = v01 + (v11 - v01) * dx;
        float v = v0 + (v1 - v0) * dy;
        pixel[c] = static_cast<uchar>(std::clamp(std::round(v), 0.0f, 255.0f));
      }
      dst.at<cv::Vec3b>(y, x) = pixel;
    }
  }
  return dst;
}

// Catmull-Rom 权重函数
float cubicWeight(float t) {
  const float a = -0.5f;
  t = std::abs(t);
  if (t <= 1.0f) {
    return (a + 2.0f) * t * t * t - (a + 3.0f) * t * t + 1.0f;
  } else if (t < 2.0f) {
    return a * t * t * t - 5.0f * a * t * t + 8.0f * a * t - 4.0f * a;
  } else {
    return 0.0f;
  }
}

// 三次插值缩放（Catmull-Rom）
cv::Mat imgResizeCubic(const cv::Mat& src, int w, int h) {
  cv::Mat dst(h, w, src.type());
  float sw = static_cast<float>(src.cols);
  float sh = static_cast<float>(src.rows);
  int maxX = src.cols - 1;
  int maxY = src.rows - 1;

  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      float fx = (x / static_cast<float>(w)) * sw;
      float fy = (y / static_cast<float>(h)) * sh;
      int ix = static_cast<int>(std::floor(fx));
      int iy = static_cast<int>(std::floor(fy));
      float dx = fx - ix;
      float dy = fy - iy;

      float rgb[3] = {0.0f, 0.0f, 0.0f};
      for (int j = -1; j <= 2; ++j) {
        float wy = cubicWeight(dy - j);
        int sy = std::clamp(iy + j, 0, maxY);
        for (int i = -1; i <= 2; ++i) {
          float wx = cubicWeight(dx - i);
          int sx = std::clamp(ix + i, 0, maxX);
          cv::Vec3b pixel = src.at<cv::Vec3b>(sy, sx);
          for (int c = 0; c < 3; ++c) {
            rgb[c] += pixel[c] * wx * wy;
          }
        }
      }

      cv::Vec3b result;
      for (int c = 0; c < 3; ++c) {
        result[c] =
            static_cast<uchar>(std::clamp(std::round(rgb[c]), 0.0f, 255.0f));
      }
      dst.at<cv::Vec3b>(y, x) = result;
    }
  }
  return dst;
}

// 使用最近邻插值缩放图像
cv::Mat imgResize(const cv::Mat& src, int w, int h) {
  return imgResizeNearest(src, w, h);
}

// 最近邻插值旋转
cv::Mat imgRotateNearest(const cv::Mat& src, float angleDeg) {
  float angle = angleDeg * CV_PI / 180.0f;
  float cosA = std::cos(angle);
  float sinA = std::sin(angle);
  int width = src.cols;
  int height = src.rows;
  float cx = width / 2.0f;
  float cy = height / 2.0f;

  cv::Mat dst(src.rows, src.cols, src.type(), cv::Scalar(0, 0, 0));

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      float xf = x - cx;
      float yf = y - cy;
      float srcX = cosA * xf + sinA * yf + cx;
      float srcY = -sinA * xf + cosA * yf + cy;
      int sx = static_cast<int>(std::round(srcX));
      int sy = static_cast<int>(std::round(srcY));
      if (sx >= 0 && sx < width && sy >= 0 && sy < height) {
        dst.at<cv::Vec3b>(y, x) = src.at<cv::Vec3b>(sy, sx);
      }
    }
  }
  return dst;
}

// 双线性插值旋转
cv::Mat imgRotateBilinear(const cv::Mat& src, float angleDeg) {
  float angle = angleDeg * CV_PI / 180.0f;
  float cosA = std::cos(angle);
  float sinA = std::sin(angle);
  int width = src.cols;
  int height = src.rows;
  float cx = width / 2.0f;
  float cy = height / 2.0f;
  int maxX = src.cols - 1;
  int maxY = src.rows - 1;

  cv::Mat dst(src.rows, src.cols, src.type(), cv::Scalar(0, 0, 0));

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      float xf = x - cx;
      float yf = y - cy;
      float srcX = cosA * xf + sinA * yf + cx;
      float srcY = -sinA * xf + cosA * yf + cy;

      if (srcX >= 0.0f && srcX < width && srcY >= 0.0f && srcY < height) {
        int x0 = static_cast<int>(std::floor(srcX));
        int y0 = static_cast<int>(std::floor(srcY));
        int x1 = std::min(x0 + 1, maxX);
        int y1 = std::min(y0 + 1, maxY);
        float dx = srcX - x0;
        float dy = srcY - y0;

        cv::Vec3b p00 = src.at<cv::Vec3b>(y0, x0);
        cv::Vec3b p10 = src.at<cv::Vec3b>(y0, x1);
        cv::Vec3b p01 = src.at<cv::Vec3b>(y1, x0);
        cv::Vec3b p11 = src.at<cv::Vec3b>(y1, x1);

        cv::Vec3b pixel;
        for (int c = 0; c < 3; ++c) {
          float v00 = p00[c];
          float v10 = p10[c];
          float v01 = p01[c];
          float v11 = p11[c];
          float v0 = v00 + (v10 - v00) * dx;
          float v1 = v01 + (v11 - v01) * dx;
          float v = v0 + (v1 - v0) * dy;
          pixel[c] =
              static_cast<uchar>(std::clamp(std::round(v), 0.0f, 255.0f));
        }
        dst.at<cv::Vec3b>(y, x) = pixel;
      }
    }
  }
  return dst;
}

// 三次插值旋转（Catmull-Rom）
cv::Mat imgRotateCubic(const cv::Mat& src, float angleDeg) {
  float angle = angleDeg * CV_PI / 180.0f;
  float cosA = std::cos(angle);
  float sinA = std::sin(angle);
  int width = src.cols;
  int height = src.rows;
  float cx = width / 2.0f;
  float cy = height / 2.0f;
  int maxX = src.cols - 1;
  int maxY = src.rows - 1;

  cv::Mat dst(src.rows, src.cols, src.type(), cv::Scalar(0, 0, 0));

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      float xf = x - cx;
      float yf = y - cy;
      float srcX = cosA * xf + sinA * yf + cx;
      float srcY = -sinA * xf + cosA * yf + cy;

      if (srcX >= 0.0f && srcX < width && srcY >= 0.0f && srcY < height) {
        int ix = static_cast<int>(std::floor(srcX));
        int iy = static_cast<int>(std::floor(srcY));
        float dx = srcX - ix;
        float dy = srcY - iy;

        float rgb[3] = {0.0f, 0.0f, 0.0f};
        for (int j = -1; j <= 2; ++j) {
          float wy = cubicWeight(dy - j);
          int sy = std::clamp(iy + j, 0, maxY);
          for (int i = -1; i <= 2; ++i) {
            float wx = cubicWeight(dx - i);
            int sx = std::clamp(ix + i, 0, maxX);
            cv::Vec3b pixel = src.at<cv::Vec3b>(sy, sx);
            for (int c = 0; c < 3; ++c) {
              rgb[c] += pixel[c] * wx * wy;
            }
          }
        }

        cv::Vec3b result;
        for (int c = 0; c < 3; ++c) {
          result[c] =
              static_cast<uchar>(std::clamp(std::round(rgb[c]), 0.0f, 255.0f));
        }
        dst.at<cv::Vec3b>(y, x) = result;
      }
    }
  }
  return dst;
}

// 绕图像中心旋转指定角度，超出边界部分填充黑色
cv::Mat imgRotate(const cv::Mat& src, float angleDeg) {
  return imgRotateCubic(src, angleDeg);
}

// 运行 Chapter 4 的图像处理流程，并保存输出文件
void runChapt4(const cv::Mat& img, const std::string& outputRoot) {
  cv::Mat translated = posTrans(img, 100, 100);
  cv::imwrite(outputRoot + "/chapt4_translated.png", translated);
  std::cout << "[TASK] translate task completed!" << std::endl;

  cv::Mat flipped = imgFlip(img, 1);
  cv::imwrite(outputRoot + "/chapt4_flipped.png", flipped);
  std::cout << "[TASK] flip task completed!" << std::endl;

  cv::Mat transposed = imgTranspose(img);
  cv::imwrite(outputRoot + "/chapt4_transposed.png", transposed);
  std::cout << "[TASK] transpose task completed!" << std::endl;

  cv::Mat resized = imgResize(img, 100, 100);
  cv::imwrite(outputRoot + "/chapt4_resized.png", resized);
  std::cout << "[TASK] resize task completed!" << std::endl;

  cv::Mat rotated = imgRotate(img, 30.0f);
  cv::imwrite(outputRoot + "/chapt4_rotated.png", rotated);
  std::cout << "[TASK] rotate task completed!" << std::endl;
}
