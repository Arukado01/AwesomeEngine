#pragma once

#include "Config/MenuConfig.h"

namespace Menu {
  struct Button {
    sf::RectangleShape shape;
    std::string name;
    sf::Text text{GetDefaultFont()};
  };

  struct Controls {
    sf::RectangleShape backdrop; // oscurece el menú detrás del panel
    sf::RectangleShape panel;    // tarjeta central con borde
    sf::Text title{GetDefaultFont()};
    sf::Text body{GetDefaultFont()};
    sf::Text hint{GetDefaultFont()};
    std::optional<int> current;
  };
} // namespace Menu
