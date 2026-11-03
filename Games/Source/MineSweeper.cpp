#include "MineSweeper.h"

using namespace MineSweeper;

/* Constructor: prepara la grilla y las estadísticas una vez, al crear la escena. */
Game::Game(EngineContext &context) : Scene(context),
                                     restartCooldown(RESTART_COOLDOWN_DURATION) {
  restartCooldown.Bind(ctx.time);
  InitGrid();
  InitStats();
}

/* Calcula dónde nace el tablero y crea las 8×8 casillas en su posición. */
void Game::InitGrid() {
  sf::Vector2f origin = GetGridOrigin(); // esquina superior izquierda del tablero (Arriba, izquierda)
  sf::Vector2f size(CELL_SIZE, CELL_SIZE);

  for (int i = 0; i < GRID_HEIGHT; i++) {
    for (int j = 0; j < GRID_WIDTH; j++) {
      auto &cell = grid.cells[i][j];

      // (columna, fila) por tamaño de casilla = desplazamiento en píxeles desde esa esquina.
      sf::Vector2f offset(j * CELL_SIZE, i * CELL_SIZE);

      InitGridCell(cell, {origin + offset + size / 2.f, size}); // se pasa el CENTRO de la casilla, no su esquina
    }
  }
}

/* Configura los dibujos de una casilla: fondo, mina, X de "mina desactivada", bandera y número, todos centrados en ella. */
void Game::InitGridCell(Cell &cell, sf::FloatRect bounds) {
  cell.visual.background.setOutlineColor(CELL_OUTLINE_COLOR);
  cell.visual.background.setOutlineThickness(-1); // grosor negativo: el contorno se dibuja hacia adentro y no se superpone con la casilla vecina
  cell.visual.background.setSize(bounds.size);
  cell.visual.background.setOrigin(cell.visual.background.getGeometricCenter()); // el punto de anclaje es su centro geométrico
  cell.visual.background.setPosition(bounds.position); // bounds.position es el centro de la casilla

  cell.visual.mine.setFillColor(CELL_MINE_COLOR);
  cell.visual.mine.setRadius(CELL_SIZE / 4); // un cuarto del tamaño de la casilla: la mina queda pequeña
  cell.visual.mine.setOrigin(cell.visual.mine.getGeometricCenter());
  cell.visual.mine.setPosition(bounds.position); // anclada al centro, igual que el fondo

  cell.visual.mineCross.setString("X"); // la X solo se dibuja al terminar la partida, si la bandera acertó
  cell.visual.mineCross.setCharacterSize(CELL_MINE_CROSS_SIZE); // más grande que el círculo de la mina, para que se note encima
  cell.visual.mineCross.setFillColor(CELL_MINE_CROSS_COLOR);
  cell.visual.mineCross.setOrigin(cell.visual.mineCross.getLocalBounds().getCenter());
  cell.visual.mineCross.setPosition(bounds.position); // misma coordenada: queda centrada sobre el círculo

  cell.visual.flag.setFillColor(CELL_FLAG_COLOR);
  cell.visual.flag.setRadius(CELL_SIZE / 4);
  cell.visual.flag.setPointCount(3); // un círculo de 3 puntos se dibuja como triángulo: así se hace la bandera
  cell.visual.flag.setOrigin(cell.visual.flag.getGeometricCenter());
  cell.visual.flag.setPosition(bounds.position);

  cell.visual.adjacentCountText.setFillColor(CELL_TEXT_COLOR);
  cell.visual.adjacentCountText.setCharacterSize((int)CELL_SIZE - 10); // casi tan alto como la casilla
  cell.visual.adjacentCountText.setPosition(bounds.position); // todavía sin texto: se centra más tarde, cuando se conozca el número
}

/* Coloca el contador de minas en la esquina superior izquierda de la ventana. */
void Game::InitStats() {
  stats.mineCountText.setPosition({10, 10});
}

