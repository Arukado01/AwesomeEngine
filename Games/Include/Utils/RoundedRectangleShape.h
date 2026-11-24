#pragma once

#include <SFML/Graphics/Shape.hpp>
#include <algorithm>
#include <cmath>

/* Rectángulo con esquinas redondeadas: un sf::Shape cuyos puntos se generan
   recorriendo los cuatro arcos de los rincones (sentido horario). */
class RoundedRectangleShape : public sf::Shape {
public:
  RoundedRectangleShape() = default;

  explicit RoundedRectangleShape(sf::Vector2f size, float radius = 0.f,
                                 std::size_t cornerPointCount = 12)
      : size_(size), radius_(radius),
        cornerPointCount_(std::max<std::size_t>(cornerPointCount, 2)) {
    UpdateGeometry();
  }

  void setSize(sf::Vector2f size) {
    size_ = size;
    UpdateGeometry();
  }

  sf::Vector2f getSize() const { return size_; }

  void setRadius(float radius) {
    radius_ = radius;
    UpdateGeometry();
  }

  float getRadius() const { return radius_; }

  std::size_t getPointCount() const override {
    return cornerPointCount_ * 4; // 4 rincones, puntos por arco
  }

  sf::Vector2f getPoint(std::size_t index) const override {
    static const float pi = 3.14159265358979f;

    const std::size_t corner = index / cornerPointCount_;
    // Reparte el arco del rincón: 90° repartidos entre sus puntos.
    const float angle =
        (pi / 2.f) * (corner + (float)(index % cornerPointCount_) / (cornerPointCount_ - 1)) -
        pi / 2.f;

    sf::Vector2f center;
    switch (corner) {
    case 0: center = {size_.x - radius_, radius_}; break;           // arriba derecha
    case 1: center = {size_.x - radius_, size_.y - radius_}; break; // abajo derecha
    case 2: center = {radius_, size_.y - radius_}; break;           // abajo izquierda
    case 3: center = {radius_, radius_}; break;                     // arriba izquierda
    }

    return center + radius_ * sf::Vector2f(std::cos(angle), std::sin(angle));
  }

private:
  /* El radio nunca supera la mitad del lado corto: si excede, se recorta. */
  void UpdateGeometry() {
    radius_ = std::clamp(radius_, 0.f, std::min(size_.x, size_.y) / 2.f);
    sf::Shape::update();
  }

  sf::Vector2f size_;
  float radius_ = 0;
  std::size_t cornerPointCount_ = 12;
};
