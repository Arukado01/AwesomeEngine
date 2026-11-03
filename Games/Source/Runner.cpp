#include "Runner.h"

#include <algorithm>
#include <cmath>
#include <format>
#include <stdexcept>

using namespace Runner;

/* Carga las texturas y registra la música y los efectos de la escena. */
Game::Game(EngineContext &context)
    : Scene(context), player(LoadTexture(context, PLAYER_TEXTURE_FILENAME)), audioGroup(context.audio.CreateGroup()) {

  InitEnvironment(); // El entorno y la pantalla de derrota se preparan una vez al crear la escena.
  gameOver.restartCooldown.Bind(ctx.time);
  InitGameOver();

  // Cargar música.
  if (auto track = ctx.resources.FetchMusic(MUSIC_FILENAME)) {
    music = audioGroup.AddMusic(std::move(*track), MUSIC_VOLUME, true); // El grupo conserva la pista y aplica el volumen configurado para esta escena.
  }

  // Se recorre las configuraciones de efectos.
  for (std::size_t index = 0; index < soundEffects.size(); ++index) {
    const auto &config = PLAYER_SOUNDS[index]; // En cada recorrido se revisa el archivo, su volumen y si se reproduce en bucle.
    soundEffects[index] = audioGroup.AddSound(ctx.resources.AcquireSound(config.filename), config.volume, config.looping); // Si un archivo de sonido falta o está vacío, se omite sin detener la partida.
  }
}

/* Carga una textura válida y configura su repetición y nitidez. */
sf::Texture &Game::LoadTexture(EngineContext &context, const std::string &filename, bool repeated) {
  auto *texture = context.resources.FetchTexture(filename);

  if (!texture || texture->getSize() == sf::Vector2u{}) {
    throw std::runtime_error("No se pudo cargar la textura del runner: " + filename);
  }

  texture->setSmooth(false); // Desactiva el suavizado para que el pixel art se vea nítido.
  texture->setRepeated(repeated); // Permite repetir las texturas que se desplazan como fondo.
  return *texture;
}

/* Prepara las capas del fondo, la plataforma y la textura de los obstáculos. */
void Game::InitEnvironment() {
  // El borde de la plataforma ocupa todo el ancho y se sitúa a mitad de la ventana.
  ground.setSize({gConfig.windowSize.x, GROUND_HEIGHT});
  ground.setPosition({0, gConfig.windowSize.y / 2});

  // Recorre las capas del fondo
  for (std::size_t index = 0; index < backgroundLayers.size(); ++index) {
    auto &layer = backgroundLayers[index]; // Cada capa tiene su propia textura y posteriormente avanzará a una velocidad distinta.
    layer.shape.setTexture(&LoadTexture(ctx, BACKGROUND_TEXTURE_FILENAMES[index], true));
    layer.shape.setSize({gConfig.windowSize.x, ground.getPosition().y});
    layer.shape.setFillColor(BACKGROUND_TINT);
  }

  // El borde y el frente de la plataforma comparten una misma textura repetida.
  auto &platformTexture = LoadTexture(ctx, PLATFORM_TEXTURE_FILENAME, true);
  ground.setTexture(&platformTexture);
  ground.setFillColor(sf::Color::White);

  platformFront.setTexture(&platformTexture);
  platformFront.setPosition({0, ground.getPosition().y + GROUND_HEIGHT});
  platformFront.setSize({gConfig.windowSize.x, gConfig.windowSize.y - platformFront.getPosition().y});

  // La ciudad cercana cubre el frente de la plataforma sin modificar su altura.
  foregroundCity.shape.setTexture(&LoadTexture(ctx, FOREGROUND_TEXTURE_FILENAME, true));
  foregroundCity.shape.setFillColor(FOREGROUND_TINT);
  foregroundCity.shape.setPosition(platformFront.getPosition());
  foregroundCity.shape.setSize(platformFront.getSize());

  // Guarda la textura que contiene las variantes visuales de los obstáculos
  obstacleTexture = &LoadTexture(ctx, OBSTACLE_TEXTURE_FILENAME);
}

