#include "Tetris.h"

using namespace Tetris;

/* Inicializa la escena: temporizadores de acción, interfaz de Game Over y grupo de audio. */
Game::Game(EngineContext &context) : Scene(context),
                                     actionCooldown(ACTION_COOLDOWN_DURATION),
                                     fallCooldown(PIECE_FALL_COOLDOWN_DURATION),
                                     softDropCooldown(SOFT_DROP_COOLDOWN_DURATION),
                                     gameOverCooldown(GAME_OVER_DELAY),
                                     audioGroup(context.audio.CreateGroup()) {
  actionCooldown.Bind(ctx.time);
  fallCooldown.Bind(ctx.time);
  softDropCooldown.Bind(ctx.time);
  gameOverCooldown.Bind(ctx.time);
  InitStats();

  // Textos de Game Over
  gameOverBackdrop.setSize(gConfig.windowSize);
  gameOverBackdrop.setFillColor(GAME_OVER_BACKDROP_COLOR);
  gameOverTitle.setString("GAME OVER");
  gameOverTitle.setCharacterSize(48);
  gameOverTitle.setFillColor(GAME_OVER_TITLE_COLOR);
  gameOverTitle.setOrigin(gameOverTitle.getLocalBounds().getCenter());
  gameOverTitle.setPosition({gConfig.windowSize.x / 2.f, gConfig.windowSize.y / 2.f - 35.f});
  gameOverScore.setCharacterSize(28);
  gameOverScore.setFillColor(SCORE_COLOR);
  gameOverScore.setPosition({gConfig.windowSize.x / 2.f, gConfig.windowSize.y / 2.f + 25.f});

  // Música de fondo en bucle y sonido al completar líneas.
  if (auto track = ctx.resources.FetchMusic(MUSIC_FILENAME)) {
    music = audioGroup.AddMusic(std::move(*track), MUSIC_VOLUME, true);
  }
  lineClearSound = audioGroup.AddSound(ctx.resources.AcquireSound(LINE_CLEAR_SOUND_FILENAME), LINE_CLEAR_SOUND_VOLUME);
}

/* Da posición, tamaño y color a los textos de puntuación y récord. */
void Game::InitStats() {
  stats.scoreText.setPosition({10, 10});
  stats.scoreText.setCharacterSize(20);
  stats.scoreText.setFillColor(SCORE_COLOR);
  stats.highScoreText.setPosition({10, 40});
  stats.highScoreText.setCharacterSize(20);
  stats.highScoreText.setFillColor(SCORE_COLOR);
}

/* Reinicia la partida: estado, inputs, tablero, piezas y estadísticas. */
void Game::Start() {
  audioGroup.StopAll();
  gameOverActive = false;
  gameOverCooldown.Reset();
  ctx.cursor.SetVisible(false);

  BindInputs();

  StartGrid();
  StartNextPiece();
  StartStats();

  EventPieceSpawn();

  actionCooldown.Restart();
  fallCooldown.Restart();
  softDropCooldown.Restart();

  if (music) {
    audioGroup.Play(*music);
  }
}

/* Pausa o reanuda el audio cuando el juego pierde o recupera el foco. */
void Game::OnPause(bool paused) {
  audioGroup.SetPaused(paused);
}

/* Detiene los sonidos al cambiar de escena. */
void Game::OnCleanup() {
  audioGroup.StopAll();
}

