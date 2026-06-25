#include <opencv2/opencv.hpp>
#include <iostream>
#include <memory>
#include <filesystem>
#include <string>

#include "CardDetector.hpp"

int kDetectEveryNFrames = 2;

//по факту просто процедура в которую передаем кадр, текст и точку отправки и считай
//уже задеффайинили форматирование текста
void drawOutlinedText(cv::Mat& frame, const std::string& text, cv::Point origin) {
    cv::putText(frame, text, origin, cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 0), 4);
    cv::putText(frame, text, origin, cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 255, 0), 2);
}

//супер сложная залупа, для очень простой задачи, найти путь к модели
//можно и захардкодить.
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

    //находим путь к модели
    const std::filesystem::path modelPath = findModelPath();

    //открываем вебкамеру
    cv::VideoCapture cap(0, cv::CAP_DSHOW);
    if (!cap.isOpened()) {
        std::cerr << "Error: can't open webcam\n";
        return 1;
    }

    //устанавливаем размеры кадра
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
    cap.set(cv::CAP_PROP_BUFFERSIZE, 1);

    //выводим что вебкамера открыта
    std::cout << "Webcam opened\n";

    //создаем детектор карт
    std::unique_ptr<CardDetector> detector;
    //проверяем что модель существует
    if (std::filesystem::exists(modelPath)) {
        try {
            //выводим что загружаем модель
            std::cout << "Loading model: " << modelPath.string() << "\n";
            //создаем детектор карт
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

    //создаем кадр
    cv::Mat frame;
    //индекс кадра
    int frameIndex = 0;
    //текст последней карты
    std::string lastCardText = "none";

    while (true) {
        //читаем кадр из вебкамеры
        cap >> frame;
        //проверяем что кадр не пустой
        if (frame.empty()) {
            break;
        }

        //проверяем что детектор существует
        if (detector) {
            //проверяем что индекс кадра делится на константу без остатка
            if (frameIndex % kDetectEveryNFrames == 0) {
                //детектим карты
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
