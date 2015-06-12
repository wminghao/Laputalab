#version 120

attribute vec3 position;
attribute vec2 texCoord;
attribute vec3 normal;

uniform mat4 MVP;
uniform mat4 World;
uniform mat4 ViewInverse;
uniform mat3 NormalMatrix;

varying vec2 texCoordFrag;
varying vec3 normalWorld;
varying vec3 lightDirWorld;

void main(){
    gl_Position = MVP * vec4(position, 1.0);
    texCoordFrag   = texCoord;

    //mat3 normalMat = transpose(inverse(mat3(World)));//remove translation and scaling
    //normalWorld = normalize(normalMat * normal);
    //TODO
    //normalWorld = normalize(NormalMatrix * normal);
    normalWorld = normalize(normal);

    //It's point light. different light for different angle.
    //calculate the vector from the light source to the vertex position.
    //light position is in camera position, 0, 0, 0
    lightDirWorld = normalize(vec3(0, 0, 0) - vec3(World * vec4(position, 1))); //no attenuation
}