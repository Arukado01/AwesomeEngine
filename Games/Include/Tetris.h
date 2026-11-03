// Copyright (c) 2025 Adel Hales

#pragma once

#include "Types/TetrisTypes.h"

namespace Tetris {
  class Game : public Scene {
  private:
    Board board;
    Piece current;
    Piece next;
    Stats stats;
    Cooldown actionCooldown;
    Cooldown fallCooldown;
    Cooldown softDropCooldown;
    Cooldown gameOverCooldown;
    bool gameOverActive = false;
    sf::RectangleShape gameOverBackdrop;
    sf::Text gameOverTitle{GetDefaultFont()};
    sf::Text gameOverScore{GetDefaultFont()};
    AudioGroup audioGroup;
    std::shared_ptr<sf::Music> music;
    std::shared_ptr<sf::Sound> lineClearSound;

  public:
    Game(EngineContext &);

    void Start() override;
    void OnEvent(const sf::Event &) override;
    void Update() override;
    void Render() const override;
    void OnPause(bool paused) override;
    void OnCleanup() override;

  private:
    void InitStats();

    void BindInputs();

    void StartGrid();
    void StartNextPiece();
    void StartStats();

    void EventPieceSpawn();
    void EventPieceRotate();
    void EventPieceMove(sf::Vector2i offset);
    void EventPieceFall();
    void EventPieceDropHard();
    void EventPieceMerge();
    void EventLinesClear();

    Piece GenerateRandomPiece() const;
    sf::Vector2i GetPointRotated(int type, int index, int rotation) const;
    sf::Vector2f GetBoardOrigin() const;
    bool IsPieceValid(const Piece &piece) const;

    void RenderBoard() const;
    void RenderPiece(const Piece &piece, sf::Vector2f origin, bool world) const;
  };
} // namespace Tetris
