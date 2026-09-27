#version 120

// Bloom pasos 2 y 3: blur gaussiano separable.
// Se usa 2 veces: vertical (texelSize=(0,1/alto)) y horizontal (1/ancho,0).

uniform sampler2D sourceTexture;
// Dirección y paso: texelSize * i desplaza i texels en esa dirección.
uniform vec2 texelSize;

void main() {
  // Campana gaussiana, simétrica y sumando ~1.0 (no cambia el brillo).
  const float weights[5] = float[5](
      0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

  vec2 uv = gl_TexCoord[0].xy;
  // Píxel central, con el mayor peso.
  vec4 color = texture2D(sourceTexture, uv) * weights[0];

  // Muestras a +i y -i texels por cada anillo.
  for (int i = 1; i < 5; i++) {
    vec2 offset = texelSize * float(i);
    color += texture2D(sourceTexture, uv + offset) * weights[i];
    color += texture2D(sourceTexture, uv - offset) * weights[i];
  }

  gl_FragColor = color;
}
