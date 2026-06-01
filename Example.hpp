#include <iostream>

#include "Game.hpp"
#include "OddsCalculator.hpp"
#include "HandEvaluator.hpp"
#include "Card.hpp"
#include "Player.hpp"

void drawPlayerHC(const Player& player){
    std::cout << player.getName() << " cards: ";
    for(int i = 0; i < player.getCards().size(); ++i){
        std::cout << player.getCards()[i].toString() << " ";
    }
    std::cout << "\n\n";
}

void drawTable(const Game& game){
    std::cout << "~~~Table~~~" << "\n"; 
    for(int i = 0; i < game.getTableCards().size(); ++i){
        std::cout << game.getTableCards()[i].toString() << " "; 
    }
    std::cout << "\n\n";
}

void probsCountAndDraw(const Game& game, const Player& player){
    vector<Card> playerCards = player.getCards();
    vector<Card> tableCards = game.getTableCards();

    std::cout << "Probability of " << player.getName() << " possible combinations\n";

    
    auto probs = OddsCalculator::calculateMonteCarlo(playerCards, tableCards, 10000);
    for(const auto& [rank , count] : probs){
        std::cout << HandResult::handRankToString(rank) << " : " << count << "%\n";
    }

    std::cout << "\n\n";
}   


void drawShowdown(Game& game){
    vector<PlayerResult> result = game.showdown();

    for(const auto& player : result){
        std::cout <<  player.name << " cards: ";
        for(const auto card : player.holeCards){
            std::cout << card.toString() << " ";
        }

        std::cout << " Combination: ";
        std:: cout << player.bestResult.handRankToString(player.bestResult.m_rank);

        std::cout << "\n"; 
    }

}


void tryGame(){
    Game game;

    //very chad player, must win this game
    game.addPlayer("Philipp", 5000);
    //Ruslan rich bitch
    game.addPlayer("Ruslan", 15000);   
    //Artem plays on smagin money
    game.addPlayer("Artem Slobodchenkov", 10000);
    //Artem spent money on motorcycle
    game.addPlayer("Artem Smagin", 3000);

    game.startNewRound();


    //make all players bet 500 chips
    for(int i = 0; i < game.getPlayers().size(); ++i){
        game.playerBet(i, 500);
    }

    //drawPhilipp cards
    drawPlayerHC(game.getPlayers()[0]); 

    game.dealFlop();

    //draw table after flop 
    drawTable(game);

    probsCountAndDraw(game, game.getPlayers()[0]);

    game.dealTurn(); 

    drawTable(game);

    probsCountAndDraw(game, game.getPlayers()[0]);

    game.dealRiver();

    drawShowdown(game);

}