#version 120
                                                                                    
in vec2 TexCoordOut;                                                                
uniform sampler2D gShadowMap;

void main()                                                                         
{                                                                                   
    float Depth = texture(gShadowMap, TexCoordOut).x;                               
    Depth = 1.0 - (1.0 - Depth) * 25.0;                                             
    gl_FragColor = vec4(Depth);
}