/* Se ejecuta al entrar a la escena y en cada reinicio: prepara una ronda nueva. */
void Game::Start() {
  StartGrid();
  StartStats();

  restartCooldown.Reset(); // el temporizador de reinicio arranca a cero y detenido
}

/* Resetea todas las casillas del tablero al estado inicial. */
void Game::StartGrid() {
  for (auto &row : grid.cells) {
    for (auto &cell : row) {
      StartGridCell(cell);
    }
  }
}

/* Deja una casilla como nueva: tapada, sin mina y sin número de vecinos. */
void Game::StartGridCell(Cell &cell) {
  cell.state = CellState::Hidden;
  cell.mined = false;
  cell.adjacentCount = 0;
}

/* Inicia las estadísticas de la ronda: primer clic pendiente, banderas en cero y contador con el total de minas. */
void Game::StartStats() {
  stats.firstClick = true;
  stats.flagCount = 0;
  stats.gameOver = false; // la partida todavía no se ganó ni se perdió
  stats.mineCountText.setString("Mines: " + std::to_string(STATS_MINE_COUNT));
  resultText.setString(""); // limpia el mensaje de victoria o derrota de la ronda anterior
}

/* Envía cada evento al HandleEvent correspondiente y, al final, actualiza el contador de minas restantes. */
void Game::OnEvent(const sf::Event &event) {
  event.visit([this](const auto &type) { this->HandleEvent(type); });

  stats.mineCountText.setString("Mines: " + std::to_string(STATS_MINE_COUNT - stats.flagCount)); // el contador pudo cambiar con el gesto (por ejemplo, al poner una bandera)
}

/* Mouse: el clic izquierdo revela casillas y el derecho pone o quita banderas. */
void Game::HandleEvent(const sf::Event::MouseButtonPressed &mouse) {
  if (mouse.button == sf::Mouse::Button::Left) {
    EventCellRevealAsk();
  } else if (mouse.button == sf::Mouse::Button::Right) {
    EventFlagToggleAsk();
  }
}

/* Mando: el botón Sur (A) revela casillas y el Oeste (X) pone o quita banderas. */
void Game::HandleEvent(const sf::Event::JoystickButtonPressed &joystick) {
  if (auto button = Input::HardwareToLogical(joystick.button, joystick.joystickId)) { // convierte el botón físico al botón lógico, porque cada fabricante los numera distinto
    if (*button == GamepadButton::South) {
      EventCellRevealAsk();
    } else if (*button == GamepadButton::West) {
      EventFlagToggleAsk();
    }
  }
}

/* Se ejecuta en cada frame: si el aviso de fin terminó su tiempo, reinicia la escena. */
void Game::Update() {
  if (restartCooldown.IsOver()) {
    ctx.scenes.RestartCurrentScene(); // así arranca una ronda nueva usando el mismo tablero
  }
}

/* Coloca las minas en casillas al azar, evitando la casilla del primer clic, y luego calcula los números de vecinos. */
void Game::EventMinesSpawn(sf::Vector2i safePosition) {
  int placed = 0;

  while (placed < STATS_MINE_COUNT) {
    sf::Vector2i position(ctx.random.Int(0, GRID_WIDTH - 1),
                          ctx.random.Int(0, GRID_HEIGHT - 1));

    auto &cell = grid.cells[position.y][position.x];

    // Reintenta mientras la casilla elegida sea la del clic inicial o ya tenga una mina.
    if ((position == safePosition) || cell.mined) {
      continue;
    }

    cell.mined = true;
    placed++;
  }

  EventMinesSpawnSetAdjacents(); // minas ya colocadas: recién ahora se puede calcular, para cada casilla, cuántas vecinas son minas
}

