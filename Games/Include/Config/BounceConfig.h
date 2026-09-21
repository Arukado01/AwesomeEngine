#pragma once

#include "Scene/Scene.h"

namespace Bounce {
  // --- Paleta ---
  const int PADDLE_LIVES = 10; // Vidas iniciales (una se pierde por cada pelota que cae)
  const sf::Vector2f PADDLE_SIZE(120, 20); // Ancho x alto de la paleta, en píxeles
  const float PADDLE_SPEED = 750; // Velocidad de la paleta, en píxeles por segundo
  const sf::Color PADDLE_COLOR(sf::Color::Green);  // Color de la paleta

  // --- Pelota ---
  const float BALL_RADIUS = 10;                 // Radio de la pelota, en píxeles
  const float BALL_SPEED = 250;                 // Velocidad de la pelota, en píxeles por segundo
  const sf::Color BALL_COLOR(sf::Color::Cyan); // Color de la pelota
  const float BALL_SPAWN_COOLDOWN_DURATION = 2; // Segundos entre la aparición de cada pelota nueva

  // --- HUD (textos de score / récord / vidas) ---
  const sf::Color STATS_SCORE_TEXT_COLOR(sf::Color::Cyan);
  const std::string_view STATS_HIGH_SCORE_KEY = "Bounce:High Score"; // Clave usada para guardar el récord en Save.json
  const sf::Color STATS_HIGH_SCORE_TEXT_COLOR(sf::Color::Yellow);
  const sf::Color STATS_LIVES_TEXT_COLOR(sf::Color::Red);

  // --- Recursos (el ResourceManager busca por tipo en Content/Textures, Sounds, etc.) ---
  const std::string BACKGROUND_TEXTURE_FILENAME = "PurpleBackground.png";

  const std::string BOUNCE_SOUND_FILENAME = "Bonus.mp3"; // Sonido al rebotar en la paleta
  const float BOUNCE_SOUND_VOLUME = 10; // Volumen del efecto (0 a 100)

  const std::string MUSIC_FILENAME = "Music.mp3"; // Música de fondo
  const float MUSIC_VOLUME = 5; // Volumen de la música (0 a 100)
  const float MUSIC_PITCH = 1; // Tono de la música (1 = velocidad/tono original)
} // namespace Bounce
