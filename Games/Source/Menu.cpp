#include "Menu.h"

using namespace Menu;

Game::Game(EngineContext &context) : Scene(context),
                                     music(*ctx.resources.FetchMusic(MUSIC_FILENAME)) {
  InitButtons();
  InitHeader();
  InitControls();
  InitMusic();
}

/* Distribuye los botones en una grilla dentro de la zona media de la ventana,
   dejando arriba el título y abajo el pie con la ayuda. Los juegos vienen de
   Content/Config.json (gConfig.games). */
void Game::InitButtons() {
  int count = (int)gConfig.games.size();

  int cols = std::min(BUTTONS_PER_ROW, count);
  int rows = (count + cols - 1) / cols;

  const float top = gConfig.windowSize.y * MENU_HEADER_HEIGHT_RATIO;
  const float availableHeight =
      gConfig.windowSize.y * (1.f - MENU_HEADER_HEIGHT_RATIO - MENU_FOOTER_HEIGHT_RATIO);

  sf::Vector2f size(gConfig.windowSize.x / (cols + BUTTONS_PADDING_RATIO),
                    availableHeight / (rows + BUTTONS_PADDING_RATIO));

  sf::Vector2f spacing(size.x * BUTTONS_PADDING_RATIO / (cols + 1),
                       size.y * BUTTONS_PADDING_RATIO / (rows + 1));

  sf::Vector2f start(spacing.x + size.x / 2.f, top + spacing.y + size.y / 2.f);
  sf::Vector2f step(spacing.x + size.x, spacing.y + size.y);

  // Tarjeta detrás de la grilla, al estilo del panel de pausa: mismo fondo y
  // borde lavanda, con un margen de CARD_PADDING alrededor de los botones.
  const sf::Vector2f gridSize(step.x * (cols - 1) + size.x,
                              step.y * (rows - 1) + size.y);
  menuCard.setSize(gridSize + sf::Vector2f(CARD_PADDING * 2, CARD_PADDING * 2));
  menuCard.setRadius(CARD_CORNER_RADIUS);
  menuCard.setFillColor(CARD_COLOR);
  menuCard.setOutlineColor(CARD_BORDER_COLOR);
  menuCard.setOutlineThickness(CARD_OUTLINE_THICKNESS);
  menuCard.setOrigin(menuCard.getGeometricCenter());
  menuCard.setPosition(start - size / 2.f + gridSize / 2.f);

  buttons.reserve(count);

  for (int i = 0; i < count; i++) {
    sf::Vector2f position(start.x + step.x * (i % cols),
                          start.y + step.y * (i / cols));

    auto &button = buttons.emplace_back();

    InitButton(button, {position, size}, gConfig.games[i]);
  }
}

/* Da a un botón su estilo: azul oscuro con borde lavanda y etiqueta centrada. */
void Game::InitButton(Button &button, sf::FloatRect bounds, const GameEntry &game) {
  button.shape.setFillColor(BUTTON_COLOR);
  button.shape.setOutlineColor(BUTTON_BORDER_COLOR);
  button.shape.setOutlineThickness(BUTTON_OUTLINE_THICKNESS);
  button.shape.setSize(bounds.size);
  button.shape.setRadius(BUTTON_CORNER_RADIUS); // esquinas redondeadas, como el overlay
  button.shape.setOrigin(button.shape.getGeometricCenter());
  button.shape.setPosition(bounds.position);

  button.game = game;

  std::string name = game.title;
  std::ranges::replace(name, ' ', '\n'); // Nombres largos parten en dos líneas

  button.text.setFillColor(BUTTON_TEXT_COLOR);
  button.text.setString(name);
  button.text.setCharacterSize(BUTTON_TEXT_SIZE);
  button.text.setOrigin(button.text.getLocalBounds().getCenter());
  button.text.setPosition(bounds.position);
}

