precision mediump float;

uniform int texCount;
uniform vec4 diffuseColor;
uniform vec4 ambientColor;
uniform sampler2D textureImage;

varying vec2 texCoord0;
varying mediump vec3 normal0;

void main()
{
    vec4 color;
    vec4 amb;
    float intensity;
    vec3 lightDir;
    vec3 n;
    
    lightDir = normalize(vec3(0,0,10)); //light direction is the same as viewing model direction
    n = normalize(normal0);
    intensity = max(dot(lightDir,n),0.0);
    
    if( texCount == 1 ) {
        color = texture2D(textureImage, texCoord0);
        amb = color * 0.33;
    } else {
        color = diffuseColor;
        amb = ambientColor;
    }
    gl_FragColor = (color * intensity) + amb;
}