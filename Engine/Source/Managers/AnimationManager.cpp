#include "Managers/AnimationManager.h"

#include <cmath>
#include <stdexcept>
#include <utility>
#include <vector>

/* Reserva un identificador único y crea el estado de un nuevo animador. */
AnimationHandle AnimationManager::Create(AnimationGroup group) {
  if (nextHandle_ == INVALID_ANIMATION) {
    throw std::overflow_error("No quedan identificadores de animación disponibles");
  }
  const AnimationHandle handle = nextHandle_++;
  auto entry = animators_.try_emplace(handle).first;
  entry->second.group = group;
  if (pausedGroups_.contains(group)) {
    entry->second.animator.Pause();
    entry->second.resumeAfterGroup = true;
  }
  return handle;
}

/* Devuelve el animador mientras siga registrado en este manager. */
SpriteAnimator *AnimationManager::Get(AnimationHandle handle) {
  if (clearPending_ || pendingRemovals_.contains(handle)) {
    return nullptr;
  }
  const auto found = animators_.find(handle);
  return found == animators_.end() ? nullptr : &found->second.animator;
}

/* Permite consultar un animador desde un manager constante. */
const SpriteAnimator *AnimationManager::Get(AnimationHandle handle) const {
  if (clearPending_ || pendingRemovals_.contains(handle)) {
    return nullptr;
  }
  const auto found = animators_.find(handle);
  return found == animators_.end() ? nullptr : &found->second.animator;
}

/* Retira una instancia cuando su objeto o escena ya no la necesita. */
bool AnimationManager::Remove(AnimationHandle handle) {
  if (!Get(handle)) {
    return false;
  }
  // Durante un evento, se aplaza la destrucción hasta que termine el avance en curso.
  if (updating_) {
    pendingRemovals_.insert(handle);
    return true;
  }
  return animators_.erase(handle) != 0;
}

/* Guarda la configuración propia de una instancia y la prepara para su grupo. */
bool AnimationManager::Configure(AnimationHandle handle, SpriteAnimationClip clip, SpritePlaybackMode mode,
                                 std::function<void(int)> onFrame) {
  if (updating_ || !Get(handle) || clip.firstFrame < 0 || clip.frameCount <= 0 || !std::isfinite(clip.framesPerSecond) || clip.framesPerSecond <= 0) {
    return false;
  }
  auto &entry = animators_.at(handle);
  entry.clip = clip;
  entry.onFrame = std::move(onFrame);
  entry.animator.SetMode(mode);
  entry.animator.Reset();
  if (pausedGroups_.contains(entry.group)) {
    entry.animator.Pause();
    entry.resumeAfterGroup = true;
  } else {
    entry.resumeAfterGroup = false;
  }
  return true;
}

/* Ajusta el multiplicador de velocidad; cero mantiene la animación en su cuadro. */
bool AnimationManager::SetSpeed(AnimationHandle handle, float factor) {
  if (!Get(handle) || !std::isfinite(factor) || factor < 0) {
    return false;
  }
  animators_.at(handle).speed = factor;
  return true;
}

/* Cambia la pausa individual sin pasar por encima de la pausa del grupo. */
bool AnimationManager::SetPaused(AnimationHandle handle, bool paused) {
  auto *animator = Get(handle);
  if (!animator) {
    return false;
  }
  auto &entry = animators_.at(handle);
  if (pausedGroups_.contains(entry.group)) {
    entry.resumeAfterGroup = !paused && !animator->IsFinished();
  } else if (paused) {
    animator->Pause();
  } else {
    animator->Resume();
  }
  return true;
}

/* Conserva el estado previo de cada instancia al pausar o reanudar su grupo. */
void AnimationManager::SetGroupPaused(AnimationGroup group, bool paused) {
  if (paused == pausedGroups_.contains(group)) {
    return;
  }
  if (paused) {
    pausedGroups_.insert(group);
  } else {
    pausedGroups_.erase(group);
  }
  for (auto &[handle, entry] : animators_) {
    if (entry.group != group || !Get(handle)) {
      continue;
    }
    if (paused) {
      entry.resumeAfterGroup = entry.animator.IsPlaying();
      if (entry.resumeAfterGroup) {
        entry.animator.Pause();
      }
    } else {
      if (entry.resumeAfterGroup) {
        entry.animator.Resume();
      }
      entry.resumeAfterGroup = false;
    }
  }
}

/* Aplica los retiros solicitados por callbacks después de actualizar el grupo. */
void AnimationManager::FinishUpdate() {
  updating_ = false;
  if (clearPending_) {
    animators_.clear();
    pausedGroups_.clear();
    clearPending_ = false;
  } else {
    for (const auto handle : pendingRemovals_) {
      animators_.erase(handle);
    }
  }
  pendingRemovals_.clear();
}

/* Avanza los clips del grupo una sola vez y permite retirarlos desde sus eventos. */
void AnimationManager::AdvanceGroup(AnimationGroup group, float deltaTime) {
  if (updating_ || pausedGroups_.contains(group)) {
    return;
  }
  // La lista fija evita recorrer instancias creadas por un callback en este mismo frame.
  std::vector<AnimationHandle> handles;
  for (const auto &[handle, entry] : animators_) {
    if (entry.group == group && entry.clip.frameCount > 0) {
      handles.push_back(handle);
    }
  }
  updating_ = true;
  try {
    for (const auto handle : handles) {
      auto *animator = Get(handle);
      if (!animator || pausedGroups_.contains(group)) {
        continue;
      }
      const auto &entry = animators_.at(handle);
      const auto clip = entry.clip;
      const auto callback = entry.onFrame;
      const float speed = entry.speed;
      animator->Advance(deltaTime, clip.frameCount, clip.framesPerSecond * speed, [&](int frame) {
        if (callback && Get(handle) && animator->IsPlaying() && !pausedGroups_.contains(group)) {
          callback(frame);
        }
      });
    }
  } catch (...) {
    FinishUpdate();
    throw;
  }
  FinishUpdate();
}

/* Libera las instancias sin reiniciar el contador de identificadores. */
void AnimationManager::Clear() {
  if (updating_) {
    clearPending_ = true;
    return;
  }
  animators_.clear();
  pausedGroups_.clear();
}

/* Cuenta los animadores que todavía están registrados. */
std::size_t AnimationManager::Count() const {
  return clearPending_ ? 0 : animators_.size() - pendingRemovals_.size();
}
