#version 120

// Vertex shader trivial compartido por todos los efectos.
// Pasa posición, UV y color por los builtins del pipeline fijo.
void main() {
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
  gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
  gl_FrontColor = gl_Color;
}
