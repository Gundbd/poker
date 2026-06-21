#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <stdexcept>

#include "Card.hpp"

using std::string;
using std::vector;
using std::map;
using std::pair;

enum class HandRank{
    HIGH_CARD,
    PAIR,
    TWO_PAIR,
    THREE_OF_A_KIND,
    STRAIGHT,
    FLUSH,
    FULL_HOUSE,
    FOUR_OF_A_KIND,
    STRAIGHT_FLUSH,
    ROYAL_FLUSH
};

struct HandResult {
    HandRank m_rank;
    vector<int> m_kickers;

    bool operator<(const HandResult& other) const {
        if(m_rank != other.m_rank) return static_cast<int>(m_rank) < static_cast<int>(other.m_rank);
        return m_kickers < other.m_kickers; 
    }
    
    static string handRankToString(HandRank rank) {
        switch(rank){
            case HandRank::HIGH_CARD: return "high card";
            case HandRank::PAIR: return "pair";
            case HandRank::TWO_PAIR: return "two pairs";
            case HandRank::THREE_OF_A_KIND: return "three of a kind";
            case HandRank::STRAIGHT: return "straight";
            case HandRank::FLUSH: return "flush";
            case HandRank::FULL_HOUSE: return "full house";
            case HandRank::STRAIGHT_FLUSH: return "straight flush";
            case HandRank::ROYAL_FLUSH: return "royal flush";
            default: return "unknown!";
        }
    }
};


class HandEvaluator {
public:

    HandResult evaluateBest(const vector<Card>& cards){
        if(cards.size() > 7 || cards.size() < 5) throw std::invalid_argument("Need from 7 to 5 cards to evaluate");

        HandResult bestResult;
        bestResult.m_rank = HandRank::HIGH_CARD;
        bestResult.m_kickers = {};

        int n = cards.size();

        for(int mask = 0; mask < (1 << n); ++mask){
            int bitCount = 0;
            for(int i = 0; i < n; ++i){
                if(mask & (1 << i)) bitCount++;
            }

            if(bitCount == 5){
                vector<Card> combo;
                for(int i = 0; i < n; ++i){
                    if(mask & (1 << i)){
                        combo.push_back(cards[i]);
                    }
                }

                HandResult currentResult = evaluate5(combo);

                if(bestResult < currentResult){
                    bestResult = currentResult;
                }
            }
        }

        return bestResult;

    }


    HandResult evaluate5(const vector<Card>& cards){
        // check does we put there 5 cards
        if(cards.size() != 5) throw std::invalid_argument("Need exactly 5 cards");
        
        map<Card::Rank, int> rankCounts;
        map<Card::Suit, int> suitCounts;

        
        for(const auto& card : cards){
            rankCounts[card.getRank()]++;
            suitCounts[card.getSuit()]++;
        }

        bool isFlush = (suitCounts.size() == 1);

        bool isStraight = false;

        //checking all pairs in the map and add ranks in the vector, cast to it's value 
        //ex: 2->2 ... QUEEN->12
        vector<int> ranks; 
        for(const auto& [rank, count] : rankCounts){
            ranks.push_back(static_cast<int>(rank));
        }
        std::sort(ranks.begin(), ranks.end());

        //Unique case A-2-3-4-5
        if(ranks.size() == 5){
            if(ranks[4] - ranks[0] == 4){
                isStraight = true;
            } else if(ranks[0] == 2 && ranks[1] == 3 && ranks[2] == 4 && ranks[3] == 5 && ranks[4] == 14 ){
                isStraight = true;
                ranks = {1, 2, 3, 4, 5}; 
            }
        }

        vector<pair<int,int>> freq;
        for(const auto& [rank, count] : rankCounts){
            freq.push_back({count, static_cast<int>(rank)});
        }

        std::sort(freq.begin(), freq.end(), [](const auto& a, const auto& b){
            if(a.first != b.first) return a.first > b.first; 
            return a.second > b.second;
        });


        vector<int> kickers;
        for(const auto& [count, rank] : freq){
            kickers.push_back(rank);
        }

        if(isFlush && isStraight){
            if(kickers[0] == 14 && ranks[0] == 10){
                return {HandRank::ROYAL_FLUSH, kickers};
            } 
            return {HandRank::STRAIGHT_FLUSH, kickers};
        }

        if(freq[0].first == 4) return {HandRank::FOUR_OF_A_KIND, kickers};
        if(freq[0].first == 3 && freq[1].first == 2) return {HandRank::FULL_HOUSE, kickers};
        if(isFlush) return {HandRank::FLUSH, kickers};
        if(isStraight) return {HandRank::STRAIGHT, kickers};
        if(freq[0].first == 3) return {HandRank::THREE_OF_A_KIND, kickers};
        if(freq[0].first == 2 && freq[1].first == 2) return {HandRank::TWO_PAIR, kickers};
        if(freq[0].first == 2) return {HandRank::PAIR, kickers};
        return {HandRank::HIGH_CARD, kickers};
    }
};