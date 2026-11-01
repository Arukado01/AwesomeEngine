#include "Runner.h"

#include <algorithm>
#include <cmath>
#include <format>
#include <limits>
#include <stdexcept>

using namespace Runner;

Game::Game(EngineContext &context)
    : Scene(context), player(LoadTexture(context, PLAYER_TEXTURE_FILENAME)), music(context.resources.FetchMusic(MUSIC_FILENAME)) {
  dustParticles.reserve(MAX_DUST_PARTICLES);
  footstepPulses.reserve(MAX_FOOTSTEP_PULSES);
  InitEnvironment();
  InitGameOver();
  if (music) {
    music->setLooping(true);
    music->setVolume(MUSIC_VOLUME);
  }
  for (std::size_t index = 0; index < soundEffects.size(); ++index) {
    const auto &config = PLAYER_SOUNDS[index];
    if (auto *buffer = ctx.resources.FetchSound(config.filename); buffer && buffer->getSampleCount() > 0) {
      auto &sound = soundEffects[index].emplace(*buffer);
      sound.setVolume(config.volume);
      sound.setLooping(config.looping);
    }
  }
}

sf::Texture &Game::LoadTexture(EngineContext &context, const std::string &filename, bool repeated) {
  auto *texture = context.resources.FetchTexture(filename);
  if (!texture || texture->getSize() == sf::Vector2u{}) {
    throw std::runtime_error("No se pudo cargar la textura del runner: " + filename);
  }
  texture->setSmooth(false);
  texture->setRepeated(repeated);
  return *texture;
}

void Game::InitEnvironment() {
  ground.setSize({gConfig.windowSize.x, GROUND_HEIGHT});
  ground.setPosition({0, gConfig.windowSize.y / 2});
  for (std::size_t index = 0; index < backgroundLayers.size(); ++index) {
    auto &layer = backgroundLayers[index];
    layer.shape.setTexture(&LoadTexture(ctx, BACKGROUND_TEXTURE_FILENAMES[index], true));
    layer.shape.setSize({gConfig.windowSize.x, ground.getPosition().y});
    layer.shape.setFillColor(BACKGROUND_TINT);
  }

  auto &platformTexture = LoadTexture(ctx, PLATFORM_TEXTURE_FILENAME, true);
  ground.setTexture(&platformTexture);
  ground.setFillColor(sf::Color::White);
  platformFront.setTexture(&platformTexture);
  platformFront.setPosition({0, ground.getPosition().y + GROUND_HEIGHT});
  platformFront.setSize({gConfig.windowSize.x, gConfig.windowSize.y - platformFront.getPosition().y});
  foregroundCity.shape.setTexture(&LoadTexture(ctx, FOREGROUND_TEXTURE_FILENAME, true));
  foregroundCity.shape.setFillColor(FOREGROUND_TINT);
  foregroundCity.shape.setPosition(platformFront.getPosition());
  foregroundCity.shape.setSize(platformFront.getSize());

  obstacleTexture = &LoadTexture(ctx, OBSTACLE_TEXTURE_FILENAME);
}

