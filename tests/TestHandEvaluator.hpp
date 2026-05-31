#pragma once
#include <iostream>
#include <stdexcept> 
#include <set>

#include "../Deck.hpp"
#include "../Card.hpp"
#include "../HandEvaluator.hpp"

using std::vector;

#define ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "❌ TEST FAILED: " << msg << " (Line: " << __LINE__ << ")" << std::endl; \
        } else { \
            std::cout << "✅ TEST PASSED: " << msg << std::endl; \
        } \
    } while(0)


void testHandEvaluator() {

    std::cout << "\n--- STARTING HAND EVALUAROT TEST ---\n";

    std::cout << "---evaluate5() func test--- \n";

    vector<Card> cards4 = {Card(Card::Suit::CLUBS,  Card::Rank::ACE),
        Card(Card::Suit::DIAMONDS, Card::Rank::EIGHT),
        Card(Card::Suit::CLUBS, Card::Rank::QUEEN),
        Card(Card::Suit::HEARTS, Card::Rank::JACK)
    };

    vector<Card> cards6 = {
        Card(Card::Suit::CLUBS,  Card::Rank::ACE),
        Card(Card::Suit::DIAMONDS, Card::Rank::EIGHT),
        Card(Card::Suit::CLUBS, Card::Rank::QUEEN),
        Card(Card::Suit::HEARTS, Card::Rank::JACK),
        Card(Card::Suit::HEARTS, Card::Rank::ACE),
        Card(Card::Suit::HEARTS, Card::Rank::TWO)
    }; 

    HandEvaluator hand;

    try {
        hand.evaluate5(cards4);
        ASSERT(false, "trying to evaluate 4 cards, should throw exception");
    } catch(const std::invalid_argument& e) {
        ASSERT(true, "trying to evaluate 4 cards, should throw invalid_argument exception");  
    } catch( ... ){
        ASSERT(false, "trying to evaluate 4 cards, threw wrong exception");
    }

    
    try {
        hand.evaluate5(cards6);
        ASSERT(false, "trying to evaluate 6 cards, should throw exception");
    } catch(const std::invalid_argument& e) {
        ASSERT(true, "trying to evaluate 6 cards, should throw invalid_argument exception");  
    } catch ( ... ) {
        ASSERT(false, "trying to evaluate 6 cards, threw wrong exception"); 
    }


    //trying to give evaluate five cards wiht high card comb
    vector<Card> highCard = {
        Card(Card::Suit::CLUBS,  Card::Rank::ACE),
        Card(Card::Suit::DIAMONDS, Card::Rank::EIGHT),
        Card(Card::Suit::CLUBS, Card::Rank::QUEEN),
        Card(Card::Suit::HEARTS, Card::Rank::JACK),
        Card(Card::Suit::HEARTS, Card::Rank::FIVE),
    }; 

    ASSERT(hand.evaluate5(highCard).m_rank == HandRank::HIGH_CARD, "should return high card");


    //trying to give evaluate five cards wiht pair comb
    vector<Card> pair = {
        Card(Card::Suit::CLUBS,  Card::Rank::ACE),
        Card(Card::Suit::DIAMONDS, Card::Rank::EIGHT),
        Card(Card::Suit::CLUBS, Card::Rank::QUEEN),
        Card(Card::Suit::HEARTS, Card::Rank::JACK),
        Card(Card::Suit::HEARTS, Card::Rank::ACE),
    }; 

    ASSERT(hand.evaluate5(pair).m_rank == HandRank::PAIR, "should return pair");


    //trying to give evaluate five cards wiht two pair comb
    vector<Card> twoPair= {
        Card(Card::Suit::CLUBS,  Card::Rank::ACE),
        Card(Card::Suit::DIAMONDS, Card::Rank::EIGHT),
        Card(Card::Suit::CLUBS, Card::Rank::QUEEN),
        Card(Card::Suit::HEARTS, Card::Rank::EIGHT),
        Card(Card::Suit::HEARTS, Card::Rank::ACE),
    }; 

    ASSERT(hand.evaluate5(twoPair).m_rank == HandRank::TWO_PAIR, "should return two pair");


    //trying to give evaluate five cards wiht three of a kind comb
    vector<Card> threeOfAKind= {
        Card(Card::Suit::CLUBS,  Card::Rank::QUEEN),
        Card(Card::Suit::DIAMONDS, Card::Rank::QUEEN),
        Card(Card::Suit::CLUBS, Card::Rank::QUEEN),
        Card(Card::Suit::HEARTS, Card::Rank::EIGHT),
        Card(Card::Suit::HEARTS, Card::Rank::ACE),
    }; 

    ASSERT(hand.evaluate5(threeOfAKind).m_rank == HandRank::THREE_OF_A_KIND, "should return three of a kind");

    

    //trying to give evaluate five cards wiht straight comb
    vector<Card> straight = {
        Card(Card::Suit::CLUBS,  Card::Rank::QUEEN),
        Card(Card::Suit::DIAMONDS, Card::Rank::KING),
        Card(Card::Suit::CLUBS, Card::Rank::JACK),
        Card(Card::Suit::HEARTS, Card::Rank::TEN),
        Card(Card::Suit::HEARTS, Card::Rank::ACE),
    }; 

    ASSERT(hand.evaluate5(straight).m_rank == HandRank::STRAIGHT, "should return straight");

    //trying to give evaluate five cards wiht flush comb
    vector<Card> flush = {
        Card(Card::Suit::CLUBS,  Card::Rank::FIVE),
        Card(Card::Suit::CLUBS, Card::Rank::TEN),
        Card(Card::Suit::CLUBS, Card::Rank::QUEEN),
        Card(Card::Suit::CLUBS, Card::Rank::EIGHT),
        Card(Card::Suit::CLUBS, Card::Rank::ACE),
    }; 

    ASSERT(hand.evaluate5(flush).m_rank == HandRank::FLUSH, "should return flush");
    
    //trying to fgive evaluate five cards with full house comb
    vector<Card> fullHouse = {
        Card(Card::Suit::CLUBS,  Card::Rank::QUEEN),
        Card(Card::Suit::DIAMONDS, Card::Rank::QUEEN),
        Card(Card::Suit::SPADES, Card::Rank::QUEEN),
        Card(Card::Suit::SPADES, Card::Rank::ACE),
        Card(Card::Suit::CLUBS, Card::Rank::ACE),
    }; 

    ASSERT(hand.evaluate5(fullHouse).m_rank == HandRank::FULL_HOUSE, "Should return full house");

    //trying to give evaluate five cards with straight flush comb
    vector<Card> straightFlush = {
        Card(Card::Suit::CLUBS, Card::Rank::ACE),
        Card(Card::Suit::CLUBS, Card::Rank::TWO),
        Card(Card::Suit::CLUBS, Card::Rank::THREE),
        Card(Card::Suit::CLUBS, Card::Rank::FOUR),
        Card(Card::Suit::CLUBS, Card::Rank::FIVE),
    };

    ASSERT(hand.evaluate5(straightFlush).m_rank == HandRank::STRAIGHT_FLUSH, "Should return straight flush");

    //tryying to give evaluate five cards with flushroyal
    vector<Card> flushRoyal = {
        Card(Card::Suit::DIAMONDS, Card::Rank::ACE),
        Card(Card::Suit::DIAMONDS, Card::Rank::KING),
        Card(Card::Suit::DIAMONDS, Card::Rank::QUEEN),
        Card(Card::Suit::DIAMONDS, Card::Rank::JACK),
        Card(Card::Suit::DIAMONDS, Card::Rank::TEN),
    }; 

    ASSERT(hand.evaluate5(flushRoyal).m_rank == HandRank::ROYAL_FLUSH,  "should return flush Royal");

    std::cout << "\n---evaluateBest() func test---\n";

    //tryying to give evaluate five cards with flushroyal
    flushRoyal.push_back(Card(Card::Suit::CLUBS, Card::Rank::TEN));
    flushRoyal.push_back(Card(Card::Suit::SPADES, Card::Rank::JACK));

    ASSERT(hand.evaluateBest(flushRoyal).m_rank == HandRank::ROYAL_FLUSH,  "flush Royal better then two pairs");

    std::cout << "\n--- HAND EVALUATOR TEST COMPLETED ---\n\n";


}