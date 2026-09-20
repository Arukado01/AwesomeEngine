#include "Core/Engine.h"
#include "Core/EngineConfig.h"
#include "Core/Overlay.h"
#include "Utils/Log.h"

// Constructor
Engine::Engine() : window_(sf::VideoMode(sf::Vector2u(gConfig.windowSize)),
                           gConfig.windowTitle),
                   context_(window_),
                   scenes_(SceneFactory::CreateScenes(context_)),
                   currentScene_(nullptr),
                   overlay_(context_.gui),
                   cursorWasVisible_(true) {

  // Configuración inicial de la ventana
  window_.setIcon(sf::Image("Content/Textures/Icon.png"));
  window_.setMinimumSize(window_.getSize() / 2u); // Mitad del tamaño actual como mínimo
  window_.setKeyRepeatEnabled(false); // Evita repetición de teclas al mantener presionadas
  window_.setMouseCursorVisible(false); // Oculta el cursor del sistema (AwesomeEngine dibuja el suyo)

  // Si disableSfmlLogs es true, silencia los logs propios de SFML (redirige el stream de errores a null para usar el sistema propio de logs)
  if (gConfig.disableSfmlLogs) {
    sf::err().rdbuf(nullptr);
  }

  // Aplica el volumen global de audio definido en la configuración
  context_.audio.SetGlobalVolume(gConfig.globalVolume);

  LOG_INFO("Window created");
}

/** Indica si el bucle principal debe seguir ejecutándose */
bool Engine::IsRunning() const { return window_.isOpen(); }

/** Procesa todos los eventos de la ventana pendientes en este frame */
void Engine::ProcessEvent() {
  // Si hay un cambio de escena pendiente, lo aplica antes de procesar eventos
  if (const auto nextScene = context_.scenes.FetchNextScene()) {
    EventSceneChange(*nextScene);
  }

  // Extrae y reparte cada evento de la cola de la ventana
  while (const auto event = window_.pollEvent()) {

    event->visit(EngineVisitor{*this}); // El visitor maneja eventos globales del motor (cerrar, redimensionar, etc.)
    context_.gui.ProcessEvent(*event); // La GUI procesa el evento para sus propios widgets

    // Reenviamos los eventos a la escena actual solo si overlay está oculta durante la actualización.
    if (!overlay_.IsVisible()) {
      currentScene_->OnEvent(*event); // La escena actual reacciona al evento
    }
  }

  // Si el usuario seleccionó una opción del overlay de pausa,
  // la consume y la procesa (FetchSelection devuelve vacío tras leerla)
  if (const auto selection = overlay_.FetchSelection()) {
    EventOverlaySelect(*selection);
  }
}

/** Actualiza la lógica del juego una vez por frame */
void Engine::Update() {
  // Actualiza el tiempo y el cursor con el delta time del frame
  context_.time.Update();
  context_.cursor.Update(context_.time.GetDeltaTime());

  // Reenviamos los eventos a la escena actual solo si overlay está oculta durante la actualización.
  if (!overlay_.IsVisible()) {
    currentScene_->Update(); // Actualiza la lógica de la escena actual
  }
}

/** Dibuja el frame completo en la ventana */
void Engine::Render() {
  window_.clear(); // Limpia el frame anterior

  // Renderiza la escena a un buffer intermedio y luego lo dibuja en la ventana
  context_.renderer.BeginDrawing();
  currentScene_->Render();
  window_.draw(sf::Sprite(context_.renderer.FinishDrawing()));

  // Encima de la escena se dibujan la GUI y el cursor personalizado
  context_.gui.Render();
  context_.cursor.Render();

  window_.display(); // Muestra el frame terminado
}

/** Evento: el usuario cerró la ventana */
void Engine::EventWindowClose() {
  window_.close();
  LOG_INFO("Window closed");
}

/** Evento: la ventana cambió de tamaño */
void Engine::EventWindowResized(sf::Vector2u size) {
  LOG_INFO("Window resized to: {}x{}", size.x, size.y);
}

