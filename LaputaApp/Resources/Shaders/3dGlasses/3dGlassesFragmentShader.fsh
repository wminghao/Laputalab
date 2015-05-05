precision mediump float;

uniform int texCount;
uniform vec4 diffuseColor;
uniform vec4 ambientColor;
uniform sampler2D textureImage;
uniform samplerCube envMap;

varying mediump vec2 texCoordFrag;
varying mediump vec3 normalFrag;
varying mediump vec3 normalWorld;
varying mediump vec3 lightDirWorld;

void main()
{
    vec4 color;
    vec4 amb;
    float brightness;
    vec3 lightDir = normalize(lightDirWorld);
    vec3 n = normalize(normalWorld);
    
    //calcuate lighting brightness
    float nDotL = dot(n, lightDir); //calculate the angle between normal and light
    brightness = max(nDotL,0.0);
    
    if( texCount == 2 ) {
        color = texture2D(textureImage, texCoordFrag);
        
        //calculate reflection, based on the untransformed normal.
        float normalDotL = dot(normalFrag, lightDir);
        vec3 reflection = (2.0 * normalize(normalFrag) * normalDotL) - lightDir;
        vec4 envColor = textureCube( envMap, reflection);
        
        //attenuate the src color plus environment color
        gl_FragColor = 0.2 * (color * brightness) + envColor;
    } else if( texCount == 1 ) {
        color = texture2D(textureImage, texCoordFrag);
        amb = color * 0.33;
        gl_FragColor = vec4(color.rgb*brightness, color.a) + amb;
    } else {
        color = diffuseColor;
        amb = ambientColor;
        gl_FragColor = (color * brightness) + amb;
    }
}