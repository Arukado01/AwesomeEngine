#include "Runner.h"

#include <algorithm>
#include <cmath>
#include <format>
#include <limits>
#include <stdexcept>

using namespace Runner;

/* Carga el arte y el sonido UNA vez, al nacer la escena; durante la carrera ya nada más se crea. */
Game::Game(EngineContext &context)
    : Scene(context), player(LoadTexture(context, PLAYER_TEXTURE_FILENAME)), music(context.resources.FetchMusic(MUSIC_FILENAME)) {
  // el sprite pide su textura desde nacer: si Runner.png no existe, esta línea explota y avisamos en el init
  footstepPulses.reserve(MAX_FOOTSTEP_PULSES);
  InitEnvironment(); // crea fondos, plataforma y hoja de sprites de los obstáculos
  InitGameOver(); // deja lista la pantalla del fin
  if (music) { // la pista de música puede no existir: si falta, la partida sigue en silencio (no tumba el juego)
    music->setLooping(true); // cuando el archivo se acaba, vuelve a empezar: siempre de fondo
    music->setVolume(MUSIC_VOLUME); // volumen de la configuración
  }
  for (std::size_t index = 0; index < soundEffects.size(); ++index) {
    const auto &config = PLAYER_SOUNDS[index]; // cada casilla: qué archivo se escucha con qué volumen y si va en bucle
    if (auto *buffer = ctx.resources.FetchSound(config.filename); buffer && buffer->getSampleCount() > 0) {
      // un efecto nace solo si su archivo carga y tiene audio (sampleCount > 0): uno roto no tumba al resto
      auto &sound = soundEffects[index].emplace(*buffer);
      sound.setVolume(config.volume);
      sound.setLooping(config.looping); // la plancha suena EN bucle; el resto suena una vez
    }
  }
}

/* Carga una textura por nombre: si no existe o vino vacía, lanza excepción y el error se agranda de frente... * nunca falla en silencio. */
sf::Texture &Game::LoadTexture(EngineContext &context, const std::string &filename, bool repeated) {
  auto *texture = context.resources.FetchTexture(filename);
  if (!texture || texture->getSize() == sf::Vector2u{}) { // nullptr o tamaño 0×0 = el PNG no está
    throw std::runtime_error("No se pudo cargar la textura del runner: " + filename);
  }
  texture->setSmooth(false); // pixel-art: el suavizado solo emborrona los bordes
  texture->setRepeated(repeated); // fondos y plataforma tuilean (la textura se repite infinito); sprites no
  return *texture;
}

/* Arma los planos del mundo: dos fondos de cielo, la plataforma y los edificios que corren por DELANTE del piso. */
void Game::InitEnvironment() {
  ground.setSize({gConfig.windowSize.x, GROUND_HEIGHT}); // la franja del piso: todo lo ancho, 10 píxeles de alto
  ground.setPosition({0, gConfig.windowSize.y / 2}); // en la MITAD de la pantalla: arriba queda cielo, abajo "sótano"
  for (std::size_t index = 0; index < backgroundLayers.size(); ++index) {
    auto &layer = backgroundLayers[index]; // capa por índice: 0 = skyline (muy lejos), 1 = ciudad (media distancia)
    layer.shape.setTexture(&LoadTexture(ctx, BACKGROUND_TEXTURE_FILENAMES[index], true)); // textura tileable
    layer.shape.setSize({gConfig.windowSize.x, ground.getPosition().y}); // cubren TODA la zona de cielo (hasta la línea de juego)
    layer.shape.setFillColor(BACKGROUND_TINT); // tinte azulado apagado: lo lejano se ve más "de menos", profundidad a mano
  }

  auto &platformTexture = LoadTexture(ctx, PLATFORM_TEXTURE_FILENAME, true);
  ground.setTexture(&platformTexture); // el suelo deja de ser gris: muestra la franja superior del tile
  ground.setFillColor(sf::Color::White); // blanco = pinta la textura 1:1, sin tinte cambiante
  platformFront.setTexture(&platformTexture);
  platformFront.setPosition({0, ground.getPosition().y + GROUND_HEIGHT}); // empieza justo DEBAJO del borde
  platformFront.setSize({gConfig.windowSize.x, gConfig.windowSize.y - platformFront.getPosition().y}); // baja hasta el pie de la ventana
  foregroundCity.shape.setTexture(&LoadTexture(ctx, FOREGROUND_TEXTURE_FILENAME, true));
  foregroundCity.shape.setFillColor(FOREGROUND_TINT);
  foregroundCity.shape.setPosition(platformFront.getPosition()); // misma zona que platformFront...
  foregroundCity.shape.setSize(platformFront.getSize()); // ...pero se dibuja ENCIMA: son los edificios de primer plano

  obstacleTexture = &LoadTexture(ctx, OBSTACLE_TEXTURE_FILENAME); // el sheet de 4 columnas de obstáculos (sin tile)
}

/* La pantalla del fin se deja lista antes de jugar: cuando se pierde, mostrarla cuesta cero trabajo en el frame del bajón. */
void Game::InitGameOver() {
  const auto initText = [](sf::Text &text, unsigned int size, sf::Color color, sf::Vector2f position) {
    text.setCharacterSize(size);
    text.setFillColor(color);
    text.setPosition(gConfig.windowSize.componentWiseMul(position)); // posición por fracción: (0.5, 0.42) × ventana
  };
  gameOver.restartCooldown.SetDuration(GAME_OVER_RESTART_DELAY); // cuánto dura el aviso: 2.5 s
  gameOver.backdrop.setSize(gConfig.windowSize);
  gameOver.backdrop.setFillColor(GAME_OVER_BACKDROP_COLOR); // velo casi negro semitransparente (tapa TODO el mundo detrás)
  gameOver.title.setString(GAME_OVER_TITLE);
  initText(gameOver.title, GAME_OVER_TITLE_SIZE, GAME_OVER_TITLE_COLOR, GAME_OVER_TITLE_POSITION); // a mitad de pantalla
  gameOver.title.setOrigin(gameOver.title.getLocalBounds().getCenter()); // puntero al centro del texto: quedar centrado de verdad
  initText(gameOver.distanceText, GAME_OVER_DISTANCE_TEXT_SIZE, GAME_OVER_DISTANCE_TEXT_COLOR, GAME_OVER_DISTANCE_TEXT_POSITION);
  initText(gameOver.restartText, GAME_OVER_RESTART_TEXT_SIZE, GAME_OVER_RESTART_TEXT_COLOR, GAME_OVER_RESTART_TEXT_POSITION);
}

