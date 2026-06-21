#pragma once

#include <string>
#include <vector>
#include <algorithm>

#include "Card.hpp"

using std::string;
using std::vector;

class Player {
private:
    string m_name;
    int m_allChips;  
    vector<Card> m_holeCards;
    int m_currentBetChips;
    bool m_isFold;
    bool m_isAllIn;

public:
    Player(const string& name, int gameChips) : m_name(name), m_allChips(gameChips) {
        m_currentBetChips = 0;
        m_isFold = false;
    }

    void addCard(const Card& card) {
        if(m_holeCards.size() < 2){
            m_holeCards.push_back(card);
        }
        
    }

    void resetForNewRound() {
        m_holeCards.clear();
        m_currentBetChips = 0;
        m_isFold = false;
    }


    void bet(int amount) {
        int actualBet = std::min(amount, m_allChips);

        m_allChips -= actualBet;
        m_currentBetChips += actualBet;

        if(m_allChips == 0){
            m_isAllIn = true;  
        }
        

    }

    void addChips(int amount) { m_allChips += amount; }

    void fold() { m_isFold = true; }

    const string& getName() const { return m_name; } 
    int getStack() const { return m_allChips; }
    const vector<Card>& getCards() const { return m_holeCards; }
    int getBet() const { return m_currentBetChips; }
    bool getFoldFlag() const { return m_isFold; } 
    bool getIsAllIn() const { return m_isAllIn; }
}; 
