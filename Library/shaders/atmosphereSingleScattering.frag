layout(location = 0) out vec3 delta_rayleigh;
layout(location = 1) out vec3 delta_mie;
layout(location = 2) out vec4 scattering;
layout(location = 3) out vec3 single_mie_scattering;
uniform mat3 luminanceFromRadiance;
uniform sampler2D texTransmittance;
uniform int layer;

void main() 
{
	ComputeSingleScatteringTexture(atmosphere, texTransmittance, vec3(gl_FragCoord.xy, layer + 0.5), delta_rayleigh, delta_mie);
	scattering = vec4(luminanceFromRadiance * delta_rayleigh.rgb, (luminanceFromRadiance * delta_mie).r);
	single_mie_scattering = luminanceFromRadiance * delta_mie;
}