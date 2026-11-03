#include "Utils/SweptAabb.h"

#include <algorithm>
#include <limits>

namespace Collision {
  /* Calcula el primer contacto usando el desplazamiento relativo de ambas cajas durante un frame. */
  std::optional<SweepHit> SweepAabb(const sf::FloatRect &previousA, const sf::FloatRect &currentA,
                                    const sf::FloatRect &previousB, const sf::FloatRect &currentB) {
    // Si ya se solapaban, no hay una cara de entrada que permita determinar la normal.
    if (previousA.findIntersection(previousB)) {
      return SweepHit{0, {}, true}; // El contacto ya existe al comenzar: tiempo cero, normal vacía y solapamiento inicial.
    }

    // Restar los desplazamientos equivale a mover A mientras B permanece quieta.
    const sf::Vector2f movement = (currentA.position - previousA.position) - (currentB.position - previousB.position);
    const float infinity = std::numeric_limits<float>::infinity(); // Sirve para representar un intervalo sin límite en un eje quieto.
    float entryX = -infinity, exitX = infinity, entryY = -infinity, exitY = infinity; // Guarda los instantes de entrada y salida de cada eje.

    // Calcula durante qué parte del frame se superponen los intervalos de un eje.
    const auto sweepAxis = [](float aMin, float aSize, float bMin, float bSize, float delta, float &entry, float &exit) {
      if (delta == 0) {
        // Sin movimiento en este eje, las cajas deben estar superpuestas desde el inicio.
        return aMin < bMin + bSize && aMin + aSize > bMin; // Si lo están, el intervalo sigue abierto entre menos y más infinito.
      }
      // Distancia entre caras dividida por movimiento: fracciones de entrada y salida en este eje.
      const float first = (bMin - aMin - aSize) / delta; // Tiempo en que el extremo máximo de A alcanza el mínimo de B.
      const float last = (bMin + bSize - aMin) / delta; // Tiempo en que el extremo mínimo de A alcanza el máximo de B.
      entry = std::min(first, last); // Ordena los tiempos porque un desplazamiento negativo invierte las caras.
      exit = std::max(first, last); // El mayor es el momento en que las cajas dejan de compartir este eje.
      return true; // En un eje con movimiento se pudieron calcular ambos tiempos.
    };

    // Calcula ambos ejes; si uno está quieto y separado, las cajas nunca llegan a tocarse.
    if (!sweepAxis(previousA.position.x, previousA.size.x, previousB.position.x, previousB.size.x, movement.x, entryX, exitX) ||
        !sweepAxis(previousA.position.y, previousA.size.y, previousB.position.y, previousB.size.y, movement.y, entryY, exitY)) {
      return std::nullopt; // Sin coincidencia en ambos ejes no hay colisión en dos dimensiones.
    }
    // El contacto empieza cuando ambos ejes coinciden y termina cuando uno deja de coincidir.
    const float entry = std::max(entryX, entryY); // Espera a que entre el último eje que faltaba.
    const float exit = std::min(exitX, exitY); // Termina en cuanto cualquiera de los ejes se separa.
    // Se descartan entradas fuera del frame, intervalos vacíos y contactos que ya terminaron.
    if (entry < 0 || entry > 1 || entry >= exit || exit <= 0) {
      return std::nullopt; // El intervalo útil debe cruzar el frame y tener una duración positiva.
    }

    // La normal apunta desde B hacia A; en una esquina se prioriza el eje vertical.
    const sf::Vector2f normal = entryY >= entryX ? sf::Vector2f{0, movement.y > 0 ? -1.f : 1.f} // La última entrada fue vertical.
                                                 : sf::Vector2f{movement.x > 0 ? -1.f : 1.f, 0}; // La última entrada fue horizontal.
    return SweepHit{entry, normal, false}; // Devuelve el primer instante de contacto y confirma que no había solapamiento previo.
  }
}
