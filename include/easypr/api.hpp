/*
 * This file provides some of the most commonly used application interfaces.
 */
#ifndef EASYPR_API_HPP
#define EASYPR_API_HPP

#include <string>
#include <vector>
#include "opencv2/opencv.hpp"
#include "easypr/config.h"

#include "easypr/core/plate_locate.h"
#include "easypr/core/plate_judge.h"
#include "easypr/core/chars_segment.h"
#include "easypr/core/chars_identify.h"
#include "easypr/core/plate_detect.h"
#include "easypr/core/chars_recognise.h"
#include "easypr/core/plate_recognize.h"

namespace easypr {

namespace api {

static bool plate_judge(const char* image, const char* model) {
  cv::Mat src = cv::imread(image);
  assert(!src.empty());

  int result;
  result = PlateJudge::instance()->plateJudge(src);

  return result == 0;
}

char* plate_detect(const char* image) {
  cv::Mat src = cv::imread(image);
  std::string locateInfo = "nil";
  std::vector<CPlate> plates;
  CPlateDetect pd;
  pd.setMaxPlates(1);
  int resultPD = pd.plateDetect(src, plates, 0);
  if (resultPD == 0 && plates.size() > 0) {
    CPlate &item = plates.at(0);
    cv::Mat plateMat = item.getPlateMat();

    // scale the rect to src;
    // float scale = 1.f;
    // item.setPlateScale(scale);
    // cout << "scale: " << item.getPlateScale() << endl;
    RotatedRect rect = item.getPlatePos();
    // rect = scaleBackRRect(rect, 1.f / scale);
    std::ostringstream oss;
    oss << rect.angle << ",";
    oss << rect.center.x << "," << rect.center.y << ",";
    oss << rect.size.width << "," << rect.size.height;
    locateInfo = oss.str();
    if (locateInfo.empty()) locateInfo = "nil";
  }

  char *ret = new char[locateInfo.length() + 1];
  strcpy(ret, locateInfo.c_str());

  return ret;
}

// unused
char* plate_locate(const char* image, const bool life_mode = true) {
  cv::Mat src = cv::imread(image);

  assert(!src.empty());

  CPlateLocate plate;
  plate.setDebug(1);
  plate.setLifemode(life_mode);

  //  std::vector<cv::Mat> results;
  std::vector<CPlate> results;
  plate.plateLocate(src, results);
  std::string locateInfo = "nil";
  if (results.size() > 0) {
    int max_score_index = 0;
    float max_score = 0;
    float tmp_score;
    int results_size = results.size();
    for (int i = 0; i < results_size; i++) {
      tmp_score = results[i].getPlateScore();
      cout << "scale: " << results[i].getPlateScale() << endl;
//      results[i].setPlateScale(1.f);
//      cout << "scale2: " << results[i].getPlateScale() << endl;
      RotatedRect rect = results[i].getPlatePos();
      results[i].setPlatePos(scaleBackRRect(rect, results[i].getPlateScale()));
      cout << "result: " << i << ", " << tmp_score << endl;
      if (tmp_score > max_score) {
        max_score = tmp_score;
        max_score_index = i;
      }
    }
    cout << "final: " << max_score_index << ", " << max_score << endl;
    CPlate plateVec = results[max_score_index];
    RotatedRect rr = plateVec.getPlatePos();
    std::ostringstream oss;
    oss << rr.angle << ",";
    oss << rr.center.x << "," << rr.center.y << ",";
    oss << rr.size.width << "," << rr.size.height;
    locateInfo = oss.str();
    if (locateInfo.empty()) locateInfo = "nil";
  }

  char *ret = new char[locateInfo.length() + 1];
  strcpy(ret, locateInfo.c_str());

  return ret;
}

static std::vector<std::string> plate_recognize(const char* image,
                                                const char* model_svm,
                                                const char* model_ann,
                                                const bool life_mode = true) {
  cv::Mat img = cv::imread(image);
  assert(!img.empty());

  CPlateRecognize pr;
  pr.setResultShow(false);
  pr.setLifemode(true);
  pr.setMaxPlates(1);
  pr.setDetectType(PR_DETECT_CMSER | PR_DETECT_COLOR);

  std::vector<std::string> results;
  std::vector<CPlate> plates;
  pr.plateRecognize(img, plates, 0);

  for (auto plate : plates) {
    RotatedRect rect = plate.getPlatePos();
    std::ostringstream oss;
    oss << rect.angle << ",";
    oss << rect.center.x << "," << rect.center.y << ",";
    oss << rect.size.width << "," << rect.size.height;
    results.push_back(oss.str() + "|" + plate.getPlateStr());
  }

  if (plates.size() == 1) {
    if (1) {
      std::stringstream ss(std::stringstream::in | std::stringstream::out);
      ss << "result.jpg";
      imwrite(ss.str(), plates.at(0).getPlateMat());
    }
  }

  return std::move(results);
}

static Color get_plate_color(const char* image) {
  cv::Mat img = cv::imread(image);

  assert(!img.empty());

  return getPlateType(img, true);
}
}
}

#endif  // EASYPR_API_HPP
