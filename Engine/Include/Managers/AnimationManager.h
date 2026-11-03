#pragma once

#include "Graphics/SpriteAnimator/SpriteAnimator.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <unordered_set>

using AnimationHandle = std::uint64_t;
using AnimationGroup = std::uint64_t;
inline constexpr AnimationHandle INVALID_ANIMATION = 0;

class AnimationManager {
private:
  struct Entry {
    SpriteAnimator animator;
    AnimationGroup group = 0;
    SpriteAnimationClip clip{0, 0, 0};
    std::function<void(int)> onFrame;
    float speed = 1;
    bool resumeAfterGroup = false;
  };

  std::unordered_map<AnimationHandle, Entry> animators_;
  std::unordered_set<AnimationGroup> pausedGroups_;
  std::unordered_set<AnimationHandle> pendingRemovals_;
  AnimationHandle nextHandle_ = 1;
  bool updating_ = false;
  bool clearPending_ = false;

  void FinishUpdate();

public:
  /* Crea un animador independiente y devuelve su identificador. */
  AnimationHandle Create(AnimationGroup group = 0);

  /* Busca un animador; devuelve nullptr si su identificador ya no está activo. */
  SpriteAnimator *Get(AnimationHandle handle);
  const SpriteAnimator *Get(AnimationHandle handle) const;

  /* Elimina un animador y devuelve si existía. */
  bool Remove(AnimationHandle handle);

  /* Configura un clip y su callback; no se permite reconfigurar durante AdvanceGroup. */
  /* El callback recibe cuadros cruzados; el cuadro inicial no emite evento hasta cerrar el ciclo. */
  bool Configure(AnimationHandle handle, SpriteAnimationClip clip, SpritePlaybackMode mode = SpritePlaybackMode::Loop,
                 std::function<void(int)> onFrame = {});

  /* Cambia la velocidad de una instancia sin alterar la velocidad base del clip. */
  bool SetSpeed(AnimationHandle handle, float factor);

  /* Pausa o reanuda una instancia; usa este método para respetar también la pausa del grupo. */
  bool SetPaused(AnimationHandle handle, bool paused);

  /* Pausa o reanuda el grupo sin activar instancias que ya estaban pausadas. */
  void SetGroupPaused(AnimationGroup group, bool paused);

  /* La escena llama una vez por frame para avanzar los clips del grupo y entregar sus eventos. */
  void AdvanceGroup(AnimationGroup group, float deltaTime);

  /* Elimina todos los animadores sin reutilizar sus identificadores anteriores. */
  void Clear();

  /* Devuelve la cantidad de animadores activos. */
  std::size_t Count() const;
};
