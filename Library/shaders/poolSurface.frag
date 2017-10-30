#version 330 core

out vec4 fragColor;
uniform sampler2D texReflection; 

const float airWaterR0 = pow(1.0-1.33,2.0)/pow(1.0+1.33,2.0);

float fresnel(float R0, float cosAlpha)
{
	return (1.0 - R0) * pow(1.0 - cosAlpha, 5.0) + R0;
}

void main()
{
	vec4 reflectedColor = texture(texReflection, vec2(gl_FragCoord.x/1500.0, gl_FragCoord.y/1000.0));
	float fresn = fresnel(airWaterR0, 0.0);
	fragColor = fresn * vec4(0.0, 0.3, 0.5, 0.2) + (1.0-fresn) * reflectedColor;
}