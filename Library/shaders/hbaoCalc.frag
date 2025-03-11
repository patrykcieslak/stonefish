/*-----------------------------------------------------------------------
  Copyright (c) 2014-2015, NVIDIA. All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:
   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   * Neither the name of its contributors may be used to endorse 
     or promote products derived from this software without specific
     prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
  PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------*/

/* 
    Based on DeinterleavedTexturing sample by Louis Bavoil
    https://github.com/NVIDIAGameWorks/D3DSamples/tree/master/samples/DeinterleavedTexturing
*/

#version 430

#define AO_RANDOMTEX_SIZE 4

struct HBAOData 
{
  float   RadiusToScreen;        // radius
  float   R2;     // 1/radius
  float   NegInvR2;     // radius * radius
  float   NDotVBias;
 
  vec2    InvFullResolution;
  vec2    InvQuarterResolution;
  
  float   AOMultiplier;
  float   PowExponent;
  vec2    _pad0;
  
  vec4    projInfo;
  vec2    projScale;
  vec2	  _pad1;
  //int     projOrtho;
  //int     _pad1;
  
  vec4    float2Offsets[AO_RANDOMTEX_SIZE*AO_RANDOMTEX_SIZE];
  vec4    jitters[AO_RANDOMTEX_SIZE*AO_RANDOMTEX_SIZE];
};

// The pragma below is critical for optimal performance
// in this fragment shader to let the shader compiler
// fully optimize the maths and batch the texture fetches
// optimally
#pragma optionNV(unroll all)

#define M_PI 3.14159265f

// tweakables
const float  NUM_STEPS = 4;
const float  NUM_DIRECTIONS = 8; // texRandom/g_Jitter initialization depends on this

layout(std140,binding=0) uniform controlBuffer 
{
	HBAOData control;
};

vec2 g_Float2Offset = control.float2Offsets[gl_PrimitiveID].xy;
vec4 g_Jitter       = control.jitters[gl_PrimitiveID];
  
layout(binding=0) uniform sampler2DArray texLinearDepth;
layout(binding=1) uniform sampler2D texViewNormal;
layout(location=0,index=0) out vec4 fragColor;

vec3 getQuarterCoord(vec2 UV)
{
	return vec3(UV, float(gl_PrimitiveID));
}
  
void outputColor(vec4 color) 
{
	fragColor = color;
}

vec3 UVToView(vec2 uv, float eye_z)
{
	return vec3((uv * control.projInfo.xy + control.projInfo.zw) * eye_z, eye_z);
}

vec3 FetchQuarterResViewPos(vec2 UV)
{
	float ViewDepth = textureLod(texLinearDepth, getQuarterCoord(UV), 0).x;
	return UVToView(UV, ViewDepth);
}

float Falloff(float DistanceSquare)
{
	// 1 scalar mad instruction
	return DistanceSquare * control.NegInvR2 + 1.0;
}

//----------------------------------------------------------------------------------
// P = view-space position at the kernel center
// N = view-space normal at the kernel center
// S = view-space position of the current sample
//----------------------------------------------------------------------------------
float ComputeAO(vec3 P, vec3 N, vec3 S)
{
	vec3 V = S - P;
	float VdotV = dot(V, V);
	float NdotV = dot(N, V) * 1.0/sqrt(VdotV);

	// Use saturate(x) instead of max(x,0.f) because that is faster on Kepler
	return clamp(NdotV - control.NDotVBias,0,1) * clamp(Falloff(VdotV),0,1);
}

vec2 RotateDirection(vec2 Dir, vec2 CosSin)
{
	return vec2(Dir.x*CosSin.x - Dir.y*CosSin.y, Dir.x*CosSin.y + Dir.y*CosSin.x);
}

float ComputeCoarseAO(vec2 FullResUV, float RadiusPixels, vec4 Rand, vec3 ViewPosition, vec3 ViewNormal)
{
	RadiusPixels /= 4.0;

	// Divide by NUM_STEPS+1 so that the farthest samples are not fully attenuated
	float StepSizePixels = RadiusPixels / (NUM_STEPS + 1);

	const float Alpha = 2.0 * M_PI / NUM_DIRECTIONS;
	float AO = 0;

	for (float DirectionIndex = 0; DirectionIndex < NUM_DIRECTIONS; ++DirectionIndex)
	{
		float Angle = Alpha * DirectionIndex;

		// Compute normalized 2D direction
		vec2 Direction = RotateDirection(vec2(cos(Angle), sin(Angle)), Rand.xy);

		// Jitter starting sample within the first step
		float RayPixels = (Rand.z * StepSizePixels + 1.0);

		for (float StepIndex = 0; StepIndex < NUM_STEPS; ++StepIndex)
		{
			vec2 SnappedUV = round(RayPixels * Direction) * control.InvQuarterResolution + FullResUV;
			vec3 S = FetchQuarterResViewPos(SnappedUV);

			RayPixels += StepSizePixels;

			AO += ComputeAO(ViewPosition, ViewNormal, S);
		}
	}

	AO *= control.AOMultiplier / (NUM_DIRECTIONS * NUM_STEPS);
	return clamp(1.0 - AO * 2.0,0,1);
}

void main()
{
	vec2 base = floor(gl_FragCoord.xy) * 4.0 + g_Float2Offset;
    vec2 uv = base * (control.InvQuarterResolution / 4.0);

    vec3 ViewPosition = FetchQuarterResViewPos(uv);
    vec4 NormalAndAO =  texelFetch(texViewNormal, ivec2(base), 0);
    vec3 ViewNormal = NormalAndAO.xyz * 2.0 - 1.0;
	ViewNormal.z = -ViewNormal.z;

    // Compute projection of disk of radius control.R into screen space
    float RadiusPixels = control.RadiusToScreen/ViewPosition.z;

    float AO = ComputeCoarseAO(uv, RadiusPixels, g_Jitter, ViewPosition, ViewNormal);

	outputColor(vec4(pow(AO, control.PowExponent), ViewPosition.z, 0, 0));  
}
