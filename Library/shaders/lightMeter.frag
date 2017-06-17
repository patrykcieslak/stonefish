#version 330 core

in vec2 texcoord;
out vec4 fragcolor;
uniform sampler2D texHDR;
uniform ivec2 samples;

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
    float avgLuminance = 0.0;
    
    for(int i = 0; i < samples.x; i++)
        for(int h = 0; h < samples.y; h++)
        {
            vec2 texCoord = vec2(float(i)/float(samples.x - 1), float(h)/float(samples.y - 1));
            vec3 hsvColor = rgb2hsv(texture(texHDR, texcoord).rgb);
            avgLuminance += log(hsvColor.z + 1.0);
        }
    
    avgLuminance /= float(samples.x * samples.y);
    
    fragcolor = vec4(avgLuminance);
}