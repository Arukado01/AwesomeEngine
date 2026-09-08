#pragma once
#include "Utils/InputBindings.h"
#include <unordered_map>

class InputManager {
private:
  std::unordered_multimap<int, Input::Binding> bindings_;

public:
  void Bind(int action, const Input::Binding &binding);
  bool Pressed(int action) const;
  void Clear();

private:
  bool Pressed(const Input::Keyboard &) const;
  bool Pressed(const Input::Mouse &) const;
  bool Pressed(const Input::Gamepad &) const;
  bool Pressed(const Input::Axis &) const;
};
