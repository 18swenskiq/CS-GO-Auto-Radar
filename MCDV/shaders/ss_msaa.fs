#version 330 core
uniform sampler2D sampler0;

in vec2 TexCoords;
out vec4 FragColor;

void main(){
   FragColor = texture(sampler0, TexCoords);
}