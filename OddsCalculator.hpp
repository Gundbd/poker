#pragma once

#include <vector>
#include <map>
#include <random>
#include <algorithm>

#include "Card.hpp"
#include "HandEvaluator.hpp"

using std::vector;
using std::map;

class OddsCalculator {
public:
    static map<HandRank, double> calculateMonteCarlo(
        const vector<Card>& holeCards,
        const vector<Card>& tableCards,
        int simulations = 10000) {
        
            map<HandRank, int> counts; 
            
            vector<Card> deck;

            for(int s = static_cast<int>(Card::Suit::HEARTS); s <= static_cast<int>(Card::Suit::SPADES); ++s){
                for(int r = static_cast<int>(Card::Rank::TWO); r <= static_cast<int>(Card::Rank::ACE); ++r){
                    deck.emplace_back(static_cast<Card::Suit>(s), static_cast<Card::Rank>(r));
                }
            }

            vector<Card> clearDeck;
            for(const auto& dCard : deck){

                bool holeCardFlag = false;
                for(const auto& hCard : holeCards){
                    if(hCard.getRank() == dCard.getRank() && hCard.getSuit() == dCard.getSuit()){
                        holeCardFlag = true;
                    }
                }

                bool tableCardFlag = false;
                for(const auto& tCard : tableCards){
                    if(tCard.getRank() == dCard.getRank() && tCard.getSuit() == dCard.getSuit()){
                        tableCardFlag = true;
                    }
                }

                if(!(tableCardFlag || holeCardFlag)) clearDeck.emplace_back(dCard);
            }

            std::random_device rd;
            std::mt19937 g(rd());
            HandEvaluator evaluator;
            

            for(int i = 0; i < simulations; ++i){
                std::shuffle(clearDeck.begin(), clearDeck.end(), g);

                vector<Card> simHole = holeCards;
                vector<Card> simTable = tableCards;
                int deckIdx = 0;

                int holeNeeded = 2 - simHole.size();
                for(int h = 0; h < holeNeeded; ++h){
                    simHole.push_back(clearDeck[deckIdx]);
                    deckIdx++;

                }

                int tableNeeded = 5 - tableCards.size();
                for(int t = 0; t < tableNeeded; ++t) {
                    simTable.push_back(clearDeck[deckIdx]);
                    deckIdx++;
                }

                vector<Card> allSimCards = simHole;
                allSimCards.insert(allSimCards.end(), simTable.begin(), simTable.end());

                HandResult result = evaluator.evaluateBest(allSimCards);

                counts[result.m_rank]++; 
            }

            map<HandRank, double> probabilities;
            for(const auto& [rank , count] : counts){
                probabilities[rank] = (static_cast<double>(count) / simulations) * 100.0;
            }
            return probabilities; 
    }
};
