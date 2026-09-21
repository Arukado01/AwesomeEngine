#pragma once

#include "Types/BounceTypes.h"

// Escena del juego Bounce
namespace Bounce {
  class Game : public Scene {
  private:
    // --- Estado del juego ---
    Paddle paddle; // La paleta del jugador
    std::vector<Ball> balls; // Pelotas activas (puede haber varias simultáneas)
    Cooldown ballSpawnCooldown; // Temporizador: cada X segundos aparece una pelota
    Stats stats; // Puntaje, récord, vidas y sus textos
    sf::RectangleShape background; // Fondo con textura, cubre toda la ventana
    sf::Sound bounceSound; // Efecto al rebotar en la paleta
    sf::Music music; // Música de fondo

  public:
    Game(EngineContext &);

    // --- Ciclo de vida de la escena (heredado de Scene) ---
    void Start();
    void Update();
    void Render() const;
    void OnPause(bool);
    void OnCleanup();

  private:
    // --- Inicialización: prepara las piezas (se llama una vez, en el constructor) ---
    void InitPaddle();
    void InitStats();
    void InitBackground();
    void InitBounceSound();
    void InitMusic();

    // Asocia acciones (MoveLeft/MoveRight) a teclado y joystick
    void BindInputs();

    // --- Inicio de partida: coloca piezas y resetea estado (corre en cada Start) ---
    void StartPaddle();
    void StartStats();
    void StartMusic();

    // --- Actualización por frame ---
    void UpdatePaddle(); // Mueve la paleta según input y la mantiene dentro de la ventana
    void UpdateBalls(); // Itera todas las pelotas
    void UpdateBall(Ball &ball); // Física de una pelota: movimiento y rebotes en paredes/techo

    // --- Eventos de juego ---
    void EventBallSpawn(); // Crea una nueva pelota con dirección aleatoria
    void EventBallsMissed(int ballsMissed); // Resta vidas; si quedan 0, guarda récord y reinicia

    // --- Colisiones ---
    void HandleCollisions(); // Coordina las dos fases de colisión
    void HandleCollisionsPaddleBalls(); // Detecta pelota que baja y toca la paleta
    void ResolveCollisionPaddleBall(Ball &ball); // Responde al rebote: invertir, sonido, +1 punto
    void HandleCollisionsBallsMap(); // Elimina las pelotas que cayeron fuera (piso)
  };
} // namespace Bounce
