#version 430

in vec2 texcoord;
layout(location=0,index=0) out vec4 fragColor;

layout(location=0) uniform vec4 projInfo; 
layout(location=1) uniform vec2 invFullResolution;
layout(binding=0)  uniform sampler2D texLinearDepth;

vec3 UVToView(vec2 uv, float eye_z)
{
	return vec3(uv * projInfo.xy + projInfo.zw * eye_z, eye_z);
}

vec3 fetchViewPos(vec2 uv)
{
	float viewDepth = textureLod(texLinearDepth, uv, 0).x;
	return UVToView(uv, viewDepth);
}

vec3 minDiff(vec3 P, vec3 Pr, vec3 Pl)
{
	vec3 v1 = Pr - P;
	vec3 v2 = P - Pl;
	return (dot(v1,v1) < dot(v2,v2)) ? v1 : v2;
}

vec3 reconstructNormal(vec2 uv, vec3 P)
{
	vec3 Pr = fetchViewPos(uv + vec2(invFullResolution.x, 0));
	vec3 Pl = fetchViewPos(uv + vec2(-invFullResolution.x, 0));
	vec3 Pt = fetchViewPos(uv + vec2(0, invFullResolution.y));
	vec3 Pb = fetchViewPos(uv + vec2(0, -invFullResolution.y));
	return normalize(cross(minDiff(P, Pr, Pl), minDiff(P, Pt, Pb)));
}

void main() 
{
  vec3 P  = fetchViewPos(texcoord);
  vec3 N  = reconstructNormal(texcoord, P);
  fragColor = vec4(N*0.5 + 0.5,0);
}

/*-----------------------------------------------------------------------
  Copyright (c) 2014, NVIDIA. All rights reserved.

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