#version 120
varying vec4 color;
varying vec4 position;
varying vec3 normal;
varying float depth;

uniform sampler2D texture;
uniform float materialData;

void main(void)
{
    vec4 texColor = texture2D(texture, gl_TexCoord[0].st);
    vec3 finalColor = mix(color.rgb, texColor.rgb, color.a);
    
	gl_FragData[0] = vec4(finalColor, 1.0);
    gl_FragData[1] = vec4(position.xyz, materialData);
	gl_FragData[2] = vec4(normal.xyz, depth);
}
