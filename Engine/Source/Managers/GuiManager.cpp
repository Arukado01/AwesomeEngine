#include "Managers/GuiManager.h"

#include "Utils/InputBindings.h"

GuiManager::GuiManager(sf::RenderWindow &window)
    : window_(window), gui_(window) {
  tgui::Theme::setDefault("Content/Themes/Dark.txt");

  gui_.setTabKeyUsageEnabled(true);
  gui_.setFont("Content/Fonts/Relidux.otf");
}

void GuiManager::ProcessEvent(const sf::Event &event) {
  event.visit([this](const auto &type) {
    this->HandleEvent(type);
    this->gui_.handleEvent(type);
  });
}

void GuiManager::Render() { gui_.draw(); }

void GuiManager::Add(const tgui::Widget::Ptr &widget) { gui_.add(widget); }

void GuiManager::Remove(const tgui::Widget::Ptr &widget) {
  gui_.remove(widget);
}

/* La GUI comparte la geometría de la escena: dibuja y recibe el ratón dentro
   del área de contenido, con las coordenadas de diseño (0,0,windowSize). Así
   los widgets guardan sus proporciones sin importar el tamaño de la ventana. */
void GuiManager::UpdateView(const sf::View &view) {
  const sf::FloatRect viewport = view.getViewport();
  const sf::FloatRect world(view.getCenter() - view.getSize() / 2.f,
                            view.getSize());

  if (viewport == viewport_ && world == world_) {
    return; // Sin cambios: evita recalcular el layout cada frame
  }
  viewport_ = viewport;
  world_ = world;

  gui_.setRelativeViewport({viewport.position.x, viewport.position.y,
                            viewport.size.x, viewport.size.y}); // Región del área de contenido en la ventana
  gui_.setAbsoluteView({world.position.x, world.position.y,
                        world.size.x, world.size.y}); // Coordenadas de diseño dentro de esa región
}

void GuiManager::HandleEvent(const sf::Event::JoystickButtonPressed &joystick) {
  if (Input::HardwareToLogical(joystick.button, joystick.joystickId) ==
      GamepadButton::South) {
    gui_.handleEvent(
        sf::Event::MouseButtonPressed{{}, sf::Mouse::getPosition(window_)});
  }
}

void GuiManager::HandleEvent(
    const sf::Event::JoystickButtonReleased &joystick) {
  if (Input::HardwareToLogical(joystick.button, joystick.joystickId) ==
      GamepadButton::South) {
    gui_.handleEvent(
        sf::Event::MouseButtonReleased{{}, sf::Mouse::getPosition(window_)});
  }
}