/* Asigna cada acción a teclas (flechas y WASD) y a ejes y botones del mando. */
void Game::BindInputs() {
  ctx.input.Bind(MoveRight, Input::Keyboard{sf::Keyboard::Scan::Right});
  ctx.input.Bind(MoveRight, Input::Keyboard{sf::Keyboard::Scan::D});
  ctx.input.Bind(MoveRight, Input::Axis{sf::Joystick::Axis::X, 0.5f});

  ctx.input.Bind(MoveLeft, Input::Keyboard{sf::Keyboard::Scan::Left});
  ctx.input.Bind(MoveLeft, Input::Keyboard{sf::Keyboard::Scan::A});
  ctx.input.Bind(MoveLeft, Input::Axis{sf::Joystick::Axis::X, -0.5f});

  ctx.input.Bind(Rotate, Input::Keyboard{sf::Keyboard::Scan::Up});
  ctx.input.Bind(Rotate, Input::Keyboard{sf::Keyboard::Scan::W});
  ctx.input.Bind(Rotate, Input::Gamepad{GamepadButton::North});

  ctx.input.Bind(SoftDrop, Input::Keyboard{sf::Keyboard::Scan::Down});
  ctx.input.Bind(SoftDrop, Input::Keyboard{sf::Keyboard::Scan::S});
  ctx.input.Bind(SoftDrop, Input::Axis{sf::Joystick::Axis::Y, 0.5f});
}

/* Vacía el tablero para empezar de cero. */
void Game::StartGrid() {
  board = {};
}

/* Genera la primera pieza en cola de espera. */
void Game::StartNextPiece() {
  next = GenerateRandomPiece();
}

/* Pone la puntuación a cero y recupera el récord persistido. */
void Game::StartStats() {
  stats.score = 0;
  stats.scoreText.setString("Score: 0");
  // El récord se conserva entre partidas y se recupera al iniciar el juego.
  stats.highScore = ctx.save.Get<int>(STATS_HIGH_SCORE_KEY, 0);
  stats.highScoreText.setString("High Score: " + std::to_string(stats.highScore));
}

/* Caída instantánea de la pieza con la barra espaciadora o el botón sur del mando. */
void Game::OnEvent(const sf::Event &event) {
  // No permite mover ni dejar caer piezas durante la pantalla de derrota.
  if (gameOverActive) {
    return;
  }
  if (auto key = event.getIf<sf::Event::KeyPressed>()) {
    if (key->scancode == sf::Keyboard::Scan::Space) {
      EventPieceDropHard();
    }
  }

  if (auto joystick = event.getIf<sf::Event::JoystickButtonPressed>()) {
    if (Input::HardwareToLogical(joystick->button, joystick->joystickId) == GamepadButton::South) {
      EventPieceDropHard();
    }
  }
}

/* Lógica por frame: rotar, mover a los lados y dejar caer la pieza con sus temporizadores. */
void Game::Update() {
  if (gameOverActive) {
    // El reloj del juego se detiene si la ventana pierde el foco o se pausa.
    if (gameOverCooldown.IsOver()) {
      ctx.scenes.RestartCurrentScene();
    }
    return;
  }
  if (ctx.input.Pressed(Rotate) && actionCooldown.IsOver()) {
    EventPieceRotate();
    actionCooldown.Restart();
  }

  if (ctx.input.Pressed(MoveRight) && actionCooldown.IsOver()) {
    EventPieceMove({1, 0});
    actionCooldown.Restart();
  } else if (ctx.input.Pressed(MoveLeft) && actionCooldown.IsOver()) {
    EventPieceMove({-1, 0});
    actionCooldown.Restart();
  }

  if (ctx.input.Pressed(SoftDrop) && softDropCooldown.IsOver()) {
    // La caída suave reinicia también la caída natural para no contar un paso extra.
    EventPieceFall();
    softDropCooldown.Restart();
    fallCooldown.Restart();
  } else if (fallCooldown.IsOver()) {
    EventPieceFall();
    fallCooldown.Restart();
  }
}

/* La pieza en espera pasa a ser la pieza actual y se genera otra en espera. */
void Game::EventPieceSpawn() {
  current = next;
  next = GenerateRandomPiece();
}

/* Gira la pieza 90° solo si la nueva orientación cabe en el tablero. */
void Game::EventPieceRotate() {
  Piece rotated = current;
  rotated.rotation = (rotated.rotation + 1) % 4;

  if (IsPieceValid(rotated)) {
    current = rotated;
  }
}