/* Recorre el tablero casilla por casilla y deja en cada una el número de minas que la rodean: -1 cuando la casilla ES una mina. */
void Game::EventMinesSpawnSetAdjacents() {
  // i = fila actual, j = columna actual: una casilla distinta por vuelta del doble loop
  for (int i = 0; i < GRID_HEIGHT; i++) {
    for (int j = 0; j < GRID_WIDTH; j++) {
      auto &cell = grid.cells[i][j];

      // adjacentCount se calcula en dos casos:
      // 1) La casilla ES una mina → -1: valor especial imposible de obtener contando vecinas reales,
      //    con él el resto del juego sabe que ahí hay una mina (por ejemplo, para no dibujarle número).
      // 2) La casilla NO es una mina → count_if recorre sus 8 vecinas y cuenta cuántas tienen mina:
      //    a) GetNeighborOffsets() entrega los 8 desplazamientos hacia las casillas vecinas (arriba, abajo, diagonales, etc.).
      //    b) vecinoX/vecinoY = posición real de la vecina: la casilla actual (j, i) más el desplazamiento offset.
      //    c) el lambda devuelve true si esa vecina existe (está dentro del tablero) y además tiene una mina.
      //    d) count_if devuelve cuántas veces dio true: cuántas de las 8 vecinas son minas (un número entre 0 y 8).
      //    El (int) convierte el retorno de count_if (entero grande) al tipo del campo adjacentCount.
      cell.adjacentCount = cell.mined ? -1 : (int)std::ranges::count_if(GetNeighborOffsets(), [&](sf::Vector2i offset) {
        int neighborX = j + offset.x;
        int neighborY = i + offset.y;
        return neighborX >= 0 && neighborX < GRID_WIDTH && // la vecina existe si su columna entra entre 0 y GRID_WIDTH - 1
               neighborY >= 0 && neighborY < GRID_HEIGHT && // y su fila entre 0 y GRID_HEIGHT - 1: si algo de esto falla, la vecina no existe
               grid.cells[neighborY][neighborX].mined; // y encima tiene una mina: recién entonces esa vecina suma al número
      });

      if (cell.adjacentCount > 0) { // número de 1 a 8: sí se dibuja; los ceros (0) y las minas (-1) no muestran número
        cell.visual.adjacentCountText.setString(std::to_string(cell.adjacentCount));
        sf::Vector2f center = cell.visual.adjacentCountText.getLocalBounds().getCenter();
        cell.visual.adjacentCountText.setOrigin(center); // el dígito se centra con su ancho real, que recién ahora se conoce
      }
    }
  }
}

/* Revela la casilla bajo el cursor: si esconde una mina se pierde; si no, se expanden los ceros y luego se chequea si se ganó. */
void Game::EventCellRevealAsk() {
  if (restartCooldown.IsRunning()) {
    return; // el aviso de fin está en pantalla: por ahora no se puede jugar
  }

  sf::Vector2i position = WorldToCell(ctx.cursor.GetPosition());

  if (!IsCellPositionValid(position)) {
    return; // el cursor está fuera del tablero: no hay casilla que revelar
  }

  if (stats.firstClick) {
    EventMinesSpawn(position); // primer clic de la ronda: aquí se colocan las minas, nunca sobre esa casilla
    stats.firstClick = false; // de esta forma la siembra ocurre una sola vez por ronda
  }

  auto &cell = grid.cells[position.y][position.x];

  if (cell.state != CellState::Hidden) {
    return; // solo reaccionan las casillas tapadas: las abiertas o con bandera no hacen nada
  }

  cell.state = CellState::Revealed; // la casilla queda abierta

  if (cell.mined) {
    LOG_INFO("You Lose!");
    EventGameFinish();
    EventResultTextSpawn(false); // muestra el mensaje de derrota
    return; // derrota inmediata: se corta aquí
  }

  if (cell.adjacentCount == 0) {
    EventCellRevealAdjacentZeros(position); // sin minas alrededor: se revelan en cadena las casillas vecinas
  }

  if (IsWin()) {
    LOG_INFO("You Win!");
    EventGameFinish();
    EventResultTextSpawn(true); // muestra el mensaje de victoria
  }
}

