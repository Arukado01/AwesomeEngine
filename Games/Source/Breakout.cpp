#include "Breakout.h"

#include <SFML/Window/Joystick.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <magic_enum/magic_enum.hpp>
#include <cmath>
#include <string>

using namespace Breakout;

/* Prepara la escena: paleta, efecto de rebote y música de fondo. */
Game::Game(EngineContext &context) : Scene(context),
                                     audioGroup(context.audio.CreateGroup()) {
  InitPlayer();
  InitBounceSound();
  InitMusic();
}

/* Configura la apariencia de la paleta: textura con tinte, tamaño proporcional
   a la ventana, origen centrado y velocidad de desplazamiento. */
void Game::InitPlayer() {
  player.shape.setTexture(ctx.resources.FetchTexture(PLAYER_TEXTURE_FILENAME));
  player.shape.setFillColor(PLAYER_COLOR);
  player.shape.setSize(gConfig.windowSize.componentWiseMul({0.12f, 0.03f}));
  player.shape.setOrigin(player.shape.getGeometricCenter()); // "position" pasa a ser el CENTRO de la figura

  player.speed = PLAYER_SPEED;
}

/* El gestor busca el efecto en Content/Sounds y comparte el buffer con el grupo. */
void Game::InitBounceSound() {
  bounceSound = audioGroup.AddSound(ctx.resources.AcquireSound(BOUNCE_FILENAME), BOUNCE_VOLUME);
}

/* El gestor busca la pista en Content/Musics. El tercer argumento activa el bucle. */
void Game::InitMusic() {
  if (auto track = ctx.resources.FetchMusic(MUSIC_FILENAME)) {
    music = audioGroup.AddMusic(std::move(*track), MUSIC_VOLUME, true);
  }
}

/* Enlaza acciones lógicas con dispositivos. Teclado: A (izquierda) y D (derecha).
   Joystick 0: eje X, con deadzone de 0.25 para ignorar el ruido del stick centrado. */
void Game::BindInputs() {
  ctx.input.Bind(MoveLeft, Input::Keyboard{sf::Keyboard::Scan::A});
  ctx.input.Bind(MoveLeft, Input::Axis{sf::Joystick::Axis::X, -0.25f});

  ctx.input.Bind(MoveRight, Input::Keyboard{sf::Keyboard::Scan::D});
  ctx.input.Bind(MoveRight, Input::Axis{sf::Joystick::Axis::X, 0.25f});
}

/* En las pantallas de resultado, Enter lanza una partida nueva. */
void Game::OnEvent(const sf::Event &event) {
  if (screen == Screen::Playing) {
    return;
  }

  if (const auto *key = event.getIf<sf::Event::KeyPressed>();
      key && key->scancode == sf::Keyboard::Scan::Enter) {
    ctx.scenes.RestartCurrentScene();
  }
}

/* Se llama cada vez que la escena (re)empieza: limpia entidades, arma el
   tablero, lanza la primera pelota y arranca la música. */
void Game::Start() {
  audioGroup.StopAll();
  ctx.cursor.SetVisible(false); // El puntero no se usa: la paleta responde a A/D y al mando

  screen = Screen::Playing;
  BindInputs();

  balls.clear(); // Pelotas de una partida anterior
  bonuses.clear(); // Bonos vencidos de una partida anterior

  StartPlayer();
  StartBricks();

  EventBallSpawn();

  if (music) {
    audioGroup.Play(*music);
  }
}

/* Coloca la paleta al centro horizontal, cerca del borde inferior (90% de la
   altura), y reinicia su estado, puntuación y vidas. */
void Game::StartPlayer() {
  player.shape.setPosition(gConfig.windowSize.componentWiseMul({0.50f, 0.90f}));
  player.shape.setScale({1, 1}); // Deshace el cambio de tamaño de los bonos Expand/Shrink

  player.magnetic = false;

  player.stats.score = 0;
  UpdateScoreText();

  // El récord sobrevive a las partidas: se recupera del archivo de guardado.
  player.stats.highScore = ctx.save.Get<int>(STATS_HIGH_SCORE_KEY, 0);
  UpdateHighScoreText();

  player.stats.lives = PLAYER_LIVES;
  UpdateLivesText();
}

