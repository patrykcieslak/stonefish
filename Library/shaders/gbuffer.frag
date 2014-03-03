#version 120
varying vec4 color;
varying vec4 position;
varying vec3 normal;
varying float depth;
varying float material;
varying float clipPos;

uniform sampler2D texture;
uniform bool isTextured;

void main(void)
{
    if(clipPos < 0.0)
        discard;
    
    vec3 finalColor;
    
    if(isTextured)
    {
        vec4 texColor = texture2D(texture, gl_TexCoord[0].st);
        finalColor = mix(color.rgb, texColor.rgb, texColor.a);
    }
    else
        finalColor = color.rgb;
    
	gl_FragData[0] = vec4(finalColor, color.a);   //Alpha - Factor1
	gl_FragData[1] = vec4(position.xyz, material);//Alpha - Material type + Factor2
	gl_FragData[2] = vec4(normal.xyz, depth);
}
