// Input vertex data, different for all executions of this shader.
attribute vec3 position;
attribute mediump vec2 texCoord;
attribute vec3 normal;

uniform mat4 MVP;
//uniform mat4 gWorld;

varying mediump vec2 texCoord0;
//varying mediump vec3 normal0;
//varying mediump vec3 worldPos0;

void main(){
    gl_Position = MVP * vec4(position, 1.0);
    texCoord0   = texCoord;
    //normal0     = (gWorld * vec4(normal, 0.0)).xyz;
    //worldPos0   = (gWorld * vec4(position, 1.0)).xyz;
}

