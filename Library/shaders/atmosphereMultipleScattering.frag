layout(location = 0) out vec3 delta_multiple_scattering;
layout(location = 1) out vec4 scattering;
uniform sampler2D texTransmittance;
uniform sampler3D texScatteringDensity;
uniform mat3 luminanceFromRadiance;
uniform int layer;

void main() 
{
	float nu;
    delta_multiple_scattering = ComputeMultipleScatteringTexture(atmosphere, texTransmittance, texScatteringDensity, vec3(gl_FragCoord.xy, layer + 0.5), nu);
	scattering = vec4(luminanceFromRadiance * delta_multiple_scattering.rgb / RayleighPhaseFunction(nu), 0.0);
}