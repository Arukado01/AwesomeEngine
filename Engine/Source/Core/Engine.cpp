#include "Core/Engine.h"
#include "Core/EngineConfig.h"
#include "Core/Overlay.h"
#include "Utils/Log.h"

/** Constructor */
Engine::Engine() : window_(sf::VideoMode(sf::Vector2u(gConfig.windowSize)),
                           gConfig.windowTitle),
                   context_(window_),
                   scenes_(SceneFactory::CreateScenes(context_)),
                   currentScene_(nullptr),
                   overlay_(context_.gui),
                   cursorWasVisible_(true) {

  /**  Configuración inicial de la ventana */
  window_.setIcon(sf::Image("Content/Textures/Icon.png"));
  window_.setMinimumSize(window_.getSize() / 2u); // Mitad del tamaño actual como mínimo
  window_.setKeyRepeatEnabled(false); // Evita repetición de teclas al mantener presionadas
  window_.setMouseCursorVisible(false); // Oculta el cursor del sistema (AwesomeEngine dibuja el suyo)

  if (gConfig.disableSfmlLogs) { // Si disableSfmlLogs es true, silencia los logs propios de SFML (redirige el stream de errores a null para usar el sistema propio de logs)
    sf::err().rdbuf(nullptr);
  }

  LOG_INFO("Window created");

  context_.audio.SetGlobalVolume(gConfig.globalVolume); // Aplica el volumen global de audio definido en la configuración
  context_.scenes.ChangeScene("Bounce");
}

/** Indica si el bucle principal debe seguir ejecutándose */
bool Engine::IsRunning() const { return window_.isOpen(); }

/** Procesa todos los eventos de la ventana pendientes en este frame */
void Engine::ProcessEvent() {
  if (const auto nextScene = context_.scenes.FetchNextScene()) { // Si hay un cambio de escena pendiente, lo aplica antes de procesar eventos
    EventSceneChange(*nextScene);
  }

  while (const auto event = window_.pollEvent()) { // Extrae y reparte cada evento de la cola de la ventana

    event->visit(EngineVisitor{*this}); // El visitor maneja eventos globales del motor (cerrar, redimensionar, etc.)
    context_.gui.ProcessEvent(*event); // La GUI procesa el evento para sus propios widgets

    if (!overlay_.IsVisible()) { // Reenviamos los eventos a la escena actual solo si el overlay está oculto (durante la pausa la GUI consume la interacción)
      currentScene_->OnEvent(*event); // La escena actual reacciona al evento
    }
  }

  if (const auto selection = overlay_.FetchSelection()) { // Si el usuario seleccionó una opción del overlay de pausa, la consume y la procesa (FetchSelection devuelve vacío tras leerla)
    EventOverlaySelect(*selection);
  }
}

/** Actualiza la lógica del juego una vez por frame */
void Engine::Update() {

  context_.time.Update(); // Actualiza el tiempo con el delta time del frame
  context_.cursor.Update(context_.time.GetDeltaTime()); // Actualiza el cursor con el delta time del frame

  if (!overlay_.IsVisible()) { // Actualizamos la escena solo si el overlay está oculto (con la pausa abierta el juego se congela)
    currentScene_->Update(); // Actualiza la lógica de la escena actual
  }
}

/** Dibuja el frame completo en la ventana */
void Engine::Render() {
  window_.clear(); // Limpia el frame anterior

  context_.renderer.BeginDrawing(); // Renderiza la escena a un buffer intermedio ->
  currentScene_->Render();
  window_.draw(sf::Sprite(context_.renderer.FinishDrawing())); // -> y luego lo dibuja en la ventana

  context_.gui.Render(); // Encima de la escena se dibuja la GUI
  context_.cursor.Render(); // y también el cursor personalizado

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

/** Evento: la ventana recuperó el foco (la escena se reanuda, salvo que el overlay esté abierto, en cuyo caso sigue pausada) */
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

  const bool overlayVisible = !overlay_.IsVisible(); // Calcula el nuevo estado: si estaba oculto, ahora se muestra (y viceversa)
  overlay_.SetVisible(overlayVisible);

  const bool cursorVisible = context_.cursor.IsVisible(); // Cursor: guarda su visibilidad actual antes de modificarla
  context_.cursor.SetVisible(overlayVisible || cursorWasVisible_); // Con el overlay abierto el cursor siempre se muestra (para navegar la GUI); al cerrarlo se recupera la visibilidad previa a la pausa
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
