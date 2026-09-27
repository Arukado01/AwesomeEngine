#version 120

// Bloom paso 4: suma el brillo difuminado sobre la imagen original.

uniform sampler2D sourceTexture;
// Ping-pong de mitad de tamaño, ya desenfocado y re-estirado.
uniform sampler2D bloomTexture;

// Intensidad con la que se suma el bloom.
const float BLOOM_STRENGTH = 0.70;

void main() {
  vec2 uv = gl_TexCoord[0].xy;

  vec4 sourceColor = texture2D(sourceTexture, uv);
  vec4 bloomColor = texture2D(bloomTexture, uv);

  // Original + brillo escalado = halo del bloom.
  gl_FragColor = sourceColor + bloomColor * BLOOM_STRENGTH;
}
