#include <opencv2/opencv.hpp>
#include <iostream>
#include <memory>
#include <filesystem>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <set> // Добавлено для удаления дубликатов

#include "CardDetector.hpp"
#include "Game.hpp"

// --- Глобальные переменные для синхронизации потоков ---
std::mutex frameMutex;
cv::Mat capturedFrame;
std::atomic<bool> captureRequested{false};
std::atomic<bool> appRunning{true};

// Функция фонового потока для отображения камеры
void cameraThreadFunc(cv::VideoCapture& cap) {
    cv::Mat frame;
    while (appRunning) {
        cap >> frame;
        if (frame.empty()) break;

        cv::putText(frame, "Press SPACE to capture | ESC to exit", 
                    cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 255, 255), 2);

        if (captureRequested) {
            std::lock_guard<std::mutex> lock(frameMutex);
            capturedFrame = frame.clone();
            captureRequested = false;
            
            cv::rectangle(frame, cv::Point(0, 0), cv::Point(frame.cols, frame.rows), cv::Scalar(0, 255, 0), 5);
            cv::putText(frame, "CAPTURED!", cv::Point(frame.cols / 2 - 100, frame.rows / 2), 
                        cv::FONT_HERSHEY_SIMPLEX, 1.5, cv::Scalar(0, 255, 0), 3);
        }

        cv::imshow("Card Capture", frame);

        int key = cv::waitKey(30);
        if (key == 27) { // ESC
            appRunning = false;
            break;
        }
        if (key == 32) { // SPACE
            captureRequested = true;
        }
    }
}

// Ожидание нажатия пробела и возврат захваченного кадра
cv::Mat waitForCapture() {
    std::cout << "[Waiting for SPACE press in camera window...]\n";
    while (appRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        std::lock_guard<std::mutex> lock(frameMutex);
        if (!capturedFrame.empty()) {
            return capturedFrame.clone();
        }
    }
    return {};
}

// Обновленная логика: фильтруем дубли и берем топ-N уникальных карт
std::vector<Card> detectAndConfirm(CardDetector& detector, int expectedCount, const std::vector<Card>& excludeList) {
    while (appRunning) {
        cv::Mat frame = waitForCapture();
        if (frame.empty()) return {};

        // 1. Детектируем все карты (тут могут быть дубли от нейросети)
        std::vector<Card> detected = detector.detect(frame);

        // 2. Фильтруем те карты, которые уже лежат на столе
        std::vector<Card> filtered;
        for (const auto& d : detected) {
            bool isKnown = false;
            for (const auto& ex : excludeList) {
                if (d.toString() == ex.toString()) {
                    isKnown = true;
                    break;
                }
            }
            if (!isKnown) {
                filtered.push_back(d);
            }
        }

        // 3. ИЩЕМ УНИКАЛЬНЫЕ КАРТЫ (решает проблему множественных детекций)
        // Так как YOLO сортирует по уверенности, первые уникальные карты будут самыми верными
        std::vector<Card> uniqueCards;
        std::set<std::string> seenStrings;
        
        for (const auto& card : filtered) {
            std::string cardStr = card.toString();
            // Если мы еще не видели такую карту в этом кадре
            if (seenStrings.find(cardStr) == seenStrings.end()) {
                seenStrings.insert(cardStr);
                uniqueCards.push_back(card);
                
                // Если набрали нужное количество - останавливаемся
                if (uniqueCards.size() == expectedCount) {
                    break;
                }
            }
        }

        // 4. Обработка результата
        if (uniqueCards.size() == expectedCount) {
            std::cout << "Detected unique cards (" << uniqueCards.size() << "/" << expectedCount << "): ";
            for (const auto& c : uniqueCards) {
                std::cout << c.toString() << " ";
            }
            std::cout << "\n";

            std::string ans;
            std::cout << "Do these match? (y/n): ";
            std::cin >> ans;
            if (ans == "y" || ans == "Y") {
                return uniqueCards;
            } else {
                std::cout << "-> Adjust cards and press SPACE again.\n";
            }
        } else {
            std::cout << "Detected (" << uniqueCards.size() << "/" << expectedCount << "): ";
            for (const auto& c : uniqueCards) {
                std::cout << c.toString() << " ";
            }
            std::cout << "\n-> Not enough UNIQUE cards detected. Try again.\n";
        }
    }
    return {};
}

// Поиск модели
static std::filesystem::path findModelPath() {
    const std::filesystem::path candidates[] = {
        "models/best.onnx", "best.onnx", "../models/best.onnx",
    };
    for (const auto& path : candidates) {
        if (std::filesystem::exists(path)) return path;
    }
    return "models/best.onnx";
}

