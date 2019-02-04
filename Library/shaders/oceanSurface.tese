#version 400

layout(quads, equal_spacing, ccw) in;

uniform sampler2DArray texWaveFFT;
uniform mat4 MVP;
uniform vec4 gridSizes;
uniform float tessDiv;

out vec4 fragPos;

vec4 interpolate(in vec4 v0, in vec4 v1, in vec4 v2, in vec4 v3)
{
	vec4 a = mix(v0, v1, gl_TessCoord.x);
	vec4 b = mix(v3, v2, gl_TessCoord.x);
	return mix(a, b, gl_TessCoord.y);
}

void main()
{
	//Interpolate positions
	vec4 P = interpolate(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position, gl_in[3].gl_Position);

    //Basic wave height (layer 0)
	float dz = 0.0;
    dz += texture(texWaveFFT, vec3(P.xy/gridSizes.x, 0.0)).x;
    dz += texture(texWaveFFT, vec3(P.xy/gridSizes.y, 0.0)).y;
    dz += texture(texWaveFFT, vec3(P.xy/gridSizes.z, 0.0)).z;
    dz += texture(texWaveFFT, vec3(P.xy/gridSizes.w, 0.0)).w;

    /*
    //Make use of anisotropic filtering -> not working properly -> cracks!
    //Calculate derivatives of position
    vec2 dy = (gl_in[1].gl_Position.xy - gl_in[0].gl_Position.xy)/tessDiv;
    vec2 dx = (gl_in[3].gl_Position.xy - gl_in[0].gl_Position.xy)/tessDiv;

    dz += textureGrad(texWaveFFT, vec3(P.xy/gridSizes.x, 0.0), dx/gridSizes.x, dy/gridSizes.x).x;
    dz += textureGrad(texWaveFFT, vec3(P.xy/gridSizes.y, 0.0), dx/gridSizes.y, dy/gridSizes.y).y;
    dz += textureGrad(texWaveFFT, vec3(P.xy/gridSizes.z, 0.0), dx/gridSizes.z, dy/gridSizes.z).z;
    dz += textureGrad(texWaveFFT, vec3(P.xy/gridSizes.w, 0.0), dx/gridSizes.w, dy/gridSizes.w).w;
    */

	fragPos = vec4(P.xy, dz, 1.0); //Position of deformed vertex in world space
	gl_Position = MVP * fragPos; //Screen space position
}
