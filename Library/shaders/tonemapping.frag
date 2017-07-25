#version 330 core

in vec2 texcoord;
out vec4 fragcolor;
uniform sampler2D texHDR;
uniform sampler2D texAverage;

const float A = 0.15;
const float B = 0.50;
const float C = 0.10;
const float D = 0.20;
const float E = 0.02;
const float F = 0.30;

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0/3.0, 2.0/3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 Uncharted2Tonemap(vec3 c)
{
    return ((c * (A * c + vec3(C * B)) + vec3(D * E))/(c * (A * c + vec3(B)) + vec3(D * F))) - vec3(E / F);
}

float Uncharted2TonemapValue(float v)
{
    return ((v * (A * v + C * B) + D * E)/(v * (A * v + B) + D * F)) - E/F;
}

void main(void)
{
    //Read textures
    float lumAvg = 1.0;// texture2D(texAverage, vec2(0.5, 0.5)).x;
    vec3 rgbColor = texture2D(texHDR, texcoord).rgb;
    
    //Correct exposure and tonemap
    float exposure = 0.5/lumAvg;
    rgbColor = Uncharted2Tonemap(exposure * rgbColor);
    vec3 whitePoint = Uncharted2Tonemap(vec3(1.0)); //1.0 is the saturated color of sun in the sky
    rgbColor /= whitePoint;
    
    //Increase saturation
    vec3 hsvColor = rgb2hsv(rgbColor);
    hsvColor.y *= 1.0;
    rgbColor = hsv2rgb(hsvColor);
    
    //Correct Gamma
    fragcolor = vec4(pow(rgbColor, vec3(1.0/2.2)), 1.0);
}
