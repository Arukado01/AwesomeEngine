#include "TicTacToe.h"

#include <cmath>

using namespace TicTacToe;

/* Construye el tablero y nace con los relojes ya configurados. */
Game::Game(EngineContext &context) : Scene(context),
                                     aiCooldown(AI_TURN_COOLDOWN_DURATION),
                                     restartCooldown(RESTART_COOLDOWN_DURATION) {
  InitGrid();
}

/* Reparte el tamaño de la ventana entre las celdas y crea las 9 con su posición en grilla. */
void Game::InitGrid() {
  grid.cellSize = gConfig.windowSize / (float)GRID_SIZE;

  for (int i = 0; i < GRID_SIZE; i++) {
    for (int j = 0; j < GRID_SIZE; j++) {
      auto &cell = grid.cells[i][j];

      // (columna, fila) × tamaño de celda = esquina superior izquierda en píxeles.
      sf::Vector2f position = grid.cellSize.componentWiseMul({(float)j, (float)i});

      InitGridCell(cell, position);
    }
  }
}

/* Viste una celda: casilla oscura con borde, y su texto apuntando al centro donde irán X/O. */
void Game::InitGridCell(Cell &cell, sf::Vector2f position) {
  cell.shape.setFillColor(CELL_COLOR);
  cell.shape.setOutlineColor(CELL_OUTLINE_COLOR);
  cell.shape.setOutlineThickness(-1); // grosor negativo: borde hacia adentro, no pisa la vecina
  cell.shape.setSize(grid.cellSize);
  cell.shape.setPosition(position);

  cell.text.setCharacterSize(CELL_TEXT_SIZE);
  cell.text.setPosition(position + grid.cellSize / 2.f); // el centro de la celda
}

/* Entrada a la escena (también reutilizada por el reinicio): tablero limpio, te toca y nada ganado aún. */
void Game::Start() {
  StartGrid();
  StartTurn();
  StartRestartCooldown();
  StartWinningLine();
  StartResultText();
}

/* Recorre la matriz fila por fila para limpiar todas las celdas. */
void Game::StartGrid() {
  for (auto &row : grid.cells) {
    for (auto &cell : row) {
      StartGridCell(cell);
    }
  }
}

/* ' ' es el valor "casilla vacía": así la copia el texto al símbolo (queda vacía en pantalla). */
void Game::StartGridCell(Cell &cell) {
  cell.symbol = ' ';
  cell.text.setString(cell.symbol);
}

/* El humano siempre abre con X. */
void Game::StartTurn() {
  isXTurn = true;
}

/* Reloj de reinicio a cero y detenido: no debe expirar antes de que salga un veredicto. */
void Game::StartRestartCooldown() {
  restartCooldown.Reset();
}

/* Raya de victoria recién nacida: tamaño 0 = invisible. */
void Game::StartWinningLine() {
  winningLine.setSize({0.f, 0.f});
}

/*  Mensaje de resultado vacío de nacimiento: solo debe verse durante el aviso de fin de partida. */
void Game::StartResultText() {
  resultText.setString("");
}

/* Al pulsar mouse o botón del mando, el humano intenta jugar (solo si es su turno). */
void Game::OnEvent(const sf::Event &event) {
  if (event.is<sf::Event::MouseButtonPressed>() ||
      event.is<sf::Event::JoystickButtonPressed>()) {
    EventPlayTurn();
  }
}

/* Árbitro por frame: reinicia la escena tras el aviso de fin, y deja jugar a la IA cuando expira su tiempo de pensamiento. */
void Game::Update() {
  if (restartCooldown.IsOver()) {
    ctx.scenes.RestartCurrentScene();
  } else if (!restartCooldown.IsRunning() && !isXTurn && aiCooldown.IsOver()) {
    EventPlayAITurn();
  }
}

/* Turno del humano: coloca X en la casilla bajo el cursor y agenda el turno de la IA (0.7 s) si el juego continúa. */
void Game::EventPlayTurn() {
  if (restartCooldown.IsRunning() || !isXTurn) {
    return; // partida terminada en anuncio, o no es tu turno: el clic no hace nada
  }

  sf::Vector2i position = GetCellPositionCursor();

  if (IsCellPositionValid(position)) {
    EventCellPlace(position);
    isXTurn = !isXTurn;
    EventCheckGrid();

    if (!restartCooldown.IsRunning()) {
      aiCooldown.Restart(); // la IA empieza a "pensar": su movimiento llegará en AI_TURN_COOLDOWN_DURATION
    }
  }
}

/* Turno de la IA (llamado desde Update, no por un clic): coloca O donde elige su cerebro. */
void Game::EventPlayAITurn() {
  sf::Vector2i position = GetCellPositionAI();

  if (IsCellPositionValid(position)) {
    EventCellPlace(position);
    isXTurn = !isXTurn;
    EventCheckGrid();
  }
}

