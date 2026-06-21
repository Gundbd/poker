#include <opencv2/opencv.hpp>
#include <iostream>
#include <memory>
#include <filesystem>
#include <string>

#include "CardDetector.hpp"

namespace {
constexpr int kDetectEveryNFrames = 2;

void drawOutlinedText(cv::Mat& frame, const std::string& text, cv::Point origin) {
    cv::putText(frame, text, origin, cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 0), 4);
    cv::putText(frame, text, origin, cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);
}
}

static std::filesystem::path findModelPath() {
    const std::filesystem::path candidates[] = {
        "models/best.onnx",
        "best.onnx",
        "../models/best.onnx",
    };
    for (const auto& path : candidates) {
        if (std::filesystem::exists(path)) {
            return path;
        }
    }
    return "models/best.onnx";
}

int main() {
    const std::filesystem::path modelPath = findModelPath();

    cv::VideoCapture cap(0, cv::CAP_DSHOW);
    if (!cap.isOpened()) {
        std::cerr << "Error: can't open webcam\n";
        return 1;
    }

    cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    cap.set(cv::CAP_PROP_BUFFERSIZE, 1);

    std::cout << "Webcam opened\n";

    std::unique_ptr<CardDetector> detector;
    if (std::filesystem::exists(modelPath)) {
        try {
            std::cout << "Loading model: " << modelPath.string() << "\n";
            detector = std::make_unique<CardDetector>(modelPath.string());
            std::cout << "Model loaded\n";
        } catch (const std::exception& e) {
            std::cerr << "Model load error: " << e.what() << "\n";
            return 1;
        }
    } else {
        std::cerr << "Model not found: " << modelPath.string() << "\n";
        return 1;
    }

    std::cout << "Press Esc to exit\n";

    cv::Mat frame;
    int frameIndex = 0;
    std::string lastCardText = "none";

    while (true) {
        cap >> frame;
        if (frame.empty()) {
            break;
        }

        if (detector) {
            if (frameIndex % kDetectEveryNFrames == 0) {
                detector->detect(frame);
                if (const Card* card = detector->lastDetectedCard()) {
                    lastCardText = card->toString();
                } else {
                    lastCardText = "none";
                }
            }
            detector->drawCached(frame);
        }

        drawOutlinedText(frame, "Last: " + lastCardText, cv::Point(20, 40));

        cv::imshow("PokerCV Tracker", frame);
        if (cv::waitKey(1) == 27) {
            break;
        }

        ++frameIndex;
    }

    return 0;
}
