#include "MemoryCard.h"

using namespace MemoryCard;

/* Constructor: inicializa la escena y prepara el temporizador del par visible. */
Game::Game(EngineContext &context) : Scene(context) {
  InitCardPair();
}

/* Configura la duración que permanecen visibles las cartas volteadas. */
void Game::InitCardPair() {
  cardPair.visibleCooldown.SetDuration(CARD_PAIR_VISIBLE_COOLDOWN_DURATION);
}

/* Arranque de la escena: reinicia estadísticas y genera las cartas. */
void Game::Start() {
  StartStats();
  StartCards();
}

/* Reinicia el nivel a 0 en cada nueva partida. */
void Game::StartStats() {
  stats.level = 0;
}

/*  Se crea la primera tanda de cartas */
void Game::StartCards() {
  EventCardsSpawn();
}

/* Al pulsar mouse o botón de joystick: voltea una carta si aún no hay par seleccionado. */
void Game::OnEvent(const sf::Event &event) {
  if (event.is<sf::Event::MouseButtonPressed>() ||
      event.is<sf::Event::JoystickButtonPressed>()) {
    if (!IsCardPairSelected()) {
      EventCardClicked();
    }
  }
}

/* Lógica principal por frame: evalúa el par tras el cooldown y avanza de nivel si se encontraron todas. */
void Game::Update() {
  if (IsCardPairSelected() && cardPair.visibleCooldown.IsOver()) {
    EventCardPairEvaluate();

    if (std::ranges::all_of(cards, [](const Card &card) { return card.found; })) {
      stats.level++;
      EventCardsSpawn();
    }
  }
}

/* Genera una nueva ronda de cartas y las distribuye en una grilla (filas x columnas). */
void Game::EventCardsSpawn() {
  std::vector<sf::Color> colors = GenerateCardColors();
  int count = (int)colors.size();

  /* Calcula filas/columnas para acomodar todas las cartas en grilla. */
  int rows = (int)std::sqrt(count);
  int cols = (count + rows - 1) / rows;

  cards.clear();
  cards.reserve(count);

  for (int i = 0; i < count; i++) {
    /* Cada carta ocupa una celda: tamaño = ventana / grilla, posición según índice. */
    sf::Vector2f size = gConfig.windowSize.componentWiseDiv({(float)cols, (float)rows});
    sf::Vector2f position(size.x * (i % cols), size.y * static_cast<float>(i / cols));

    EventCardSpawn({position, size}, colors[i]);
  }

  EventCardPairReset();
}

/* Crea una carta: textura base transparente, borde, color oculto y estado inicial. */
void Game::EventCardSpawn(sf::FloatRect bounds, sf::Color color) {
  auto &card = cards.emplace_back();

  card.shape.setTexture(ctx.resources.FetchTexture(CARD_TEXTURE_FILENAME));
  card.shape.setFillColor(sf::Color::Transparent);
  card.shape.setOutlineColor(CARD_OUTLINE_COLOR);
  card.shape.setOutlineThickness(-1);
  card.shape.setSize(bounds.size);
  card.shape.setPosition(bounds.position);

  card.color = color;
  card.flipped = false;
  card.found = false;
}

/* Voltea la primera carta no volteada bajo el cursor (una por clic). */
void Game::EventCardClicked() {
  for (auto &card : cards) {
    if (!card.flipped && IsCardHovered(card)) {
      EventCardFlip(card);
      break;
    }
  }
}

/* Muestra el color de la carta y la registra como primer o segundo elemento del par. */
void Game::EventCardFlip(Card &card) {
  card.shape.setFillColor(card.color);
  card.flipped = true;

  if (!cardPair.first) {
    cardPair.first = &card;
  } else {
    cardPair.second = &card;
    cardPair.visibleCooldown.Restart();
  }
}

/* Compara el par: si coinciden se marcan como encontradas; si no, se vuelven a ocultar. */
void Game::EventCardPairEvaluate() {
  if (cardPair.first->color == cardPair.second->color) {
    cardPair.first->found = true;
    cardPair.second->found = true;
  } else {
    cardPair.first->shape.setFillColor(sf::Color::Transparent);
    cardPair.second->shape.setFillColor(sf::Color::Transparent);

    cardPair.first->flipped = false;
    cardPair.second->flipped = false;
  }

  EventCardPairReset();
}

/* Limpia las referencias del par actual. */
void Game::EventCardPairReset() {
  cardPair.first = nullptr;
  cardPair.second = nullptr;
}

/* Genera los colores de la ronda: pares duplicados según el nivel, luego mezclados al azar. */
std::vector<sf::Color> Game::GenerateCardColors() const {
  std::vector<sf::Color> colors;

  /* La cantidad de cartas crece con el nivel. */
  int count = CARDS_COUNT_INITIAL + stats.level * CARDS_COUNT_PER_LEVEL;

  colors.reserve(count);

  /* Cada color se agrega dos veces para formar pares; la paleta cicla si faltan. */
  for (int i = 0; i < count / 2; i++) {
    sf::Color color = CARDS_COLORS[i % CARDS_COLORS.size()];
    colors.emplace_back(color);
    colors.emplace_back(color);
  }

  std::ranges::shuffle(colors, std::default_random_engine(std::random_device{}()));

  return colors;
}

/* Indica si el cursor está sobre la carta. */
bool Game::IsCardHovered(const Card &card) const {
  return Contains(card.shape, ctx.cursor.GetPosition());
}

/* Un par está completo cuando hay primera y segunda carta seleccionadas. */
bool Game::IsCardPairSelected() const {
  return cardPair.first && cardPair.second;
}

/* Dibuja todas las cartas en pantalla. */
void Game::Render() const {
  for (const auto &card : cards) {
    ctx.renderer.Draw(card.shape);
  }
}

/* Pausa/reanuda el cooldown del par para que no expire mientras la escena está pausada. */
void Game::OnPause(bool paused) {
  if (paused) {
    cardPair.visibleCooldown.Stop();
  } else {
    cardPair.visibleCooldown.Start();
  }
}