/* Configura el fondo, los textos y la duración de la pantalla de derrota. */
void Game::InitGameOver() {
  // Las posiciones se expresan como proporciones del tamaño de la ventana.
  const auto initText = [](sf::Text &text, unsigned int size, sf::Color color, sf::Vector2f position) {
    text.setCharacterSize(size);
    text.setFillColor(color);
    text.setPosition(gConfig.windowSize.componentWiseMul(position));
  };
  gameOver.restartCooldown.SetDuration(GAME_OVER_RESTART_DELAY);
  gameOver.backdrop.setSize(gConfig.windowSize);
  gameOver.backdrop.setFillColor(GAME_OVER_BACKDROP_COLOR);
  gameOver.title.setString(GAME_OVER_TITLE);
  initText(gameOver.title, GAME_OVER_TITLE_SIZE, GAME_OVER_TITLE_COLOR, GAME_OVER_TITLE_POSITION);
  gameOver.title.setOrigin(gameOver.title.getLocalBounds().getCenter());
  initText(gameOver.distanceText, GAME_OVER_DISTANCE_TEXT_SIZE, GAME_OVER_DISTANCE_TEXT_COLOR, GAME_OVER_DISTANCE_TEXT_POSITION);
  initText(gameOver.restartText, GAME_OVER_RESTART_TEXT_SIZE, GAME_OVER_RESTART_TEXT_COLOR, GAME_OVER_RESTART_TEXT_POSITION);
}

/* Reinicia el estado de la partida y comienza la música. */
void Game::Start() {
  // Evita que se escuche algún sonido que haya quedado de la ronda anterior.
  audioGroup.StopAll();
  ctx.cursor.SetVisible(false);

  BindInputs();

  // Limpia el estado visual y el temporizador antes de comenzar la nueva partida.
  obstacles.clear();
  ctx.particles.Clear();
  gameOver.active = false;
  gameOver.restartRequested = false;
  gameOver.restartCooldown.Reset();

  // Los desplazamientos vuelven a cero para que el escenario empiece desde el inicio.
  for (auto &layer : backgroundLayers) {
    layer.offset = 0;
  }
  platformOffset = 0;
  foregroundCity.offset = 0;

  StartStats();
  UpdateEnvironment(0); // Prepara los recortes iniciales sin avanzar el escenario.
  StartPlayer();

  EventObstacleSpawn();
  if (music) {
    audioGroup.SetSourceVolume(*music, MUSIC_VOLUME);
    // Reproduce la música registrada al comenzar la ronda.
    audioGroup.Play(*music);
  }
}

/* Asocia los controles de salto, deslizamiento y caída al teclado y al mando. */
void Game::BindInputs() {
  // W o el botón sur del mando impulsan al personaje hacia arriba.
  ctx.input.Bind(Jump, Input::Keyboard{sf::Keyboard::Scan::W});
  ctx.input.Bind(Jump, Input::Gamepad{GamepadButton::South});

  // S o el botón oeste del mando solicitan el deslizamiento cuando hay apoyo.
  ctx.input.Bind(Crouch, Input::Keyboard{sf::Keyboard::Scan::S});
  ctx.input.Bind(Crouch, Input::Gamepad{GamepadButton::West});

  // La misma tecla S aumenta la gravedad cuando el personaje está en el aire.
  ctx.input.Bind(Fall, Input::Keyboard{sf::Keyboard::Scan::S});
  ctx.input.Bind(Fall, Input::Axis{sf::Joystick::Axis::Y});
}

/* Reinicia el jugador y lo coloca sobre el suelo con su primer cuadro. */
void Game::StartPlayer() {
  // Al reiniciar, descarta la animación anterior antes de crear una nueva.
  ctx.animations.Remove(player.animator);
  // Se conserva la textura, pero se reinician la velocidad, animación y demás estados.
  player = Player(player.sprite.getTexture());
  player.animator = ctx.animations.Create();
  player.sprite.setOrigin({PLAYER_FRAME_SIZE.x / 2.f, static_cast<float>(PLAYER_FRAME_SIZE.y)}); // El origen del sprite queda en sus pies.
  player.sprite.setScale(PLAYER_SPRITE_SCALE);
  player.shape.setSize(PLAYER_SIZE);
  player.shape.setOrigin(player.shape.getGeometricCenter());

  float positionY = ground.getPosition().y - player.shape.getSize().y / 2;
  player.shape.setPosition({gConfig.windowSize.x / 6, positionY});

  // Conserva la caja anterior para detectar contactos durante el movimiento.
  player.previousBounds = player.shape.getGlobalBounds();
  UpdatePlayerAnimation(false); // Coloca el primer cuadro sin reproducir una pisada al iniciar.
}

/* Reinicia la velocidad, la distancia y la separación de obstáculos. */
void Game::StartStats() {
  stats.distanceText.setFillColor(STATS_DISTANCE_TEXT_COLOR);
  stats.distanceText.setPosition({10, 10});
  stats.speed = STATS_SPEED_START;
  stats.obstacleNextX = ctx.random.Float(OBSTACLE_MIN_DISTANCE, OBSTACLE_MAX_DISTANCE);

  stats.distance = 0;
  stats.distanceText.setString("Distance: 0");
}

