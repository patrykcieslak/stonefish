layout(location = 0) out vec3 delta_irradiance;
layout(location = 1) out vec3 irradiance;
uniform sampler3D texSingleRayleighScattering;
uniform sampler3D texSingleMieScattering;
uniform sampler3D texMultipleScattering;
uniform mat3 luminanceFromRadiance;
uniform int scatteringOrder;

void main() 
{
	delta_irradiance = ComputeIndirectIrradianceTexture(atmosphere, texSingleRayleighScattering, texSingleMieScattering, 
																	texMultipleScattering, gl_FragCoord.xy, scatteringOrder);
	irradiance = luminanceFromRadiance * delta_irradiance;
}