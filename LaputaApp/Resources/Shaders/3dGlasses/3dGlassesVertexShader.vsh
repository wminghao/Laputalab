// Input vertex data, different for all executions of this shader.
attribute vec3 position;
attribute mediump vec2 texCoord;
attribute vec3 normal;

uniform mat4 MVP;
//uniform mat4 gWorld;

varying mediump vec2 TexCoord0;
//varying mediump vec3 Normal0;
//varying mediump vec3 WorldPos0;

void main(){
    gl_Position = MVP * vec4(position, 1.0);
    TexCoord0   = texCoord;
    //Normal0     = (gWorld * vec4(normal, 0.0)).xyz;
    //WorldPos0   = (gWorld * vec4(position, 1.0)).xyz;
}

