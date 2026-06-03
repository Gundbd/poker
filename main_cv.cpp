#include <opencv2/opencv.hpp>
#include <iostream>

#include "Card.hpp"
#include "HandEvaluator.hpp"
#include "OddsCalculator.hpp"


using std::vector;

int main(){
    cv::VideoCapture cap(0, cv::CAP_DSHOW);
    if(!(cap.isOpened())){
        std::cerr << "Error: can't open webcam \n"; 
        return -1; 
    }

    vector<Card> myHole = {
        Card(Card::Suit::SPADES, Card::Rank::ACE),
        Card(Card::Suit::SPADES, Card::Rank::KING)
    };

    vector<Card> tableCards = {
        Card(Card::Suit::SPADES, Card::Rank::QUEEN),
        Card(Card::Suit::SPADES, Card::Rank::JACK),
        Card(Card::Suit::HEARTS, Card::Rank::TWO)
    };

    auto probs = OddsCalculator::calculateMonteCarlo(myHole, tableCards, 5000); 

    cv::Mat frame; 
    std::cout << "Press Esc to exit\n"; 

    while(true){
        cap >> frame;
        if(frame.empty()) break; 

        cv::rectangle(frame, cv::Point(10,10), cv::Point(400,250), cv::Scalar(0,0,0), -1);

        string handText = "Hand: " + myHole[0].toString() + " " + myHole[1].toString();
        cv::putText(frame, handText, cv::Point(20,40), 
            cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255,255,255), 2); 
            
        string tableText = "Table: ";
        cv::putText(frame, tableText, cv::Point(20, 80), 0.7,
            cv::FONT_HERSHEY_SIMPLEX ,cv::Scalar(255,255,255), 2);
            
        int y_offset = 120; 
        for(const auto& [rank, prob] : probs){
            if(static_cast<int>(rank) >= static_cast<int>(HandRank::PAIR)) {
                string probText = HandResult::handRankToString(rank) + " : " + std::to_string(prob).substr(0,5) + "%"; 
                
                cv::Scalar color = (static_cast<int>(rank) >= static_cast<int>(HandRank::FLUSH)) ? cv::Scalar(0,255,0) : cv::Scalar(0, 255, 255); 
                cv::putText(frame, probText, cv::Point(20, y_offset), cv::FONT_HERSHEY_SIMPLEX, 0.6, color, 2); 

                y_offset += 30;
            }
        }

        cv::imshow("PokerCV Tracker", frame); 
        if(cv::waitKey(1) == 27) break; 

    }

    return 0;


    

}