/* Entrada a la escena (y a cada reinicio): entidades a cero, mundo en su arranque, jugador nuevo y música otra vez arriba. */
void Game::Start() {
  StopPlayerSounds(); // si venimos de otra ronda, ningún sonido andaba por ahí
  ctx.cursor.SetVisible(false); // el mouse no se usa: que no distraiga

  BindInputs();

  obstacles.clear(); // sin los obstáculos de la ronda anterior
  ctx.particles.Clear();
  footstepPulses.clear();
  gameOver.active = false; // ronda viva
  gameOver.restartRequested = false;
  gameOver.restartCooldown.Reset(); // el contador del fin: a cero y detenido

  for (auto &layer : backgroundLayers) {
    layer.offset = 0; // el parallax vuelve al arranque del tile
  }
  platformOffset = 0;
  foregroundCity.offset = 0;

  StartStats();
  UpdateEnvironment(0); // con deltaTime cero: solo posiciona los recortes de textura sin avanzar (sin frame-1 torcido)
  StartPlayer();

  EventObstacleSpawn(); // el primer obstáculo nace ya: nunca arrancáis con la lista vacía
  if (music) {
    music->stop();
    music->setVolume(MUSIC_VOLUME); // (por si venimos de un Game Over, que lo había dejado bajito)
    music->play(); // la banda nueva de la ronda
  }
}

/* Los controles: W salta (o A del mando); S es resbalada en el suelo y/plugin caída rápida en el aire; la palanca-Y también cae. */
void Game::BindInputs() {
  ctx.input.Bind(Jump, Input::Keyboard{sf::Keyboard::Scan::W});
  ctx.input.Bind(Jump, Input::Gamepad{GamepadButton::South});

  ctx.input.Bind(Crouch, Input::Keyboard{sf::Keyboard::Scan::S});
  ctx.input.Bind(Crouch, Input::Gamepad{GamepadButton::West});

  ctx.input.Bind(Fall, Input::Keyboard{sf::Keyboard::Scan::S}); // la misma S: en el aire significa "cae más rápido"
  ctx.input.Bind(Fall, Input::Axis{sf::Joystick::Axis::Y});
}

/* Crea el jugador de cero: objeto limpio, caja de colisión apoyada en el suelo y su dibujo en el primer fotito. */
void Game::StartPlayer() {
  player = Player(player.sprite.getTexture()); // objeto NUEVO: sin memoria de otra ronda (animación, fases, impacto en default)
  player.sprite.setOrigin({PLAYER_FRAME_SIZE.x / 2.f, static_cast<float>(PLAYER_FRAME_SIZE.y)}); // agarre del sprite: pies del fotito (32,64)
  player.sprite.setScale(PLAYER_SPRITE_SCALE); // escala 1:1 del arte
  player.shape.setSize(PLAYER_SIZE); // caja de colisión 28×48
  player.shape.setOrigin(player.shape.getGeometricCenter()); // la caja se agarra por su centro (facilita centrar sin importar el alto)

  float positionY = ground.getPosition().y - player.shape.getSize().y / 2; // bajar la mitad del alto: los pies tocan el suelo
  player.shape.setPosition({gConfig.windowSize.x / 6, positionY}); // a 1/6 de la pantalla, mirarido a la derecha

  player.previousBounds = player.shape.getGlobalBounds(); // sin "recorrido fantasma" en el primer frame
  UpdatePlayerAnimation(false); // coloca el primer fotito SIN sonido: por ahora hasta la música recién arrancó
}

/* Estadísticas de ronda nueva: velocidad inicial, separación del próximo obstáculo y distancia en cero. */
void Game::StartStats() {
  stats.distanceText.setFillColor(STATS_DISTANCE_TEXT_COLOR);
  stats.distanceText.setPosition({10, 10}); // el HUD esquina superior izquierda
  stats.speed = STATS_SPEED_START; // 200 píxeles por segundo
  stats.obstacleNextX = ctx.random.Float(OBSTACLE_MIN_DISTANCE, OBSTACLE_MAX_DISTANCE); // separación del primer al segundo obstáculo

  stats.distance = 0;
  stats.distanceText.setString("Distance: 0");
}

/* El director por frame: con fin de partida, solo el aviso; sin él, toda la simulación en orden de dependencias. */
void Game::Update() {
  // Mantiene el resultado visible sin avanzar la simulación durante la espera.
  if (gameOver.active) {
    UpdateGameOver(); // solo el contador del reinicio corre
    return; // todo lo demás está CONGELADO: la pantalla cuentagotas se ve quieta detrás
  }

  UpdateEffects(); // polvo y marcas vivas avanzan SIEMPRE, juegues o no
  UpdateEnvironment(ctx.time.GetDeltaTime()); // el parallax también avanza solo: el mundo corre aunque el jugador esté en el aire
  if (ctx.input.Pressed(Jump) && player.grounded && !player.sliding) { // saltar: solo con contactos y sin plancha activa
    player.velocity.y = PLAYER_JUMP_VELOCITY; // impulso: −600 (negativo = hacia arriba, porque la pantalla crece hacia abajo)
    player.grounded = false;
    PlayPlayerSound(PlayerSound::Jump);
  }

  UpdatePlayer(); // gravedad + tamaño (plancha) + paso de integración
  UpdateObstacles(); // nacen, viajan y se retiran
  UpdateStats(); // velocidad y distancia

  HandleCollisionsPlayerObstacles(); // vida o muerte: el PRIMER contacto del frame decide
  UpdatePlayerGrounded(); // ajuste final contra el suelo (PUNTO de no-pisadazo)
  UpdatePlayerAnimation(); // fotito del frame, calculado con la posición YA definitiva
  UpdatePlayerEffects(); // polvo de plancha y aplastamiento según la pisada
}

