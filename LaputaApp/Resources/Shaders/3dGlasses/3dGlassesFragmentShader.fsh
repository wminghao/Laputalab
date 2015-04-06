precision mediump float;

uniform int texCount;
uniform vec4 diffuseColor;
uniform sampler2D textureImage;

varying vec2 TexCoord0;

void main()
{
    if( texCount == 1 ) {
        gl_FragColor = texture2D(textureImage, TexCoord0.xy);
    } else {
        gl_FragColor = diffuseColor;
    }
}