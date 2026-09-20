#include "Core/Overlay.h"

#include <magic_enum/magic_enum.hpp>

#include <utility>

Overlay::Overlay(GuiManager &gui) {
  group_ = tgui::Group::create();
  group_->setVisible(false);

  InitBackground();
  InitButtons();

  gui.Add(group_);
}

std::optional<OverlaySelection> Overlay::FetchSelection() {
  return std::exchange(selection_, std::nullopt);
}

void Overlay::SetVisible(bool visible) {
  group_->setVisible(visible);
}

bool Overlay::IsVisible() const {
  return group_->isVisible();
}

void Overlay::InitBackground() {
  auto background = tgui::Panel::create();
  background->getRenderer()->setBackgroundColor({0, 0, 0, 175});

  group_->add(background);
}

void Overlay::InitButtons() {
  auto layout = tgui::VerticalLayout::create();
  layout->setSize("37%", "63%");
  layout->setPosition("31%", "18%");

  // Un botón por cada valor del enum OverlaySelection:
  // magic_enum genera en compilación una lista de pares
  // cada par contiene valor + nombre, el nombre es el texto tal como se escribió en el enum (como std::string_view)
  for (const auto &[selection, name] : magic_enum::enum_entries<OverlaySelection>()) {
    // Para cada entrada crea un botón con su $name; <<Resume>> <<Restart>> <<Menu>> <<Quit>>
    auto button = tgui::Button::create(name.data());
    button->setTextSize(30);
    // Captura selection POR VALOR: cada botón guarda su propio valor
    button->onPress([this, selection] { this->selection_ = selection; });

    // Apila los botones verticalmente en el layout
    layout->add(button);
  }

  group_->add(layout);
}
