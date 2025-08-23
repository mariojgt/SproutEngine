#version 330 core
in vec3 vNormal;
out vec4 FragColor;
uniform vec3 uTint;

void main(){
  float l = max(dot(normalize(vNormal), normalize(vec3(0.3,0.6,0.7))), 0.15);
  vec3 base = vec3(0.35, 0.65, 0.95) * l;
  FragColor = vec4(base * uTint, 1.0);
}
