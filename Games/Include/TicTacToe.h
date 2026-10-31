#pragma once

#include "Types/TicTacToeTypes.h"

#include <span>

namespace TicTacToe {
  class Game : public Scene {
  private:
    Grid grid;
    bool isXTurn;
    Cooldown aiCooldown;
    Cooldown restartCooldown;
    sf::RectangleShape winningLine;
    sf::Text resultText{GetDefaultFont()};

  public:
    Game(EngineContext &);

    void Start();
    void OnEvent(const sf::Event &);
    void Update();
    void Render() const;

  private:
    void InitGrid();
    void InitGridCell(Cell &cell, sf::Vector2f position);

    void StartGrid();
    void StartGridCell(Cell &cell);
    void StartTurn();
    void StartRestartCooldown();
    void StartWinningLine();
    void StartResultText();

    void EventPlayTurn();
    void EventPlayAITurn();
    void EventCellPlace(sf::Vector2i position);
    void EventCheckGrid();
    void EventWinningLineSpawn(char symbol);
    void EventResultTextSpawn(char symbol);

    sf::Vector2i GetCellPositionCursor() const;
    sf::Vector2f GetCellCenter(const sf::Vector2i &position) const;
    sf::Vector2i GetCellPositionAI();
    std::vector<sf::Vector2i> FindCellPositionsEmpty() const;
    std::optional<sf::Vector2i> FindCellPositionWinning(std::span<sf::Vector2i> candidates, char symbol);
    std::vector<sf::Vector2i> FindCellPositionsWinning(char symbol) const;
    bool IsCellPositionValid(sf::Vector2i position) const;
    bool IsWin(char symbol) const;
    bool IsGridFull() const;
  };
} // namespace TicTacToe
