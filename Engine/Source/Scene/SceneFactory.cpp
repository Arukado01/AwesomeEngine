#include "Scene/SceneFactory.h"

#include "Bounce.h"
#include "Clicker.h"
#include "MemoryCard.h"
#include "TicTacToe.h"
#include "MineSweeper.h"
#include "Runner.h"
#include "Tetris.h"
#include "Pong.h"
#include "Breakout.h"

namespace SceneFactory {
  Scenes CreateScenes(EngineContext &context) {
    Scenes scenes;

    scenes.emplace("Bounce", std::make_unique<Bounce::Game>(context));
    scenes.emplace("Clicker", std::make_unique<Clicker::Game>(context));
    scenes.emplace("MemoryCard", std::make_unique<MemoryCard::Game>(context));
    scenes.emplace("TicTacToe", std::make_unique<TicTacToe::Game>(context));
    scenes.emplace("MineSweeper", std::make_unique<MineSweeper::Game>(context));
    scenes.emplace("Runner", std::make_unique<Runner::Game>(context));
    scenes.emplace("Tetris", std::make_unique<Tetris::Game>(context));
    scenes.emplace("Pong", std::make_unique<Pong::Game>(context));
    scenes.emplace("Breakout", std::make_unique<Breakout::Game>(context));

    return scenes;
  }
} // namespace SceneFactory
