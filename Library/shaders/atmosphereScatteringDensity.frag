layout(location = 0) out vec3 scattering_density;
uniform sampler2D texTransmittance;
uniform sampler3D texSingleRayleighScattering;
uniform sampler3D texSingleMieScattering;
uniform sampler3D texMultipleScattering;
uniform sampler2D texIrradiance;
uniform int scatteringOrder;
uniform int layer;

void main() 
{
	scattering_density = ComputeScatteringDensityTexture(atmosphere, texTransmittance, texSingleRayleighScattering,
														 texSingleMieScattering, texMultipleScattering,
														 texIrradiance, vec3(gl_FragCoord.xy, layer + 0.5), scatteringOrder);
}