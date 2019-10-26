#version 330 core

out vec4 out_colour;
in vec2 blurTexCoords[11];

uniform sampler2D originalTexture;
uniform float weight[6] = float[] (0.0093, 0.028002, 0.065984, 0.121703, 0.175713, 0.198596);

void main(void) {
  out_colour = vec4(0.0);
  for(int i = 0; i <= 10; i++) {
    out_colour += texture(originalTexture, blurTexCoords[i]) * weight[i % 6];
  }
}