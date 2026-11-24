#pragma once

#include "Types/MemoryCardTypes.h"
#include <memory>

namespace MemoryCard {
  class Game : public Scene {
  private:
    std::vector<Card> cards;
    CardPair cardPair;
    Stats stats;

    AudioGroup audioGroup;
    std::shared_ptr<sf::Music> music;

  public:
    Game(EngineContext &);

    void Start();
    void OnEvent(const sf::Event &);
    void Update();
    void Render() const;
    void OnPause(bool paused);
    void OnCleanup();

  private:
    void InitCardPair();
    void InitMusic();

    void StartStats();
    void StartCards();

    void EventCardsSpawn();
    void EventCardSpawn(sf::FloatRect bounds, sf::Color color);
    void EventCardClicked();
    void EventCardFlip(Card &card);
    void EventCardPairEvaluate();
    void EventCardPairReset();

    std::vector<sf::Color> GenerateCardColors() const;
    bool IsCardHovered(const Card &card) const;
    bool IsCardPairSelected() const;
  };
} // namespace MemoryCard
