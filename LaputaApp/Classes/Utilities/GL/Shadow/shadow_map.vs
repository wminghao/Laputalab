#version 120
                                                                                    
in vec3 Position;
in vec2 TexCoord;
in vec3 Normal;                                               
                                                                                    
uniform mat4 gWVP;                                                                  
                                                                                    
out vec2 TexCoordOut;                                                               
                                                                                    
void main()                                                                         
{                                                                                   
    gl_Position = gWVP * vec4(Position, 1.0);                                       
    TexCoordOut = TexCoord;                                                         
}
