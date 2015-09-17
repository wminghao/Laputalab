#version 120

uniform int texCount;
uniform vec4 diffuseColor;
uniform vec4 ambientColor;
uniform vec4 specularColor;
uniform sampler2D textureImage;
uniform samplerCube envMap;

invariant varying vec2 texCoordFrag;
invariant varying vec3 normalWorld;
invariant varying vec3 surfacePosWorld;
invariant varying vec3 surfaceToLightWorld;
invariant varying vec3 surfaceToCameraWorld;

//tangent map
uniform sampler2D bumpImage;
invariant varying vec3 surfaceToLightTangent;
invariant varying vec3 surfaceToCameraTangent;

void main()
{
    vec3 diffuse;
    vec3 amb;
    vec3 spec;
    vec4 surfaceColor;
    
    //normalize the vector and calculate the distance
    vec3 surfaceToLightWorldNormalized = normalize( surfaceToLightWorld );
    vec3 surfaceToCameraWorldNormalized = normalize( surfaceToCameraWorld );
    float distanceToLight = length( surfaceToLightWorldNormalized );
    
    //calcuate lighting diffuseCoefficent(brightness)
    float normalDotL = dot(normalWorld, surfaceToLightWorldNormalized);
    float diffuseCoefficent = max(normalDotL, 0.0);
    
    //calculate the specularCoefficient, shininess is alway 0, means max shininess = 1.
    float specularCoefficent = 0.0;
    if( diffuseCoefficent > 0 ) {
        specularCoefficent = pow(max(0.0, dot(surfaceToCameraWorldNormalized, reflect(-surfaceToLightWorldNormalized, normalWorld))), 1);
    }
    
    //light ambient coefficient
    float lightAmbientCoefficient = 0.33;
    
    //calculate attenuation
    float lightAttenuationFactor = 0.0000025; //almost no effect,don't know why?
    float attenuation = 1.0/(1.0 + lightAttenuationFactor * pow(distanceToLight, 2));
    
    //ratio of environment vs. surface.
    float surfaceColorToEnvRatio = 0.7;
    
    //gamma correction, does not seem to work, maybe unecessary?
    vec3 gamma = vec3(1.0/2.2);
    vec3 linearColor;
    
    if( texCount == 2 ) {
        //environment mapping color
        surfaceColor = texture2D(textureImage, texCoordFrag);
        diffuse = diffuseCoefficent * surfaceColor.rgb;
        spec = specularCoefficent * specularColor.rgb;
        
        //calculate reflection, based on the untransformed normal.
        vec3 reflection = (2.0 * normalize(normalWorld) * normalDotL) - surfaceToLightWorldNormalized;
        vec4 envColor = textureCube( envMap, reflection);
        
        //attenuate the src color plus environment color
        gl_FragColor = surfaceColorToEnvRatio * vec4(attenuation * (diffuse+spec), surfaceColor.a) + (1-surfaceColorToEnvRatio) * vec4(diffuseCoefficent * envColor.rgb, envColor.a);
    } else if( texCount == 1 ) {
        //texture color
        surfaceColor = texture2D(textureImage, texCoordFrag);
        diffuse = diffuseCoefficent * surfaceColor.rgb;
        amb =  lightAmbientCoefficient * surfaceColor.rgb; //light's ambient coefficient=0.33
        spec = specularCoefficent * specularColor.rgb;
        
        linearColor = attenuation * (diffuse+spec) + amb;
        gl_FragColor = vec4(linearColor, surfaceColor.a);
        //gl_FragColor = vec4(pow(linearColor, gamma), surfaceColor.a);
    } else if( texCount == 3 ) {
        //candide3 simple mask, do not display
        gl_FragColor = texture2D(textureImage, texCoordFrag);
        //outFrag = vec4(0, 1, 0, 1); //green
    } else if( texCount == 4 ) {
        //per fragment normal, don't use normal
        surfaceColor = texture2D( textureImage, texCoordFrag );
        
        ////////////////////////////////////////
        //convert from height map to normal map
        ////////////////////////////////////////
        /*
         http://stackoverflow.com/questions/5281261/generating-a-normal-map-from-a-height-map
         #version 130
         uniform sampler2D unit_wave
         noperspective in vec2 tex_coord;
         const vec2 size = vec2(2.0,0.0);
         const ivec3 off = ivec3(-1,0,1);
         
         vec4 wave = texture(unit_wave, tex_coord);
         float s11 = wave.x;
         float s01 = textureOffset(unit_wave, tex_coord, off.xy).x;
         float s21 = textureOffset(unit_wave, tex_coord, off.zy).x;
         float s10 = textureOffset(unit_wave, tex_coord, off.yx).x;
         float s12 = textureOffset(unit_wave, tex_coord, off.yz).x;
         vec3 va = normalize(vec3(size.xy,s21-s01));
         vec3 vb = normalize(vec3(size.yx,s12-s10));
         vec4 bump = vec4( cross(va,vb), s11 );
         */
        const vec2 size = vec2(2.0,0.0);
        const ivec3 off = ivec3(-1,0,1);
        
        float s11 = texture2D(bumpImage, texCoordFrag).x;
        vec2 coord01 = texCoordFrag + off.xy;
        float s01 = texture2D(bumpImage, coord01).x;
        
        vec2 coord21 = texCoordFrag + off.zy;
        float s21 = texture2D(bumpImage, coord21).x;
        
        vec2 coord10 = texCoordFrag + off.yx;
        float s10 = texture2D(bumpImage, coord10).x;
        
        vec2 coord12 = texCoordFrag + off.yz;
        float s12 = texture2D(bumpImage, coord12).x;

        vec3 va = normalize(vec3(size.xy,s21-s01));
        vec3 vb = normalize(vec3(size.yx,s12-s10));
        vec3 normalBump = vec4( cross(va,vb), s11 ).xyz;
        
        
        ////////////////////////////////////////
        //directly use normal map
        ////////////////////////////////////////
        //read normal map directly
        //vec3 normalBump = texture2D( bumpImage, texCoordFrag).xyz;
        normalBump = normalize( normalBump * 2.0 - 1.0);
        
        vec3 surfaceToLightTangentNormalized = normalize(surfaceToLightTangent);
        vec3 surfaceToCameraTangentNormalized = normalize(surfaceToCameraTangent);
        
        float nDotL = dot(normalBump, surfaceToLightTangentNormalized);
        vec3 reflection = ( 2.0 * normalBump * nDotL) - surfaceToLightTangentNormalized;
        float rDotL = max(0.0, dot(reflection, surfaceToCameraTangentNormalized));
        
        //recalculate diffuseCoefficent, & specularCoefficent
        diffuse = max(0.0, nDotL) * surfaceColor.rgb;
        amb = lightAmbientCoefficient * surfaceColor.rgb;
        spec = pow(rDotL, 1) * specularColor.rgb;
        linearColor = attenuation * (diffuse+spec) + amb;
        gl_FragColor = vec4(linearColor, surfaceColor.a);
        
    } else { //if( texCount == 0 ) {
        //normal color
        surfaceColor = diffuseColor;
        diffuse = diffuseCoefficent * surfaceColor.rgb;
        amb = lightAmbientCoefficient * ambientColor.rgb;
        spec = specularCoefficent * specularColor.rgb;
        
        linearColor = attenuation * (diffuse+spec) + amb;
        gl_FragColor = vec4(linearColor, surfaceColor.a);
        //gl_FragColor = vec4(pow(linearColor, gamma), surfaceColor.a);
    }
}