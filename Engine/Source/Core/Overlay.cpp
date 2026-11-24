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

/* Fondo oscurecido + tarjeta central con borde, al estilo del panel del menú principal. */
void Overlay::InitBackground() {
  auto background = tgui::Panel::create();
  background->getRenderer()->setBackgroundColor({0, 0, 0, 200});

  auto card = tgui::Panel::create();
  card->getRenderer()->setBackgroundColor({18, 23, 37, 235}); // fondo azul muy oscuro
  card->getRenderer()->setBorderColor({150, 150, 225});       // borde lavanda del panel del menú
  card->getRenderer()->setBorders(2);
  card->setSize("40%", "64%");
  card->setPosition("30%", "18%");

  // Título de la tarjeta, arriba de los botones.
  auto title = tgui::Label::create("PAUSA");
  title->setTextSize(34);
  title->getRenderer()->setTextColor({235, 240, 250});
  title->setHorizontalAlignment(tgui::HorizontalAlignment::Center);
  title->setPosition("30%", "22%");
  title->setSize("40%", "8%");

  group_->add(background);
  group_->add(card);
  group_->add(title);
}

/* Un botón por cada valor del enum OverlaySelection, colocados a mano dentro de la tarjeta. */
void Overlay::InitButtons() {
  // magic_enum genera en compilación una lista de pares
  // cada par contiene valor + nombre, el nombre es el texto tal como se escribió en el enum (como std::string_view)
  int index = 0;

  for (const auto &[selection, name] : magic_enum::enum_entries<OverlaySelection>()) {
    // Para cada entrada crea un botón con su $name; <<Resume>> <<Restart>> <<Menu>> <<Quit>>
    auto button = tgui::Button::create(name.data());
    button->setTextSize(30);

    auto renderer = button->getRenderer();
    renderer->setBackgroundColor({28, 34, 66});       // reposo: azul oscuro
    renderer->setBackgroundColorHover({40, 48, 90});  // resaltado al pasar el cursor
    renderer->setBorderColor({150, 150, 225, 90});    // borde lavanda tenue
    renderer->setBorders(1);
    renderer->setTextColor({235, 240, 250});
    renderer->setTextColorHover(tgui::Color::Cyan);   // texto cian bajo el cursor

    // Reparte los 4 botones en la franja de la tarjeta bajo el título.
    const int row = index++;
    button->setPosition("33%", tgui::Layout(std::to_string(33 + row * 12) + "%"));
    button->setSize("34%", "10%");

    // Captura selection POR VALOR: cada botón guarda su propio valor
    button->onPress([this, selection] { this->selection_ = selection; });

    group_->add(button);
  }
}
