#version 330 core
in vec3 vNormal;
out vec4 FragColor;

void main(){
  float l = max(dot(normalize(vNormal), normalize(vec3(0.3,0.6,0.7))), 0.15);
  FragColor = vec4(vec3(0.35, 0.65, 0.95) * l, 1.0);
}
