#pragma once
#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <optional>

#include "Card.hpp"

class CardDetector {
private:
    struct Overlay {
        cv::Rect rect;
        std::string label;
    };

    cv::dnn::Net m_net;
    float m_confThreshold = 0.25f;
    float m_nmsThreshold = 0.45f;
    int m_inputSize = 640;
    std::vector<Overlay> m_cachedOverlays;
    std::optional<Card> m_lastCard;
    float m_lastConf = 0.0f;

    std::vector<std::string> m_classNames = {
        "10C","10D","10H","10S",
        "2C", "2D", "2H", "2S",
        "3C", "3D", "3H", "3S",
        "4C", "4D", "4H", "4S",
        "5C", "5D", "5H", "5S",
        "6C", "6D", "6H", "6S",
        "7C", "7D", "7H", "7S",
        "8C", "8D", "8H", "8S",
        "9C", "9D", "9H", "9S",
        "AC", "AD", "AH", "AS",
        "JC", "JD", "JH", "JS",
        "KC", "KD", "KH", "KS",
        "QC", "QD", "QH", "QS"
    };

    static cv::Mat letterbox(const cv::Mat& src, float& scale, int& padX, int& padY, int size) {
        scale = std::min(size / static_cast<float>(src.cols), size / static_cast<float>(src.rows));
        const int newW = static_cast<int>(std::round(src.cols * scale));
        const int newH = static_cast<int>(std::round(src.rows * scale));
        padX = (size - newW) / 2;
        padY = (size - newH) / 2;

        cv::Mat resized;
        cv::resize(src, resized, cv::Size(newW, newH));
        cv::Mat out(size, size, CV_8UC3, cv::Scalar(114, 114, 114));
        resized.copyTo(out(cv::Rect(padX, padY, newW, newH)));
        return out;
    }

    void rememberDetection(
        int left, int top, int right, int bottom,
        float conf, int classId,
        std::vector<Card>& detectedCards) {

        if (classId < 0 || classId >= static_cast<int>(m_classNames.size())) {
            return;
        }

        const std::string label = m_classNames[classId] + " " +
            std::to_string(static_cast<int>(conf * 100)) + "%";
        m_cachedOverlays.push_back({cv::Rect(left, top, right - left, bottom - top), label});

        const Card card = parseCardString(m_classNames[classId]);
        detectedCards.push_back(card);

        if (conf >= m_lastConf) {
            m_lastConf = conf;
            m_lastCard = card;
        }
    }

    void parseEnd2EndDetections(
        const float* outputData,
        int numSlots,
        float scale,
        int padX,
        int padY,
        int frameCols,
        int frameRows,
        std::vector<Card>& detectedCards) {

        for (int i = 0; i < numSlots; ++i) {
            const float x1 = outputData[i * 6 + 0];
            const float y1 = outputData[i * 6 + 1];
            const float x2 = outputData[i * 6 + 2];
            const float y2 = outputData[i * 6 + 3];
            const float conf = outputData[i * 6 + 4];
            const int classId = static_cast<int>(outputData[i * 6 + 5]);

            if (conf < m_confThreshold || x2 <= x1 || y2 <= y1) {
                continue;
            }

            int left = static_cast<int>((x1 - padX) / scale);
            int top = static_cast<int>((y1 - padY) / scale);
            int right = static_cast<int>((x2 - padX) / scale);
            int bottom = static_cast<int>((y2 - padY) / scale);

            left = std::max(0, std::min(left, frameCols - 1));
            top = std::max(0, std::min(top, frameRows - 1));
            right = std::max(left + 1, std::min(right, frameCols));
            bottom = std::max(top + 1, std::min(bottom, frameRows));

            rememberDetection(left, top, right, bottom, conf, classId, detectedCards);
        }
    }

