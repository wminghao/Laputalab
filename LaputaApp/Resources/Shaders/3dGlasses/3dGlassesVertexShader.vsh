// Input vertex data, different for all executions of this shader.
attribute vec3 position;
attribute mediump vec2 texCoord;
attribute vec3 normal;

uniform mat4 MVP;
uniform mat4 World;
uniform mat4 ViewInverse;

varying mediump vec2 texCoord0;
varying mediump vec3 normal0;
varying mediump vec3 lightDir0;

void main(){
    gl_Position = MVP * vec4(position, 1.0);
    texCoord0   = texCoord;
    normal0     = (World * vec4(normal, 0.0)).xyz;
    lightDir0   = (ViewInverse * vec4(0, 0, 10, 1)).xyz; //light direction is the same as viewing model direction
}

