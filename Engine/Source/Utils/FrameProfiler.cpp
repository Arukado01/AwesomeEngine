#include "Utils/FrameProfiler.h"

#include <cstdio>
#include <cstdlib>

// Constructor: abre el CSV solo si AE_FRAME_LOG trae una ruta; si no, queda apagado.
FrameProfiler::FrameProfiler() {
  const char *path = std::getenv("AE_FRAME_LOG"); // Ruta del CSV
  if (!path || !*path) {
    return; // Sin variable o vacía: profiler desactivado
  }

  // trunc: cada ejecución empieza un archivo nuevo, sin mezclar mediciones
  out_.open(path, std::ios::out | std::ios::trunc);
  if (!out_) {
    std::fprintf(stderr, "FrameProfiler: no se pudo abrir %s\n", path);
    return; // Si falla la apertura, se comporta como desactivado
  }

  start_ = Clock::now(); // Referencia para tMs (tiempo desde el arranque)
  previous_ = start_; // Referencia para ms (intervalo entre frames)
  out_ << "frame,tMs,ms\n"; // Cabecera del CSV
}

// Destructor: vuelca el buffer para no perder las últimas filas (el archivo se cierra solo).
FrameProfiler::~FrameProfiler() {
  if (out_) {
    out_.flush();
  }
}

// Registra un frame (tras Render y solo con foco): escribe frame, tMs y ms al CSV.
void FrameProfiler::Frame() {
  if (!out_) {
    return;
  }

  // steady_clock es monotónico: no salta si cambia la hora del sistema
  const auto now = Clock::now();

  // tMs: tiempo absoluto desde el arranque; permite descartar el calentamiento.
  const double tMs =
      std::chrono::duration<double, std::milli>(now - start_).count();

  // ms: tiempo desde el frame anterior; es la métrica real de rendimiento.
  const double ms =
      std::chrono::duration<double, std::milli>(now - previous_).count();

  previous_ = now; // El siguiente frame se mide desde este instante
  out_ << frame_++ << ',' << tMs << ',' << ms << '\n'; // Una fila por frame
}