int main() {
    const std::filesystem::path modelPath = findModelPath();
    if (!std::filesystem::exists(modelPath)) {
        std::cerr << "Model not found: " << modelPath.string() << "\n";
        return 1;
    }

    std::cout << "Loading model...\n";
    CardDetector detector(modelPath.string());
    std::cout << "Model loaded successfully!\n\n";

    cv::VideoCapture cap(0, cv::CAP_DSHOW);
    if (!cap.isOpened()) {
        std::cerr << "Error: can't open webcam\n";
        return 1;
    }
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);

    std::thread camThread(cameraThreadFunc, std::ref(cap));

    int numPlayers;
    std::cout << "=== POKER CONSOLE GAME ===\n";
    while (true) {
        std::cout << "Enter number of players (2-10): ";
        std::cin >> numPlayers;
        if (numPlayers >= 2 && numPlayers <= 10) break;
        std::cout << "Invalid input!\n";
    }

    Game game;
    std::vector<std::vector<Card>> opponentCards;

    std::cout << "\n--- DETECTING OPPONENTS' CARDS ---\n";
    for (int i = 0; i < numPlayers - 1; ++i) {
        std::cout << "\n> Opponent " << (i + 1) << ". Show 2 cards to the camera.\n";
        std::vector<Card> cards = detectAndConfirm(detector, 2, {});
        if (!appRunning) break; // Если нажали ESC во время детекции
        opponentCards.push_back(cards);
    }

    if (appRunning) {
        try {
            game.setNewRound(numPlayers, opponentCards);
        } catch (const std::exception& e) {
            std::cerr << "Game setup error: " << e.what() << "\n";
            appRunning = false;
        }
    }

    if (appRunning) {
        const auto& players = game.getPlayers();
        const auto& hero = players.back();
        std::cout << "\n=== YOUR CARDS ===\n";
        for (const auto& c : hero.getCards()) {
            std::cout << c.toString() << " ";
        }
        std::cout << "\n\n";

        // ВЫНЕСЛИ ПЕРЕМЕННЫЕ СЮДА, чтобы они существовали на всех этапах ниже
        std::vector<Card> flopCards;
        std::vector<Card> turnCards;
        std::vector<Card> riverCards;

        // --- ФЛОП ---
        std::cout << "--- DETECTING FLOP (3 cards) ---\n";
        if (appRunning) {
            flopCards = detectAndConfirm(detector, 3, {});
            try {
                game.setFlop(flopCards);
            } catch (const std::exception& e) {
                std::cerr << "Flop error: " << e.what() << "\n";
                appRunning = false;
            }
        }

        // --- ТЕРН ---
        if (appRunning) {
            std::cout << "\n--- DETECTING TURN (1 new card) ---\n";
            std::cout << "> Leave the 3 flop cards on the table and add 1 new card.\n";
            turnCards = detectAndConfirm(detector, 1, flopCards); 
            try {
                game.setTurn(turnCards[0]);
            } catch (const std::exception& e) {
                std::cerr << "Turn error: " << e.what() << "\n";
                appRunning = false;
            }
        }

        // --- РИВЕР ---
        if (appRunning) {
            std::cout << "\n--- DETECTING RIVER (1 new card) ---\n";
            std::cout << "> Leave the 4 table cards and add 1 final card.\n";
            
            std::vector<Card> knownTableCards = flopCards;
            knownTableCards.push_back(turnCards[0]); // Теперь turnCards здесь видна!
            
            riverCards = detectAndConfirm(detector, 1, knownTableCards); 
            try {
                game.setRiver(riverCards[0]);
            } catch (const std::exception& e) {
                std::cerr << "River error: " << e.what() << "\n";
                appRunning = false;
            }
        }

        // --- ШОУДАУН ---
        if (appRunning) {
            std::cout << "\n================= SHOWDOWN =================\n";
            std::cout << "Table cards: ";
            for (const auto& c : game.getTableCards()) std::cout << c.toString() << " ";
            std::cout << "\n\n";

            std::vector<PlayerResult> results = game.showdown();
            for (const auto& res : results) {
                std::cout << "Player: " << res.name << "\n";
                std::cout << "  Hand: ";
                for (const auto& c : res.holeCards) std::cout << c.toString() << " ";
                std::cout << "\n  Best Hand: " << res.bestResult.handRankToString(res.bestResult.m_rank) << "\n\n";
            }
        }
    }

    if (appRunning) {
        std::cout << "Press Enter to exit...";
        std::cin.ignore();
        std::cin.get();
    }

    appRunning = false;
    if (camThread.joinable()) camThread.join();
    cv::destroyAllWindows();

    return 0;
}