    void parseRawYoloDetections(
        const float* outputData,
        const std::vector<int>& outputShape,
        float scale,
        int padX,
        int padY,
        int frameCols,
        int frameRows,
        std::vector<Card>& detectedCards) {

        const int numFeatures = outputShape[1];
        const int numDetections = outputShape[2];
        const int numClasses = numFeatures - 4;

        std::vector<cv::Rect> nmsBoxes;
        std::vector<float> scores;
        std::vector<int> classIds;
        nmsBoxes.reserve(256);
        scores.reserve(256);
        classIds.reserve(256);

        for (int i = 0; i < numDetections; ++i) {
            float maxConf = 0.0f;
            int classId = 0;
            for (int c = 0; c < numClasses; ++c) {
                const float conf = outputData[(4 + c) * numDetections + i];
                if (conf > maxConf) {
                    maxConf = conf;
                    classId = c;
                }
            }

            if (maxConf < m_confThreshold) {
                continue;
            }

            const float cx = outputData[0 * numDetections + i];
            const float cy = outputData[1 * numDetections + i];
            const float w = outputData[2 * numDetections + i];
            const float h = outputData[3 * numDetections + i];

            const int left = static_cast<int>(std::round(cx - w / 2.0f));
            const int top = static_cast<int>(std::round(cy - h / 2.0f));
            const int width = static_cast<int>(std::round(w));
            const int height = static_cast<int>(std::round(h));

            if (width <= 0 || height <= 0) {
                continue;
            }

            nmsBoxes.emplace_back(left, top, width, height);
            scores.push_back(maxConf);
            classIds.push_back(classId);
        }

        std::vector<int> keepIndices;
        cv::dnn::NMSBoxes(nmsBoxes, scores, m_confThreshold, m_nmsThreshold, keepIndices);

        for (const int idx : keepIndices) {
            const cv::Rect& box = nmsBoxes[idx];
            const float x1 = static_cast<float>(box.x);
            const float y1 = static_cast<float>(box.y);
            const float x2 = static_cast<float>(box.x + box.width);
            const float y2 = static_cast<float>(box.y + box.height);

            int left = static_cast<int>((x1 - padX) / scale);
            int top = static_cast<int>((y1 - padY) / scale);
            int right = static_cast<int>((x2 - padX) / scale);
            int bottom = static_cast<int>((y2 - padY) / scale);

            left = std::max(0, std::min(left, frameCols - 1));
            top = std::max(0, std::min(top, frameRows - 1));
            right = std::max(left + 1, std::min(right, frameCols));
            bottom = std::max(top + 1, std::min(bottom, frameRows));

            rememberDetection(left, top, right, bottom, scores[idx], classIds[idx], detectedCards);
        }
    }

    Card parseCardString(const std::string& label) {
        Card::Suit suit = Card::Suit::HEARTS;
        Card::Rank rank = Card::Rank::TWO;

        const char suitChar = label.back();
        switch (suitChar) {
            case 'C': suit = Card::Suit::CLUBS; break;
            case 'D': suit = Card::Suit::DIAMONDS; break;
            case 'H': suit = Card::Suit::HEARTS; break;
            case 'S': suit = Card::Suit::SPADES; break;
        }

        const std::string rankStr = label.substr(0, label.length() - 1);
        if (rankStr == "2") rank = Card::Rank::TWO;
        else if (rankStr == "3") rank = Card::Rank::THREE;
        else if (rankStr == "4") rank = Card::Rank::FOUR;
        else if (rankStr == "5") rank = Card::Rank::FIVE;
        else if (rankStr == "6") rank = Card::Rank::SIX;
        else if (rankStr == "7") rank = Card::Rank::SEVEN;
        else if (rankStr == "8") rank = Card::Rank::EIGHT;
        else if (rankStr == "9") rank = Card::Rank::NINE;
        else if (rankStr == "10") rank = Card::Rank::TEN;
        else if (rankStr == "A") rank = Card::Rank::ACE;
        else if (rankStr == "Q") rank = Card::Rank::QUEEN;
        else if (rankStr == "K") rank = Card::Rank::KING;
        else if (rankStr == "J") rank = Card::Rank::JACK;

        return Card(suit, rank);
    }

public:
    explicit CardDetector(const std::string& modelPath) {
        m_net = cv::dnn::readNetFromONNX(modelPath);
        m_net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        m_net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    }

    std::vector<Card> detect(const cv::Mat& frame) {
        std::vector<Card> detectedCards;
        m_cachedOverlays.clear();
        m_lastConf = 0.0f;
        m_lastCard.reset();

        float scale = 1.0f;
        int padX = 0;
        int padY = 0;
        cv::Mat letterboxed = letterbox(frame, scale, padX, padY, m_inputSize);

        cv::Mat blob;
        cv::dnn::blobFromImage(
            letterboxed, blob, 1.0 / 255.0, cv::Size(m_inputSize, m_inputSize), cv::Scalar(), true, false);

        m_net.setInput(blob);
        cv::Mat output = m_net.forward();
        const float* outputData = output.ptr<float>();

        std::vector<int> shape(output.dims);
        for (int i = 0; i < output.dims; ++i) {
            shape[i] = output.size[i];
        }

        if (shape.size() == 3 && shape[2] == 6) {
            parseEnd2EndDetections(
                outputData, shape[1], scale, padX, padY, frame.cols, frame.rows, detectedCards);
        } else if (shape.size() == 3 && shape[1] > 4) {
            parseRawYoloDetections(
                outputData, shape, scale, padX, padY, frame.cols, frame.rows, detectedCards);
        }

        return detectedCards;
    }

    void drawCached(cv::Mat& frame) const {
        for (const auto& overlay : m_cachedOverlays) {
            cv::rectangle(frame, overlay.rect, cv::Scalar(0, 255, 0), 2);
            cv::putText(
                frame, overlay.label,
                cv::Point(overlay.rect.x, std::max(15, overlay.rect.y - 8)),
                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 0), 2);
        }
    }

    const Card* lastDetectedCard() const {
        return m_lastCard.has_value() ? &m_lastCard.value() : nullptr;
    }
};
