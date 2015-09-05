#version 120

attribute vec3 position;
attribute vec2 texCoord;
attribute vec3 normal;

uniform mat4 MVP;
uniform mat4 World;
uniform mat3 NormalMatrix;

invariant varying vec2 texCoordFrag;
invariant varying vec3 normalWorld;
invariant varying vec3 surfacePosWorld;
invariant varying vec3 surfaceToLightWorld;
invariant varying vec3 surfaceToCameraWorld;

//tangent for normal mapping
attribute vec3 binormal;
attribute vec3 tangent;
invariant varying vec3 surfaceToLightTangent;
invariant varying vec3 surfaceToCameraTangent;

invariant gl_Position;

void main(){
    gl_Position = MVP * vec4(position, 1.0);
    texCoordFrag   = texCoord;

    normalWorld = normalize(NormalMatrix * normal);

    surfacePosWorld = vec3(World * vec4(position, 1));
    
    //It's point light. color=white, different light for different angle.
    //calculate the vector from the light source & camera source to the vertex position.
    // object is initially located at z = -600 position
    vec3 cameraPositionWorld = vec3(0, 0, 0.01);
    vec3 lightPositionWorld = vec3(10, 10, 0);
    surfaceToLightWorld = lightPositionWorld - surfacePosWorld;
    surfaceToCameraWorld = cameraPositionWorld - surfacePosWorld;
    
    //tangent space
    mat3 tangentMat = mat3( tangent,
                            binormal,
                            normal);
    surfaceToLightTangent = surfaceToLightWorld * tangentMat;
    surfaceToCameraTangent = surfaceToCameraWorld * tangentMat;
}