/* Desplaza la pieza según el desplazamiento dado si la posición resultante es válida. */
void Game::EventPieceMove(sf::Vector2i offset) {
  Piece moved = current;
  moved.position += offset;

  if (IsPieceValid(moved)) {
    current = moved;
  }
}

/* Baja la pieza una fila; si no puede, se fusiona con el tablero. */
void Game::EventPieceFall() {
  Piece moved = current;
  moved.position.y++;

  if (IsPieceValid(moved)) {
    current = moved;
  } else {
    EventPieceMerge();
  }
}

/* Deja caer la pieza hasta el fondo de golpe y la fusiona de inmediato. */
void Game::EventPieceDropHard() {
  while (IsPieceValid(current)) {
    current.position.y++;
  }
  // Retrocede una fila para que la pieza quede apoyada y no hundida en el fondo.
  current.position.y--;

  EventPieceMerge();
}

/* Fija la pieza en el tablero, limpia líneas completas y saca la siguiente pieza. */
void Game::EventPieceMerge() {
  for (int i = 0; i < PIECE_BLOCK_COUNT; i++) {
    sf::Vector2i block = GetPointRotated(current.type, i, current.rotation) + current.position;

    if (block.y >= 0 && block.y < GRID_HEIGHT &&
        block.x >= 0 && block.x < GRID_WIDTH) {
      board[block.y][block.x] = current.color;
    }
  }

  EventLinesClear();
  EventPieceSpawn();

  // Si la pieza recién sacada no cabe, el tablero está lleno: fin de la partida.
  if (!IsPieceValid(current)) {
    LOG_INFO("You Lose!");
    // Conserva la puntuación final en pantalla antes de reiniciar la escena.
    gameOverScore.setString("Score: " + std::to_string(stats.score));
    gameOverScore.setOrigin(gameOverScore.getLocalBounds().getCenter());
    gameOverActive = true;
    gameOverCooldown.Restart();
  }
}

/* Compacta el tablero bajando las filas incompletas hacia el fondo y suma puntos. */
void Game::EventLinesClear() {
  Board newBoard = {};
  int cleared = 0;
  int destinationY = GRID_HEIGHT - 1;

  // Recorre de abajo arriba: las filas completas se descartan y las demás se apilan al fondo.
  for (int y = GRID_HEIGHT - 1; y >= 0; y--) {
    if (std::ranges::all_of(board[y], [](auto color) { return color.has_value(); })) {
      cleared++;
    } else {
      newBoard[destinationY] = board[y];
      destinationY--;
    }
  }

  if (cleared > 0) {
    board = newBoard;
    stats.score += cleared * STATS_SCORE_PER_LINE;
    stats.scoreText.setString("Score: " + std::to_string(stats.score));
    // Actualiza el récord en cuanto la puntuación actual lo supera.
    if (stats.score > stats.highScore) {
      stats.highScore = stats.score;
      stats.highScoreText.setString("High Score: " + std::to_string(stats.highScore));
      ctx.save.Set(STATS_HIGH_SCORE_KEY, stats.highScore);
    }
    // Suena una sola vez por jugada, aunque se eliminen varias líneas juntas.
    if (lineClearSound) {
      audioGroup.Play(*lineClearSound, true);
    }
  }
}

/* Crea una pieza de tipo aleatorio, sin rotar y centrada en la fila superior. */
Piece Game::GenerateRandomPiece() const {
  Piece piece;

  piece.type = ctx.random.Int(0, PIECE_TYPE_COUNT - 1);
  piece.rotation = 0;
  piece.position = {(GRID_WIDTH / 2) - 2, 0};
  piece.color = PIECE_COLORS[piece.type];

  return piece;
}

