precision mediump float;

varying mediump vec2 coordinate;
uniform sampler2D videoframe;

void main()
{
    gl_FragColor = texture2D(videoframe, coordinate);
}