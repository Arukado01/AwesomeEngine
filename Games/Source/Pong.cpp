#include "Pong.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <utility>

namespace Pong {
  /* Prepara las figuras y carga las dos fuentes de audio una vez al crear la escena. */
  Game::Game(EngineContext &context)
      : Scene(context), audioGroup(context.audio.CreateGroup()) {
    InitPlayer(player1, 0, PLAYER_1_COLOR);
    InitPlayer(player2, 1, PLAYER_2_COLOR);

    ball.shape.setRadius(BALL_RADIUS);
    ball.shape.setOrigin(ball.shape.getGeometricCenter());
    ball.shape.setFillColor(BALL_COLOR);

    // El gestor busca la música en Content/Musics. El tercer argumento activa el bucle.
    if (auto track = ctx.resources.FetchMusic(MUSIC_FILENAME)) {
      music = audioGroup.AddMusic(std::move(*track), MUSIC_VOLUME, true);
    }

    // AcquireSound busca en Content/Sounds y comparte el audio con el grupo.
    bounceSound = audioGroup.AddSound(
        ctx.resources.AcquireSound(BOUNCE_FILENAME), BOUNCE_VOLUME);
  }

  /* Configura el aspecto de una paleta y el mando que corresponde a su lado. */
  void Game::InitPlayer(Player &player, unsigned int joystickId, sf::Color color) {
    player.joystickId = joystickId;
    player.shape.setSize(PLAYER_SIZE);
    player.shape.setOrigin(player.shape.getGeometricCenter());
    player.shape.setFillColor(color);
    player.stats.livesText.setCharacterSize(26);
    player.stats.livesText.setFillColor(color);
  }

  /* Al entrar en la escena se abre el menú y se registran los controles. */
  void Game::Start() {
    ctx.cursor.SetVisible(false);
    BindInputs();
    screen = Screen::Menu;
  }

  /* Registra las teclas que se consultan continuamente durante la partida. */
  void Game::BindInputs() {
    using Key = sf::Keyboard::Scan;

    ctx.input.Bind(Player1MoveUp, Input::Keyboard{Key::W});
    ctx.input.Bind(Player1MoveDown, Input::Keyboard{Key::S});
    ctx.input.Bind(Player2MoveUp, Input::Keyboard{Key::Up});
    ctx.input.Bind(Player2MoveDown, Input::Keyboard{Key::Down});
  }

  /* Lee pulsaciones individuales para elegir opciones o reiniciar. */
  void Game::OnEvent(const sf::Event &event) {
    const auto *key = event.getIf<sf::Event::KeyPressed>();
    if (!key) {
      return;
    }

    using Key = sf::Keyboard::Scan;
    if (screen == Screen::Menu) {
      switch (key->scancode) {
      case Key::W:
      case Key::Up:
        menuSelection = 0;
        break;
      case Key::S:
      case Key::Down:
        menuSelection = 1;
        break;
      case Key::Left:
        startingLives = std::max(MIN_LIVES, startingLives - 1);
        break;
      case Key::Right:
        startingLives = std::min(MAX_LIVES, startingLives + 1);
        break;
      case Key::Enter:
        mode = menuSelection == 0 ? Mode::VersusAI : Mode::VersusPlayer;
        StartMatch();
        break;
      default:
        break;
      }
    } else if (screen == Screen::GameOver) {
      if (key->scancode == Key::Enter) {
        StartMatch();
      } else if (key->scancode == Key::M) {
        screen = Screen::Menu;
      }
    }
  }

  /* Arranca una partida nueva con el modo y las vidas que eligió el jugador. */
  void Game::StartMatch() {
    const auto center = gConfig.windowSize / 2.f;
    const float halfWidth = PLAYER_SIZE.x / 2.f;
    player1.shape.setPosition({halfWidth, center.y});
    player2.shape.setPosition({gConfig.windowSize.x - halfWidth, center.y});

    player1.stats.lives = startingLives;
    player2.stats.lives = startingLives;
    UpdateLivesText(player1);
    UpdateLivesText(player2);
    EventBallReset();
    screen = Screen::Playing;

    if (music) {
      audioGroup.Play(*music);
    }
  }