/* Física del jugador por frame: el tamaño (plancha), la foto previa para colisiones, la gravedad y el paso de integración. */
void Game::UpdatePlayer() {
  UpdatePlayerSize(); // puede entrar/salir de plancha: la caja cambia ANTES de capturar el "antes"
  player.previousBounds = player.shape.getGlobalBounds(); // FOTO ANTES: la colisión swept necesita de dónde ARRANCó este frame
  const float delta = ctx.time.GetDeltaTime(); // duración del frame en segundos (≈0.0167 a 60 fps)
  const float gravity = PLAYER_GRAVITY * (ctx.input.Pressed(Fall) && !player.grounded ? PLAYER_FALL_FACTOR : 1.f);
  player.velocity.y += gravity * delta; // la velocidad vertical CRECE con la gravedad: es una caída acelerada, no constante
  player.shape.move({0, player.velocity.y * delta}); // mueve SOLO en Y: el "correr" horizontal lo pone el mundo que viene
  player.grounded = false; // asumir aire: el suelo y las colisiones lo confirman (o no) al final del frame
}

/* Qué hacer con la plancha: pedidos de entrar/salir, el bloqueo bajo obstáculos y los tiempos de la animación. */
void Game::UpdatePlayerSize() {
  if (!player.grounded) {
    return; // en el aire no cambia el tamaño: evita bugs de "agacharse mientras cae"
  }

  const bool requested = ctx.input.Pressed(Crouch); // ¿S está presionada AHORA?
  if (player.sliding) { // Estado: ya va en plancha
    if (requested) { // sigue presionando: se queda así
      if (player.slidePhase == SlidePhase::Recover) {
        SetSlidePhase(SlidePhase::Loop); // se arrepientó en pleno "levantando": vuelve al ciclo de plancha
      }
      return;
    }
    // Bajo un obstáculo conserva la plancha hasta disponer de espacio para levantarse.
    if (!CanPlayerStand()) {
      SetSlidePhase(SlidePhase::Loop); // hay algo arriba: te mantiene deslizando aunque sueltes S (no morirías si te levantas)
      return;
    }
    if (player.slidePhase != SlidePhase::Recover) {
      SetSlidePhase(SlidePhase::Recover); // soltó y hay espacio: arranca la mini-animación de levantarse
      return;
    }
    const auto &clip = PLAYER_ANIMATIONS[static_cast<std::size_t>(PlayerAnimation::Slide)];
    if (player.slidePhaseTime < SLIDE_RECOVER_FRAME_COUNT / clip.framesPerSecond) {
      return; // el "levantándose" no terminó sus 2 fotitos (2/12 s): la caja TODAVÍA se mantiene baja
    }
  } else if (!requested) { // no va en plancha y no la pedidió: nada que hacer
    return;
  }

  // Solo se llega aquí si corresponde alternar el estado (entrar o salir de la plancha):
  const bool sliding = !player.sliding;
  const float oldHeight = player.shape.getSize().y; // altura actual (48 de pie, 25 en plancha)
  const auto newSize = sliding ? PLAYER_SLIDE_SIZE : PLAYER_SIZE; // 54×25 ↔ 28×48
  player.shape.setSize(newSize);
  player.shape.setOrigin(player.shape.getGeometricCenter()); // re-agarrar por el centro, que cambió al cambiar el tamaño
  player.shape.move({0, (oldHeight - newSize.y) / 2}); // compensa al Uruguay: los pies quedan en el MISMO píxel del suelo
  player.sliding = sliding;
  SetSlidePhase(SlidePhase::Prepare); // (re)arranque de la mini-secuencia del estado
}

/* Cambia la fase de la plancha y reinicia su reloj: solo si la fase in real cambia (nada de resets eternos). */
void Game::SetSlidePhase(SlidePhase phase) {
  if (player.slidePhase != phase) {
    player.slidePhase = phase;
    player.slidePhaseTime = 0;
  }
}

/* ¿Cabe de pie aquí? Mide la caja que tendría DE PIE y busca que ningún obstáculo la atraviese. */
bool Game::CanPlayerStand() const {
  const auto bounds = player.shape.getGlobalBounds(); // caja de plancha (54×25) actual
  const sf::FloatRect standingBounds({player.shape.getPosition().x - PLAYER_SIZE.x / 2, // ...mismo centro-X
                                      bounds.position.y + bounds.size.y - PLAYER_SIZE.y}, // ...misma base, altura de pie
                                     PLAYER_SIZE); // "la caja de solo levantarme acá mismo"
  return std::none_of(obstacles.begin(), obstacles.end(), [&](const Obstacle &obstacle) {
    const auto &obstacleBounds = obstacle.bounds;
    return obstacleBounds.position.y < bounds.position.y + bounds.size.y - PLAYER_COLLISION_TOLERANCE && // solo cuenta obstáculos que SOBRESALDRÍN por encima
           standingBounds.findIntersection(obstacleBounds).has_value(); // y si la caja de pie chocara ese algo: pararse aquí sería morir
  });
}

/* Si la caída del frame atravesó el suelo, se reconstruye sobre él: pies exactos, caída frenada y "está en el suelo". */
void Game::UpdatePlayerGrounded() {
  float halfHeight = player.shape.getSize().y / 2; // la mitad del alto actual (de pie o plancha): depende

  if (player.shape.getPosition().y + halfHeight >= ground.getPosition().y) { // los pies superaron la línea del suelo
    player.shape.setPosition({player.shape.getPosition().x, ground.getPosition().y - halfHeight}); // volvé a la superficie EXACTA
    player.velocity.y = 0; // la caida murió: ya no te sigue bajando
    player.grounded = true;
  } // SIN else: si NO cruzó el suelo (cayó sobre un obstáculo, por ejemplo) no se toca nada
}