/* Reconstruye el muro: una cuadrícula de BRICK_MAP_SIZE con una vida inicial aleatoria por ladrillo. */
void Game::StartBricks() {
  bricks.clear();
  bricks.reserve(BRICK_MAP_SIZE.y * BRICK_MAP_SIZE.x);

  for (int i = 0; i < BRICK_MAP_SIZE.y; i++) {
    for (int j = 0; j < BRICK_MAP_SIZE.x; j++) {
      auto &brick = bricks.emplace_back();

      StartBrick(brick, {j, i});
    }
  }
}

/* Da a un ladrillo su vida (que define el color), tamaño y posición según su celda. */
void Game::StartBrick(Brick &brick, sf::Vector2i tile) {
  brick.lives = ctx.random.Int(1, BRICK_LIVES_MAX);

  brick.shape.setFillColor(BRICK_COLORS[brick.lives - 1]);
  brick.shape.setOutlineColor(sf::Color::Black);
  brick.shape.setOutlineThickness(-1);
  brick.shape.setSize({gConfig.windowSize.x / BRICK_MAP_SIZE.x,
                       gConfig.windowSize.y / (BRICK_MAP_SIZE.y * 4.5f)}); // Ocupa solo la franja superior
  brick.shape.setPosition(sf::Vector2f(tile).componentWiseMul(brick.shape.getSize()));
}

/* Lógica por frame: paleta, pelotas, bonos y colisiones. La simulación se
   congela mientras se muestre una pantalla de resultado. */
void Game::Update() {
  if (screen != Screen::Playing) {
    return;
  }

  UpdatePlayer();
  UpdateBalls();
  UpdateBonuses();

  HandleCollisions();
}

/* Mueve la paleta en horizontal: usa el eje X del mando cuando el stick se
   aparta del centro y, si está quieto, las teclas A/D. El desplazamiento se
   multiplica por deltaTime para que la velocidad (px/s) no dependa de los FPS.
   El chequeo de bordes impide que salga de la ventana. */
void Game::UpdatePlayer() {
  float axis = 0.f;
  if (sf::Joystick::isConnected(0) && sf::Joystick::hasAxis(0, sf::Joystick::Axis::X)) {
    // SFML entrega el eje entre -100 y 100; se convierte a un valor entre -1 y 1.
    axis = sf::Joystick::getAxisPosition(0, sf::Joystick::Axis::X) / 100.f;
  }

  if (std::abs(axis) <= gConfig.joystickDeadzone) {
    axis = (ctx.input.Pressed(MoveRight) ? 1.f : 0.f) -
           (ctx.input.Pressed(MoveLeft) ? 1.f : 0.f);
  }

  player.shape.move({axis * player.speed * ctx.time.GetDeltaTime(), 0});

  sf::FloatRect bounds = player.shape.getGlobalBounds();
  if (IsOutsideWindowLeft(player.shape)) {
    player.shape.setPosition({bounds.size.x / 2, player.shape.getPosition().y});
  } else if (IsOutsideWindowRight(player.shape)) {
    player.shape.setPosition({gConfig.windowSize.x - bounds.size.x / 2, player.shape.getPosition().y});
  }
}

/* Actualiza cada pelota viva. */
void Game::UpdateBalls() {
  for (auto &ball : balls) {
    UpdateBall(ball);
  }
}

/* Física de una pelota: si el bono Imán está activo y la pelota baja cerca de
   la paleta, su dirección se sespa hacia la paleta. Luego avanza según su
   velocidad; el rebote contra paredes lo resuelve HandleCollisionsBallMap. */
void Game::UpdateBall(Ball &ball) {
  if (player.magnetic && ball.direction.y > 0 && IsBallNearPlayer(ball)) {
    sf::Vector2f attraction = player.shape.getPosition() - ball.shape.getPosition();
    ball.direction = (ball.direction + attraction.normalized()).normalized();
  }

  ball.shape.move(ball.direction * ball.speed * ctx.time.GetDeltaTime());
}

/* Cronometra la vida de cada bono activo: al vencer se desactiva y la lista se compacta. */
void Game::UpdateBonuses() {
  for (const auto &bonus : bonuses) {
    if (bonus.endCooldown.IsOver()) {
      EventBonusDisable(bonus);
    }
  }

  UpdateBonusesCleanup();
}