/* Devuelve el bloque `index` de la pieza rotado `rotation` cuartos de vuelta sobre su pivote. */
sf::Vector2i Game::GetPointRotated(int type, int index, int rotation) const {
  sf::Vector2f point(PIECE_SHAPES[type][index]);
  sf::Vector2f pivot = PIECE_PIVOTS[type];
  sf::Vector2f origin = point - pivot;

  sf::Vector2f rotated;
  switch (rotation) {
  case 0:
    rotated = origin;
    break;
  case 1:
    rotated = {-origin.y, origin.x};
    break;
  case 2:
    rotated = {-origin.x, -origin.y};
    break;
  case 3:
    rotated = {origin.y, -origin.x};
    break;
  }
  rotated += pivot;

  return {int(std::round(rotated.x)), int(std::round(rotated.y))};
}

/* Esquina superior izquierda del tablero, centrado en la ventana. */
sf::Vector2f Game::GetBoardOrigin() const {
  return {(gConfig.windowSize.x - GRID_WIDTH * BLOCK_SIZE) / 2,
          (gConfig.windowSize.y - GRID_HEIGHT * BLOCK_SIZE) / 2};
}

/* Indica si la pieza está dentro del tablero y sin solapar bloques ya fijados. */
bool Game::IsPieceValid(const Piece &piece) const {
  for (int i = 0; i < PIECE_BLOCK_COUNT; i++) {
    sf::Vector2i block = GetPointRotated(piece.type, i, piece.rotation) + piece.position;

    if (block.x < 0 || block.x >= GRID_WIDTH ||
        block.y < 0 || block.y >= GRID_HEIGHT ||
        (block.y >= 0 && board[block.y][block.x].has_value())) {
      return false;
    }
  }

  return true;
}

/* Dibuja fondo, tablero, pieza actual, pieza siguiente y textos de interfaz. */
void Game::Render() const {
  sf::RectangleShape background(sf::Vector2f(gConfig.windowSize));
  background.setFillColor(BACKGROUND_COLOR);
  ctx.renderer.Draw(background);

  RenderBoard();
  RenderPiece(current, GetBoardOrigin(), true);
  RenderPiece(next, {gConfig.windowSize.x - 5.5f * BLOCK_SIZE, BLOCK_SIZE * 2.5f}, false);

  ctx.renderer.Draw(stats.scoreText);
  ctx.renderer.Draw(stats.highScoreText);
  if (gameOverActive) {
    ctx.renderer.Draw(gameOverBackdrop);
    ctx.renderer.Draw(gameOverTitle);
    ctx.renderer.Draw(gameOverScore);
  }
}

/* Dibuja celda por celda: bloques fijos con su color y huecos vacíos con el color del tablero. */
void Game::RenderBoard() const {
  sf::Vector2f origin = GetBoardOrigin();

  sf::RectangleShape shape({BLOCK_SIZE, BLOCK_SIZE});
  shape.setOutlineColor(GRID_COLOR);
  shape.setOutlineThickness(-1);

  for (int y = 0; y < GRID_HEIGHT; y++) {
    for (int x = 0; x < GRID_WIDTH; x++) {
      shape.setFillColor(board[y][x].value_or(BOARD_COLOR));
      shape.setPosition(origin + sf::Vector2f(x * BLOCK_SIZE, y * BLOCK_SIZE));

      ctx.renderer.Draw(shape);
    }
  }
}

/* Dibuja los bloques de una pieza; con `world` se suma su posición real dentro del tablero. */
void Game::RenderPiece(const Piece &piece, sf::Vector2f origin, bool world) const {
  sf::RectangleShape shape({BLOCK_SIZE, BLOCK_SIZE});
  shape.setFillColor(piece.color);

  for (int i = 0; i < PIECE_BLOCK_COUNT; i++) {
    sf::Vector2i block = GetPointRotated(piece.type, i, piece.rotation);
    if (world) {
      block += piece.position;
    }

    shape.setPosition(origin + sf::Vector2f(block.x * BLOCK_SIZE, block.y * BLOCK_SIZE));

    ctx.renderer.Draw(shape);
  }
}
