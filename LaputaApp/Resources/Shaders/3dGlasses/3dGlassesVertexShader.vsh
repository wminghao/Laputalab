// Input vertex data, different for all executions of this shader.
attribute vec3 position;
attribute mediump vec2 texCoord;
attribute vec3 normal;

uniform mat4 MVP;
uniform mat4 World;
uniform mat4 ViewInverse;
uniform mat3 NormalMatrix;

varying mediump vec2 texCoord0;
varying mediump vec3 normal0;
varying mediump vec3 normalWorld;
varying mediump vec3 lightDirWorld;

void main(){
    gl_Position = MVP * vec4(position, 1.0);
    texCoord0   = texCoord;
    normal0     = normal;
    normalWorld = vec3(World * vec4(normal, 0.0));//NormalMatrix * normal;
    
    //calculate the vector from the light source to the position.
    //light position is in camera position (3,3,3)
    lightDirWorld   = vec3(ViewInverse * vec4(vec3(3, 3, 3), 1.0)) - vec3(World * vec4(position, 1));
}