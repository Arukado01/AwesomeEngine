#pragma once

#include <SFML/System/String.hpp>

class ClipboardManager {
public:
  void SetString(const sf::String &text);
  sf::String GetString() const;
};
