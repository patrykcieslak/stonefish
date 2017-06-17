#version 330 core

in vec4 position;
in vec3 normal;
in vec2 texcoord;
in float depth;

out vec4[3] fragData;

uniform vec4 color;
uniform sampler2D tex;
uniform float materialData;

void main(void)
{
    vec4 texColor = texture2D(tex, texcoord);
    vec3 finalColor = mix(color.rgb, texColor.rgb, color.a);
    
	fragData[0] = vec4(finalColor, 1.0);
    fragData[1] = vec4(position.xyz, materialData);
	fragData[2] = vec4(normal.xyz, depth);
}