/* El cerebro de la animación: elige la animación según tu estado, avanza su reloj y traduce "número de fotito" a la casilla del PNG. */
void Game::UpdatePlayerAnimation(bool emitSound) { // false solo al iniciar la escena: sin inputs vivos, las pisadas nacen mudas
  auto animation = PlayerAnimation::Run; // por defecto: corriendo
  if (player.sliding) {
    animation = PlayerAnimation::Slide; // plancha tiene prioridad máxima
  } else if (!player.grounded) {
    animation = player.velocity.y < 0 ? PlayerAnimation::Jump : PlayerAnimation::Fall; // ¿estás SUBiendo (velocidad negativa) o cayendo?
  }
  if (animation != player.animation) { // cambió la animación: reinicia su reloj y el fotito actual
    player.animation = animation;
    player.animationTime = 0; // reloj del ciclo, EN UNIDADES DE FOTITO
    player.animationFrame = -1; // −1 = "aún no pinté ningún frame": dispara la pisada inicial
    if (animation == PlayerAnimation::Slide && player.grounded) {
      const auto bounds = player.shape.getGlobalBounds();
      EmitDust({bounds.position.x, bounds.position.y + bounds.size.y}, SLIDE_DUST_BURST_COUNT, true); // frenada: ráfaga grande de 10
    }
  }

  const auto &clip = PLAYER_ANIMATIONS[static_cast<std::size_t>(animation)]; // clip = {donde arranca, cuantos, a qué velocidad}
  int frame = 0;
  if (animation == PlayerAnimation::Run) {
    // Conserva una cadencia legible sin acelerar excesivamente los pasos.
    const float playbackSpeed = std::min(clip.framesPerSecond * stats.speed / STATS_SPEED_START, PLAYER_ANIMATION_MAX_FPS);
    // fps efectivos: 12 × (velocidad actual / 200), con tope de 18 (más rápido sería una mancha)
    const float nextTime = player.animationTime + ctx.time.GetDeltaTime() * playbackSpeed; // a OPEN llega el reloj luego de este frame
    if (player.animationFrame < 0) {
      EventPlayerFootstep(PLAYER_FOOTSTEP_FRAMES.front(), emitSound); // ciclo recién nacido: pisada inicial
    }
    // Emite una pisada únicamente al cruzar un cuadro de contacto, incluso al cerrar el ciclo.
    for (int step = static_cast<int>(player.animationTime) + 1; step <= static_cast<int>(nextTime); ++step) {
      if (std::ranges::find(PLAYER_FOOTSTEP_FRAMES, step % clip.frameCount) != PLAYER_FOOTSTEP_FRAMES.end()) {
        EventPlayerFootstep(step % clip.frameCount, emitSound); // el % convierte el fotito 8 de la vuelta en el 0 del siguiente
      }
    }
    player.animationTime = std::fmod(nextTime, static_cast<float>(clip.frameCount)); // nunca rebasa el largo del ciclo: da vueltas
    frame = static_cast<int>(player.animationTime);
  } else if (animation == PlayerAnimation::Slide) {
    if (player.grounded) { // en el aire la fase NO avanza: el fotito queda congelado (no se ve raro al saltar en plancha)
      player.slidePhaseTime += ctx.time.GetDeltaTime();
    }
    if (player.slidePhase == SlidePhase::Prepare && player.slidePhaseTime >= SLIDE_PREPARE_DURATION) {
      SetSlidePhase(SlidePhase::Loop); // 0.10 s de "entrando" → pasa al ciclo de plancha
    }
    if (player.slidePhase == SlidePhase::Loop) {
      const float cycleDuration = SLIDE_LOOP_FRAME_COUNT / SLIDE_LOOP_FPS; // 2 fotitos a 6 fps = 0.333 s por vuelta
      player.slidePhaseTime = std::fmod(player.slidePhaseTime, cycleDuration);
      frame = SLIDE_LOOP_FIRST_FRAME + static_cast<int>(player.slidePhaseTime * SLIDE_LOOP_FPS); // alterna el 1 y el 2
    } else if (player.slidePhase == SlidePhase::Recover) {
      frame = SLIDE_RECOVER_FIRST_FRAME + std::min(static_cast<int>(player.slidePhaseTime * clip.framesPerSecond),
                                                   SLIDE_RECOVER_FRAME_COUNT - 1); // 3 → 4 y SE QUEDA en 4 (avança una sola vez)
    }
  } else if (animation == PlayerAnimation::Jump) {
    // Pasa de impulso a recogida de piernas al acercarse al punto más alto.
    frame = player.velocity.y < PLAYER_JUMP_VELOCITY / 2 ? 0 : clip.frameCount - 1;
  } else if (animation == PlayerAnimation::Fall) {
    frame = ctx.input.Pressed(Fall) ? clip.frameCount - 1 : 0; // con S: piernas recogidas (caída rápida); sin: caída normal
  }

  const int sheetFrame = clip.firstFrame + frame; // el fotito del clip → número de fila y columna en el sheet completo
  player.animationFrame = frame;
  player.sprite.setTextureRect({{(sheetFrame % PLAYER_SHEET_COLUMNS) * PLAYER_FRAME_SIZE.x, // columna = fotito % 4
                                 (sheetFrame / PLAYER_SHEET_COLUMNS) * PLAYER_FRAME_SIZE.y}, // fila = fotito ÷ 4
                                PLAYER_FRAME_SIZE}); // la casilla 64×64 del PNG a mostrar

  // Redondea solo el dibujo, sin alterar la precisión de las colisiones.
  const auto bounds = player.shape.getGlobalBounds();
  player.sprite.setPosition({std::round(bounds.position.x + bounds.size.x / 2), // centro-X de la caja (¡el redondeo no va aquí!)
                             std::round(bounds.position.y + bounds.size.y)}); // PIES de la caja: el sprite nace pegado a sus pies
}

/* Física del polvo y las marcas de pisada: viven, caen, el mundo las arrastra y las muertas se descartan. */
void Game::UpdateEffects() {
  const float delta = ctx.time.GetDeltaTime();
  player.stepImpact = std::max(0.f, player.stepImpact - delta / FOOTSTEP_DURATION); // el aplastamiento baja 0→0.12 s (lineal)
  ctx.particles.Update(delta, {-stats.speed * DUST_SCROLL_FACTOR, 0}); // El polvo se desplaza con el mundo.
  for (auto &pulse : footstepPulses) {
    pulse.remainingLife -= delta;
    pulse.position.x -= stats.speed * delta; // la marca corre al MISMO ritmo del suelo: queda pegada al piso
  }
  std::erase_if(footstepPulses, [](const FootstepPulse &pulse) { return pulse.remainingLife <= 0; });
}

