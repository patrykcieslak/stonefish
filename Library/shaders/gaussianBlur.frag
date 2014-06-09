#version 120

uniform sampler2D source;
varying vec2 texCoord;
varying vec2 blurTexCoord[4];

void main()
{
    gl_FragColor = vec4(0.0);
    gl_FragColor += texture2D(source, blurTexCoord[0]) * 0.0702702703;
    gl_FragColor += texture2D(source, blurTexCoord[1]) * 0.3162162162;
    gl_FragColor += texture2D(source, texCoord) * 0.2270270270;
    gl_FragColor += texture2D(source, blurTexCoord[2]) * 0.3162162162;
    gl_FragColor += texture2D(source, blurTexCoord[3]) * 0.0702702703;
}