/* Retira los bonos vencidos y reordena los textos restantes para que no queden huecos. */
void Game::UpdateBonusesCleanup() {
  std::size_t erased = std::erase_if(bonuses, [](const Bonus &bonus) {
    return bonus.endCooldown.IsOver();
  });

  if (erased > 0) {
    for (std::size_t i = 0; i < bonuses.size(); i++) {
      bonuses[i].text.setPosition({gConfig.windowSize.x * 0.75f,
                                   gConfig.windowSize.y * 0.375f + BONUS_TEXT_OFFSET_Y * i});
    }
  }
}

/* Crea una pelota nueva sobre la paleta con las propiedades por defecto. */
void Game::EventBallSpawn() {
  auto &ball = balls.emplace_back();

  ball.shape.setFillColor(BALL_COLOR);
  ball.shape.setRadius(BALL_RADIUS);
  ball.shape.setOrigin(ball.shape.getGeometricCenter());

  ball.alive = true;

  EventBallReset(ball);
}

/* Lanza (o relanza) una pelota: posición sobre la paleta, ángulo aleatorio
   dentro de BALL_ANGLE_MAX y estado de bonos de fuego/invulnerabilidad limpio. */
void Game::EventBallReset(Ball &ball) {
  sf::Vector2f direction(1, ctx.random.Angle(-BALL_ANGLE_MAX, BALL_ANGLE_MAX));
  ball.direction = {direction.y, -direction.x}; // Se rota el vector para que apunte hacia arriba
  ball.speed = BALL_SPEED;

  sf::Vector2f offset(0, player.shape.getGlobalBounds().size.y);
  ball.shape.setPosition(player.shape.getPosition() - offset);

  ball.fire = false;
  ball.invincible = false;
}

/* Activa un bono aleatorio entre los compatibles con los que ya están en curso. */
void Game::EventBonusEnable() {
  std::vector<BonusType> types = GetBonusTypesAllowed();

  if (types.empty()) {
    return;
  }

  Bonus bonus;
  bonus.type = types[ctx.random.Int(0, (int)types.size() - 1)];
  bonus.endCooldown.SetDuration(BONUS_COOLDOWN_DURATION);

  // Cada bono aplica su efecto inmediato sobre paleta o pelotas.
  switch (bonus.type) {
  case ExpandPaddle:
    player.shape.setScale({1.5f, 1});
    break;
  case ShrinkPaddle:
    player.shape.setScale({0.5f, 1});
    break;
  case MagnetPaddle:
    player.magnetic = true;
    break;
  case AccelerateBall:
    for (Ball &ball : balls) {
      ball.speed = BALL_SPEED * 1.25f;
    }
    break;
  case DecelerateBall:
    for (Ball &ball : balls) {
      ball.speed = BALL_SPEED * 0.75f;
    }
    break;
  case MultiBall:
    EventBallSpawn();
    break;
  case FireBall:
    for (Ball &ball : balls) {
      ball.fire = true;
      ball.shape.setFillColor(BALL_FIRE_COLOR);
    }
    break;
  case InvincibleBall:
    for (Ball &ball : balls) {
      ball.invincible = true;
    }
    break;
  default:
    break;
  }

  // El nombre del bono se muestra en pantalla mientras dura el efecto.
  bonus.text.setCharacterSize(BONUS_TEXT_SIZE);
  bonus.text.setString(std::string(magic_enum::enum_name(bonus.type)));
  bonus.text.setPosition({gConfig.windowSize.x * 0.75f,
                          gConfig.windowSize.y * 0.375f + BONUS_TEXT_OFFSET_Y * bonuses.size()});

  bonuses.emplace_back(bonus);
}

/* Deshace el efecto del bono al expirar, devolviendo paleta y pelotas a su estado base. */
void Game::EventBonusDisable(const Bonus &bonus) {
  switch (bonus.type) {
  case ExpandPaddle:
  case ShrinkPaddle:
    player.shape.setScale({1, 1});
    break;
  case MagnetPaddle:
    player.magnetic = false;
    break;
  case AccelerateBall:
  case DecelerateBall:
    for (Ball &ball : balls) {
      ball.speed = BALL_SPEED;
    }
    break;
  case MultiBall:
    break; // No requiere deshacer nada: las pelotas extra siguen en juego
  case FireBall:
    for (Ball &ball : balls) {
      ball.fire = false;
      ball.shape.setFillColor(BALL_COLOR);
    }
    break;
  case InvincibleBall:
    for (Ball &ball : balls) {
      ball.invincible = false;
    }
    break;
  default:
    break;
  }
}

