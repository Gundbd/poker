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
        m_tableCards.clear();

        for(auto& player : m_players) {
            player.addCard(m_deck.dealCard());
            player.addCard(m_deck.dealCard());
        }
    }

     
    void setNewRound(int amountPlayers, const vector<vector<Card>>& cards) {
        if (amountPlayers < 2 || amountPlayers > 10) {
            throw std::invalid_argument("Players count must be in [2..10]");
        }
        if (cards.size() != static_cast<size_t>(amountPlayers - 1)) {
            throw std::invalid_argument("Need cards for amountPlayers - 1 opponents");
        }

        m_players.clear();
        m_tableCards.clear();
        m_pot = 0;
        m_deck.reset();

        for (int i = 0; i < amountPlayers - 1; ++i) {
            m_players.emplace_back("player" + std::to_string(i), 1000);
        }
        m_players.emplace_back("You", 1000);

    
        for (int i = 0; i < amountPlayers - 1; ++i) {
            if (cards[i].size() != 2) {
                throw std::invalid_argument("Each opponent must have exactly 2 cards");
            }
            m_players[i].addCard(cards[i][0]);
            m_players[i].addCard(cards[i][1]);
            m_deck.removeCards(cards[i]);
        }

        m_deck.shuffle();
        m_players.back().addCard(m_deck.dealCard());
        m_players.back().addCard(m_deck.dealCard());
    }



 
    void dealFlop() {
        for(int i = 0; i < 3; ++i){
            m_tableCards.push_back(m_deck.dealCard());
        }
    }

    void setFlop(const vector<Card>& cards) {
        if(m_tableCards.size() != 0){
            throw std::invalid_argument("There is have to be clear table before flop!");
        }

        if(cards.size() != 3){
            throw std::invalid_argument("Flop has to set 3 cards!");
        }

        for(int i = 0; i < cards.size(); ++i){
            m_tableCards.push_back(cards[i]);
        }
        m_deck.removeCards(cards);
    }

    void dealTurn() {
        m_tableCards.push_back(m_deck.dealCard());
    }

    void setTurn(const Card& card) {
        if(m_tableCards.size() != 3){
            throw std::invalid_argument("there is have to be setted flop before turn!");
        }

        m_tableCards.push_back(card);
        m_deck.removeCard(card);
    }

    void dealRiver() {
        m_tableCards.push_back(m_deck.dealCard());
    }

    void setRiver(const Card& card){
        if(m_tableCards.size() != 4){
            throw std::invalid_argument("there is must be flop and turn on table");
        }

        m_tableCards.push_back(card);
        m_deck.removeCard(card);
    }

    void playerBet(int playerIndex, int amount) {
        if(playerIndex >= 0 && playerIndex < m_players.size()){
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