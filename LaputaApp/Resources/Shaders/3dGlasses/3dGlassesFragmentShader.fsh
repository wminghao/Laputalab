precision mediump float;

uniform int texCount;
uniform vec4 diffuseColor;
uniform sampler2D textureImage;

varying vec2 texCoord0;

void main()
{
    if( texCount == 1 ) {
        gl_FragColor = texture2D(textureImage, texCoord0);
    } else {
        gl_FragColor = diffuseColor;
    }
}