/* El parallax: cada plano avanza su offset según su velocidad relativa y recalcula la "ventana" de recorte de la textura repetida. */
void Game::UpdateEnvironment(float deltaTime) {
  const int width = static_cast<int>(std::ceil(gConfig.windowSize.x / ENVIRONMENT_PIXEL_SCALE)); // pixel-art × 2: en textura son 480 px
  const auto scrollLayer = [&](BackgroundLayer &layer, float speedFactor) {
    const float textureWidth = static_cast<float>(layer.shape.getTexture()->getSize().x);
    layer.offset = std::fmod(layer.offset + stats.speed * speedFactor * deltaTime / ENVIRONMENT_PIXEL_SCALE, textureWidth);
    // offset avanza (velocidad × factor ▸ dTIME) y fmod lo devuelve al Início al pasar el ancho: bucle infinito
    const int height = static_cast<int>(std::ceil(layer.shape.getSize().y / ENVIRONMENT_PIXEL_SCALE));
    layer.shape.setTextureRect({{static_cast<int>(layer.offset), 0}, {width, height}}); // la FORMA no se mueve: corre el RECORTE dentro del tile
  };
  for (std::size_t index = 0; index < backgroundLayers.size(); ++index) {
    scrollLayer(backgroundLayers[index], BACKGROUND_SPEED_FACTORS[index]); // lejano 0.08×, medio 0.24×: de lejos = lento
  }
  scrollLayer(foregroundCity, FOREGROUND_SPEED_FACTOR); // primer plano: 1.2×, MÁS RÁPIDO que el suelo (está MUY cerca)

  const auto textureWidth = ground.getTexture()->getSize().x;
  platformOffset = std::fmod(platformOffset + stats.speed * deltaTime / ENVIRONMENT_PIXEL_SCALE, static_cast<float>(textureWidth));
  const int offset = static_cast<int>(platformOffset); // en el piso no hay factor: 1.0× EXACTO — los pies no derrapan
  const int lipHeight = static_cast<int>(GROUND_HEIGHT / ENVIRONMENT_PIXEL_SCALE); // la franja del borde: 10/2 = 5 px de textura
  ground.setTextureRect({{offset, 0}, {width, lipHeight}}); // el borde SIEMPRE muestra la parte de arriba del tile
  platformFront.setTextureRect({{offset, lipHeight}, {width, static_cast<int>(platformFront.getSize().y / ENVIRONMENT_PIXEL_SCALE)}});
} // y el cuerpo muestra lo demás: comparten el offset → borde y cuerpo caminan emparejados

/* Cosas pegadas al jugador: su sonido de plancha (arranque/parada inteligente), el polvo mientras resbala y el aplastamiento. */
void Game::UpdatePlayerEffects() {
  auto &slideSound = soundEffects[static_cast<std::size_t>(PlayerSound::Slide)];
  if (player.sliding && player.grounded && !gameOver.active) {
    PlayPlayerSound(PlayerSound::Slide); // plancha activa: el arrastre sigue sonando
  } else if (slideSound && slideSound->getStatus() != sf::SoundSource::Status::Stopped) {
    // SFML espera al hilo de audio en stop(); no repetir esa espera en cada cuadro.
    slideSound->stop(); // solo se llama cuando HAY que cortarlo (ni actúe cada frame)
  }
  if (player.sliding && player.grounded) { // el polvo expulsado durante la plancha con acumulador:
    player.slideDustTime += ctx.time.GetDeltaTime();
    const auto bounds = player.shape.getGlobalBounds();
    while (player.slideDustTime >= SLIDE_DUST_INTERVAL) { // while (no if): a fps bajos no se pierde dobras tandas
      player.slideDustTime -= SLIDE_DUST_INTERVAL;
      EmitDust({bounds.position.x, bounds.position.y + bounds.size.y}, SLIDE_DUST_COUNT, true);
    }
  } else {
    player.slideDustTime = 0; // sin plancha: reinicia el acumulador
  }

  // Comprime brevemente el cuerpo contra el suelo sin mover la caja de colisión.
  const float impact = player.grounded && !player.sliding ? player.stepImpact : 0; // SOLO al estar pisado en la carrera normal
  player.sprite.setScale({PLAYER_SPRITE_SCALE.x * (1 + FOOTSTEP_COMPRESSION * impact), // se ensancha un 4% respecto al impacto
                          PLAYER_SPRITE_SCALE.y * (1 - FOOTSTEP_COMPRESSION * impact)}); // y se aplasta un 4%: efecto "cartoon"
}

/* El mundo corre: nace quien toque a la derecha, todos viajan a la izquierda y algo que salió queda fuera. */
void Game::UpdateObstacles() {
  if (obstacles.empty() || obstacles.back().bounds.position.x < gConfig.windowSize.x - stats.obstacleNextX) {
    EventObstacleSpawn(); // el último quedó a X píxeles del borde: toca crear otro (y empty() PRIMERO: nunca back() de lista vacía)
    stats.obstacleNextX = ctx.random.Float(OBSTACLE_MIN_DISTANCE, OBSTACLE_MAX_DISTANCE); // la separación del SIGUIENTE también es al azar
  }
  const float movement = stats.speed * ctx.time.GetDeltaTime();
  for (auto &obstacle : obstacles) {
    obstacle.previousBounds = obstacle.bounds; // FOTO ANTES de mover: la colisión swept necesita el "antes"
    obstacle.bounds.position.x -= movement;
  }
  while (!obstacles.empty() && obstacles.front().bounds.position.x + obstacles.front().bounds.size.x < 0) {
    obstacles.pop_front(); // salió por la izquierda: fuera de la cola (pop_front en deque es barato)
  }
}

/* La velocidad del juego crece con el tiempo (a +10/s, hasta 800) y el HUD solo se refresca cuando el número mostrado CAMBIA. */
void Game::UpdateStats() {
  const int previousDistance = GetDistance(); // el "antes", para comparar
  stats.speed = std::min(stats.speed + STATS_SPEED_INCREASE * ctx.time.GetDeltaTime(), STATS_SPEED_MAX);

  stats.distance += stats.speed * ctx.time.GetDeltaTime(); // la distancia acumula el recorrido en píxeles
  if (GetDistance() != previousDistance) {
    stats.distanceText.setString(std::format("Distance: {}", GetDistance()));
  } // solo cuando cambió el entero: 60 setString/frame sería desperdicio
}

int Game::GetDistance() const { return static_cast<int>(stats.distance) / STATS_DISTANCE_SCALE; } // píxeles recorridos ÷ 100 = "metros" del HUD

/* El aviso del fin: cada frame reescribe el conteo con lo que falta; vencido el plazo, reinicia UNA sola vez. */
void Game::UpdateGameOver() {
  if (gameOver.restartRequested) { // ya pedí reiniciar: nada más
    return;
  }

  const float remaining = std::max(0.f, GAME_OVER_RESTART_DELAY - gameOver.restartCooldown.GetElapsedTime()); // lo que falta (0 min = 0)
  gameOver.restartText.setString(std::format("Reinicio en {:.1f} s", remaining)); // texto con vida: cuenta cada décima
  gameOver.restartText.setOrigin(gameOver.restartText.getLocalBounds().getCenter()); // re-centrar: "2.9 s" y "2.0 s" miden distinto
  if (gameOver.restartCooldown.IsOver()) {
    gameOver.restartRequested = true; // bandera anti-doble-reinicio
    ctx.scenes.RestartCurrentScene();
  }
}

