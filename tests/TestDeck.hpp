#pragma once

#include <iostream>
#include <stdexcept> 
#include <set>

#include "Deck.hpp"
#include "Card.hpp"

#define ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "❌ TEST FAILED: " << msg << " (Line: " << __LINE__ << ")" << std::endl; \
        } else { \
            std::cout << "✅ TEST PASSED: " << msg << std::endl; \
        } \
    } while(0)


using std::set;

void testDeck() {

    std::cout << "\n--- STARTING DECK TEST ---\n\n";

    Deck deck;

    ASSERT(deck.remainingCards() == 52, "New deck should have 52 cards");
    
    set<Card> dealtCards; 
    for(int i = 0; i < 52; ++i){
        dealtCards.insert(deck.dealCard());
    }
    ASSERT(dealtCards.size() == 52, "All 52 cards must be uniqe!");

    deck.reset();
    ASSERT(deck.remainingCards() == 52, "Deck after reset should have 52 cards");

    Deck emptyDeck;
    for(int i = 0; i < 52; ++i){
        emptyDeck.dealCard();
    }

    try{
        emptyDeck.dealCard();
        ASSERT(false, "Dealing from empty deck should throw exception");
    } catch(const std::out_of_range& e){
        ASSERT(true, "Dealing from empty deck should throw out_of_range");
    } catch ( ... ) {
        ASSERT(false, "Dealing from empty deck threw wrong exception types");
    }

    Deck shuffledDeck;
    Card firstCardBefore = shuffledDeck.dealCard();
    shuffledDeck.reset();
    shuffledDeck.shuffle();
    Card firstCardAfter = shuffledDeck.dealCard(); 

    ASSERT(firstCardAfter.toString() != firstCardBefore.toString(), "Shuffle should change deck order");

    shuffledDeck.reset(); 
    shuffledDeck.shuffle();
    ASSERT(shuffledDeck.remainingCards() == 52, "Shuffled deck should have 52 cards");

    std::cout << "\n--- DECK TEST COMPLETED ---\n\n"; 

}
