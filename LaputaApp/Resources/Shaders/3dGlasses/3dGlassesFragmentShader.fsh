precision mediump float;

uniform int texCount;
uniform vec4 diffuseColor;
uniform vec4 ambientColor;
uniform sampler2D textureImage;
uniform samplerCube envMap;

varying mediump vec2 texCoord0;
varying mediump vec3 normal0;
varying mediump vec3 normalWorld;
varying mediump vec3 lightDirWorld;

void main()
{
    vec4 color;
    vec4 amb;
    float intensity;
    vec3 lightDir = normalize(lightDirWorld);
    vec3 n = normalize(normalWorld);
    float nDotL = dot(n, lightDir);
    
    //calcuate lighting
    intensity = max(nDotL,0.0);
    
    if( texCount == 2 ) {
        color = texture2D(textureImage, texCoord0);
        
        //calculate reflection
        vec3 reflection = (2.0 * normalize(normal0) * nDotL) - lightDir;
        vec4 envColor = textureCube( envMap, reflection);
        //attenuate the src color plus environment color
        gl_FragColor = 0.5 * color + 0.5 * envColor;
    } else if( texCount == 1 ) {
        color = texture2D(textureImage, texCoord0);
        amb = color * 0.33;
        gl_FragColor = (color * intensity) + amb;
    } else {
        color = diffuseColor;
        amb = ambientColor;
        gl_FragColor = (color * intensity) + amb;
    }
}