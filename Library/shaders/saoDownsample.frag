#version 330 core

out vec4 fragcolor;
uniform sampler2D source;
uniform vec2 srcViewport;

// Used for packing Z into the GB channels
void packKey(float key, out vec2 p)
{
    // Round to the nearest 1/256.0
    float temp = floor(key * 256.0);
    
    // Integer part
    p.x = temp * (1.0 / 256.0);
    
    // Fractional part
    p.y = key * 256.0 - temp;
}

float unpackKey(vec2 p)
{
    return p.x * (256.0 / 257.0) + p.y * (1.0 / 257.0);
}

vec2 sourceSample(ivec2 offset)
{
    ivec2 dstCoord = ivec2(gl_FragCoord.xy);
    ivec2 srcCoord = 2 * dstCoord + offset;
    vec2 srcTexCoord = (2.0 * vec2(srcCoord) + vec2(1.0))/(2.0 * srcViewport);
    
    vec4 srcSample = texture(source, srcTexCoord);
    return vec2(srcSample.r, unpackKey(srcSample.gb));
}

void main(void)
{
    vec2 avg = (sourceSample(ivec2(0,0)) +
                sourceSample(ivec2(1,0)) +
                sourceSample(ivec2(0,1)) +
                sourceSample(ivec2(1,1))) * 0.25;
    
    fragcolor.r = avg.x;
    packKey(avg.y, fragcolor.gb);
}

/*
 vec4 sourceSample(ivec2 offset)
 {
 ivec2 dstCoord = ivec2(gl_FragCoord.xy);
 ivec2 srcCoord = 2 * dstCoord + offset;
 vec2 srcTexCoord = (2.0 * vec2(srcCoord) + vec2(1.0))/(2.0 * srcViewport);
 return texture2D(source, srcTexCoord);
 }
 
 void main(void)
 {
 vec4 color = (sourceSample(ivec2(0,0)) +
 sourceSample(ivec2(1,0)) +
 sourceSample(ivec2(0,1)) +
 sourceSample(ivec2(1,1))) * 0.25;
 gl_FragColor = color;
 }
 */