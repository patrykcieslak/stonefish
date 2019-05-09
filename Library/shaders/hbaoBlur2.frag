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

#version 430

const float KERNEL_RADIUS = 3;
  
layout(location=0) uniform float sharpness;
layout(location=1) uniform vec2  invResolutionDirection; // either set x to 1/width or y to 1/height
layout(binding=0) uniform sampler2D texSource;
in vec2 texcoord;
layout(location=0,index=0) out vec4 fragColor;

float BlurFunction(vec2 uv, float r, float center_c, float center_d, inout float w_total)
{
	vec2  aoz = texture2D(texSource, uv).xy;
	float c = aoz.x;
	float d = aoz.y;
  
	const float BlurSigma = float(KERNEL_RADIUS) * 0.5;
	const float BlurFalloff = 1.0 / (2.0*BlurSigma*BlurSigma);
  
	float ddiff = (d - center_d) * sharpness;
	float w = exp2(-r*r*BlurFalloff - ddiff*ddiff);
	w_total += w;

	return c*w;
}

void main()
{
	vec2  aoz = texture2D(texSource, texcoord).xy;
	float center_c = aoz.x;
	float center_d = aoz.y;
  
	float c_total = center_c;
	float w_total = 1.0;
  
	for(float r = 1; r <= KERNEL_RADIUS; ++r)
	{
		vec2 uv = texcoord + invResolutionDirection * r;
		c_total += BlurFunction(uv, r, center_c, center_d, w_total);  
	}
  
	for(float r = 1; r <= KERNEL_RADIUS; ++r)
	{
		vec2 uv = texcoord - invResolutionDirection * r;
		c_total += BlurFunction(uv, r, center_c, center_d, w_total);  
	}
  
	fragColor = vec4(c_total/w_total);
}