  /* Actualiza el texto y alinea el contador derecho con su margen. */
  void Game::UpdateLivesText(Player &player) {
    auto &text = player.stats.livesText;
    text.setString("Vidas: " + std::to_string(player.stats.lives));

    constexpr float margin = 56.f;
    if (&player == &player1) {
      text.setPosition({margin, 24.f});
    } else {
      // El ancho se mide después de poner el número para alinear bien cualquier dígito.
      const auto bounds = text.getLocalBounds();
      text.setPosition({gConfig.windowSize.x - margin - bounds.position.x - bounds.size.x, 24.f});
    }
  }

  /* Centra la pelota y elige el ángulo y el lado del siguiente saque. */
  void Game::EventBallReset() {
    ball.shape.setPosition(gConfig.windowSize / 2.f);
    const auto angle = ctx.random.Angle(-BALL_ANGLE_MAX, BALL_ANGLE_MAX);
    const float side = ctx.random.Bool() ? 1.f : -1.f;
    ball.direction = {side * std::cos(angle.asRadians()), std::sin(angle.asRadians())};
    ball.speed = BALL_SPEED;
  }

  /* Solo avanza la simulación cuando hay una partida en curso. */
  void Game::Update() {
    if (screen != Screen::Playing) {
      return;
    }

    UpdatePlayer(player1, Player1MoveUp, Player1MoveDown);
    if (mode == Mode::VersusPlayer) {
      UpdatePlayer(player2, Player2MoveUp, Player2MoveDown);
    } else {
      UpdatePlayerAI(player2);
    }

    ball.shape.move(ball.direction * ball.speed * ctx.time.GetDeltaTime());
    HandlePaddleCollisions();
    HandleCourtBoundaries();
  }

  /* Usa el mando asignado cuando se mueve su eje vertical y el teclado cuando está quieto. */
  void Game::UpdatePlayer(Player &player, Action up, Action down) {
    float axis = 0.f;
    if (sf::Joystick::isConnected(player.joystickId) &&
        sf::Joystick::hasAxis(player.joystickId, sf::Joystick::Axis::Y)) {
      // SFML entrega el eje entre -100 y 100; lo convertimos a un valor entre -1 y 1.
      axis = sf::Joystick::getAxisPosition(player.joystickId, sf::Joystick::Axis::Y) / 100.f;
    }

    if (std::abs(axis) <= gConfig.joystickDeadzone) {
      axis = (ctx.input.Pressed(down) ? 1.f : 0.f) -
             (ctx.input.Pressed(up) ? 1.f : 0.f);
    }
    MovePlayer(player, axis * PLAYER_SPEED);
  }

  /* Acerca la paleta de la máquina a la pelota sin superar la velocidad de una paleta. */
  void Game::UpdatePlayerAI(Player &player) {
    const float distance = ball.shape.getPosition().y - player.shape.getPosition().y;
    const float velocity = std::clamp(
        distance * PLAYER_AI_SPEED_FACTOR, -PLAYER_SPEED, PLAYER_SPEED);
    MovePlayer(player, velocity);
  }

  /* Aplica el movimiento vertical y limita el centro para que la paleta quede visible. */
  void Game::MovePlayer(Player &player, float velocityY) {
    auto position = player.shape.getPosition();
    const float halfHeight = PLAYER_SIZE.y / 2.f;
    position.y = std::clamp(position.y + velocityY * ctx.time.GetDeltaTime(),
                            halfHeight, gConfig.windowSize.y - halfHeight);
    player.shape.setPosition(position);
  }

  /* Comprueba solo la paleta hacia la que va la pelota para evitar dos rebotes seguidos. */
  void Game::HandlePaddleCollisions() {
    const auto bounds = ball.shape.getGlobalBounds();
    if (ball.direction.x < 0.f &&
        ctx.collisions.Intersect(bounds, player1.shape.getGlobalBounds())) {
      BounceFromPaddle(player1, 1.f);
    } else if (ball.direction.x > 0.f &&
               ctx.collisions.Intersect(bounds, player2.shape.getGlobalBounds())) {
      BounceFromPaddle(player2, -1.f);
    }
  }