/* Se perdió: congela el mundo, configura los textos del fin, silencia al jugador y baja el volumen de la música. */
void Game::EventGameOver() {
  if (gameOver.active) { // solo puede ocurrir una vez
    return;
  }
  const int distance = GetDistance();
  LOG_INFO("Game Over! Distance: {}", distance); // también al log (para depurar sin mirar la pantalla)
  gameOver.distanceText.setString(std::format("Distancia: {}", distance));
  gameOver.distanceText.setOrigin(gameOver.distanceText.getLocalBounds().getCenter());
  gameOver.active = true;
  StopPlayerSounds(); // ni el salto ni la plancha alrededor; juego congelado
  if (music) {
    music->setVolume(MUSIC_GAME_OVER_VOLUME); // la música baja (no muere): clima de derrota
  }
  gameOver.restartCooldown.Restart(); // el cuenta regresivo de vuelta a escena
  UpdateGameOver(); // pinta el texto al instante (sin esperar al siguiente frame)
}

/* El juego pausó o reanudó: efectos y música pausan con vos, y si había aviso de fin, su contador también espera. */
void Game::OnPause(bool paused) {
  if (music) {
    if (paused) {
      music->pause();
    } else if (music->getStatus() == sf::SoundSource::Status::Paused) {
      // Reanuda únicamente una pista pausada: play() sobre una activa la reiniciaría.
      music->play(); // retoma donde quedó
    }
  }
  for (auto &sound : soundEffects) { // los efectos obedecen igual
    if (!sound) { // este efecto nunca cargó: saltarlo
      continue;
    }
    if (paused) {
      sound->pause();
    } else if (sound->getStatus() == sf::SoundSource::Status::Paused) {
      sound->play();
    }
  }
  if (!gameOver.active || gameOver.restartRequested) {
    return; // en juego normal la pausa no toca nada del fin (no hay contador vivo aún)
  }
  if (paused) {
    gameOver.restartCooldown.Stop(); // pausa: la cuenta del aviso TAMBIÉN espera
  } else {
    gameOver.restartCooldown.Start(); // retomar donde quedó
  }
}

/* Se va la escena para siempre: apagar controles de audio para no dejar voces colgadas en el motor. */
void Game::OnCleanup() {
  ctx.particles.Clear();
  StopPlayerSounds();
  if (music) {
    music->stop();
  }
}

/* Toca un efecto del jugador; el de bucle (la plancha) si ya suena NO se reinicia: conserva su posición y sigue de corrido. */
void Game::PlayPlayerSound(PlayerSound sound) {
  if (gameOver.active) {
    return; // partida terminada: silencio
  }
  auto &effect = soundEffects[static_cast<std::size_t>(sound)];
  // Un bucle activo conserva su cursor para que el arrastre no se reinicie en cada cuadro.
  if (effect && (!effect->isLooping() || effect->getStatus() != sf::SoundSource::Status::Playing)) {
    effect->play(); // suena si es disparo único, o si es bucle pero estaba silenciado
  }
}

/* Apaga TODOS los efectos del jugador: sirve al morir, al reiniciar la ronda y al cerrar la escena. */
void Game::StopPlayerSounds() {
  for (auto &sound : soundEffects) {
    if (sound) {
      sound->stop();
    }
  }
}

/* Un paso: suena el pie que toca (izquierdo o derecho según el fotito), imprime el aplastamiento y deja marca y polvo. */
void Game::EventPlayerFootstep(int contactFrame, bool emitSound) {
  if (emitSound) {
    PlayPlayerSound(contactFrame == PLAYER_FOOTSTEP_FRAMES.front() ? PlayerSound::FootstepLeft : PlayerSound::FootstepRight);
  } // fotito 0 = pie izquierdo; cualquier otro contacto = pie derecho: pasos alternados
  const auto bounds = player.shape.getGlobalBounds();
  const sf::Vector2f contact(player.shape.getPosition().x + PLAYER_FOOTSTEP_OFFSET, // el punto exacto donde EL PIE sostiene
                             bounds.position.y + bounds.size.y);
  player.stepImpact = 1; // aplastamiento a máxima fuerza (decae linealmente en UpdateEffects)
  if (footstepPulses.size() < MAX_FOOTSTEP_PULSES) { // tope de marcas: las viejas mueren antes
    footstepPulses.push_back({contact, FOOTSTEP_DURATION}); // la marca vivirá 0.12 s
  }
  EmitDust(contact, FOOTSTEP_DUST_COUNT, false); // polvo de pisada normal: chiquito y liviano
}

/* Crea motas de polvo con vida, tamaño y velocidad al azar; de plancha salen más gordas, rápidas y con más tirón. */
void Game::EmitDust(sf::Vector2f position, int count, bool sliding) {
  const float speed = sliding ? SLIDE_DUST_SPEED : FOOTSTEP_DUST_SPEED; // velocidad lateral máxima: 100 vs 45
  const float lift = sliding ? SLIDE_DUST_LIFT : FOOTSTEP_DUST_LIFT; // impulso hacia arriba: 48 vs 25
  const int maxSize = sliding ? SLIDE_DUST_MAX_SIZE : FOOTSTEP_DUST_MAX_SIZE; // tamaño máximo: 4 vs 2px
  for (int index = 0; index < count && ctx.particles.Count() < MAX_DUST_PARTICLES; ++index) {
    const float lifetime = ctx.random.Float(DUST_MIN_LIFETIME, DUST_MAX_LIFETIME); // entre 0.22 y 0.48 s de vida
    // pos = el punto de contacto con jitter antira; vel = hacia atrás (−speed..−speed/3) y algo de tirón arriba...
    ctx.particles.Emit({position + sf::Vector2f(ctx.random.Float(-DUST_SPAWN_RADIUS, DUST_SPAWN_RADIUS), -DUST_SPAWN_HEIGHT),
                        {ctx.random.Float(-speed, -speed / 3), ctx.random.Float(-lift, -DUST_MIN_LIFT)},
                        {0, DUST_GRAVITY},
                        lifetime, // vida restante
                        lifetime, // vida TOTAL (para calcular cuánta opacidad queda)
                        static_cast<float>(ctx.random.Int(1, maxSize)),
                        sf::Color(DUST_COLOR.r, DUST_COLOR.g, DUST_COLOR.b, static_cast<std::uint8_t>(DUST_OPACITY))});
  } // todas nacen un pelito ARRIBA del punto de contacto, con dispersión lateral
}

