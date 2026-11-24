#pragma once
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <SFML/Graphics/View.hpp>

class GuiManager {
private:
  sf::RenderWindow &window_;
  tgui::Gui gui_;

  sf::FloatRect viewport_ = {{-1.f, -1.f}, {-1.f, -1.f}}; // Última región aplicada (sentinela inválida para forzar la primera)
  sf::FloatRect world_ = {{-1.f, -1.f}, {-1.f, -1.f}};

public:
  GuiManager(sf::RenderWindow &window);

  void Add(const tgui::Widget::Ptr &widget);
  void Remove(const tgui::Widget::Ptr &widget);

private:
  friend class Engine;
  void ProcessEvent(const sf::Event &);
  void Render();

private:
  /** Alinea la GUI con el área de contenido de la escena: mismos viewport y
      coordenadas de diseño que la vista letterbox del render. */
  void UpdateView(const sf::View &view);

  void HandleEvent(const sf::Event::JoystickButtonPressed &);
  void HandleEvent(const sf::Event::JoystickButtonReleased &);
  void HandleEvent(const auto &) {}
};