/* Coordina las colisiones y el fin de partida: al vaciarse el muro, gana el jugador. */
void Game::HandleCollisions() {
  HandleCollisionsBallsMap();

  std::size_t lastBricksCount = bricks.size();

  for (auto &ball : balls) {
    HandleCollisionsBallBricks(ball);
  }

  if (bricks.empty()) {
    LOG_INFO("You Win!");
    EventGameEnd(Screen::Win, L"¡GANASTE!", WIN_TITLE_COLOR);
    return;
  }

  // Cada ladrillo destruido habilita un bono nuevo, hasta el máximo por tipo.
  std::size_t newBonuses = lastBricksCount - bricks.size();

  for (std::size_t i = 0; i < newBonuses && (bonuses.size() < BonusType::Count); i++) {
    EventBonusEnable();
  }
}

/* Resuelve paredes y fondo de cada pelota, y retira las muertas del vector. */
void Game::HandleCollisionsBallsMap() {
  for (auto &ball : balls) {
    HandleCollisionsBallMap(ball);
  }

  std::erase_if(balls, [](const Ball &ball) { return !ball.alive; });
}

/* Rebotes de una pelota: contra la paleta (con sonido), contra paredes laterales
   y techo (con sonido), y pérdida de vida al caer por debajo de la paleta. */
void Game::HandleCollisionsBallMap(Ball &ball) {
  if (ball.direction.y > 0 && Intersects(ball.shape, player.shape)) {
    ResolveCollisionBallPlayer(ball);
  }

  if ((ball.direction.x < 0 && IsOutsideWindowLeft(ball.shape)) ||
      (ball.direction.x > 0 && IsOutsideWindowRight(ball.shape))) {
    ball.direction.x *= -1;
    PlayBounce();
  }

  if (ball.direction.y < 0 && IsOutsideWindowTop(ball.shape)) {
    ball.direction.y *= -1;
    PlayBounce();
  }

  // La pelota se pierde cuando baja más allá de la altura de la paleta.
  if (ball.direction.y > 0 && ball.shape.getPosition().y > player.shape.getPosition().y) {
    ResolveCollisionBallDown(ball);
  }
}

/* Rebote en la paleta: el ángulo de salida depende del punto de impacto; un
   impacto centrado sale recto y cuanto más al borde, más se abre el ángulo. */
void Game::ResolveCollisionBallPlayer(Ball &ball) {
  float ballRelativePositionX = ball.shape.getPosition().x - player.shape.getPosition().x;
  float impactFactorX = ballRelativePositionX / (player.shape.getGlobalBounds().size.x / 2);
  float ballHitPositionX = std::clamp(impactFactorX, -1.f, 1.f);

  float angle = BALL_ANGLE_MAX.asRadians() * ballHitPositionX;
  ball.direction = {std::sin(angle), -std::cos(angle)};

  PlayBounce();
}

/* La pelota cayó: desaparece, pero la vida solo se cobra cuando es la última
   en pantalla. La invencibilidad, en cambio, la hace rebotar sin perderla. */
void Game::ResolveCollisionBallDown(Ball &ball) {
  if (ball.invincible) {
    ball.direction.y *= -1;
    return;
  }

  ball.alive = false; // La pelota caída queda marcada para retirarse

  // Mientras quede otra pelota viva en pantalla, no se pierde vida aún.
  const bool lastBallOnScreen = std::ranges::none_of(
      balls, [](const Ball &other) { return other.alive; });
  if (!lastBallOnScreen) {
    return;
  }

  player.stats.lives--;
  UpdateLivesText();

  if (player.stats.lives == 0) {
    LOG_INFO("You Lose!");
    EventGameEnd(Screen::GameOver, L"GAME OVER", GAME_OVER_TITLE_COLOR);
  } else {
    // La misma pelota se relanza desde la paleta para seguir la partida.
    EventBallReset(ball);
    ball.alive = true;
  }
}