/* Revela las 8 casillas vecinas de una casilla sin minas alrededor; cuando alguna vecina también es un cero, repite el proceso (recursión). */
void Game::EventCellRevealAdjacentZeros(sf::Vector2i position) {
  for (sf::Vector2i offset : GetNeighborOffsets()) {
    sf::Vector2i neighborPosition = position + offset;

    if (!IsCellPositionValid(neighborPosition)) {
      continue; // la vecina cae fuera del tablero: se salta
    }

    auto &cell = grid.cells[neighborPosition.y][neighborPosition.x];

    // Solo casillas tapadas y sin mina: las ya abiertas no entran de nuevo y así el proceso no se repite infinito.
    if ((cell.state != CellState::Hidden && cell.state != CellState::Flagged) || cell.mined) {
      continue;
    }

    if (cell.state == CellState::Flagged) {
      stats.flagCount--; // la bandera se libera y vuelve a estar disponible
    }

    cell.state = CellState::Revealed; // se marca abierta ANTES de seguir: así esta misma casilla no vuelve a entrar (evita un ciclo infinito)

    if (cell.adjacentCount == 0) {
      EventCellRevealAdjacentZeros(neighborPosition); // la vecina también es un cero: la cadena continúa con sus propias vecinas
    }
  }
}

/* Fin de la partida: cambia el estado global y programa el reinicio de la escena. El render se encarga de mostrar las minas. */
void Game::EventGameFinish() {
  stats.gameOver = true; // el render reacciona: muestra todas las minas y las banderas acertadas como círculo con X
  restartCooldown.Start(); // al expirar, Update() reinicia la escena y arranca una ronda nueva
}

/* Muestra en el centro de la ventana el mensaje de fin: "¡GANASTE!" (verde) o "¡PERDISTE!" (rojo), con contorno para leerse sobre el tablero. */
void Game::EventResultTextSpawn(bool win) {
  // std::wstring evita que el "¡" aparezca como "Â¡" (problema de codificación con std::string)
  std::wstring message = win ? L"¡GANASTE!" : L"¡PERDISTE!";

  resultText.setString(message);
  resultText.setCharacterSize(RESULT_TEXT_SIZE);
  resultText.setFillColor(win ? RESULT_WIN_COLOR : RESULT_LOSE_COLOR); // verde para la victoria, rojo para la derrota
  resultText.setOutlineColor(RESULT_OUTLINE_COLOR);
  resultText.setOutlineThickness(RESULT_TEXT_OUTLINE_THICKNESS);
  resultText.setOrigin(resultText.getLocalBounds().getCenter()); // centrado después de conocer el tamaño real del texto
  resultText.setPosition(gConfig.windowSize / 2.f); // centro de la ventana
}

/* Filtro antes de tocar bandera: la partida debe estar en curso y el cursor debe apuntar a una casilla real. */
void Game::EventFlagToggleAsk() {
  if (restartCooldown.IsRunning()) {
    return; // con el aviso de fin en pantalla no se pueden marcar casillas
  }

  sf::Vector2i position = WorldToCell(ctx.cursor.GetPosition());

  if (IsCellPositionValid(position)) {
    EventFlagToggle(position);
  }
}

/* Pone o quita una bandera en una casilla tapada, sin superar el límite de una bandera por mina. */
void Game::EventFlagToggle(sf::Vector2i position) {
  auto &cell = grid.cells[position.y][position.x];

  if (cell.state == CellState::Revealed) {
    return; // las casillas ya abiertas no pueden llevar bandera
  }

  if (cell.state == CellState::Hidden) {
    if (stats.flagCount < STATS_MINE_COUNT) { // límite: no se permiten más banderas que minas
      cell.state = CellState::Flagged;
      stats.flagCount++;
    }
  } else if (cell.state == CellState::Flagged) {
    cell.state = CellState::Hidden;
    stats.flagCount--; // la bandera vuelve a estar disponible
  }
}

