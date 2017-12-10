#version 330 core

out vec3 fragColor;
uniform sampler2D texHDR;
uniform ivec2 samples;

const float pi2 = 3.14159/2.0;

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0/3.0, 2.0/3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

void main(void)
{
    float luminance = 0.0;
    
    for(int a = 0; a < samples.x; ++a) //angle
	{
		float angle = (a+0.5)/float(samples.x)*pi2;
		
        for(int r = 1; r <= samples.y; ++r) //radius
        {
            vec2 texCoord = vec2(0.5) + (gl_FragCoord.xy-vec2(1.0)) * float(r)/float(samples.y) * vec2(cos(angle), sin(angle));
            vec3 hsvColor = rgb2hsv(texture(texHDR, texCoord).rgb);
            luminance += log(clamp(hsvColor.z,0.0,1000.0)+1.1);
        }
    }
	
	fragColor = vec3(luminance/float(samples.x * samples.y));
}