/* Actualiza la simulación o mantiene el resultado visible mientras se espera el reinicio. */
void Game::Update() {
  // Durante Game Over solo se actualiza la espera del reinicio.
  if (gameOver.active) {
    UpdateGameOver();
    return;
  }

  // Los efectos existentes se mueven antes de generar nuevas partículas este frame.
  UpdateEffects();
  UpdateEnvironment(ctx.time.GetDeltaTime());
  // Solo se puede saltar desde una superficie y cuando no hay deslizamiento activo.
  if (ctx.input.Pressed(Jump) && player.grounded && !player.sliding) {
    player.velocity.y = PLAYER_JUMP_VELOCITY;
    player.grounded = false;
    PlayPlayerSound(PlayerSound::Jump);
  }

  UpdatePlayer();
  UpdateObstacles();
  UpdateStats();

  // Las colisiones y el apoyo se resuelven antes de elegir la animación visible.
  HandleCollisionsPlayerObstacles();
  UpdatePlayerGrounded();
  UpdatePlayerAnimation();
  UpdatePlayerEffects();
}

/* Ajusta el tamaño del jugador y aplica la gravedad y el movimiento vertical. */
void Game::UpdatePlayer() {
  UpdatePlayerSize();
  // Conserva la caja anterior para detectar contactos durante el movimiento.
  player.previousBounds = player.shape.getGlobalBounds();
  const float delta = ctx.time.GetDeltaTime();
  // Mantener caída rápida aumenta la gravedad únicamente mientras está en el aire.
  const float gravity = PLAYER_GRAVITY * (ctx.input.Pressed(Fall) && !player.grounded ? PLAYER_FALL_FACTOR : 1.f);
  player.velocity.y += gravity * delta;
  player.shape.move({0, player.velocity.y * delta});
  player.grounded = false; // El suelo o un obstáculo confirmarán el apoyo después del movimiento.
}

/* Controla la entrada y salida del deslizamiento sin levantar al jugador bajo un obstáculo. */
void Game::UpdatePlayerSize() {
  // El cambio de caja solo se permite mientras el personaje tiene apoyo.
  if (!player.grounded) {
    return;
  }

  const bool requested = ctx.input.Pressed(Crouch);
  if (player.sliding) {
    if (requested) {
      if (player.slidePhase == SlidePhase::Recover) {
        SetSlidePhase(SlidePhase::Loop);
      }
      return;
    }

    // La caja se mantiene baja mientras un obstáculo impida levantarse.
    if (!CanPlayerStand()) {
      SetSlidePhase(SlidePhase::Loop);
      return;
    }
    // Al soltar el control comienza la recuperación antes de volver a la caja alta.
    if (player.slidePhase != SlidePhase::Recover) {
      SetSlidePhase(SlidePhase::Recover);
      return;
    }
    const auto &clip = PLAYER_ANIMATIONS[static_cast<std::size_t>(PlayerAnimation::Slide)];
    // La caja baja se mantiene hasta completar los cuadros de recuperación.
    if (player.slidePhaseTime < SLIDE_RECOVER_FRAME_COUNT / clip.framesPerSecond) {
      return;
    }
  } else if (!requested) {
    return;
  }

  const bool sliding = !player.sliding;
  const float oldHeight = player.shape.getSize().y;
  const auto newSize = sliding ? PLAYER_SLIDE_SIZE : PLAYER_SIZE;
  player.shape.setSize(newSize);
  player.shape.setOrigin(player.shape.getGeometricCenter());
  // Compensa el cambio de altura para mantener los pies sobre la superficie.
  player.shape.move({0, (oldHeight - newSize.y) / 2});
  player.sliding = sliding;
  SetSlidePhase(SlidePhase::Prepare);
}

/* Cambia la fase del deslizamiento y reinicia su temporizador. */
void Game::SetSlidePhase(SlidePhase phase) {
  if (player.slidePhase != phase) {
    player.slidePhase = phase;
    player.slidePhaseTime = 0;
  }
}

/* Comprueba si hay espacio suficiente para volver a ponerse de pie. */
bool Game::CanPlayerStand() const {
  const auto bounds = player.shape.getGlobalBounds();
  // Construye la caja que ocuparía el personaje de pie, manteniendo la misma base.
  const sf::FloatRect standingBounds({player.shape.getPosition().x - PLAYER_SIZE.x / 2,
                                      bounds.position.y + bounds.size.y - PLAYER_SIZE.y},
                                     PLAYER_SIZE);
  return std::none_of(obstacles.begin(), obstacles.end(), [&](const Obstacle &obstacle) {
    const auto &obstacleBounds = obstacle.bounds;
    return obstacleBounds.position.y < bounds.position.y + bounds.size.y - PLAYER_COLLISION_TOLERANCE &&
           ctx.collisions.Intersect(standingBounds, obstacleBounds).has_value();
  });
}