  /* Cambia el ángulo según dónde impactó la pelota y la separa de la cara de la paleta. */
  void Game::BounceFromPaddle(const Player &player, float directionX) {
    auto position = ball.shape.getPosition();
    const auto center = player.shape.getPosition();
    const float relativeHit = std::clamp(
        (position.y - center.y) / (PLAYER_SIZE.y / 2.f), -1.f, 1.f);
    const float angle = relativeHit * BALL_ANGLE_MAX.asRadians();

    // Un impacto centrado sale recto; cuanto más cerca del borde, mayor es el ángulo.
    ball.direction = {directionX * std::cos(angle), std::sin(angle)};
    position.x = center.x + directionX * (PLAYER_SIZE.x / 2.f + BALL_RADIUS);
    ball.shape.setPosition(position);
    ball.speed = std::min(ball.speed * BALL_SPEED_BONUS_FACTOR, BALL_SPEED_MAX);
    PlayBounce();
  }

  /* Hace rebotar la pelota arriba o abajo y comprueba si salió por un lado. */
  void Game::HandleCourtBoundaries() {
    auto position = ball.shape.getPosition();
    const float top = BALL_RADIUS;
    const float bottom = gConfig.windowSize.y - BALL_RADIUS;
    const bool hitTop = position.y <= top && ball.direction.y < 0.f;
    const bool hitBottom = position.y >= bottom && ball.direction.y > 0.f;

    if (hitTop || hitBottom) {
      position.y = hitTop ? top : bottom;
      ball.shape.setPosition(position);
      ball.direction.y = -ball.direction.y;
      PlayBounce();
    }

    // Descuenta la vida del lado por el que salió la pelota completa.
    if (position.x + BALL_RADIUS <= 0.f) {
      EventLifeLost(player1);
    } else if (position.x - BALL_RADIUS >= gConfig.windowSize.x) {
      EventLifeLost(player2);
    }
  }

  /* Descuenta una vida y decide si corresponde otro saque o mostrar el resultado. */
  void Game::EventLifeLost(Player &loser) {
    --loser.stats.lives;
    UpdateLivesText(loser);

    if (loser.stats.lives > 0) {
      EventBallReset();
      return;
    }

    const bool player1Lost = &loser == &player1;
    resultColor = player1Lost ? PLAYER_2_COLOR : PLAYER_1_COLOR;
    if (mode == Mode::VersusAI) {
      resultMessage = player1Lost ? L"Gana la máquina" : L"Ganaste";
    } else {
      resultMessage = player1Lost ? L"Gana el jugador 2" : L"Gana el jugador 1";
    }

    screen = Screen::GameOver;
    audioGroup.Stop(AudioChannel::Music);
  }

  /* Reproduce el efecto desde el comienzo en cada impacto. */
  void Game::PlayBounce() {
    if (bounceSound) {
      audioGroup.Play(*bounceSound, true);
    }
  }

  /* Hace que el audio siga la pausa del motor y la pérdida de foco. */
  void Game::OnPause(bool paused) {
    audioGroup.SetPaused(paused);
  }

  /* Detiene el audio cuando el motor cambia o reinicia la escena. */
  void Game::OnCleanup() {
    audioGroup.StopAll();
  }

  /* Dibuja el menú, la cancha o la cancha congelada con el resultado encima. */
  void Game::Render() const {
    if (screen == Screen::Menu) {
      RenderMenu();
      return;
    }

    RenderMap();
    ctx.renderer.Draw(player1.shape);
    ctx.renderer.Draw(player2.shape);
    ctx.renderer.Draw(ball.shape);
    ctx.renderer.Draw(player1.stats.livesText);
    ctx.renderer.Draw(player2.stats.livesText);

    if (screen == Screen::GameOver) {
      RenderGameOver();
    }
  }