/* Nace un obstáculo en el borde derecho: de piso (una de 3 figuras al azar) o volador, con pasaje a ras de la plancha. */
void Game::EventObstacleSpawn() {
  auto &obstacle = obstacles.emplace_back(); // uno al final del camino

  const float height = ctx.random.Float(OBSTACLE_MIN_HEIGHT, OBSTACLE_MAX_HEIGHT); // altura aleatoria
  obstacle.bounds.size = {OBSTACLE_WIDTH, height}; // solo datos: colisión geométrica pura (sin shapes SFML)

  const bool airborne = ctx.random.Bool(OBSTACLE_AIR_PROBABILITY);
  obstacle.spriteIndex = airborne ? OBSTACLE_AIR_SPRITE_INDEX : ctx.random.Int(0, OBSTACLE_GROUND_SPRITE_COUNT - 1); // figura ómiga del sheet
  obstacle.bounds.position = {gConfig.windowSize.x, ground.getPosition().y - height - (airborne ? PLAYER_SLIDE_SIZE.y : 0.f)};
} // el volador flota a altura plancha: el hueco mide EXACTO lo que ocupa deslizándose

/* La decisión de vida o muerte: el PRIMER contacto del frame manda; aterrizar de arriba es válido, rozar de lado no. */
void Game::HandleCollisionsPlayerObstacles() {
  const auto currentBounds = player.shape.getGlobalBounds(); // posición del jugador DESPUÉS de este frame
  const Obstacle *support = nullptr; // la caja sobre la que aterrizó (si hubo)
  float landingTime = 2; // "2" = infinito dentro del frame: solo existe el rango 0..1
  float crashTime = 2;

  // Elige el primer contacto y distingue el aterrizaje de un golpe lateral.
  for (const auto &obstacle : obstacles) {
    const auto hit = SweepObstacle(player.previousBounds, currentBounds, obstacle.previousBounds,
                                   obstacle.bounds); // comparar los cuatro rectángulos (antes/después por lado)
    if (!hit) {
      continue; // con este obstáculo no hubo cruce en el frame
    }

    if (hit->landing && hit->time < landingTime) { // aterrizaje: conservar el MÁS temprano de todos
      landingTime = hit->time;
      support = &obstacle;
    } else if (!hit->landing) {
      crashTime = std::min(crashTime, hit->time); // choque: ídem, todas son malas y queda la primera
    }
  }

  if (crashTime <= landingTime && crashTime <= 1) {
    EventGameOver(); // el choque ganó (llegó antes, o ni apoyo hubo): derrotado
    return; // NI ATERRIZAR en la misma caja ni nada más: fin
  }

  if (support && OverlapsHorizontally(currentBounds, support->bounds)) { // el aterrizaje ganó y lo sigo solapando +X
    player.shape.setPosition({player.shape.getPosition().x, // el centro = cima del obstáculo − mitad de mí
                              support->bounds.position.y - player.shape.getSize().y / 2});
    player.velocity.y = 0; // truco de ascensor: la caí se detiene
    player.grounded = true; // ¡piso nuevo! puede saltar desde la cima
  }

  const auto correctedBounds = player.shape.getGlobalBounds(); // posición DE POST-ATERRIZAJE (el snap la pudo mover)
  for (const auto &obstacle : obstacles) {
    if (&obstacle != support && correctedBounds.findIntersection(obstacle.bounds)) {
      EventGameOver(); // mi apoyo es válido... pero no puedo quedar metido en OTRA caja al mismo tiempo
      return;
    }
  }
}

/* Dibuja las capas de atrás hacia adelante: fondos, obstáculos, efectos, jugador, HUD y (si hay) el aviso del fin encima de todo. */
void Game::Render() const {
  RenderEnvironment();

  for (const auto &obstacle : obstacles) {
    RenderObstacle(obstacle);
  }

  RenderEffects(); // el polvo y las marcas quedan BAJO el jugador, como debe ser
  ctx.renderer.Draw(player.sprite);

  ctx.renderer.Draw(stats.distanceText); // HUD último: nunca lo tapa nada de la pista
  if (gameOver.active) {
    RenderGameOver(); // capa SUPERIOR: el aviso tapa hasta al HUD
  }
}

/* Orden de profundidad al dibujar: cielos lejanos, cuerpo de plataforma, edificios de primer plano y al final el borde del piso. */
void Game::RenderEnvironment() const {
  for (const auto &layer : backgroundLayers) { // 0.08× y 0.24×: lo lejano se dibuja PRIMERO (queda detrás)
    ctx.renderer.Draw(layer.shape);
  }
  ctx.renderer.Draw(platformFront); // el macizo
  // Los edificios cercanos se superponen a la plataforma, que conserva toda su altura.
  ctx.renderer.Draw(foregroundCity.shape); // delante del macizo: "la ciudad está a DERECHA del jugador"
  ctx.renderer.Draw(ground); // el borde SÍ tapa la base de los edificios de primer plano
}

/* El obstáculo se dibuja en 3 franjas: gorro y base tal cual el PNG (nítidas), y el cuerpo del medio se estira a la altura. */
void Game::RenderObstacle(const Obstacle &obstacle) const {
  // Conserva tapas y base a tamaño nativo; adapta el cuerpo recortando el PNG, sin estirarlo.
  const int cap = OBSTACLE_SPRITE_CAP_HEIGHT; // la franja de "gorro"/"base": 8px
  const int width = static_cast<int>(OBSTACLE_WIDTH); // 30px de ancho
  const int sourceX = obstacle.spriteIndex * OBSTACLE_SPRITE_FRAME_SIZE.x; // qué columna del sheet usar
  const auto position = obstacle.bounds.position;
  const float top = std::round(position.y); // dónde empieza y dónde acaba en PANTALLA (íntegro)
  const float bottom = std::round(position.y + obstacle.bounds.size.y);
  const int bodyHeight = static_cast<int>(bottom - top) - cap * 2; // bodyHeight = alto − (gorro + base): el medio absorbe TODO el estiramiento
  sf::Sprite part(*obstacleTexture);
  const auto drawPart = [&](int sourceY, int height, float y) { // pequeña fábrica: (fila del PNG, alto del recorte, dónde pegarlo)
    part.setTextureRect({{sourceX, sourceY}, {width, height}});
    part.setPosition({std::round(position.x), y});
    ctx.renderer.Draw(part);
  };
  drawPart(0, cap, top); // TAPA: filas 0..8 del PNG, en la cima del obstáculo
  drawPart(cap, bodyHeight, top + cap); // CUERPO: rellena todo el medio (puede estirarse muchísimo sin deformar extremos)
  drawPart(OBSTACLE_SPRITE_FRAME_SIZE.y - cap, cap, bottom - cap); // BASE: las últimas 8 filas del PNG, en el pie
}

