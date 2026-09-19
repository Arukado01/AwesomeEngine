#include "Core/Engine.h"
#include "Core/EngineConfig.h"
#include "Utils/Log.h"

// Constructor
Engine::Engine()
    : window_(sf::VideoMode(sf::Vector2u(gConfig.windowSize)),
              gConfig.windowTitle),
      context_(window_),
      scenes_(SceneFactory::CreateScenes(context_)),
      currentScene_(nullptr) {
  // Configuración inicial de la ventana
  window_.setIcon(sf::Image("Content/Textures/Icon.png"));
  window_.setMinimumSize(window_.getSize() / 2u); // Mitad del tamaño actual como mínimo
  window_.setKeyRepeatEnabled(false); // Evita repetición de teclas al mantener presionadas
  window_.setMouseCursorVisible(false); // Oculta el cursor del sistema (AwesomeEngine dibuja el suyo)

  // Si disableSfmlLogs es true, silencia los logs propios de SFML
  // (redirige el stream de errores a null para usar el sistema propio de logs)
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

    // El visitor maneja eventos globales del motor (cerrar, redimensionar, etc.)
    event->visit(EngineVisitor{*this});

    // La GUI procesa el evento para sus propios widgets
    context_.gui.ProcessEvent(*event);

    // La escena actual reacciona al evento
    currentScene_->OnEvent(*event);
  }
}

/** Actualiza la lógica del juego una vez por frame */
void Engine::Update() {
  // Actualiza el tiempo y el cursor con el delta time del frame
  context_.time.Update();
  context_.cursor.Update(context_.time.GetDeltaTime());

  // Actualiza la lógica de la escena actual
  currentScene_->Update();
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
  currentScene_->OnPause(false);
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
  // En debug, aborta si el nombre no existe en el registro de escenas
  assert(scenes_.contains(name));

  // Obtiene el puntero crudo de la nueva escena (el mapa conserva el ownership)
  Scene *nextScene = scenes_.at(name).get();

  // Limpia la escena actual antes de salir (si existe)
  if (currentScene_) {
    currentScene_->OnCleanup();
  }

  // Reinicia el estado de entrada para que la nueva escena empiece limpia
  context_.input.Clear();

  // Cambia a la nueva escena e inicializa su estado
  currentScene_ = nextScene;
  currentScene_->Start();
}

/** Evento: reiniciar la escena actual */
void Engine::EventSceneRestart() { context_.scenes.RestartCurrentScene(); }

/** Evento: volver al menú principal */
void Engine::EventSceneMenuReturn() { context_.scenes.ChangeScene("Menu"); }
