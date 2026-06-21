#pragma once
#include <string>

using std::string;

class Card {
public: 
    enum class Rank {
        TWO = 2,
        THREE,
        FOUR,
        FIVE,
        SIX,
        SEVEN,
        EIGHT,
        NINE,
        TEN,
        JACK,
        QUEEN,
        KING,
        ACE
    };

    enum class Suit {
        HEARTS,
        DIAMONDS,
        CLUBS,
        SPADES
    };

    Card(Suit s, Rank r) : m_suit(s), m_rank(r) {}

    string toString() const {
        string rankStr;
        switch(m_rank){
            case Rank::JACK: rankStr = "J"; break;
            case Rank::QUEEN: rankStr = "Q"; break;
            case Rank::KING: rankStr = "K"; break;
            case Rank::ACE: rankStr = "A"; break;
            default: rankStr = std::to_string(static_cast<int>(m_rank)); break;
        }

        string suitStr;
        switch(m_suit){
            case Suit::HEARTS: suitStr = "\u2665"; break;
            case Suit::DIAMONDS: suitStr = "\u2666"; break;
            case Suit::CLUBS: suitStr = "\u2663"; break;
            case Suit::SPADES: suitStr = "\u2660"; break;
        }

        return rankStr + suitStr; 
    }

    bool operator==(const Card& other) const {
        return m_suit == other.m_suit && m_rank == other.m_rank;  
    }

    bool operator<(const Card& other) const {
        if (m_suit != other.m_suit) return m_suit < other.m_suit;
            return m_rank < other.m_rank;
    }


    Rank getRank() const {
        return m_rank;
    }

    Suit getSuit() const {
        return m_suit;
    }


private:
    Suit m_suit;
    Rank m_rank;
};