layout(location = 0) out vec3 delta_irradiance;
layout(location = 1) out vec3 irradiance;
uniform sampler2D texTransmittance;

void main() 
{
	delta_irradiance = ComputeDirectIrradianceTexture(atmosphere, texTransmittance, gl_FragCoord.xy);
	irradiance = vec3(0.0);
}