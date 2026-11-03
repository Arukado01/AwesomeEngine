#include "Bounce.h"
#include "Config/BounceConfig.h"
#include "SFML/Window/Joystick.hpp"
#include "SFML/Window/Keyboard.hpp"
#include "Scene/SceneUtils.h"
#include "Types/BounceTypes.h"
#include "Utils/InputBindings.h"
#include <string>

using namespace Bounce;

Game::Game(EngineContext &context)
    : Scene(context),
      ballSpawnCooldown(BALL_SPAWN_COOLDOWN_DURATION), // Temporizador de 2s entre pelotas
      audioGroup(context.audio.CreateGroup()) {
  ballSpawnCooldown.Bind(ctx.time);
  InitPaddle();
  InitStats();
  InitBackground();
  InitBounceSound();
  InitMusic();
}

// Configura la apariencia y velocidad de la paleta.
void Game::InitPaddle() {
  paddle.shape.setFillColor(PADDLE_COLOR);
  paddle.shape.setSize(PADDLE_SIZE);
  paddle.shape.setOrigin(paddle.shape.getGeometricCenter()); // Hace que "position" sea el CENTRO de la figura

  paddle.speed = PADDLE_SPEED;
}

// Coloca los 3 textos del HUD en columna, en la esquina superior izquierda.
void Game::InitStats() {
  stats.scoreText.setFillColor(STATS_SCORE_TEXT_COLOR);
  stats.scoreText.setPosition({10, 10});

  stats.highScoreText.setFillColor(STATS_HIGH_SCORE_TEXT_COLOR);
  stats.highScoreText.setPosition({10, 60});

  stats.livesText.setFillColor(STATS_LIVES_TEXT_COLOR);
  stats.livesText.setPosition({10, 110});
}

// Fondo con textura del tamaño exacto de la ventana (lo cubre por completo).
void Game::InitBackground() {
  background.setTexture(ctx.resources.FetchTexture(BACKGROUND_TEXTURE_FILENAME));
  background.setSize(gConfig.windowSize);
}

void Game::InitBounceSound() {
  bounceSound = audioGroup.AddSound(ctx.resources.AcquireSound(BOUNCE_SOUND_FILENAME), BOUNCE_SOUND_VOLUME);
}

// Música en loop infinito, con el tono original (pitch 1).
void Game::InitMusic() {
  if (auto track = ctx.resources.FetchMusic(MUSIC_FILENAME)) {
    music = audioGroup.AddMusic(std::move(*track), MUSIC_VOLUME, true);
    music->setPitch(MUSIC_PITCH);
  }
}

// Se llama cada vez que la escena (re)empieza, incluyendo el reinicio tras perder.
void Game::Start() {
  audioGroup.StopAll();
  ctx.cursor.SetVisible(false); // El mouse no se usa en este juego

  BindInputs();

  balls.clear(); // Elimina pelotas de una partida anterior

  StartPaddle();
  StartStats();
  StartMusic();

  ballSpawnCooldown.Restart(); // Arranca la cuenta de 2s hasta la primera pelota
}

// Enlaza acciones lógicas con dispositivos:
//   Teclado: A (izquierda) y D (derecha)
//   Joystick: eje X, con deadzone de 0.25 para ignorar el ruido del stick centrado
void Game::BindInputs() {
  ctx.input.Bind(MoveLeft, Input::Keyboard{sf::Keyboard::Scan::A});
  ctx.input.Bind(MoveLeft, Input::Axis{sf::Joystick::Axis::X, -0.25f});

  ctx.input.Bind(MoveRight, Input::Keyboard{sf::Keyboard::Scan::D});
  ctx.input.Bind(MoveRight, Input::Axis{sf::Joystick::Axis::X, 0.25f});
}

// Pone la paleta al centro horizontal, cerca del borde inferior (90% de altura).
// Los factores 0.50/0.90 hacen que la posición escale con el tamaño de la ventana.
void Game::StartPaddle() {
  paddle.shape.setPosition(gConfig.windowSize.componentWiseMul({0.50f, 0.90f}));
}

// Resetea el estado del HUD. El récord se recupera del archivo de guardado
// (Content/Save.json, clave "Bounce:High Score"); si no existe, devuelve 0.
void Game::StartStats() {
  stats.score = 0;
  stats.scoreText.setString("Score: 0");

  stats.highScore = ctx.save.Get<int>(STATS_HIGH_SCORE_KEY);
  stats.highScoreText.setString("High: " + std::to_string(stats.highScore));

  stats.lives = PADDLE_LIVES;
  stats.livesText.setString("Lives: " + std::to_string(stats.lives));
}

void Game::StartMusic() {
  if (music) {
    audioGroup.Play(*music);
  }
}

// Corre cada frame
void Game::Update() {
  if (ballSpawnCooldown.IsOver()) { // ¿Ya pasaron los 2 segundos?
    EventBallSpawn(); // -> nueva pelota
    ballSpawnCooldown.Restart(); // -> reinicia la cuenta para la siguiente
  }

  UpdatePaddle();
  UpdateBalls();

  HandleCollisions();
}

// Mueve la paleta según el input y evita que salga de la ventana.
// Patrón: guarda la posición previa; si el movimiento dejó la figura
// parcialmente fuera, hace rollback a esa posición.
void Game::UpdatePaddle() {
  sf::Vector2f lastPosition = paddle.shape.getPosition();
  paddle.previousBounds = paddle.shape.getGlobalBounds();

  if (ctx.input.Pressed(MoveLeft)) {
    paddle.shape.move({-paddle.speed * ctx.time.GetDeltaTime(), 0});
  }
  if (ctx.input.Pressed(MoveRight)) {
    paddle.shape.move({paddle.speed * ctx.time.GetDeltaTime(), 0});
  }

  if (IsOutsideWindowLeft(paddle.shape) || IsOutsideWindowRight(paddle.shape)) {
    paddle.shape.setPosition(lastPosition);
  }
}

