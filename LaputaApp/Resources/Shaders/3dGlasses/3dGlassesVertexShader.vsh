// Input vertex data, different for all executions of this shader.
attribute vec3 position;
attribute mediump vec2 texCoord;
attribute vec3 normal;

uniform mat4 MVP;
uniform mat4 World;
uniform mat4 ViewInverse;

varying mediump vec2 texCoord0;
varying mediump vec3 normal0;
varying mediump vec3 normalWorld;
varying mediump vec3 lightDirWorld;

void main(){
    gl_Position = MVP * vec4(position, 1.0);
    texCoord0   = texCoord;
    normal0     = normal;
    normalWorld = (World * vec4(normal, 0.0)).xyz;
    lightDirWorld   = (ViewInverse * vec4(vec3(0, 0, 10), 1.0)).xyz - position.xyz; //light position is the same as viewing model position
}