/* Estampa el símbolo del turno actual en la celda: estado, color y letra centrada al mismo tiempo. */
void Game::EventCellPlace(sf::Vector2i position) {
  auto &cell = grid.cells[position.x][position.y];

  cell.symbol = isXTurn ? 'X' : 'O';

  cell.text.setFillColor(isXTurn ? CELL_SYMBOL_X_COLOR : CELL_SYMBOL_O_COLOR);
  cell.text.setString(cell.symbol);
  cell.text.setOrigin(cell.text.getLocalBounds().getCenter()); // punto de agarre al centro de la letra para centrarla en la celda
}

/* Veredicto tras cada jugada: victoria nuestra, victoria de la IA o tablero lleno sin ganador. */
void Game::EventCheckGrid() {
  if (IsWin('X')) {
    EventWinningLineSpawn('X'); // marca en pantalla las 3 casillas ganadoras
    EventResultTextSpawn('X'); // anuncio "¡La X gana!"
    LOG_INFO("You Win!");
    restartCooldown.Start();
  } else if (IsWin('O')) {
    EventWinningLineSpawn('O');
    EventResultTextSpawn('O');
    LOG_INFO("You Lose!");
    restartCooldown.Start();
  } else if (IsGridFull()) {
    EventResultTextSpawn(' '); // ' ' = nadie ganó: anuncio de empate
    LOG_INFO("Draw!");
    restartCooldown.Start();
  }
}

/* Dibuja la raya de victoria: rectángulo alargado rotado pasando por el centro de las 3 fichas ganadoras. */
void Game::EventWinningLineSpawn(char symbol) {
  std::vector<sf::Vector2i> positions = FindCellPositionsWinning(symbol);

  sf::Vector2f first = GetCellCenter(positions.front());
  sf::Vector2f last = GetCellCenter(positions.back());

  sf::Vector2f delta = last - first; // vector del extremo izquierdo al derecho
  float length = std::hypot(delta.x, delta.y); // distancia directa entre los dos extremos
  sf::Vector2f direction = delta / length; // dirección normalizada (flecha unitaria)

  winningLine.setFillColor(CELL_WINNING_LINE_COLOR);
  winningLine.setSize({length + CELL_WINNING_LINE_OVERHANG * 2.f, CELL_WINNING_LINE_THICKNESS}); // sobresale OVERHANG por cada extremo
  winningLine.setOrigin({0.f, CELL_WINNING_LINE_THICKNESS / 2.f}); // agarre en el centro-izquierda: al rotar no se despega del borde
  winningLine.setPosition(first - direction * CELL_WINNING_LINE_OVERHANG); // el arranque retrocede fuera de la primera ficha
  winningLine.setRotation(sf::radians(std::atan2(delta.y, delta.x))); // atan2 da el ángulo de la flecha (0° filas, 90° columnas, 45°/135° diagonales)
}

/* Anuncia en el centro de la pantalla quién ganó (o empate), con el color del ganador y borde para que destaque sobre las fichas. */
void Game::EventResultTextSpawn(char symbol) {
  // std::wstring: SFML lo lleva a Unicode de verdad; un std::string con '¡' UTF-8 se pinta como "Â¡"
  std::wstring message = symbol == 'X' ? L"¡La X gana!" : symbol == 'O' ? L"¡La O gana!" : L"¡Empate!";
  sf::Color color = symbol == 'X' ? CELL_SYMBOL_X_COLOR : symbol == 'O' ? CELL_SYMBOL_O_COLOR : CELL_OUTLINE_COLOR;

  resultText.setString(message);
  resultText.setCharacterSize(RESULT_TEXT_SIZE);
  resultText.setFillColor(color);
  resultText.setOutlineColor(CELL_COLOR); // borde casi negro: recorta el texto del mismo color que las fichas de fondo
  resultText.setOutlineThickness(RESULT_TEXT_OUTLINE_THICKNESS);
  resultText.setOrigin(resultText.getLocalBounds().getCenter()); // centrado tras conocer su tamaño real
  resultText.setPosition(gConfig.windowSize / 2.f); // centro de la ventana
}

/* Píxel del cursor ÷ tamaño de celda = índice de casilla (con swap por la convención fila/columna del juego). */
sf::Vector2i Game::GetCellPositionCursor() const {
  sf::Vector2i position(ctx.cursor.GetPosition().componentWiseDiv(grid.cellSize));
  return {position.y, position.x};
}

/* Convierte índice de casilla (fila, columna) al píxel exacto de su centro. */
sf::Vector2f Game::GetCellCenter(const sf::Vector2i &position) const {
  return grid.cellSize.componentWiseMul({(float)position.y, (float)position.x}) + grid.cellSize / 2.f;
}

