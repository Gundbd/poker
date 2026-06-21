#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>

#include "Deck.hpp"
#include "Card.hpp"
#include "HandEvaluator.hpp"
#include "Game.hpp"
#include "Player.hpp"
#include "OddsCalculator.hpp"
#include "Example.hpp"

#include "tests/TestDeck.hpp"
#include "tests/TestHandEvaluator.hpp"


using std::string;
using std::vector;


int main() {
    system("chcp 65001 > nul"); 

    //took from example.hpp
    tryGame();
    
    return 0;
}