/* Apoya al jugador sobre el suelo cuando sus pies llegan a la plataforma. */
void Game::UpdatePlayerGrounded() {
  float halfHeight = player.shape.getSize().y / 2;

  // Si los pies cruzan el suelo, corrige la posición y detiene la caída.
  if (player.shape.getPosition().y + halfHeight >= ground.getPosition().y) {
    player.shape.setPosition({player.shape.getPosition().x, ground.getPosition().y - halfHeight});
    player.velocity.y = 0;
    player.grounded = true;
  }
}

/* Selecciona la animación según el estado y actualiza el cuadro y la posición del sprite. */
void Game::UpdatePlayerAnimation(bool emitSound) {
  auto *animator = ctx.animations.Get(player.animator);
  if (!animator) {
    // Si el manager se limpió, recupera el animador y vuelve a colocar el cuadro.
    player.animator = ctx.animations.Create();
    animator = ctx.animations.Get(player.animator);
    player.animationFrame = -1;
  }
  // El deslizamiento tiene prioridad; sin apoyo se elige salto o caída por la velocidad vertical.
  auto animation = PlayerAnimation::Run;
  if (player.sliding) {
    animation = PlayerAnimation::Slide;
  } else if (!player.grounded) {
    animation = player.velocity.y < 0 ? PlayerAnimation::Jump : PlayerAnimation::Fall;
  }
  // Al cambiar de animación se reinicia el avance y se fuerza el primer recorte.
  if (animation != player.animation) {
    player.animation = animation;
    animator->Reset();
    player.animationFrame = -1;
    // El contacto inicial de la plancha levanta una ráfaga de polvo.
    if (animation == PlayerAnimation::Slide && player.grounded) {
      const auto bounds = player.shape.getGlobalBounds();
      EmitDust({bounds.position.x, bounds.position.y + bounds.size.y}, SLIDE_DUST_BURST_COUNT, true);
    }
  }

  const auto &clip = PLAYER_ANIMATIONS[static_cast<std::size_t>(animation)];
  int frame = 0;
  if (animation == PlayerAnimation::Run) {
    // La carrera acelera con el juego, pero tiene un límite para conservar la legibilidad.
    const float playbackSpeed = std::min(clip.framesPerSecond * stats.speed / STATS_SPEED_START, PLAYER_ANIMATION_MAX_FPS);

    // El cuadro inicial se procesa aquí porque Advance solo notifica cuadros cruzados.
    if (player.animationFrame < 0) {
      EventPlayerFootstep(PLAYER_FOOTSTEP_FRAMES.front(), emitSound);
    }

    // Comprueba todos los cuadros cruzados para no perder pisadas en frames largos.
    frame = animator->Advance(ctx.time.GetDeltaTime(),
                              clip.frameCount,
                              playbackSpeed,
                              [&](int crossedFrame) {
                                if (std::ranges::find(PLAYER_FOOTSTEP_FRAMES, crossedFrame) != PLAYER_FOOTSTEP_FRAMES.end()) {
                                  EventPlayerFootstep(crossedFrame, emitSound);
                                }
                              });

  } else if (animation == PlayerAnimation::Slide) {
    // Las fases del deslizamiento no avanzan mientras el personaje está en el aire.
    if (player.grounded) {
      player.slidePhaseTime += ctx.time.GetDeltaTime();
    }
    if (player.slidePhase == SlidePhase::Prepare && player.slidePhaseTime >= SLIDE_PREPARE_DURATION) {
      // Después de la preparación comienza el ciclo de deslizamiento.
      SetSlidePhase(SlidePhase::Loop);
    }
    if (player.slidePhase == SlidePhase::Loop) {
      // El resto de la división hace que los cuadros de la plancha vuelvan a comenzar.
      const float cycleDuration = SLIDE_LOOP_FRAME_COUNT / SLIDE_LOOP_FPS;
      player.slidePhaseTime = std::fmod(player.slidePhaseTime, cycleDuration);
      frame = SLIDE_LOOP_FIRST_FRAME + static_cast<int>(player.slidePhaseTime * SLIDE_LOOP_FPS);
    } else if (player.slidePhase == SlidePhase::Recover) {
      // Limita el cuadro para que la recuperación se quede en su última pose.
      frame = SLIDE_RECOVER_FIRST_FRAME + std::min(static_cast<int>(player.slidePhaseTime * clip.framesPerSecond),
                                                   SLIDE_RECOVER_FRAME_COUNT - 1);
    }
  } else if (animation == PlayerAnimation::Jump) {
    // Cambia de pose cuando la velocidad de subida se acerca a cero.
    frame = player.velocity.y < PLAYER_JUMP_VELOCITY / 2 ? 0 : clip.frameCount - 1;
  } else if (animation == PlayerAnimation::Fall) {
    // Usa la pose de caída rápida mientras se mantiene la acción de caer.
    frame = ctx.input.Pressed(Fall) ? clip.frameCount - 1 : 0;
  }

  if (player.animationFrame != frame) {
    // Solo cambia el recorte cuando la animación alcanza otro cuadro.
    player.animationFrame = frame;
    player.sprite.setTextureRect(SpriteAnimator::FrameRect(clip, frame, PLAYER_SHEET_COLUMNS, PLAYER_FRAME_SIZE));
  }

  // Alinea los pies del sprite con la base de la caja sin redondear la física.
  const auto bounds = player.shape.getGlobalBounds();
  player.sprite.setPosition({std::round(bounds.position.x + bounds.size.x / 2),
                             std::round(bounds.position.y + bounds.size.y)});
}