/* Primer ladrillo que toca la pelota recibe el impacto; luego se retiran los destruidos. */
void Game::HandleCollisionsBallBricks(Ball &ball) {
  for (auto &brick : bricks) {
    if (Intersects(ball.shape, brick.shape)) {
      ResolveCollisionBallBrick(ball, brick);
      break;
    }
  }

  std::erase_if(bricks, [](const Brick &brick) { return brick.lives == 0; });
}

/* Daña el ladrillo (el fuego lo destruye de golpe), actualiza su color o suma
   puntos al destruirlo, y rebota la pelota fuera del ladrillo. */
void Game::ResolveCollisionBallBrick(Ball &ball, Brick &brick) {
  if (ball.fire) {
    brick.lives = 0;
  } else {
    brick.lives--;
  }

  if (brick.lives > 0) {
    brick.shape.setFillColor(BRICK_COLORS[brick.lives - 1]);
  } else {
    player.stats.score++;
    UpdateScoreText();

    // El récord se actualiza y se guarda en cuanto la puntuación lo supera.
    if (player.stats.score > player.stats.highScore) {
      player.stats.highScore = player.stats.score;
      UpdateHighScoreText();
      ctx.save.Set(STATS_HIGH_SCORE_KEY, player.stats.highScore);
    }
  }

  ResolveCollisionBallBrickBounce(ball, brick);

  PlayBounce();
}

/* Rebote contra un ladrillo: se invierte el eje del lado de menor penetración
   (ese es el lado que la pelota atravesó) y se corrige la posición 1px fuera. */
void Game::ResolveCollisionBallBrickBounce(Ball &ball, Brick &brick) {
  if (auto intersection = Intersects(ball.shape, brick.shape)) {
    if (intersection->size.x < intersection->size.y) {
      ball.direction.x *= -1;
      float correction = intersection->size.x / 2 + 1;
      ball.shape.move({(ball.direction.x > 0) ? correction : -correction, 0});
    } else {
      ball.direction.y *= -1;
      float correction = intersection->size.y / 2 + 1;
      ball.shape.move({0, (ball.direction.y > 0) ? correction : -correction});
    }
  }
}

/* Devuelve los bonos que no estén activos ni choquen con un bono activo. */
std::vector<BonusType> Game::GetBonusTypesAllowed() const {
  std::vector<BonusType> types;

  for (int i = 0; i < BonusType::Count; i++) {
    BonusType candidate = BonusType(i);

    bool allowed = std::ranges::none_of(bonuses, [&](const Bonus &bonus) {
      return bonus.type == candidate || !IsBonusCompatible(bonus.type, candidate);
    });

    if (allowed) {
      types.emplace_back(candidate);
    }
  }

  return types;
}

/* Dos bonos son incompatibles si aplican efectos opuestos (agrandar/achicar,
   acelerar/frenar): activar juntos anularía el efecto. */
bool Game::IsBonusCompatible(BonusType a, BonusType b) const {
  if ((a == ExpandPaddle && b == ShrinkPaddle) ||
      (a == ShrinkPaddle && b == ExpandPaddle)) {
    return false;
  }

  if ((a == AccelerateBall && b == DecelerateBall) ||
      (a == DecelerateBall && b == AccelerateBall)) {
    return false;
  }

  return true;
}

/* La zona del imán: cerca en horizontal y por encima de la paleta. */
bool Game::IsBallNearPlayer(const Ball &ball) const {
  float distanceX = std::abs(ball.shape.getPosition().x - player.shape.getPosition().x);

  bool nearPositionX = distanceX < BALL_MAGNET_RANGE;
  bool nearPositionY = ball.shape.getPosition().y > player.shape.getPosition().y - BALL_MAGNET_RANGE;

  return nearPositionX && nearPositionY;
}

/* Dibujo por capas: muro -> pelotas -> paleta -> textos de bonos -> HUD. */
void Game::Render() const {
  for (const auto &brick : bricks) {
    ctx.renderer.Draw(brick.shape);
  }

  for (const auto &ball : balls) {
    ctx.renderer.Draw(ball.shape);
  }

  ctx.renderer.Draw(player.shape);

  for (const auto &bonus : bonuses) {
    ctx.renderer.Draw(bonus.text);
  }

  ctx.renderer.Draw(player.stats.scoreText);
  ctx.renderer.Draw(player.stats.highScoreText);
  ctx.renderer.Draw(player.stats.livesText);

  if (screen != Screen::Playing) {
    RenderResultScreen();
  }
}

