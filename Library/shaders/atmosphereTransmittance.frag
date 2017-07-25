layout(location = 0) out vec3 transmittance;

void main() 
{
	transmittance = ComputeTransmittanceToTopAtmosphereBoundaryTexture(atmosphere, gl_FragCoord.xy);
}