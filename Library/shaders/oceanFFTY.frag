#version 430 core
uniform sampler2D texButterfly;
uniform sampler2DArray texSource; // 2 complex inputs (= 4 values) per layer
uniform float pass;

in vec2 gTexcoord;
out vec4 fragColor;

// performs two FFTs on two inputs packed in a single texture
// returns two results packed in a single vec4
vec4 fft2(int layer, vec2 i, vec2 w) 
{
    vec4 input1 = textureLod(texSource, vec3(gTexcoord.x, i.x, layer), 0.0);
    vec4 input2 = textureLod(texSource, vec3(gTexcoord.x, i.y, layer), 0.0);
    float res1x = w.x * input2.x - w.y * input2.y;
    float res1y = w.y * input2.x + w.x * input2.y;
    float res2x = w.x * input2.z - w.y * input2.w;
    float res2y = w.y * input2.z + w.x * input2.w;
    return input1 + vec4(res1x, res1y, res2x, res2y);
}

void main() 
{
	vec4 data = textureLod(texButterfly, vec2(gTexcoord.y, pass), 0.0);
	vec2 i = data.xy;
    vec2 w = data.zw;
    fragColor = fft2(gl_PrimitiveID, i, w);
}