/* El velo oscuro y los tres textos del fin, en orden de dibujo. */
void Game::RenderGameOver() const {
  ctx.renderer.Draw(gameOver.backdrop); // primero el velo: en carne oscurece detrás de los textos
  ctx.renderer.Draw(gameOver.title); // GAME OVER
  ctx.renderer.Draw(gameOver.distanceText); // Distancia: N
  ctx.renderer.Draw(gameOver.restartText); // Reinicio en X.X s
}

/* Polvo y marcas de pisada: cuadritas cuyo brillo baja con la edad (más viejas = más tenues). */
void Game::RenderEffects() const {
  sf::RectangleShape pixel; // la plantilla reutilizada para cada cuadrita
  const auto drawPixel = [&](sf::Vector2f position, sf::Vector2f size, sf::Color color, float opacity) {
    color.a = static_cast<std::uint8_t>(opacity); // aplica transparencia (canal alpha)
    pixel.setFillColor(color);
    pixel.setSize(size);
    pixel.setPosition({std::round(position.x), std::round(position.y)}); // redondeado: partículas nítidas
    ctx.renderer.Draw(pixel);
  };
  ctx.particles.Render(ctx.renderer);
  for (const auto &pulse : footstepPulses) {
    const float opacity = pulse.remainingLife / FOOTSTEP_DURATION;
    const float width = std::lerp(FOOTSTEP_PULSE_WIDTH, FOOTSTEP_PULSE_MIN_WIDTH, opacity); // nace chico (4px) y CRECE hasta 18 mientras muere
    drawPixel({pulse.position.x - width / 2, pulse.position.y - 1}, {width, 1}, FOOTSTEP_COLOR, FOOTSTEP_OPACITY * opacity);
  } // 1px de alto: una línea fina por el suelo
}

/* ¿Cuándo chocan por primera vez DENTRO de este frame (t: 0..1), y el contacto fue de arriba (aterrizar) o de lado (crash)? */
std::optional<ObstacleHit> Game::SweepObstacle(const sf::FloatRect &previousPlayer, const sf::FloatRect &currentPlayer,
                                               const sf::FloatRect &previousObstacle, const sf::FloatRect &currentObstacle) {
  const auto movement = (currentPlayer.position - previousPlayer.position) -
                        (currentObstacle.position - previousObstacle.position); // movimiento RELATIVO: cómo se acercaron ENTRE SÍ

  // Acepta los pequeños errores de redondeo al permanecer sobre una plataforma.
  const float previousFeet = previousPlayer.position.y + previousPlayer.size.y;
  if (movement.y >= 0 && std::abs(previousFeet - previousObstacle.position.y) <= PLAYER_COLLISION_TOLERANCE &&
      previousPlayer.position.x < previousObstacle.position.x + previousObstacle.size.x &&
      previousPlayer.position.x + previousPlayer.size.x > previousObstacle.position.x) {
    return ObstacleHit{0, true}; // pies EXACTOS sobre la superficie (±tolerancia): apoyo válido en t=0, no error de float
  }
  if (previousPlayer.findIntersection(previousObstacle)) {
    return ObstacleHit{0, false}; // ya estaba metido AL INICIAR el frame → choque inmediato
  }

  const float infinity = std::numeric_limits<float>::infinity();
  float entryX = -infinity, exitX = infinity; // los tiempos de cruce (t) por eje
  float entryY = -infinity, exitY = infinity;
  const auto sweepAxis = [](float playerMin, float playerSize, float obstacleMin, float obstacleSize,
                            float delta, float &entry, float &exit) {
    if (delta == 0) { // este eje no se movió: solo importa si de arranque estaban superpuestos
      return playerMin < obstacleMin + obstacleSize && playerMin + playerSize > obstacleMin;
    }
    const float first = (obstacleMin - playerMin - playerSize) / delta; // t en que la cara TRASERA llega
    const float last = (obstacleMin + obstacleSize - playerMin) / delta; // t en que la cara DELANTERA sale
    entry = std::min(first, last); // podría estar "al revés" según el signo: ordenar
    exit = std::max(first, last);
    return true;
  };
  if (!sweepAxis(previousPlayer.position.x, previousPlayer.size.x, previousObstacle.position.x,
                 previousObstacle.size.x, movement.x, entryX, exitX) ||
      !sweepAxis(previousPlayer.position.y, previousPlayer.size.y, previousObstacle.position.y,
                 previousObstacle.size.y, movement.y, entryY, exitY)) {
    return std::nullopt; // algún eje jamás se toca: incluso viajando, no hay contacto
  }
  const float entry = std::max(entryX, entryY); // el contacto INICIA cuando AMBOS ejes solapan...
  const float exit = std::min(exitX, exitY); // ...y termina cuando el PRIMERO se despega
  if (entry < 0 || entry > 1 || entry >= exit || exit <= 0) {
    return std::nullopt; // el cruce quedó fuera de este frame (antes o después) o la ventana contacta no existe
  }
  return ObstacleHit{entry, movement.y > 0 && entryY >= entryX}; // ¿descendiendo y el eje Y cerró PRIMERO? → aterrizó
}

/* ¿Los dos cuerpos comparten el mismo tramo en X? Sirve para saber si un apoyo sigue bajo los pies. */
bool Game::OverlapsHorizontally(const sf::FloatRect &player, const sf::FloatRect &obstacle) {
  return player.position.x < obstacle.position.x + obstacle.size.x &&
         player.position.x + player.size.x > obstacle.position.x;
}
