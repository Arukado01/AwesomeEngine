#include "Managers/InputManager.h"
#include "SFML/Window/Joystick.hpp"
#include "Utils/InputBindings.h"
#include <algorithm>

void InputManager::Bind(int action, const Input::Binding &binding) {
  bindings_.emplace(action, binding);
}

/**Busca todos los bindings de esta acción. Luego revisa cada binding.
 * Dependiendo del tipo de entrada que sea, comprueba si esa entrada está
 * presionada. Si al menos una está presionada, devuelve true **/

//  Pressed(action):

//     bindings = todos los bindings de action <Keyboard, Mouse, Gamepad, Axis>

//     para cada binding:
//         si la entrada está presionada:
//             return true

//     return false
bool InputManager::Pressed(int action) const {
  const auto [first, last] = bindings_.equal_range(action);

  return std::any_of(first, last, [this](const auto &entry) {
    return std::visit(
        [this](const auto &binding) { return this->Pressed(binding); },
        entry.second);
  });
}

void InputManager::Clear() { bindings_.clear(); }

bool InputManager::Pressed(const Input::Keyboard &binding) const {
  return sf::Keyboard::isKeyPressed(binding.key);
}

bool InputManager::Pressed(const Input::Mouse &binding) const {
  return sf::Mouse::isButtonPressed(binding.button);
}

bool InputManager::Pressed(const Input::Gamepad &binding) const {
  const auto button = Input::LogicalToHardware(binding.button, binding.id);
  return button ? sf::Joystick::isButtonPressed(binding.id, *button) : false;
}

/** Se lee el valor del joysticj axis y se divide entre 100 para normalizarlo
 * entre -1 y 1 **/
bool InputManager::Pressed(const Input::Axis &binding) const {
  const float axis =
      sf::Joystick::getAxisPosition(binding.id, binding.axis) / 100;
  return (binding.threshold >= 0) ? axis > binding.threshold
                                  : axis < binding.threshold;
}