/** Evento: la ventana perdió el foco (la escena se pausa) */
void Engine::EventWindowFocusLost() {
  currentScene_->OnPause(true);
  LOG_INFO("Window focus lost");
}

/** Evento: la ventana recuperó el foco (la escena se reanuda) */
void Engine::EventWindowFocusGained() {
  currentScene_->OnPause(overlay_.IsVisible());
  LOG_INFO("Window focus gained");
}

/** Evento: se conectó un gamepad */
void Engine::EventGamepadConnected(int id) {
  LOG_INFO("Gamepad {} connected", id);
}

/** Evento: se desconectó un gamepad */
void Engine::EventGamepadDisconnected(int id) {
  LOG_INFO("Gamepad {} disconnected", id);
}

/** Evento: el usuario pidió una captura de pantalla */
void Engine::EventWindowScreenshot() const { context_.screenshot.Take(); }

/** Evento: cambio de escena por nombre */
void Engine::EventSceneChange(const std::string &name) {

  assert(scenes_.contains(name)); // En debug, aborta si el nombre no existe en el registro de escenas

  Scene *nextScene = scenes_.at(name).get(); // Obtiene el puntero crudo de la nueva escena (el mapa conserva el ownership)

  // Limpia la escena actual antes de salir (si existe)
  if (currentScene_) {
    currentScene_->OnCleanup();
  }

  context_.input.Clear(); // Reinicia el estado de entrada para que la nueva escena empiece limpia

  currentScene_ = nextScene; // Cambia a la nueva escena e inicializa su estado
  currentScene_->Start();
}

/** Evento: reiniciar la escena actual */
void Engine::EventSceneRestart() {
  overlay_.SetVisible(false);
  context_.scenes.RestartCurrentScene();
}

/** Evento: volver al menú principal */
void Engine::EventSceneMenuReturn() {
  // Cierra el overlay de pausa antes de cambiar de escena
  overlay_.SetVisible(false);
  context_.scenes.ChangeScene("Menu");

  // Restaura el cursor del juego para navegar el menú
  context_.cursor.SetVisible(true);
  context_.cursor.SetSpeed(gConfig.cursorSpeed); // Restablece su velocidad por defecto
}

/** Evento: alterna el estado de pausa (muestra/oculta el overlay de pausa) y pausa/reanuda la escena en consecuencia */
void Engine::EventOverlayPauseToggle() {
  // Calcula el nuevo estado: si estaba oculto, ahora se muestra (y viceversa)
  const bool overlayVisible = !overlay_.IsVisible();
  overlay_.SetVisible(overlayVisible);

  const bool cursorVisible = context_.cursor.IsVisible(); // Cursor: guarda su visibilidad actual antes de modificarla
  context_.cursor.SetVisible(overlayVisible || cursorWasVisible_); // Con el overlay abierto se oculta el cursor del juego; al cerrarlo se recupera la visibilidad que tenía antes de la pausa
  cursorWasVisible_ = cursorVisible; // Recuerda el estado para restaurarlo luego

  currentScene_->OnPause(overlayVisible); // Pausa la escena al abrir el overlay; la reanuda al cerrarlo
  LOG_INFO(overlayVisible ? "Game paused" : "Game resumed");
}

/** Evento: el usuario eligió una opción del overlay de pausa. Redirige la elección al evento del motor correspondiente */
void Engine::EventOverlaySelect(OverlaySelection selection) {
  switch (selection) {
  case OverlaySelection::Resume: // Continuar el juego (cierra el overlay)
    EventOverlayPauseToggle();
    break;
  case OverlaySelection::Restart: // Reiniciar la escena actual
    EventSceneRestart();
    break;
  case OverlaySelection::Menu: // Volver al menú principal
    EventSceneMenuReturn();
    break;
  case OverlaySelection::Quit: // Cerrar la ventana y salir
    EventWindowClose();
    break;
  default:
    break;
  }
}