/* Prepara el logo y el título del menú arriba y la ayuda de navegación abajo. */
void Game::InitHeader() {
  menuTitle.setString(MENU_TITLE_TEXT);
  menuTitle.setCharacterSize(MENU_TITLE_TEXT_SIZE);
  menuTitle.setFillColor(MENU_TITLE_COLOR);

  const auto titleBounds = menuTitle.getLocalBounds();
  menuTitle.setOrigin(titleBounds.getCenter());

  const float centerX = gConfig.windowSize.x / 2.f;
  const float headerY = gConfig.windowSize.y * MENU_HEADER_HEIGHT_RATIO / 2.f;
  menuTitle.setPosition({centerX, headerY});

  if (auto *texture = ctx.resources.FetchTexture(MENU_LOGO_FILENAME)) {
    menuLogo.setTexture(texture);
    menuLogo.setSize({MENU_LOGO_SIZE, MENU_LOGO_SIZE});
    menuLogo.setOrigin(menuLogo.getGeometricCenter());

    // Calcula dónde empieza el conjunto para centrar el logo y el título juntos.
    const float groupWidth = MENU_LOGO_SIZE + MENU_LOGO_GAP + titleBounds.size.x;
    const float left = centerX - groupWidth / 2.f;
    menuLogo.setPosition({left + MENU_LOGO_SIZE / 2.f, headerY});
    menuTitle.setPosition({
        left + MENU_LOGO_SIZE + MENU_LOGO_GAP + titleBounds.size.x / 2.f,
        headerY});
  }

  menuHint.setString(MENU_HINT_TEXT);
  menuHint.setCharacterSize(MENU_HINT_TEXT_SIZE);
  menuHint.setFillColor(MENU_HINT_COLOR);
  menuHint.setOrigin(menuHint.getLocalBounds().getCenter());
  menuHint.setPosition({centerX,
                        gConfig.windowSize.y * (1.f - MENU_FOOTER_HEIGHT_RATIO / 2.f)});
}

/* Prepara panel y textos de la pantalla de controles; el contenido por juego
   se asigna al mostrarla. */
void Game::InitControls() {
  controls.backdrop.setFillColor(CONTROLS_BACKDROP_COLOR);
  controls.backdrop.setSize(gConfig.windowSize);

  controls.panel.setFillColor(CONTROLS_PANEL_COLOR);
  controls.panel.setOutlineColor(CONTROLS_PANEL_BORDER_COLOR);
  controls.panel.setOutlineThickness(CONTROLS_PANEL_OUTLINE_THICKNESS);
  controls.panel.setSize(gConfig.windowSize.componentWiseMul(CONTROLS_PANEL_SIZE_RATIO));
  controls.panel.setOrigin(controls.panel.getGeometricCenter());
  controls.panel.setPosition(gConfig.windowSize / 2.f);

  controls.title.setCharacterSize(CONTROLS_TITLE_TEXT_SIZE);
  controls.title.setFillColor(CONTROLS_TITLE_COLOR);

  controls.body.setCharacterSize(CONTROLS_BODY_TEXT_SIZE);
  controls.body.setFillColor(CONTROLS_TEXT_COLOR);

  controls.hint.setCharacterSize(CONTROLS_HINT_TEXT_SIZE);
  controls.hint.setFillColor(MENU_HINT_COLOR);
  controls.hint.setString("Click derecho / West: volver");
}

/* Música de fondo con volumen y tono configurados. */
void Game::InitMusic() {
  music.setVolume(MUSIC_VOLUME);
  music.setPitch(MUSIC_PITCH);
}

/* Al entrar en la escena se cierra la pantalla de controles y suena la música. */
void Game::Start() {
  StartControls();
  StartMusic();
}

void Game::StartControls() {
  controls.current.reset();
}

void Game::StartMusic() {
  music.play();
}

/* Reparte cada evento al manejador que le corresponde. */
void Game::OnEvent(const sf::Event &event) {
  event.visit([this](const auto &type) { this->HandleEvent(type); });
}

/* Mouse: clic izquierdo lanza el juego bajo el cursor; el derecho muestra o
   cierra la pantalla de controles. */
void Game::HandleEvent(const sf::Event::MouseButtonPressed &mouse) {
  if (mouse.button == sf::Mouse::Button::Left && !controls.current) {
    EventSceneLoad();
  } else if (mouse.button == sf::Mouse::Button::Right) {
    EventControlsShow();
  }
}