/* Cerebro de la IA: si no hay casillas (-1,-1), ganar si puede, bloquear al rival, y si no, tirada al azar. */
sf::Vector2i Game::GetCellPositionAI() {
  std::vector<sf::Vector2i> positions = FindCellPositionsEmpty();

  return positions.empty() ? sf::Vector2i(-1, -1) : FindCellPositionWinning(positions, 'O').value_or(FindCellPositionWinning(positions, 'X').value_or(positions[ctx.random.Int(0, (int)positions.size() - 1)]));
}

/* Lista de casillas vacías en formato (fila, columna), recorridas por orden. */
std::vector<sf::Vector2i> Game::FindCellPositionsEmpty() const {
  std::vector<sf::Vector2i> positions;

  for (int i = 0; i < GRID_SIZE; i++) {
    for (int j = 0; j < GRID_SIZE; j++) {
      if (grid.cells[i][j].symbol == ' ') {
        positions.emplace_back(i, j);
      }
    }
  }

  return positions;
}

/* Prueba cada candidata "con lápiz": pone el símbolo, mira si ganaría y lo borra; devuelve la primera ganadora. */
std::optional<sf::Vector2i> Game::FindCellPositionWinning(std::span<sf::Vector2i> candidates, char symbol) {
  for (sf::Vector2i position : candidates) {
    auto &cell = grid.cells[position.x][position.y];

    cell.symbol = symbol;
    bool win = IsWin(symbol);
    cell.symbol = ' '; // el tablero real jamás cambia al hipotetizar

    if (win) {
      return position;
    }
  }

  return std::nullopt; // ninguna candidata gana
}

/* Portero: dentro de los límites del tablero Y casilla aún vacía. */
bool Game::IsCellPositionValid(sf::Vector2i position) const {
  return position.x >= 0 && position.x < GRID_SIZE &&
         position.y >= 0 && position.y < GRID_SIZE &&
         grid.cells[position.x][position.y].symbol == ' ';
}

/* Devuelve las 3 casillas ganadoras en orden (de extremo a extremo) o lista vacía si nadie ganó. */
std::vector<sf::Vector2i> Game::FindCellPositionsWinning(char symbol) const {
  std::vector<sf::Vector2i> positions;

  // Filas + Columnas
  for (int i = 0; i < GRID_SIZE; i++) {
    if (std::ranges::all_of(grid.cells[i], [&](const Cell &cell) { return cell.symbol == symbol; })) {
      for (int j = 0; j < GRID_SIZE; j++) {
        positions.emplace_back(i, j); // fila i completa: de izquierda a derecha
      }
      return positions;
    }

    if (std::ranges::all_of(grid.cells, [&](const auto &row) { return row[i].symbol == symbol; })) {
      for (int j = 0; j < GRID_SIZE; j++) {
        positions.emplace_back(j, i); // columna i completa: de arriba a abajo
      }
      return positions;
    }
  }

  // Diagonales
  if (std::ranges::all_of(std::views::iota(0, GRID_SIZE), [&](int i) { return grid.cells[i][i].symbol == symbol; })) {
    for (int i = 0; i < GRID_SIZE; i++) {
      positions.emplace_back(i, i); // diagonal principal (de arriba-izquierda a abajo-derecha)
    }
    return positions;
  }

  if (std::ranges::all_of(std::views::iota(0, GRID_SIZE), [&](int i) { return grid.cells[i][GRID_SIZE - 1 - i].symbol == symbol; })) {
    for (int i = 0; i < GRID_SIZE; i++) {
      positions.emplace_back(i, GRID_SIZE - 1 - i); // diagonal anti (de arriba-derecha a abajo-izquierda)
    }
  }

  return positions;
}

/* ¿Hay victoria? Reutiliza la búsqueda: ganó si la lista de casillas ganadoras no está vacía. */
bool Game::IsWin(char symbol) const {
  return !FindCellPositionsWinning(symbol).empty();
}

/* Empate = todas las filas sin ninguna casilla vacía (ganador ya descartado por la cascada anterior). */
bool Game::IsGridFull() const {
  return std::ranges::all_of(grid.cells, [](const auto &row) {
    return std::ranges::none_of(row, [](const Cell &cell) {
      return cell.symbol == ' ';
    });
  });
}

/* Pinta cada frame: casillas, letras, raya de victoria y por encima el anuncio de resultado. */
void Game::Render() const {
  for (const auto &row : grid.cells) {
    for (const auto &cell : row) {
      ctx.renderer.Draw(cell.shape);
      ctx.renderer.Draw(cell.text);
    }
  }

  ctx.renderer.Draw(winningLine);

  ctx.renderer.Draw(resultText); // el anuncio, la capa más alta de todas
}
