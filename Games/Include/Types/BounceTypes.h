#pragma once

#include "Config/BounceConfig.h"

namespace Bounce {
  // Acciones lógicas del juego. Se asocian a dispositivos (teclado, joystick)
  enum Action {
    MoveLeft,
    MoveRight
  };

  // Paleta controlada por el jugador.
  struct Paddle {
    sf::RectangleShape shape; // Forma
    float speed; // Velocidad de movimiento, en píxeles por segundo
  };

  // Pelota que rebota por la pantalla.
  struct Ball {
    sf::CircleShape shape; // Forma
    sf::Vector2f direction; // Dirección unitaria del movimiento (solo orienta, el módulo no importa)
    float speed; // Velocidad, en píxeles por segundo.
  };

  // Puntaje y HUD.
  struct Stats {
    int score; // Puntos: se suma 1 por cada rebote en la paleta
    int highScore; // Récord histórico, se carga y guarda vía ctx.save (Save.json)
    int lives; // Vidas restantes; al llegar a 0 la escena se reinicia

    // Textos en pantalla. Se construyen con la fuente compartida del motor
    sf::Text scoreText{GetDefaultFont()};
    sf::Text highScoreText{GetDefaultFont()};
    sf::Text livesText{GetDefaultFont()};
  };
} // namespace Bounce
