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
    
    //testDeck();
    //testHandEvaluator(); 
    /* 
    Game game;
    
    game.addPlayer("Vasya", 5000);
    game.addPlayer("Vanya", 7000);
    game.addPlayer("Sveta", 4500);

    game.startNewRound();

    game.playerBet(0, 500);
    game.playerBet(1, 500);
    game.playerBet(2, 500);


    game.dealFlop();
    game.dealTurn();
    game.dealRiver(); 

    vector<PlayerResult> result = game.showdown();

    std::cout << "~~~ TABLE ~~~\n";

    for(const auto& card : game.getTableCards()){
        std::cout << card.toString() << " ";
    }

    std::cout << "\n\n";

    int idxPlayer = 0;
    for(const auto& player : result) {
        

        std::cout << player.name << "\n";

        std::cout << "Hand: ";
        for(const auto& card : player.holeCards) {
            std::cout << card.toString(); 
        }

        std::cout << "\nCombination: " << 
            player.bestResult.handRankToString(player.bestResult.m_rank) << "\n";
        std::cout << "Chips left: " << game.getPlayers()[idxPlayer].getStack() << "\n\n";


        idxPlayer++;
    }

    std::cout << "Bank: " << game.getPot() << "\n";

    auto winnerIt = std::max_element(result.begin(), result.end(), 
        [](const PlayerResult& a, const PlayerResult b) {
            return a.bestResult < b.bestResult;
        });

    std::cout << "\n WINNER: " << winnerIt->name; 
     */



    //took from example.hpp
    tryGame();
    
    return 0;
}

