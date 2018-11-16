uniform sampler2D transmittance_texture;
uniform sampler3D scattering_texture;
uniform sampler3D single_mie_scattering_texture;
uniform sampler2D irradiance_texture;

#ifdef RADIANCE_API_ENABLED
RadianceSpectrum GetSolarRadiance() 
{
	return atmosphere.solar_irradiance / (PI * atmosphere.sun_angular_radius * atmosphere.sun_angular_radius);
}

RadianceSpectrum GetSkyRadiance(Position camera, Direction view_ray, Length shadow_length, Direction sun_direction, out DimensionlessSpectrum transmittance) 
{
    return GetSkyRadiance(atmosphere, transmittance_texture, scattering_texture, single_mie_scattering_texture, camera, view_ray, shadow_length, sun_direction, transmittance);
}

RadianceSpectrum GetSkyRadianceToPoint(Position camera, Position point, Length shadow_length, Direction sun_direction, out DimensionlessSpectrum transmittance) 
{
	return GetSkyRadianceToPoint(atmosphere, transmittance_texture, scattering_texture, single_mie_scattering_texture, camera, point, shadow_length, sun_direction, transmittance);
}

IrradianceSpectrum GetSunAndSkyIrradiance(Position p, Direction normal, Direction sun_direction, out IrradianceSpectrum sky_irradiance) 
{
	return GetSunAndSkyIrradiance(atmosphere, transmittance_texture, irradiance_texture, p, normal, sun_direction, sky_irradiance);
}
#endif

Luminance3 GetSolarLuminance() 
{
	return atmosphere.solar_irradiance /(PI * atmosphere.sun_angular_radius * atmosphere.sun_angular_radius) * SUN_SPECTRAL_RADIANCE_TO_LUMINANCE;
}

Luminance3 GetSkyLuminance(Position camera, Direction view_ray, Length shadow_length, Direction sun_direction,
                           out DimensionlessSpectrum transmittance)
{
    return GetSkyRadiance(atmosphere, transmittance_texture, scattering_texture, single_mie_scattering_texture, camera, view_ray, shadow_length, sun_direction, transmittance) * SKY_SPECTRAL_RADIANCE_TO_LUMINANCE;
}

Luminance3 GetSkyLuminanceToPoint(Position camera, Position point, Length shadow_length, Direction sun_direction,
                                  out DimensionlessSpectrum transmittance) 
{
	return GetSkyRadianceToPoint(atmosphere, transmittance_texture, scattering_texture, single_mie_scattering_texture, camera, point, shadow_length, sun_direction, transmittance) * SKY_SPECTRAL_RADIANCE_TO_LUMINANCE;
}

Illuminance3 GetSunAndSkyIlluminance(Position p, Direction normal, Direction sun_direction, out IrradianceSpectrum sky_irradiance) 
{
	IrradianceSpectrum sun_irradiance = GetSunAndSkyIrradiance(atmosphere, transmittance_texture, irradiance_texture, p, normal, sun_direction, sky_irradiance);
	sky_irradiance *= SKY_SPECTRAL_RADIANCE_TO_LUMINANCE;
	return sun_irradiance * SUN_SPECTRAL_RADIANCE_TO_LUMINANCE;
}
