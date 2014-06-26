#version 120

#define EDGE_SHARPNESS     (0.8)

uniform sampler2D texSource;
varying vec2 texCoord;
varying vec2 blurTexCoord[4];

const float blurWeight[4] = float[4](0.0702702703, 0.3162162162, 0.3162162162, 0.0702702703);

float unpackKey(vec2 p)
{
    return p.x * (256.0 / 257.0) + p.y * (1.0 / 257.0);
}

void main()
{
    vec4 temp = texture2D(texSource, texCoord);
    gl_FragColor.gb = temp.gb;
    
    float key = unpackKey(temp.gb);
    float sum = temp.r * 0.2270270270;
    float totalWeight = 0.2270270270;
    
    for(int i = 0; i < 4; ++i)
    {
        temp = texture2D(texSource, blurTexCoord[i]);
        float tapKey = unpackKey(temp.gb);
        float value  = temp.r;
        
        // spatial domain: offset gaussian tap
        float weight = 0.3 + blurWeight[i];
        
        // range domain (the "bilateral" weight). As depth difference increases, decrease weight.
        weight *= max(0.0, 1.0 - (EDGE_SHARPNESS * 2000.0) * abs(tapKey - key));
        
        sum += value * weight;
        totalWeight += weight;
    }
    
    gl_FragColor.r = sum/totalWeight;
}
