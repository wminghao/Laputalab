// Input vertex data, different for all executions of this shader.
attribute vec3 position;
attribute mediump vec2 texCoord;
attribute vec3 normal;

uniform mat4 MVP;
uniform mat4 World;
uniform mat4 ViewInverse;
uniform mat3 NormalMatrix;

varying mediump vec2 texCoordFrag;
varying mediump vec3 normalFrag;
varying mediump vec3 normalWorld;
varying mediump vec3 lightDirWorld;

void main(){
    gl_Position = MVP * vec4(position, 1.0);
    texCoordFrag   = texCoord;
    normalFrag     = normal;
    normalWorld = vec3(World * vec4(normal, 1.0));//NormalMatrix * normal;
    
    //It's point light. different light for different angle.
    //calculate the vector from the light source to the vertex position.
    //light position is in camera position (3,3,3)
    lightDirWorld   = vec3(ViewInverse * vec4(vec3(3, 3, 3), 1.0)) - vec3(World * vec4(position, 1));
}