/* Actualiza el impacto de las pisadas y el movimiento de las partículas. */
void Game::UpdateEffects() {
  const float delta = ctx.time.GetDeltaTime();
  // El impacto disminuye de forma gradual hasta que el sprite recupera su escala normal.
  player.stepImpact = std::max(0.f, player.stepImpact - delta / FOOTSTEP_DURATION);
  // El polvo retrocede con el escenario y conserva su propia velocidad.
  ctx.particles.Update(delta, {-stats.speed * DUST_SCROLL_FACTOR, 0});
}

/* Desplaza las capas del fondo y la plataforma según la velocidad de la partida. */
void Game::UpdateEnvironment(float deltaTime) {
  const int width = static_cast<int>(std::ceil(gConfig.windowSize.x / ENVIRONMENT_PIXEL_SCALE));
  const auto scrollLayer = [&](BackgroundLayer &layer, float speedFactor) {
    const float textureWidth = static_cast<float>(layer.shape.getTexture()->getSize().x);
    // Cada factor controla la velocidad de la capa; fmod vuelve al comienzo al completar la textura.
    layer.offset = std::fmod(layer.offset + stats.speed * speedFactor * deltaTime / ENVIRONMENT_PIXEL_SCALE, textureWidth);

    const int height = static_cast<int>(std::ceil(layer.shape.getSize().y / ENVIRONMENT_PIXEL_SCALE));
    layer.shape.setTextureRect({{static_cast<int>(layer.offset), 0}, {width, height}}); // Mueve el recorte, no la forma.
  };
  for (std::size_t index = 0; index < backgroundLayers.size(); ++index) {
    scrollLayer(backgroundLayers[index], BACKGROUND_SPEED_FACTORS[index]);
  }
  scrollLayer(foregroundCity, FOREGROUND_SPEED_FACTOR);

  const auto textureWidth = ground.getTexture()->getSize().x;
  platformOffset = std::fmod(platformOffset + stats.speed * deltaTime / ENVIRONMENT_PIXEL_SCALE, static_cast<float>(textureWidth));
  const int offset = static_cast<int>(platformOffset);
  // El borde y el frente comparten el mismo desplazamiento para que no se separen.
  const int lipHeight = static_cast<int>(GROUND_HEIGHT / ENVIRONMENT_PIXEL_SCALE);
  ground.setTextureRect({{offset, 0}, {width, lipHeight}});
  platformFront.setTextureRect({{offset, lipHeight}, {width, static_cast<int>(platformFront.getSize().y / ENVIRONMENT_PIXEL_SCALE)}});
}

/* Controla el sonido y el polvo del deslizamiento y la compresión al pisar. */
void Game::UpdatePlayerEffects() {
  auto &slideSound = soundEffects[static_cast<std::size_t>(PlayerSound::Slide)];
  // El sonido solo acompaña la plancha mientras el personaje esté apoyado y la partida siga activa.
  if (player.sliding && player.grounded && !gameOver.active) {
    PlayPlayerSound(PlayerSound::Slide);
  } else if (slideSound && slideSound->getStatus() != sf::SoundSource::Status::Stopped) {
    // Detiene el arrastre solo si estaba activo o pausado.
    audioGroup.Stop(*slideSound);
  }
  if (player.sliding && player.grounded) {
    // Acumula el tiempo para emitir polvo a intervalos constantes, independientemente de los FPS.
    player.slideDustTime += ctx.time.GetDeltaTime();
    const auto bounds = player.shape.getGlobalBounds();
    // Genera todas las tandas correspondientes al tiempo acumulado.
    while (player.slideDustTime >= SLIDE_DUST_INTERVAL) {
      player.slideDustTime -= SLIDE_DUST_INTERVAL;
      EmitDust({bounds.position.x, bounds.position.y + bounds.size.y}, SLIDE_DUST_COUNT, true);
    }
  } else {
    player.slideDustTime = 0;
  }

  // La escala cambia el sprite brevemente, pero no modifica la caja de colisión.
  const float impact = player.grounded && !player.sliding ? player.stepImpact : 0;
  player.sprite.setScale({PLAYER_SPRITE_SCALE.x * (1 + FOOTSTEP_COMPRESSION * impact),
                          PLAYER_SPRITE_SCALE.y * (1 - FOOTSTEP_COMPRESSION * impact)});
}

