/*
    Downsample x4 by using only one pass achieved by making use of the built-in bilinear filtering
 */

#version 120

uniform sampler2D source;
uniform vec2 srcViewport;

vec4 sourceSample(vec2 offset)
{
    ivec2 dstCoord = ivec2(gl_FragCoord.xy);
    vec2 srcCoord = 4.0 * dstCoord + offset;
    vec2 srcTexCoord = (2.0 * srcCoord + vec2(1.0))/(2.0 * srcViewport);
    return texture2D(source, srcTexCoord);
}

void main(void)
{
    gl_FragColor = (sourceSample(vec2(0.5, 0.5)) +
                    sourceSample(vec2(2.5, 0.5)) +
                    sourceSample(vec2(0.5, 2.5)) +
                    sourceSample(vec2(2.5, 2.5))) * 0.25;
	gl_FragColor = mix(gl_FragColor, vec4(1.0), 0.33);
}