void Game::InitGameOver() {
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

void Game::Start() {
  StopPlayerSounds();
  ctx.cursor.SetVisible(false);

  BindInputs();

  obstacles.clear();
  dustParticles.clear();
  footstepPulses.clear();
  gameOver.active = false;
  gameOver.restartRequested = false;
  gameOver.restartCooldown.Reset();

  for (auto &layer : backgroundLayers) {
    layer.offset = 0;
  }
  platformOffset = 0;
  foregroundCity.offset = 0;

  StartStats();
  UpdateEnvironment(0);
  StartPlayer();

  EventObstacleSpawn();
  if (music) {
    music->stop();
    music->setVolume(MUSIC_VOLUME);
    music->play();
  }
}

void Game::BindInputs() {
  ctx.input.Bind(Jump, Input::Keyboard{sf::Keyboard::Scan::W});
  ctx.input.Bind(Jump, Input::Gamepad{GamepadButton::South});

  ctx.input.Bind(Crouch, Input::Keyboard{sf::Keyboard::Scan::S});
  ctx.input.Bind(Crouch, Input::Gamepad{GamepadButton::West});

  ctx.input.Bind(Fall, Input::Keyboard{sf::Keyboard::Scan::S});
  ctx.input.Bind(Fall, Input::Axis{sf::Joystick::Axis::Y});
}

void Game::StartPlayer() {
  player = Player(player.sprite.getTexture());
  player.sprite.setOrigin({PLAYER_FRAME_SIZE.x / 2.f, static_cast<float>(PLAYER_FRAME_SIZE.y)});
  player.sprite.setScale(PLAYER_SPRITE_SCALE);
  player.shape.setSize(PLAYER_SIZE);
  player.shape.setOrigin(player.shape.getGeometricCenter());

  float positionY = ground.getPosition().y - player.shape.getSize().y / 2;
  player.shape.setPosition({gConfig.windowSize.x / 6, positionY});

  player.previousBounds = player.shape.getGlobalBounds();
  UpdatePlayerAnimation(false);
}

void Game::StartStats() {
  stats.distanceText.setFillColor(STATS_DISTANCE_TEXT_COLOR);
  stats.distanceText.setPosition({10, 10});
  stats.speed = STATS_SPEED_START;
  stats.obstacleNextX = ctx.random.Float(OBSTACLE_MIN_DISTANCE, OBSTACLE_MAX_DISTANCE);

  stats.distance = 0;
  stats.distanceText.setString("Distance: 0");
}

void Game::Update() {
  // Mantiene el resultado visible sin avanzar la simulación durante la espera.
  if (gameOver.active) {
    UpdateGameOver();
    return;
  }

  UpdateEffects();
  UpdateEnvironment(ctx.time.GetDeltaTime());
  if (ctx.input.Pressed(Jump) && player.grounded && !player.sliding) {
    player.velocity.y = PLAYER_JUMP_VELOCITY;
    player.grounded = false;
    PlayPlayerSound(PlayerSound::Jump);
  }

  UpdatePlayer();
  UpdateObstacles();
  UpdateStats();

  HandleCollisionsPlayerObstacles();
  UpdatePlayerGrounded();
  UpdatePlayerAnimation();
  UpdatePlayerEffects();
}

void Game::UpdatePlayer() {
  UpdatePlayerSize();
  player.previousBounds = player.shape.getGlobalBounds();
  const float delta = ctx.time.GetDeltaTime();
  const float gravity = PLAYER_GRAVITY * (ctx.input.Pressed(Fall) && !player.grounded ? PLAYER_FALL_FACTOR : 1.f);
  player.velocity.y += gravity * delta;
  player.shape.move({0, player.velocity.y * delta});
  player.grounded = false;
}

void Game::UpdatePlayerSize() {
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
    // Bajo un obstáculo conserva la plancha hasta disponer de espacio para levantarse.
    if (!CanPlayerStand()) {
      SetSlidePhase(SlidePhase::Loop);
      return;
    }
    if (player.slidePhase != SlidePhase::Recover) {
      SetSlidePhase(SlidePhase::Recover);
      return;
    }
    const auto &clip = PLAYER_ANIMATIONS[static_cast<std::size_t>(PlayerAnimation::Slide)];
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
  player.shape.move({0, (oldHeight - newSize.y) / 2});
  player.sliding = sliding;
  SetSlidePhase(SlidePhase::Prepare);
}

void Game::SetSlidePhase(SlidePhase phase) {
  if (player.slidePhase != phase) {
    player.slidePhase = phase;
    player.slidePhaseTime = 0;
  }
}

bool Game::CanPlayerStand() const {
  const auto bounds = player.shape.getGlobalBounds();
  const sf::FloatRect standingBounds({player.shape.getPosition().x - PLAYER_SIZE.x / 2,
                                      bounds.position.y + bounds.size.y - PLAYER_SIZE.y},
                                     PLAYER_SIZE);
  return std::none_of(obstacles.begin(), obstacles.end(), [&](const Obstacle &obstacle) {
    const auto &obstacleBounds = obstacle.bounds;
    return obstacleBounds.position.y < bounds.position.y + bounds.size.y - PLAYER_COLLISION_TOLERANCE &&
           standingBounds.findIntersection(obstacleBounds).has_value();
  });
}

void Game::UpdatePlayerGrounded() {
  float halfHeight = player.shape.getSize().y / 2;

  if (player.shape.getPosition().y + halfHeight >= ground.getPosition().y) {
    player.shape.setPosition({player.shape.getPosition().x, ground.getPosition().y - halfHeight});
    player.velocity.y = 0;
    player.grounded = true;
  }
}

void Game::UpdatePlayerAnimation(bool emitSound) {
  auto animation = PlayerAnimation::Run;
  if (player.sliding) {
    animation = PlayerAnimation::Slide;
  } else if (!player.grounded) {
    animation = player.velocity.y < 0 ? PlayerAnimation::Jump : PlayerAnimation::Fall;
  }
  if (animation != player.animation) {
    player.animation = animation;
    player.animationTime = 0;
    player.animationFrame = -1;
    if (animation == PlayerAnimation::Slide && player.grounded) {
      const auto bounds = player.shape.getGlobalBounds();
      EmitDust({bounds.position.x, bounds.position.y + bounds.size.y}, SLIDE_DUST_BURST_COUNT, true);
    }
  }

  const auto &clip = PLAYER_ANIMATIONS[static_cast<std::size_t>(animation)];
  int frame = 0;
  if (animation == PlayerAnimation::Run) {
    // Conserva una cadencia legible sin acelerar excesivamente los pasos.
    const float playbackSpeed = std::min(clip.framesPerSecond * stats.speed / STATS_SPEED_START, PLAYER_ANIMATION_MAX_FPS);
    const float nextTime = player.animationTime + ctx.time.GetDeltaTime() * playbackSpeed;
    if (player.animationFrame < 0) {
      EventPlayerFootstep(PLAYER_FOOTSTEP_FRAMES.front(), emitSound);
    }
    // Emite una pisada únicamente al cruzar un cuadro de contacto, incluso al cerrar el ciclo.
    for (int step = static_cast<int>(player.animationTime) + 1; step <= static_cast<int>(nextTime); ++step) {
      if (std::ranges::find(PLAYER_FOOTSTEP_FRAMES, step % clip.frameCount) != PLAYER_FOOTSTEP_FRAMES.end()) {
        EventPlayerFootstep(step % clip.frameCount, emitSound);
      }
    }
    player.animationTime = std::fmod(nextTime, static_cast<float>(clip.frameCount));
    frame = static_cast<int>(player.animationTime);
  } else if (animation == PlayerAnimation::Slide) {
    if (player.grounded) {
      player.slidePhaseTime += ctx.time.GetDeltaTime();
    }
    if (player.slidePhase == SlidePhase::Prepare && player.slidePhaseTime >= SLIDE_PREPARE_DURATION) {
      SetSlidePhase(SlidePhase::Loop);
    }
    if (player.slidePhase == SlidePhase::Loop) {
      const float cycleDuration = SLIDE_LOOP_FRAME_COUNT / SLIDE_LOOP_FPS;
      player.slidePhaseTime = std::fmod(player.slidePhaseTime, cycleDuration);
      frame = SLIDE_LOOP_FIRST_FRAME + static_cast<int>(player.slidePhaseTime * SLIDE_LOOP_FPS);
    } else if (player.slidePhase == SlidePhase::Recover) {
      frame = SLIDE_RECOVER_FIRST_FRAME + std::min(static_cast<int>(player.slidePhaseTime * clip.framesPerSecond),
                                                   SLIDE_RECOVER_FRAME_COUNT - 1);
    }
  } else if (animation == PlayerAnimation::Jump) {
    // Pasa de impulso a recogida de piernas al acercarse al punto más alto.
    frame = player.velocity.y < PLAYER_JUMP_VELOCITY / 2 ? 0 : clip.frameCount - 1;
  } else if (animation == PlayerAnimation::Fall) {
    frame = ctx.input.Pressed(Fall) ? clip.frameCount - 1 : 0;
  }

  const int sheetFrame = clip.firstFrame + frame;
  player.animationFrame = frame;
  player.sprite.setTextureRect({{(sheetFrame % PLAYER_SHEET_COLUMNS) * PLAYER_FRAME_SIZE.x,
                                 (sheetFrame / PLAYER_SHEET_COLUMNS) * PLAYER_FRAME_SIZE.y},
                                PLAYER_FRAME_SIZE});

  // Redondea solo el dibujo, sin alterar la precisión de las colisiones.
  const auto bounds = player.shape.getGlobalBounds();
  player.sprite.setPosition({std::round(bounds.position.x + bounds.size.x / 2), std::round(bounds.position.y + bounds.size.y)});
}

void Game::UpdateEffects() {
  const float delta = ctx.time.GetDeltaTime();
  player.stepImpact = std::max(0.f, player.stepImpact - delta / FOOTSTEP_DURATION);
  for (auto &particle : dustParticles) {
    particle.remainingLife -= delta;
    particle.velocity.y += DUST_GRAVITY * delta;
    particle.position += sf::Vector2f(particle.velocity.x - stats.speed * DUST_SCROLL_FACTOR, particle.velocity.y) * delta;
  }
  std::erase_if(dustParticles, [](const DustParticle &particle) { return particle.remainingLife <= 0; });
  for (auto &pulse : footstepPulses) {
    pulse.remainingLife -= delta;
    pulse.position.x -= stats.speed * delta;
  }
  std::erase_if(footstepPulses, [](const FootstepPulse &pulse) { return pulse.remainingLife <= 0; });
}

void Game::UpdateEnvironment(float deltaTime) {
  const int width = static_cast<int>(std::ceil(gConfig.windowSize.x / ENVIRONMENT_PIXEL_SCALE));
  const auto scrollLayer = [&](BackgroundLayer &layer, float speedFactor) {
    const float textureWidth = static_cast<float>(layer.shape.getTexture()->getSize().x);
    layer.offset = std::fmod(layer.offset + stats.speed * speedFactor * deltaTime / ENVIRONMENT_PIXEL_SCALE, textureWidth);
    const int height = static_cast<int>(std::ceil(layer.shape.getSize().y / ENVIRONMENT_PIXEL_SCALE));
    layer.shape.setTextureRect({{static_cast<int>(layer.offset), 0}, {width, height}});
  };
  for (std::size_t index = 0; index < backgroundLayers.size(); ++index) {
    scrollLayer(backgroundLayers[index], BACKGROUND_SPEED_FACTORS[index]);
  }
  scrollLayer(foregroundCity, FOREGROUND_SPEED_FACTOR);

  const auto textureWidth = ground.getTexture()->getSize().x;
  platformOffset = std::fmod(platformOffset + stats.speed * deltaTime / ENVIRONMENT_PIXEL_SCALE, static_cast<float>(textureWidth));
  const int offset = static_cast<int>(platformOffset);
  const int lipHeight = static_cast<int>(GROUND_HEIGHT / ENVIRONMENT_PIXEL_SCALE);
  ground.setTextureRect({{offset, 0}, {width, lipHeight}});
  platformFront.setTextureRect({{offset, lipHeight}, {width, static_cast<int>(platformFront.getSize().y / ENVIRONMENT_PIXEL_SCALE)}});
}

void Game::UpdatePlayerEffects() {
  auto &slideSound = soundEffects[static_cast<std::size_t>(PlayerSound::Slide)];
  if (player.sliding && player.grounded && !gameOver.active) {
    PlayPlayerSound(PlayerSound::Slide);
  } else if (slideSound && slideSound->getStatus() != sf::SoundSource::Status::Stopped) {
    // SFML espera al hilo de audio en stop(); no repetir esa espera en cada cuadro.
    slideSound->stop();
  }
  if (player.sliding && player.grounded) {
    player.slideDustTime += ctx.time.GetDeltaTime();
    const auto bounds = player.shape.getGlobalBounds();
    while (player.slideDustTime >= SLIDE_DUST_INTERVAL) {
      player.slideDustTime -= SLIDE_DUST_INTERVAL;
      EmitDust({bounds.position.x, bounds.position.y + bounds.size.y}, SLIDE_DUST_COUNT, true);
    }
  } else {
    player.slideDustTime = 0;
  }

  // Comprime brevemente el cuerpo contra el suelo sin mover la caja de colisión.
  const float impact = player.grounded && !player.sliding ? player.stepImpact : 0;
  player.sprite.setScale({PLAYER_SPRITE_SCALE.x * (1 + FOOTSTEP_COMPRESSION * impact),
                          PLAYER_SPRITE_SCALE.y * (1 - FOOTSTEP_COMPRESSION * impact)});
}

void Game::UpdateObstacles() {
  if (obstacles.empty() || obstacles.back().bounds.position.x < gConfig.windowSize.x - stats.obstacleNextX) {
    EventObstacleSpawn();
    stats.obstacleNextX = ctx.random.Float(OBSTACLE_MIN_DISTANCE, OBSTACLE_MAX_DISTANCE);
  }
  const float movement = stats.speed * ctx.time.GetDeltaTime();
  for (auto &obstacle : obstacles) {
    obstacle.previousBounds = obstacle.bounds;
    obstacle.bounds.position.x -= movement;
  }
  while (!obstacles.empty() && obstacles.front().bounds.position.x + obstacles.front().bounds.size.x < 0) {
    obstacles.pop_front();
  }
}

void Game::UpdateStats() {
  const int previousDistance = GetDistance();
  stats.speed = std::min(stats.speed + STATS_SPEED_INCREASE * ctx.time.GetDeltaTime(), STATS_SPEED_MAX);

  stats.distance += stats.speed * ctx.time.GetDeltaTime();
  if (GetDistance() != previousDistance) {
    stats.distanceText.setString(std::format("Distance: {}", GetDistance()));
  }
}

int Game::GetDistance() const { return static_cast<int>(stats.distance) / STATS_DISTANCE_SCALE; }

void Game::UpdateGameOver() {
  if (gameOver.restartRequested) {
    return;
  }

  const float remaining = std::max(0.f, GAME_OVER_RESTART_DELAY - gameOver.restartCooldown.GetElapsedTime());
  gameOver.restartText.setString(std::format("Reinicio en {:.1f} s", remaining));
  gameOver.restartText.setOrigin(gameOver.restartText.getLocalBounds().getCenter());
  if (gameOver.restartCooldown.IsOver()) {
    gameOver.restartRequested = true;
    ctx.scenes.RestartCurrentScene();
  }
}

void Game::EventGameOver() {
  if (gameOver.active) {
    return;
  }
  const int distance = GetDistance();
  LOG_INFO("Game Over! Distance: {}", distance);
  gameOver.distanceText.setString(std::format("Distancia: {}", distance));
  gameOver.distanceText.setOrigin(gameOver.distanceText.getLocalBounds().getCenter());
  gameOver.active = true;
  StopPlayerSounds();
  if (music) {
    music->setVolume(MUSIC_GAME_OVER_VOLUME);
  }
  gameOver.restartCooldown.Restart();
  UpdateGameOver();
}

void Game::OnPause(bool paused) {
  if (music) {
    if (paused) {
      music->pause();
    } else if (music->getStatus() == sf::SoundSource::Status::Paused) {
      // Reanuda únicamente una pista pausada: play() sobre una activa la reiniciaría.
      music->play();
    }
  }
  for (auto &sound : soundEffects) {
    if (!sound) {
      continue;
    }
    if (paused) {
      sound->pause();
    } else if (sound->getStatus() == sf::SoundSource::Status::Paused) {
      sound->play();
    }
  }
  if (!gameOver.active || gameOver.restartRequested) {
    return;
  }
  if (paused) {
    gameOver.restartCooldown.Stop();
  } else {
    gameOver.restartCooldown.Start();
  }
}

void Game::OnCleanup() {
  StopPlayerSounds();
  if (music) {
    music->stop();
  }
}

void Game::PlayPlayerSound(PlayerSound sound) {
  if (gameOver.active) {
    return;
  }
  auto &effect = soundEffects[static_cast<std::size_t>(sound)];
  // Un bucle activo conserva su cursor para que el arrastre no se reinicie en cada cuadro.
  if (effect && (!effect->isLooping() || effect->getStatus() != sf::SoundSource::Status::Playing)) {
    effect->play();
  }
}

void Game::StopPlayerSounds() {
  for (auto &sound : soundEffects) {
    if (sound) {
      sound->stop();
    }
  }
}

void Game::EventPlayerFootstep(int contactFrame, bool emitSound) {
  if (emitSound) {
    PlayPlayerSound(contactFrame == PLAYER_FOOTSTEP_FRAMES.front() ? PlayerSound::FootstepLeft : PlayerSound::FootstepRight);
  }
  const auto bounds = player.shape.getGlobalBounds();
  const sf::Vector2f contact(player.shape.getPosition().x + PLAYER_FOOTSTEP_OFFSET,
                             bounds.position.y + bounds.size.y);
  player.stepImpact = 1;
  if (footstepPulses.size() < MAX_FOOTSTEP_PULSES) {
    footstepPulses.push_back({contact, FOOTSTEP_DURATION});
  }
  EmitDust(contact, FOOTSTEP_DUST_COUNT, false);
}

void Game::EmitDust(sf::Vector2f position, int count, bool sliding) {
  const float speed = sliding ? SLIDE_DUST_SPEED : FOOTSTEP_DUST_SPEED;
  const float lift = sliding ? SLIDE_DUST_LIFT : FOOTSTEP_DUST_LIFT;
  const int maxSize = sliding ? SLIDE_DUST_MAX_SIZE : FOOTSTEP_DUST_MAX_SIZE;
  for (int index = 0; index < count && dustParticles.size() < MAX_DUST_PARTICLES; ++index) {
    const float lifetime = ctx.random.Float(DUST_MIN_LIFETIME, DUST_MAX_LIFETIME);
    dustParticles.push_back({position + sf::Vector2f(ctx.random.Float(-DUST_SPAWN_RADIUS, DUST_SPAWN_RADIUS), -DUST_SPAWN_HEIGHT),
                             {ctx.random.Float(-speed, -speed / 3), ctx.random.Float(-lift, -DUST_MIN_LIFT)},
                             lifetime,
                             lifetime,
                             static_cast<float>(ctx.random.Int(1, maxSize))});
  }
}

void Game::EventObstacleSpawn() {
  auto &obstacle = obstacles.emplace_back();

  const float height = ctx.random.Float(OBSTACLE_MIN_HEIGHT, OBSTACLE_MAX_HEIGHT);
  obstacle.bounds.size = {OBSTACLE_WIDTH, height};

  const bool airborne = ctx.random.Bool(OBSTACLE_AIR_PROBABILITY);
  obstacle.spriteIndex = airborne ? OBSTACLE_AIR_SPRITE_INDEX : ctx.random.Int(0, OBSTACLE_GROUND_SPRITE_COUNT - 1);
  obstacle.bounds.position = {gConfig.windowSize.x, ground.getPosition().y - height - (airborne ? PLAYER_SLIDE_SIZE.y : 0.f)};
}

void Game::HandleCollisionsPlayerObstacles() {
  const auto currentBounds = player.shape.getGlobalBounds();
  const Obstacle *support = nullptr;
  float landingTime = 2;
  float crashTime = 2;

  // Elige el primer contacto y distingue el aterrizaje de un golpe lateral.
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

  if (crashTime <= landingTime && crashTime <= 1) {
    EventGameOver();
    return;
  }

  if (support && OverlapsHorizontally(currentBounds, support->bounds)) {
    player.shape.setPosition({player.shape.getPosition().x,
                              support->bounds.position.y - player.shape.getSize().y / 2});
    player.velocity.y = 0;
    player.grounded = true;
  }

  const auto correctedBounds = player.shape.getGlobalBounds();
  for (const auto &obstacle : obstacles) {
    if (&obstacle != support && correctedBounds.findIntersection(obstacle.bounds)) {
      EventGameOver();
      return;
    }
  }
}

void Game::Render() const {
  RenderEnvironment();

  for (const auto &obstacle : obstacles) {
    RenderObstacle(obstacle);
  }

  RenderEffects();
  ctx.renderer.Draw(player.sprite);

  ctx.renderer.Draw(stats.distanceText);
  if (gameOver.active) {
    RenderGameOver();
  }
}

void Game::RenderEnvironment() const {
  for (const auto &layer : backgroundLayers) {
    ctx.renderer.Draw(layer.shape);
  }
  ctx.renderer.Draw(platformFront);
  // Los edificios cercanos se superponen a la plataforma, que conserva toda su altura.
  ctx.renderer.Draw(foregroundCity.shape);
  ctx.renderer.Draw(ground);
}

void Game::RenderObstacle(const Obstacle &obstacle) const {
  // Conserva tapas y base a tamaño nativo; adapta el cuerpo recortando el PNG, sin estirarlo.
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

void Game::RenderGameOver() const {
  ctx.renderer.Draw(gameOver.backdrop);
  ctx.renderer.Draw(gameOver.title);
  ctx.renderer.Draw(gameOver.distanceText);
  ctx.renderer.Draw(gameOver.restartText);
}

void Game::RenderEffects() const {
  sf::RectangleShape pixel;
  const auto drawPixel = [&](sf::Vector2f position, sf::Vector2f size, sf::Color color, float opacity) {
    color.a = static_cast<std::uint8_t>(opacity);
    pixel.setFillColor(color);
    pixel.setSize(size);
    pixel.setPosition({std::round(position.x), std::round(position.y)});
    ctx.renderer.Draw(pixel);
  };
  for (const auto &particle : dustParticles) {
    const float opacity = particle.remainingLife / particle.lifetime;
    drawPixel(particle.position, {particle.size, particle.size}, DUST_COLOR, DUST_OPACITY * opacity);
  }
  for (const auto &pulse : footstepPulses) {
    const float opacity = pulse.remainingLife / FOOTSTEP_DURATION;
    const float width = std::lerp(FOOTSTEP_PULSE_WIDTH, FOOTSTEP_PULSE_MIN_WIDTH, opacity);
    drawPixel({pulse.position.x - width / 2, pulse.position.y - 1}, {width, 1}, FOOTSTEP_COLOR, FOOTSTEP_OPACITY * opacity);
  }
}

// Calcula el primer contacto durante el movimiento relativo de ambos cuerpos.
std::optional<ObstacleHit> Game::SweepObstacle(const sf::FloatRect &previousPlayer, const sf::FloatRect &currentPlayer,
                                               const sf::FloatRect &previousObstacle, const sf::FloatRect &currentObstacle) {
  const auto movement = (currentPlayer.position - previousPlayer.position) -
                        (currentObstacle.position - previousObstacle.position);

  // Acepta los pequeños errores de redondeo al permanecer sobre una plataforma.
  const float previousFeet = previousPlayer.position.y + previousPlayer.size.y;
  if (movement.y >= 0 && std::abs(previousFeet - previousObstacle.position.y) <= PLAYER_COLLISION_TOLERANCE &&
      previousPlayer.position.x < previousObstacle.position.x + previousObstacle.size.x &&
      previousPlayer.position.x + previousPlayer.size.x > previousObstacle.position.x) {
    return ObstacleHit{0, true};
  }
  if (previousPlayer.findIntersection(previousObstacle)) {
    return ObstacleHit{0, false};
  }

  const float infinity = std::numeric_limits<float>::infinity();
  float entryX = -infinity, exitX = infinity;
  float entryY = -infinity, exitY = infinity;
  const auto sweepAxis = [](float playerMin, float playerSize, float obstacleMin, float obstacleSize,
                            float delta, float &entry, float &exit) {
    if (delta == 0) {
      return playerMin < obstacleMin + obstacleSize && playerMin + playerSize > obstacleMin;
    }
    const float first = (obstacleMin - playerMin - playerSize) / delta;
    const float last = (obstacleMin + obstacleSize - playerMin) / delta;
    entry = std::min(first, last);
    exit = std::max(first, last);
    return true;
  };
  if (!sweepAxis(previousPlayer.position.x, previousPlayer.size.x, previousObstacle.position.x,
                 previousObstacle.size.x, movement.x, entryX, exitX) ||
      !sweepAxis(previousPlayer.position.y, previousPlayer.size.y, previousObstacle.position.y,
                 previousObstacle.size.y, movement.y, entryY, exitY)) {
    return std::nullopt;
  }
  const float entry = std::max(entryX, entryY);
  const float exit = std::min(exitX, exitY);
  if (entry < 0 || entry > 1 || entry >= exit || exit <= 0) {
    return std::nullopt;
  }
  return ObstacleHit{entry, movement.y > 0 && entryY >= entryX};
}

bool Game::OverlapsHorizontally(const sf::FloatRect &player, const sf::FloatRect &obstacle) {
  return player.position.x < obstacle.position.x + obstacle.size.x &&
         player.position.x + player.size.x > obstacle.position.x;
}
