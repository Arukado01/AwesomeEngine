#pragma once

#include "Types/TicTacToeTypes.h"

#include <optional>
#include <span>

namespace TicTacToe {
  class Game : public Scene {
  private:
    std::optional<GameMode> mode; // vacío = pantalla de selección de modo activa
    ModeButton modeButtonAI;
    ModeButton modeButtonPvP;
    sf::Text menuTitle{GetDefaultFont()};
    sf::Text turnText{GetDefaultFont()};

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
    void InitMenu();
    void InitMenuButton(ModeButton &button, sf::Vector2f position, sf::Vector2f size);
    void FitText(sf::Text &text, float maxWidth, float size);

    void StartMenu();
    void StartGrid();
    void StartGridCell(Cell &cell);
    void StartTurn();
    void StartRestartCooldown();
    void StartWinningLine();
    void StartResultText();

    void UpdateMenu();

    void EventMenuSelect();
    void EventModeSelect(GameMode mode);
    void EventTurnTextSpawn();
    void EventPlayTurn();
    void EventPlayAITurn();
    void EventCellPlace(sf::Vector2i position);
    void EventCheckGrid();
    void EventWinningLineSpawn(char symbol);
    void EventResultTextSpawn(char symbol);

    std::optional<GameMode> FindModeHovered() const;
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