/* Genera y desplaza obstáculos, y elimina los que salen de la pantalla. */
void Game::UpdateObstacles() {
  // Cuando hay suficiente separación desde el último obstáculo, se genera el siguiente.
  if (obstacles.empty() || obstacles.back().bounds.position.x < gConfig.windowSize.x - stats.obstacleNextX) {
    EventObstacleSpawn();
    stats.obstacleNextX = ctx.random.Float(OBSTACLE_MIN_DISTANCE, OBSTACLE_MAX_DISTANCE);
  }
  const float movement = stats.speed * ctx.time.GetDeltaTime();
  for (auto &obstacle : obstacles) {
    // Guarda la caja previa antes de moverla para detectar contactos durante el frame.
    obstacle.previousBounds = obstacle.bounds;
    obstacle.bounds.position.x -= movement;
  }
  while (!obstacles.empty() && obstacles.front().bounds.position.x + obstacles.front().bounds.size.x < 0) {
    // Los obstáculos que ya salieron por la izquierda dejan de participar en el juego.
    obstacles.pop_front();
  }
}

/* Aumenta la velocidad y la distancia y actualiza el marcador cuando cambia. */
void Game::UpdateStats() {
  const int previousDistance = GetDistance();
  stats.speed = std::min(stats.speed + STATS_SPEED_INCREASE * ctx.time.GetDeltaTime(), STATS_SPEED_MAX);

  stats.distance += stats.speed * ctx.time.GetDeltaTime();
  // Actualiza el texto solo cuando cambia la distancia entera que ve el jugador.
  if (GetDistance() != previousDistance) {
    stats.distanceText.setString(std::format("Distance: {}", GetDistance()));
  }
}

/* Convierte la distancia recorrida a las unidades mostradas en pantalla. */
int Game::GetDistance() const { return static_cast<int>(stats.distance) / STATS_DISTANCE_SCALE; }

/* Actualiza la cuenta regresiva y solicita el reinicio una sola vez. */
void Game::UpdateGameOver() {
  // Evita solicitar el reinicio más de una vez mientras cambia la escena.
  if (gameOver.restartRequested) {
    return;
  }

  // El tiempo mostrado nunca baja de cero y se vuelve a centrar porque el texto cambia de ancho.
  const float remaining = std::max(0.f, GAME_OVER_RESTART_DELAY - gameOver.restartCooldown.GetElapsedTime());
  gameOver.restartText.setString(std::format("Reinicio en {:.1f} s", remaining));
  gameOver.restartText.setOrigin(gameOver.restartText.getLocalBounds().getCenter());
  if (gameOver.restartCooldown.IsOver()) {
    gameOver.restartRequested = true;
    ctx.scenes.RestartCurrentScene();
  }
}

/* Muestra la derrota, registra la distancia y reduce el volumen de la música. */
void Game::EventGameOver() {
  // Una colisión adicional en el mismo frame no debe reiniciar la cuenta regresiva.
  if (gameOver.active) {
    return;
  }
  const int distance = GetDistance();
  LOG_INFO("Game Over! Distance: {}", distance);
  gameOver.distanceText.setString(std::format("Distancia: {}", distance));
  gameOver.distanceText.setOrigin(gameOver.distanceText.getLocalBounds().getCenter());
  gameOver.active = true;
  StopPlayerSounds(); // Detiene los efectos, pero la música continúa a menor volumen.
  if (music) {
    audioGroup.SetSourceVolume(*music, MUSIC_GAME_OVER_VOLUME);
  }
  gameOver.restartCooldown.Restart();
  UpdateGameOver();
}

/* Pausa o reanuda las fuentes de audio que pertenecen a la escena. */
void Game::OnPause(bool paused) {
  audioGroup.SetPaused(paused);
}

/* Limpia las partículas y detiene la música y los efectos al salir. */
void Game::OnCleanup() {
  ctx.animations.Remove(player.animator);
  player.animator = INVALID_ANIMATION;
  ctx.particles.Clear();
  audioGroup.StopAll();
}

