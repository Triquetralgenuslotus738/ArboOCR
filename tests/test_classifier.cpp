#include <doctest/doctest.h>
#include "arboOCR/classifier.hpp"

using namespace arbo::ocr;

TEST_CASE("Classifier with doAngle=false returns index -1 for every image, no model needed") {
    Classifier net; // never loadModel() — doAngle=false must short-circuit before touching the session
    std::vector<cv::Mat> images = {
        cv::Mat::zeros(48, 100, CV_8UC3),
        cv::Mat::zeros(48, 120, CV_8UC3),
    };
    auto angles = net.getAngles(images, /*doAngle=*/false, /*mostAngle=*/false);
    REQUIRE(angles.size() == 2);
    CHECK(angles[0].index == -1);
    CHECK(angles[1].index == -1);
}

TEST_CASE("Classifier default-constructs without a loaded model and doAngle=true degrades gracefully") {
    Classifier net; // unloaded model
    std::vector<cv::Mat> images = {cv::Mat::zeros(48, 100, CV_8UC3)};
    // Must not crash even if doAngle is requested but no model was loaded —
    // implementation guards on a null session.
    auto angles = net.getAngles(images, /*doAngle=*/true, /*mostAngle=*/false);
    REQUIRE(angles.size() == 1);
}
