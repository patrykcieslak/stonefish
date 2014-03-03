#version 120
uniform sampler2D texScene;

void main(void)
{
    vec3 scene = texture2D(texScene, gl_TexCoord[0].st).rgb;
    gl_FragColor = vec4(scene, 1.0);
}