/* Reproduce un efecto y evita reiniciar los sonidos que están en bucle. */
void Game::PlayPlayerSound(PlayerSound sound) {
  if (gameOver.active) {
    return;
  }
  auto &effect = soundEffects[static_cast<std::size_t>(sound)];

  if (effect) {
    // Reinicia los efectos breves; los sonidos en bucle continúan sin interrupciones.
    audioGroup.Play(*effect, !effect->isLooping());
  }
}

/* Detiene los efectos del personaje sin detener la música. */
void Game::StopPlayerSounds() {
  audioGroup.Stop(AudioChannel::Effect);
}

/* Reproduce el sonido del pie, aplica el impacto y genera polvo. */
void Game::EventPlayerFootstep(int contactFrame, bool emitSound) {
  // Los cuadros de contacto alternan entre los sonidos de los dos pies.
  if (emitSound) {
    PlayPlayerSound(contactFrame == PLAYER_FOOTSTEP_FRAMES.front() ? PlayerSound::FootstepLeft : PlayerSound::FootstepRight);
  }
  const auto bounds = player.shape.getGlobalBounds();
  // El polvo nace cerca del pie, a la altura de la base de la caja.
  const sf::Vector2f contact(player.shape.getPosition().x + PLAYER_FOOTSTEP_OFFSET,
                             bounds.position.y + bounds.size.y);
  player.stepImpact = 1;
  EmitDust(contact, FOOTSTEP_DUST_COUNT, false);
}

/* Emite partículas con velocidad y tamaño adecuados para la pisada o el deslizamiento. */
void Game::EmitDust(sf::Vector2f position, int count, bool sliding) {
  // La configuración de plancha aumenta el tamaño y el impulso respecto a una pisada.
  if (count <= 0 || ctx.particles.Count() >= MAX_DUST_PARTICLES)
    return;
  const auto available = MAX_DUST_PARTICLES - ctx.particles.Count();
  ctx.particles.EmitBurst(position, std::min(static_cast<std::size_t>(count), available), DUST_EMITTERS[sliding ? 1 : 0], ctx.random);
}

/* Crea un obstáculo terrestre o aéreo en el borde derecho. */
void Game::EventObstacleSpawn() {
  auto &obstacle = obstacles.emplace_back();

  const float height = ctx.random.Float(OBSTACLE_MIN_HEIGHT, OBSTACLE_MAX_HEIGHT);
  obstacle.bounds.size = {OBSTACLE_WIDTH, height};

  const bool airborne = ctx.random.Bool(OBSTACLE_AIR_PROBABILITY);
  // La variante aérea deja un espacio inferior para que pase el personaje deslizándose.
  obstacle.spriteIndex = airborne ? OBSTACLE_AIR_SPRITE_INDEX : ctx.random.Int(0, OBSTACLE_GROUND_SPRITE_COUNT - 1);
  obstacle.bounds.position = {gConfig.windowSize.x, ground.getPosition().y - height - (airborne ? PLAYER_SLIDE_SIZE.y : 0.f)};
}

/* Resuelve el primer contacto y distingue un aterrizaje de un choque. */
void Game::HandleCollisionsPlayerObstacles() {
  const auto currentBounds = player.shape.getGlobalBounds();
  const Obstacle *support = nullptr;
  // El valor 2 indica que todavía no hay contacto dentro del intervalo del frame, que va de 0 a 1.
  float landingTime = 2;
  float crashTime = 2;

  // Compara todos los obstáculos para conservar el aterrizaje y el choque más tempranos.
  for (const auto &obstacle : obstacles) {
    const auto hit = SweepObstacle(player.previousBounds, currentBounds, obstacle.previousBounds,
                                   obstacle.bounds);
    if (!hit) {
      continue;
    }

    if (hit->landing && hit->time < landingTime) {
      landingTime = hit->time;
      support = &obstacle;
    } else if (!hit->landing) {
      crashTime = std::min(crashTime, hit->time);
    }
  }

  // Un choque lateral anterior al aterrizaje termina la partida.
  if (crashTime <= landingTime && crashTime <= 1) {
    EventGameOver();
    return;
  }

  // Solo conserva el apoyo si el jugador todavía está sobre el obstáculo.
  if (support && OverlapsHorizontally(currentBounds, support->bounds)) {
    // Coloca los pies sobre la parte superior y elimina la velocidad de caída.
    player.shape.setPosition({player.shape.getPosition().x,
                              support->bounds.position.y - player.shape.getSize().y / 2});
    player.velocity.y = 0;
    player.grounded = true;
  }

  // Comprueba que el aterrizaje no deje al jugador dentro de otro obstáculo.
  const auto correctedBounds = player.shape.getGlobalBounds();
  for (const auto &obstacle : obstacles) {
    if (&obstacle != support && ctx.collisions.Intersect(correctedBounds, obstacle.bounds)) {
      EventGameOver();
      return;
    }
  }
}

