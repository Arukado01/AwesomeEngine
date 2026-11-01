#include "Scene/SceneFactory.h"

#include "Bounce.h"
#include "Clicker.h"
#include "MemoryCard.h"
#include "TicTacToe.h"
#include "MineSweeper.h"

namespace SceneFactory {
  Scenes CreateScenes(EngineContext &context) {
    Scenes scenes;

    scenes.emplace("Bounce", std::make_unique<Bounce::Game>(context));
    scenes.emplace("Clicker", std::make_unique<Clicker::Game>(context));
    scenes.emplace("MemoryCard", std::make_unique<MemoryCard::Game>(context));
    scenes.emplace("TicTacToe", std::make_unique<TicTacToe::Game>(context));
    scenes.emplace("MineSweeper", std::make_unique<MineSweeper::Game>(context));

    return scenes;
  }
} // namespace SceneFactory
