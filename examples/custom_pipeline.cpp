// examples/custom_pipeline.cpp
//
// Engine runs every detected box through recognition unconditionally. If
// you want to skip low-confidence detections before paying for the (much
// more expensive) recognition pass, you need to drive Detector/Classifier/
// Recognizer directly — this is that pattern, with a real filter step
// Engine has no hook for.
//
// This mirrors Engine::recognize()'s own pipeline (see src/arboOCR/engine.cpp)
// almost line for line; the only difference is the score-filter step
// between detection and recognition.
//
// Usage:
//   custom_pipeline <image> [models-dir] [min-box-score]
//
#include <arboOCR/classifier.hpp>
#include <arboOCR/detector.hpp>
#include <arboOCR/ocr_utils.hpp>
#include <arboOCR/recognizer.hpp>
#include <arboOCR/types.hpp>

#include <opencv2/imgcodecs.hpp>

#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <image> [models-dir] [min-box-score]\n";
        return 1;
    }
    std::string imagePath = argv[1];
    std::string modelsDir = argc > 2 ? argv[2] : "models";
    float minBoxScore = argc > 3 ? std::stof(argv[3]) : 0.6f; // stricter than Engine's default 0.5

    cv::Mat src = cv::imread(imagePath, cv::IMREAD_COLOR);
    if (src.empty()) {
        std::cerr << "Could not read image: " << imagePath << "\n";
        return 1;
    }

    // Load each stage independently — same model file naming convention
    // Engine uses internally (see EngineConfig's doc comments).
    arbo::ocr::Detector detector;
    detector.loadModel(modelsDir + "/PP-OCRv6_det.onnx");

    arbo::ocr::Classifier classifier; // not loaded: this example skips angle
                                      // classification entirely (see below)

    arbo::ocr::Recognizer recognizer;
    recognizer.loadModel(modelsDir + "/PP-OCRv6_rec_medium.onnx");
    if (!recognizer.loadKeysFromModelMetadata()) {
        recognizer.loadKeysFromFile(modelsDir + "/PP-OCRv6_rec_medium_dict.txt");
    }
    if (recognizer.keyCount() == 0) {
        std::cerr << "Recognizer has no character dictionary loaded — check modelsDir\n";
        return 1;
    }

    // 1. Detect text boxes.
    arbo::ocr::ScaleParam scale = arbo::ocr::getScaleParam(src, /*targetSize=*/1536);
    auto textBoxes = detector.getTextBoxes(src, scale, /*boxScoreThresh=*/0.5f,
                                            /*boxThresh=*/0.3f, /*unClipRatio=*/1.6f);
    std::cout << "Detector found " << textBoxes.size() << " boxes\n";

    // 2. THE CUSTOMIZATION: drop low-confidence boxes before recognition.
    // Engine has no equivalent hook — it recognizes every detected box.
    // Skipping here saves a full CRNN forward pass per dropped box.
    std::vector<arbo::ocr::RawTextBox> keptBoxes;
    for (auto& box : textBoxes) {
        if (box.score >= minBoxScore) keptBoxes.push_back(box);
    }
    std::cout << "Kept " << keptBoxes.size() << " boxes with score >= " << minBoxScore << "\n\n";

    // 3. Crop each kept box out of the source image.
    std::vector<cv::Mat> partImages;
    partImages.reserve(keptBoxes.size());
    for (auto& box : keptBoxes) {
        partImages.push_back(arbo::ocr::getRotateCropImage(src, box.boxPoint));
    }

    // 4. Skip angle classification (classifier never loaded above) — every
    // crop is treated as upright. Pass doAngle=false so Classifier
    // short-circuits without touching a model (see classifier.hpp doc
    // comment); this is safe precisely because we never called loadModel().
    auto angles = classifier.getAngles(partImages, /*doAngle=*/false, /*mostAngle=*/false);
    for (size_t i = 0; i < partImages.size(); i++) {
        if (angles[i].index == 1) {
            partImages[i] = arbo::ocr::matRotateClockWise180(partImages[i]);
        }
    }

    // 5. Recognize. Internally batched (see Recognizer::getTextLines() doc
    // comment) — no extra work needed here to get that.
    auto textLines = recognizer.getTextLines(partImages);

    for (size_t i = 0; i < keptBoxes.size(); i++) {
        std::cout << textLines[i].text << "  (box score=" << keptBoxes[i].score << ")\n";
    }

    return 0;
}