void Game::UpdateBalls() {
  for (auto &ball : balls) {
    UpdateBall(ball);
  }
}

// Física de una pelota: movimiento + rebotes.
// El desplazamiento usa deltaTime para que la velocidad (px/s) sea la misma
// a 30 o a 144 FPS. El rebote se logra invirtiendo un componente de la
// dirección (izq/der -> x, techo -> y). El setPosition(lastPosition) evita
// que la pelota se atraviese la pared si avanzó de más en un frame.
void Game::UpdateBall(Ball &ball) {
  sf::Vector2f lastPosition = ball.shape.getPosition();
  ball.previousBounds = ball.shape.getGlobalBounds();

  ball.shape.move(ball.direction * ball.speed * ctx.time.GetDeltaTime());

  if (IsOutsideWindowLeft(ball.shape) || IsOutsideWindowRight(ball.shape)) {
    ball.direction.x *= -1;
    ball.shape.setPosition(lastPosition);
  }

  if (IsOutsideWindowTop(ball.shape)) {
    ball.direction.y *= -1;
    ball.shape.setPosition(lastPosition);
  }
}

// Crea una pelota nueva: centrada horizontalmente, al 25% de la altura,
// con dirección aleatoria (vector unitario en un ángulo de 0 a 360 grados).
void Game::EventBallSpawn() {
  auto &ball = balls.emplace_back();

  ball.shape.setFillColor((BALL_COLOR));
  ball.shape.setRadius(BALL_RADIUS);
  ball.shape.setOrigin(ball.shape.getGeometricCenter());
  ball.shape.setPosition(gConfig.windowSize.componentWiseMul({0.50f, 0.25f}));

  ball.direction = {1, ctx.random.Angle(sf::Angle::Zero, sf::degrees(360))};
  ball.speed = BALL_SPEED;
}

// Cada pelota perdida resta 1 vida. Al llegar a 0 vidas: guarda el récord, reinicia la escena.
void Game::EventBallsMissed(int ballsMissed) {
  stats.lives -= ballsMissed;
  stats.livesText.setString("Lives: " + std::to_string(stats.lives));

  if (stats.lives <= 0) {
    ctx.save.Set(STATS_HIGH_SCORE_KEY, std::max(stats.score, stats.highScore));

    LOG_INFO("You Lose! Score: {}", stats.score);
    ctx.scenes.RestartCurrentScene();
  }
}

// Coordina la detección de colisiones en dos fases.
void Game::HandleCollisions() {
  HandleCollisionsPaddleBalls();
  HandleCollisionsBallsMap();
}

/* Detecta el contacto de cada pelota con la paleta, incluso si la atravesó durante el frame. */
void Game::HandleCollisionsPaddleBalls() {
  for (auto &ball : balls) {
    // Solo pueden rebotar en la paleta las pelotas que están bajando.
    if (ball.direction.y <= 0) {
      continue;
    }
    const auto bounds = ball.shape.getGlobalBounds();
    // Compara las posiciones anteriores y actuales de la pelota y la paleta.
    const auto hit = ctx.collisions.Sweep(ball.previousBounds, bounds, paddle.previousBounds, paddle.shape.getGlobalBounds());
    const bool overlaps = ctx.collisions.Intersect(bounds, paddle.shape.getGlobalBounds()).has_value();
    if (hit && !hit->initialOverlap && hit->normal.y < 0 && !overlaps) {
      // Si atravesó la paleta en un frame, vuelve al punto de impacto antes de rebotar.
      const auto contact = ball.previousBounds.position + (bounds.position - ball.previousBounds.position) * hit->time;
      ball.shape.setPosition(contact + ball.shape.getOrigin());
    }
    // Conserva el rebote por solapamiento y añade el impacto por arriba detectado durante el movimiento.
    if (overlaps || (hit && !hit->initialOverlap && hit->normal.y < 0)) {
      ResolveCollisionPaddleBall(ball);
    }
  }
}

// Respuesta al rebote en la paleta: invierte el movimiento vertical, reproduce el efecto y suma 1 punto.
void Game::ResolveCollisionPaddleBall(Ball &ball) {
  ball.direction.y *= -1;
  if (bounceSound) {
    audioGroup.Play(*bounceSound, true);
  }

  stats.score++;
  stats.scoreText.setString("Score: " + std::to_string(stats.score));
}

// Pelotas perdidas: borra del vector las que cruzaron el borde inferior.
void Game::HandleCollisionsBallsMap() {
  int ballsMissed = (int)std::erase_if(balls, [](const Ball &ball) {
    return IsOutsideWindowBottom(ball.shape);
  });

  if (ballsMissed > 0) {
    EventBallsMissed(ballsMissed);
  }
}

// Dibujo por capas. Fondo -> paleta -> pelotas -> HUD.
void Game::Render() const {
  ctx.renderer.Draw(background);

  ctx.renderer.Draw(paddle.shape);

  for (const auto &ball : balls) {
    ctx.renderer.Draw(ball.shape);
  }

  ctx.renderer.Draw(stats.scoreText);
  ctx.renderer.Draw(stats.highScoreText);
  ctx.renderer.Draw(stats.livesText);
}

// Pausa y reanuda el audio que estaba activo en la escena.
void Game::OnPause(bool paused) {
  audioGroup.SetPaused(paused);
}

// Al salir de la escena detiene el audio.
void Game::OnCleanup() {
  audioGroup.StopAll();
}
