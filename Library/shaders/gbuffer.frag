#version 120
varying vec4 color;
varying vec4 position;
varying vec3 normal;
varying float depth;
varying float material;
uniform sampler2D texture;
uniform bool isTextured;

void main(void)
{
    //rendering
    vec3 finalColor;
    
    if(isTextured)
    {
        vec4 texColor = texture2D(texture, gl_TexCoord[0].st);
        finalColor = color.rgb * texColor.rgb;
    }
    else
        finalColor = color.rgb;
    
	gl_FragData[0] = vec4(finalColor, color.a);   //Alpha - Factor1
	gl_FragData[1] = vec4(position.xyz, material);//Alpha - Material type + Factor2
	gl_FragData[2] = vec4(normal.xyz, depth);
}
