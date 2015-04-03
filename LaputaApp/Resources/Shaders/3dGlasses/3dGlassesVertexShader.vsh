// Input vertex data, different for all executions of this shader.
attribute vec3 vertexPosition_modelspace;
uniform mat4 MVP;

attribute mediump vec4 texturecoordinate;
varying mediump vec2 coordinate;

void main(){
    gl_Position = MVP * vec4(vertexPosition_modelspace, 1.0);
    coordinate = texturecoordinate.xy;
}