/* Dibuja el entorno, los obstáculos, las partículas, el jugador y los textos. */
void Game::Render() const {
  RenderEnvironment();

  for (const auto &obstacle : obstacles) {
    RenderObstacle(obstacle);
  }

  // Las partículas se dibujan antes del jugador para quedar detrás.
  ctx.particles.Render(ctx.renderer);
  ctx.renderer.Draw(player.sprite);

  ctx.renderer.Draw(stats.distanceText);
  if (gameOver.active) {
    RenderGameOver();
  }
}

/* Dibuja las capas del escenario en orden de profundidad. */
void Game::RenderEnvironment() const {
  // Las capas lejanas se dibujan primero para que los elementos cercanos queden delante.
  for (const auto &layer : backgroundLayers) {
    ctx.renderer.Draw(layer.shape);
  }
  ctx.renderer.Draw(platformFront);

  ctx.renderer.Draw(foregroundCity.shape);
  ctx.renderer.Draw(ground);
}

/* Dibuja las tapas y el cuerpo del obstáculo usando regiones de su textura. */
void Game::RenderObstacle(const Obstacle &obstacle) const {
  // Conserva las tapas del sprite y recorta el cuerpo para adaptarlo a la altura del obstáculo.
  const int cap = OBSTACLE_SPRITE_CAP_HEIGHT;
  const int width = static_cast<int>(OBSTACLE_WIDTH);
  const int sourceX = obstacle.spriteIndex * OBSTACLE_SPRITE_FRAME_SIZE.x;
  const auto position = obstacle.bounds.position;
  const float top = std::round(position.y);
  const float bottom = std::round(position.y + obstacle.bounds.size.y);
  const int bodyHeight = static_cast<int>(bottom - top) - cap * 2;
  sf::Sprite part(*obstacleTexture);
  const auto drawPart = [&](int sourceY, int height, float y) {
    part.setTextureRect({{sourceX, sourceY}, {width, height}});
    part.setPosition({std::round(position.x), y});
    ctx.renderer.Draw(part);
  };
  drawPart(0, cap, top);
  drawPart(cap, bodyHeight, top + cap);
  drawPart(OBSTACLE_SPRITE_FRAME_SIZE.y - cap, cap, bottom - cap);
}

/* Dibuja la pantalla de derrota sobre la partida. */
void Game::RenderGameOver() const {
  ctx.renderer.Draw(gameOver.backdrop);
  ctx.renderer.Draw(gameOver.title);
  ctx.renderer.Draw(gameOver.distanceText);
  ctx.renderer.Draw(gameOver.restartText);
}

/* Detecta el primer contacto con un obstáculo y reconoce un apoyo sobre su superficie. */
std::optional<ObstacleHit> Game::SweepObstacle(const sf::FloatRect &previousPlayer, const sf::FloatRect &currentPlayer,
                                               const sf::FloatRect &previousObstacle, const sf::FloatRect &currentObstacle) const {
  const auto movement = (currentPlayer.position - previousPlayer.position) -
                        (currentObstacle.position - previousObstacle.position);
  // Se usa el movimiento relativo porque el jugador y el obstáculo pueden moverse a la vez.

  // Una pequeña tolerancia evita perder el apoyo por errores de redondeo.
  const float previousFeet = previousPlayer.position.y + previousPlayer.size.y;
  if (movement.y >= 0 && std::abs(previousFeet - previousObstacle.position.y) <= PLAYER_COLLISION_TOLERANCE &&
      previousPlayer.position.x < previousObstacle.position.x + previousObstacle.size.x &&
      previousPlayer.position.x + previousPlayer.size.x > previousObstacle.position.x) {
    return ObstacleHit{0, true};
  }

  // El cálculo general devuelve el momento y la normal; Runner decide si el contacto es un aterrizaje.
  const auto hit = ctx.collisions.Sweep(previousPlayer, currentPlayer, previousObstacle, currentObstacle);
  if (!hit) {
    return std::nullopt;
  }
  return ObstacleHit{hit->time, !hit->initialOverlap && hit->normal.y < 0};
}

/* Comprueba si dos cajas comparten espacio horizontal. */
bool Game::OverlapsHorizontally(const sf::FloatRect &player, const sf::FloatRect &obstacle) {
  return player.position.x < obstacle.position.x + obstacle.size.x &&
         player.position.x + player.size.x > obstacle.position.x;
}