  /* Dibuja la línea discontinua que separa las dos mitades de la cancha. */
  void Game::RenderMap() const {
    sf::RectangleShape segment(MAP_LINE_SEGMENT_SIZE);
    segment.setOrigin(segment.getGeometricCenter());
    segment.setFillColor(MAP_LINE_SEGMENT_COLOR);

    for (float y = MAP_LINE_SEGMENT_SIZE.y; y < gConfig.windowSize.y;
         y += MAP_LINE_SEGMENT_SIZE.y * 2.f) {
      segment.setPosition({gConfig.windowSize.x / 2.f, y});
      ctx.renderer.Draw(segment);
    }
  }

  /* Superpone una capa oscura y centra un texto en la pantalla. */
  void Game::RenderMenu() const {
    sf::RectangleShape backdrop(gConfig.windowSize);
    backdrop.setFillColor(SCREEN_BACKDROP_COLOR);
    ctx.renderer.Draw(backdrop);

    const auto drawCentered = [this](const sf::String &value, unsigned int size,
                                     float yRatio, sf::Color color) {
      sf::Text text(GetDefaultFont(), value, size);
      text.setFillColor(color);
      // La posición y el tamaño del texto cambian con su contenido.
      text.setOrigin(text.getLocalBounds().getCenter());
      text.setPosition({gConfig.windowSize.x / 2.f, gConfig.windowSize.y * yRatio});
      ctx.renderer.Draw(text);
    };

    drawCentered(L"P O N G", MENU_TITLE_SIZE, 0.20f, MENU_TITLE_COLOR);

    sf::RectangleShape divider({260.f, 3.f});
    sf::Color dividerColor = MENU_ACCENT_COLOR;
    dividerColor.a = 110;
    divider.setFillColor(dividerColor);
    divider.setOrigin(divider.getGeometricCenter());
    divider.setPosition({gConfig.windowSize.x / 2.f, gConfig.windowSize.y * 0.28f});
    ctx.renderer.Draw(divider);

    // El modo elegido se resalta con el acento y marcadores de selección.
    const bool aiSelected = menuSelection == 0;
    drawCentered(aiSelected ? L"> 1 JUGADOR <" : L"1 JUGADOR", MENU_OPTION_SIZE, 0.42f,
                 aiSelected ? MENU_ACCENT_COLOR : MENU_OPTION_COLOR);
    drawCentered(aiSelected ? L"2 JUGADORES" : L"> 2 JUGADORES <", MENU_OPTION_SIZE, 0.50f,
                 aiSelected ? MENU_OPTION_COLOR : MENU_ACCENT_COLOR);

    drawCentered(L"VIDAS", 22, 0.60f, MENU_HINT_COLOR);
    drawCentered(L"< " + std::to_wstring(startingLives) + L" >", MENU_LIVES_SIZE, 0.67f,
                 MENU_ACCENT_COLOR);

    // El aviso de inicio parpadea suavemente con el tiempo real.
    const float blink = 140.f + 115.f * std::sin(MENU_BLINK_SPEED * ctx.time.GetElapsedTime());
    sf::Color playColor = MENU_ACCENT_COLOR;
    playColor.a = static_cast<std::uint8_t>(blink);
    drawCentered(L"ENTER = JUGAR", 18, 0.85f, playColor);
  }

  /* Congela la cancha y muestra el resultado con el color del ganador. */
  void Game::RenderGameOver() const {
    sf::RectangleShape backdrop(gConfig.windowSize);
    backdrop.setFillColor(SCREEN_BACKDROP_COLOR);
    ctx.renderer.Draw(backdrop);

    const auto drawCentered = [this](const sf::String &value, unsigned int size,
                                     float yRatio, sf::Color color) {
      sf::Text text(GetDefaultFont(), value, size);
      text.setFillColor(color);
      text.setOrigin(text.getLocalBounds().getCenter());
      text.setPosition({gConfig.windowSize.x / 2.f, gConfig.windowSize.y * yRatio});
      ctx.renderer.Draw(text);
    };

    drawCentered(L"GAME OVER", SCREEN_TITLE_SIZE, 0.30f, MENU_HINT_COLOR);
    drawCentered(resultMessage, SCREEN_MESSAGE_SIZE, 0.45f, resultColor);
    drawCentered(L"ENTER = REVANCHA      M = MENÚ", SCREEN_HINT_SIZE, 0.62f, MENU_HINT_COLOR);
  }
} // namespace Pong
