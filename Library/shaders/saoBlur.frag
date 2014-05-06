#version 120

#define EDGE_SHARPNESS     (1.0)

/** Step in 2-pixel intervals since we already blurred against neighbors in the
 first AO pass.  This constant can be increased while R decreases to improve
 performance at the expense of some dithering artifacts.
 
 Morgan found that a scale of 3 left a 1-pixel checkerboard grid that was
 unobjectionable after shading was applied but eliminated most temporal incoherence
 from using small numbers of sample taps.
 */
#define SCALE               (2)

/** Filter radius in pixels. This will be multiplied by SCALE. */
#define R                   (4)

uniform sampler2D texSource;
uniform vec2 axis;

const float epsilon = 0.0001;

float unpackKey(vec2 p)
{
    return p.x * (256.0 / 257.0) + p.y * (1.0 / 257.0);
}

void main()
{
    float gaussian[R + 1];
#       if R == 3
    gaussian[0] = 0.153170; gaussian[1] = 0.144893; gaussian[2] = 0.122649; gaussian[3] = 0.092902;  // stddev = 2.0
#       elif R == 4
    gaussian[0] = 0.153170; gaussian[1] = 0.144893; gaussian[2] = 0.122649; gaussian[3] = 0.092902; gaussian[4] = 0.062970;  // stddev = 2.0
#       elif R == 6
    gaussian[0] = 0.111220; gaussian[1] = 0.107798; gaussian[2] = 0.098151; gaussian[3] = 0.083953; gaussian[4] = 0.067458; gaussian[5] = 0.050920; gaussian[6] = 0.036108;
#       endif
    
    ivec2 ssC = ivec2(gl_FragCoord.xy);
    
    vec4 temp = texture2D(texSource, gl_TexCoord[0].st);
    float key = unpackKey(temp.gb);
    float sum = temp.r;
    gl_FragColor.gb = temp.gb;
    
    if (key == 1.0)
    {
        // Sky pixel (if you aren't using depth keying, disable this test)
        gl_FragColor.r = sum;
        return;
    }
    
    // Base weight for depth falloff.  Increase this for more blurriness,
    // decrease it for better edge discrimination
    float BASE = gaussian[0];
    float totalWeight = BASE;
    sum *= totalWeight;
    
    for (int r = -R; r <= R; ++r)
    {
        // We already handled the zero case above.  This loop should be unrolled and the static branch optimized out,
        // so the IF statement has no runtime cost
        if (r != 0)
        {
            temp = texture2D(texSource, gl_TexCoord[0].st + axis * float(r * SCALE));
            float tapKey = unpackKey(temp.gb);
            float value  = temp.r;
            
            // spatial domain: offset gaussian tap
            float weight = 0.3 + gaussian[int(abs(r))];
            
            // range domain (the "bilateral" weight). As depth difference increases, decrease weight.
            weight *= max(0.0, 1.0 - (EDGE_SHARPNESS * 2000.0) * abs(tapKey - key));
            
            sum += value * weight;
            totalWeight += weight;
        }
    }
    
    gl_FragColor.r = sum/(totalWeight + epsilon);
}
