#pragma once

#include <vector>
#include <string>
#include <utility>

#include "Player.hpp"
#include "HandEvaluator.hpp"
#include "Deck.hpp"

using std::vector;
using std::string;
using std::pair;

struct PlayerResult {
    string name;
    vector<Card> holeCards;
    HandResult bestResult;
};

class Game {
private:
    vector<Player> m_players;
    Deck m_deck;
    vector<Card> m_tableCards;
    HandEvaluator m_handEval;
    int m_pot;

public:
    void addPlayer(const string& name, int chips) {
        Player newPlayer(name, chips);
        m_players.push_back(newPlayer);
    }

    void startNewRound(){
        for(auto& player : m_players){
            player.resetForNewRound(); 
        }
        m_pot = 0;
        m_deck.reset();
        m_deck.shuffle();

        for(auto& player : m_players) {
            player.addCard(m_deck.dealCard());
            player.addCard(m_deck.dealCard());
        }
    }

    void dealFlop() {
        for(int i = 0; i < 3; ++i){
            m_tableCards.push_back(m_deck.dealCard());
        }
    }

    void dealTurn() {
        m_tableCards.push_back(m_deck.dealCard());
    }

    void dealRiver() {
        m_tableCards.push_back(m_deck.dealCard());
    }


    void PlayerBet(int playerIndex, int amount) {
        if(playerIndex >= 0 || playerIndex < m_players.size()){
            m_players[playerIndex].bet(amount);
            m_pot += amount;
        }

    }

    void awardPlayer(int playerIndex, int amount) {
        m_players[playerIndex].addChips(amount);
    }

    vector<PlayerResult> showdown() {

        vector<PlayerResult> results;
        for(const auto& player : m_players) {

            if(player.getFoldFlag()){
                continue;
            }

            vector<Card> allCards = player.getCards();
            allCards.insert(allCards.end(), m_tableCards.begin(), m_tableCards.end());

            HandResult best = m_handEval.evaluateBest(allCards);
            results.push_back({player.getName(), player.getCards(), best}); 

        }

        return results;

    }

    //GETTERS SECTION
    const vector<Card>& getTableCards() const { return m_tableCards; }
    const vector<Player>& getPlayers() const { return m_players; }
    int getPot() const { return m_pot; }

    
};