// Input vertex data, different for all executions of this shader.
attribute vec3 Position;
attribute mediump vec2 TexCoord;
attribute vec3 Normal;

uniform mat4 MVP;
//TODO should we export 2 matrixes as in example???
//uniform mat4 gWorld;

varying mediump vec2 TexCoord0;
//varying mediump vec3 Normal0;
//varying mediump vec3 WorldPos0;

void main(){
    gl_Position = MVP * vec4(Position, 1.0);
    TexCoord0   = TexCoord;
    //Normal0     = (gWorld * vec4(Normal, 0.0)).xyz;
    //WorldPos0   = (gWorld * vec4(Position, 1.0)).xyz;
}

