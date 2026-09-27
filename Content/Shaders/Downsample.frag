#version 120

// Bloom paso 1: promedia central + 8 vecinos y baja la resolución.

uniform sampler2D sourceTexture;
// Tamaño de un texel, para muestrear "un píxel" por textura.
uniform vec2 texelSize;

void main() {
  // 4 direcciones (con sus opuestas cubren los 8 vecinos).
  const vec2 offsets[4] = vec2[4](
      vec2(1.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 1.0), vec2(1.0, -1.0));

  vec2 uv = gl_TexCoord[0].xy;
  // Central + 8 vecinos = 9 muestras.
  vec4 color = texture2D(sourceTexture, uv);

  for (int i = 0; i < 4; i++) {
    vec2 offset = texelSize * offsets[i];
    color += texture2D(sourceTexture, uv + offset);
    color += texture2D(sourceTexture, uv - offset);
  }

  // Promedio de las 9 muestras.
  gl_FragColor = color / 9.0;
}
