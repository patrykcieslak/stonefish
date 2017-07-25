uniform sampler2D texTransmittance;
uniform sampler3D texScattering;
uniform sampler3D texSingleMieScattering; /* not used */
uniform vec3 eyePos;
uniform vec3 sunDir;
uniform mat4 invView; //model matrix from view matrix
uniform mat4 invClip; //view matrix from clip matrix

//in vec3 viewRay;
out vec3 fragColor;

const vec3 whitePoint = vec3(1.0);
const float exposure = 0.0000001;
const vec2 viewport = vec2(1200.0,900.0);
const vec2 sunSize = vec2(tan(0.00935 / 2.0),cos(0.00935 / 2.0));

vec3 get_world_normal()
{
    vec2 frag_coord =  gl_FragCoord.xy/viewport;
    frag_coord = (frag_coord-0.5)*2.0;
    vec4 device_normal = vec4(frag_coord, 0.0, 1.0);
    vec3 eye_normal = normalize((invClip * device_normal).xyz);
    vec3 world_normal = normalize(mat3(invView) * eye_normal);
    return world_normal;
}

Luminance3 GetSolarLuminance() 
{
	return atmosphere.solar_irradiance / (PI * atmosphere.sun_angular_radius * atmosphere.sun_angular_radius) * SUN_SPECTRAL_RADIANCE_TO_LUMINANCE;
}


Luminance3 GetSkyLuminance(Position camera, Direction view_ray, Length shadow_length, Direction sun_direction, 
							out DimensionlessSpectrum transmittance) 
{
	return GetSkyRadiance(atmosphere, texTransmittance, texScattering, texSingleMieScattering,
						   camera, view_ray, shadow_length, sun_direction, transmittance) * SKY_SPECTRAL_RADIANCE_TO_LUMINANCE;
}

void main()
{
	vec3 viewDir = get_world_normal();
	vec3 center = vec3(0, 0, -atmosphere.bottom_radius);
	
	DimensionlessSpectrum trans;
	vec3 luminance = GetSkyLuminance(eyePos - center, viewDir, 2.f, sunDir, trans);

	if(dot(viewDir, sunDir) > sunSize.y) 
	{
		luminance = luminance + trans * GetSolarLuminance();
	}

	fragColor = luminance/10000.0;//pow(vec3(1.0) - exp(-luminance/whitePoint * exposure), vec3(1.0/2.2));
}