/* Mando: el botón Sur (A) lanza el juego y el Oeste (X) alterna los controles. */
void Game::HandleEvent(const sf::Event::JoystickButtonPressed &joystick) {
  if (auto button = Input::HardwareToLogical(joystick.button, joystick.joystickId)) {
    if (*button == GamepadButton::South && !controls.current) {
      EventSceneLoad();
    } else if (*button == GamepadButton::West) {
      EventControlsShow();
    }
  }
}

/* Solo se anima el hover de los botones; la pantalla de controles es estática. */
void Game::Update() {
  if (!controls.current) {
    UpdateButtons();
  }
}

void Game::UpdateButtons() {
  for (auto &button : buttons) {
    UpdateButton(button);
  }
}

/* El botón bajo el cursor se ilumina y su texto pasa a cian. */
void Game::UpdateButton(Button &button) {
  const bool hovered = IsButtonHovered(button);
  const sf::Color fill = hovered ? BUTTON_HOVERED_COLOR : BUTTON_COLOR;
  const sf::Color text = hovered ? BUTTON_TEXT_HOVERED_COLOR : BUTTON_TEXT_COLOR;

  if (button.shape.getFillColor() != fill) {
    button.shape.setFillColor(fill);
  }
  if (button.text.getFillColor() != text) {
    button.text.setFillColor(text);
  }
}

/* Cambia a la escena del botón bajo el cursor. */
void Game::EventSceneLoad() {
  for (const auto &button : buttons) {
    if (IsButtonHovered(button)) {
      ctx.scenes.ChangeScene(button.game.name);
      return;
    }
  }
}

/* Muestra los controles del juego bajo el cursor, o los cierra si ya había
   una pantalla abierta. Los textos cambian por juego: se recentran. */
void Game::EventControlsShow() {
  if (controls.current) {
    controls.current.reset();
    return;
  }

  for (std::size_t i = 0; i < buttons.size(); i++) {
    if (IsButtonHovered(buttons[i])) {
      controls.current = (int)i;

      const GameEntry &game = buttons[i].game;

      controls.title.setString(game.title);
      controls.title.setOrigin(controls.title.getLocalBounds().getCenter());
      controls.title.setPosition({controls.panel.getPosition().x,
                                  controls.panel.getPosition().y -
                                      controls.panel.getSize().y / 2.f + 60.f});

      controls.body.setString(game.controls);
      controls.body.setOrigin(controls.body.getLocalBounds().getCenter());
      controls.body.setPosition(controls.panel.getPosition());

      controls.hint.setOrigin(controls.hint.getLocalBounds().getCenter());
      controls.hint.setPosition({controls.panel.getPosition().x,
                                 controls.panel.getPosition().y +
                                     controls.panel.getSize().y / 2.f + 40.f});
      return;
    }
  }
}

/* El cursor está sobre el botón. */
bool Game::IsButtonHovered(const Button &button) const {
  return Contains(button.shape, ctx.cursor.GetPosition());
}

/* Dibujo del menú principal: encabezado, botones y pie. */
void Game::Render() const {
  if (controls.current) {
    RenderControls();
    return;
  }

  RenderMenu();
}

void Game::RenderMenu() const {
  ctx.renderer.Draw(menuCard); // la tarjeta va primero, detrás de todo

  if (menuLogo.getTexture()) {
    ctx.renderer.Draw(menuLogo);
  }
  ctx.renderer.Draw(menuTitle);
  ctx.renderer.Draw(menuHint);

  for (const auto &button : buttons) {
    ctx.renderer.Draw(button.shape);
    ctx.renderer.Draw(button.text);
  }
}

/* Dibujo de los controles: fondo oscurecido, panel, título del juego,
   acciones y ayuda para volver. */
void Game::RenderControls() const {
  ctx.renderer.Draw(controls.backdrop);
  ctx.renderer.Draw(controls.panel);
  ctx.renderer.Draw(controls.title);
  ctx.renderer.Draw(controls.body);
  ctx.renderer.Draw(controls.hint);
}

/* La música sigue el estado de pausa de la ventana. */
void Game::OnPause(bool paused) {
  if (paused) {
    music.pause();
  } else {
    music.play();
  }
}

/* Al salir de la escena detiene la música. */
void Game::OnCleanup() {
  music.stop();
}