/* Congela la partida en una pantalla de resultado y apaga la música. */
void Game::EventGameEnd(Screen result, const sf::String &title, sf::Color titleColor) {
  screen = result;
  resultTitle = title;
  resultColor = titleColor;
  audioGroup.Stop(AudioChannel::Music);
}

/* Superpone la cancha congelada con el resultado final y el reinicio. */
void Game::RenderResultScreen() const {
  sf::RectangleShape backdrop(gConfig.windowSize);
  backdrop.setFillColor(SCREEN_BACKDROP_COLOR);
  ctx.renderer.Draw(backdrop);

  const auto drawCentered = [this](const sf::String &value, unsigned int size,
                                   float yRatio, sf::Color color) {
    sf::Text text(GetDefaultFont(), value, size);
    text.setFillColor(color);
    // El origen centrado permite fijar la posición por el centro del texto,
    // aunque luego cambie su contenido o tamaño.
    text.setOrigin(text.getLocalBounds().getCenter());
    text.setPosition({gConfig.windowSize.x / 2.f, gConfig.windowSize.y * yRatio});
    ctx.renderer.Draw(text);
  };

  drawCentered(resultTitle, SCREEN_TITLE_SIZE, 0.35f, resultColor);

  sf::String message = L"Score: " + std::to_wstring(player.stats.score);
  message += L"\nHigh: " + std::to_wstring(player.stats.highScore);
  drawCentered(message, SCREEN_MESSAGE_SIZE, 0.50f, SCREEN_MESSAGE_COLOR);

  drawCentered(L"ENTER = JUGAR DE NUEVO", SCREEN_HINT_SIZE, 0.68f, SCREEN_HINT_COLOR);
}

/* Congela o reanuda el audio y los temporizadores de bonos al perder el foco. */
void Game::OnPause(bool paused) {
  audioGroup.SetPaused(paused);

  if (paused) {
    for (auto &bonus : bonuses) {
      bonus.endCooldown.Stop();
    }
  } else {
    for (auto &bonus : bonuses) {
      bonus.endCooldown.Start();
    }
  }
}

/* Detiene todos los sonidos cuando el motor cambia o reinicia la escena. */
void Game::OnCleanup() {
  audioGroup.StopAll();
}

/* Reproduce el efecto de rebote desde el comienzo en cada impacto. */
void Game::PlayBounce() {
  if (bounceSound) {
    audioGroup.Play(*bounceSound, true);
  }
}

/* Actualiza el contador de puntuación y lo ancla a la esquina inferior
   izquierda con el mismo margen del HUD que usa el texto de vidas. */
void Game::UpdateScoreText() {
  auto &text = player.stats.scoreText;
  text.setString("Score: " + std::to_string(player.stats.score));

  const auto bounds = text.getLocalBounds();
  text.setPosition({STATS_HUD_MARGIN - bounds.position.x,
                    gConfig.windowSize.y - STATS_HUD_MARGIN - bounds.size.y});
}

/* Actualiza el contador de vidas y lo alinea por su borde derecho con un
   margen fijo. Posicionar por el borde (y no con una X fija) evita que el
   texto se recorte en la ventana, sin importar cuántos dígitos tenga. */
void Game::UpdateLivesText() {
  auto &text = player.stats.livesText;
  text.setString("Lives: " + std::to_string(player.stats.lives));

  const auto bounds = text.getLocalBounds();
  text.setPosition({gConfig.windowSize.x - STATS_HUD_MARGIN - bounds.position.x - bounds.size.x,
                    gConfig.windowSize.y - STATS_HUD_MARGIN - bounds.size.y});
}

/* Actualiza el contador del récord y lo coloca justo encima del score, en la
   misma columna del margen izquierdo. */
void Game::UpdateHighScoreText() {
  auto &text = player.stats.highScoreText;
  text.setString("High: " + std::to_string(player.stats.highScore));

  const auto bounds = text.getLocalBounds();
  const auto scoreBounds = player.stats.scoreText.getLocalBounds();
  text.setPosition({STATS_HUD_MARGIN - bounds.position.x,
                    gConfig.windowSize.y - STATS_HUD_MARGIN - bounds.size.y -
                        scoreBounds.size.y - bounds.size.y * 0.5f});
}
