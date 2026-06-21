#pragma once
#include <vector>
#include <random>
#include <algorithm>
#include <stdexcept>
    
#include "Card.hpp"

using std::vector;

class Deck {
private:
    vector<Card> m_cards;

public:
    Deck() {
        reset();
    }

    void reset() {
        m_cards.clear();
        for(int s = static_cast<int>(Card::Suit::HEARTS); s <= static_cast<int>(Card::Suit::SPADES); ++s){
            for(int r = static_cast<int>(Card::Rank::TWO); r <= static_cast<int>(Card::Rank::ACE); ++r){
                m_cards.emplace_back(static_cast<Card::Suit>(s), static_cast<Card::Rank>(r));
            }
        }
    }

    void shuffle() {
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(m_cards.begin(), m_cards.end(), g);
    }

    Card dealCard(){
        if(m_cards.empty()){
            throw std::out_of_range("deck is empty!");
        }

        Card topCard = m_cards.back();
        m_cards.pop_back();
        return topCard;
    }

    int remainingCards() const {
        return m_cards.size();
    }

};