/* Convierte píxeles a coordenadas de casilla (columna, fila): resta el origen del tablero y divide por el tamaño. */
sf::Vector2i Game::WorldToCell(sf::Vector2f position) const {
  sf::Vector2f offset = position - GetGridOrigin(); // posición en píxeles desde la esquina del tablero

  return {int(std::floor(offset.x / CELL_SIZE)), int(std::floor(offset.y / CELL_SIZE))}; // el floor redondea hacia abajo y evita errores con valores negativos
}

/* Esquina superior izquierda del tablero: el espacio libre de la ventana se reparte en dos mitades. */
sf::Vector2f Game::GetGridOrigin() const {
  return (gConfig.windowSize - sf::Vector2f(GRID_WIDTH, GRID_HEIGHT) * CELL_SIZE) / 2.f;
}

/* Devuelve los 8 desplazamientos hacia las casillas vecinas, diagonales incluidas. */
std::array<sf::Vector2i, 8> Game::GetNeighborOffsets() const {
  return std::to_array<sf::Vector2i>(
      {{-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}});
}

/* Indica si unas coordenadas (columna, fila) están dentro del tablero. */
bool Game::IsCellPositionValid(sf::Vector2i position) const {
  return position.x >= 0 && position.x < GRID_WIDTH &&
         position.y >= 0 && position.y < GRID_HEIGHT;
}

/* Se ganó cuando la cantidad de casillas abiertas sin mina equivale al total de casillas libres de mina. */
bool Game::IsWin() const {
  int revealed = 0;

  for (const auto &row : grid.cells) {
    revealed += (int)std::ranges::count_if(row, [](const Cell &cell) {
      return !cell.mined && cell.state == CellState::Revealed;
    });
  }

  return revealed == (GRID_WIDTH * GRID_HEIGHT - STATS_MINE_COUNT);
}

/* Dibuja todas las casillas, el contador de minas y, si ya terminó la partida, el mensaje de fin por encima de todo. */
void Game::Render() const {
  for (const auto &row : grid.cells) {
    for (const auto &cell : row) {
      RenderCell(cell);
    }
  }

  ctx.renderer.Draw(stats.mineCountText); // el contador vive fuera de la grilla

  ctx.renderer.Draw(resultText); // el mensaje de fin se dibuja al final para que nada lo tape
}

/* Dibuja una casilla: color de fondo según su estado, y encima lo que corresponda: bandera, mina, número o la X de "mina desactivada". */
void Game::RenderCell(const Cell &cell) const {
  // La mina se ve si su casilla está abierta, o si la partida terminó (todas las minas salen a la vista).
  bool showMine = cell.mined && (cell.state == CellState::Revealed || stats.gameOver);

  // El fondo se dibuja de una copia: el color se decide aquí y no queda regado por los eventos del juego.
  sf::RectangleShape background = cell.visual.background;
  background.setFillColor(showMine || cell.state != CellState::Hidden ? CELL_REVEALED_COLOR : CELL_HIDDEN_COLOR);

  ctx.renderer.Draw(background);

  if (showMine) {
    ctx.renderer.Draw(cell.visual.mine); // el círculo de la mina

    if (cell.state == CellState::Flagged) {
      ctx.renderer.Draw(cell.visual.mineCross); // además tiene bandera: estaba bien puesta → X encima = mina desactivada
    }
  } else if (cell.state == CellState::Flagged) {
    ctx.renderer.Draw(cell.visual.flag); // bandera común de una casilla tapada
  } else if (cell.state == CellState::Revealed && cell.adjacentCount > 0) {
    ctx.renderer.Draw(cell.visual.adjacentCountText); // número de minas vecinas; las casillas abiertas con 0 quedan limpias
  }
  // sin nada extra: una tapada solo muestra el fondo y una abierta sin minas alrededor tampoco agrega dibujo
}
