#version 330

in vec2 texcoord;
uniform sampler2D texColor;
uniform sampler2D texViewNormal;
uniform sampler2D texLinearDepth;
uniform vec2 viewport;
uniform mat4 invP;

out vec4 fragColor;

vec3 getCameraRay()
{
    vec4 deviceNormal = vec4((texcoord-0.5)*2.0, 1.0, 1.0);
    vec4 eyeNormal = invP * deviceNormal;
    return normalize(eyeNormal.xyz); ///eyeNormal.w);
}
//vec3 eyeNormal = normalize((invP * deviceNormal).xyz);
//vec3 worldNormal = normalize(invView * eyeNormal);

vec3 getFragColor(vec2 coord)
{
    vec2 uv = coord/2.0 + 0.5;
    return texture(texColor, uv).rgb;
}

float getFragDepth(vec2 coord)
{
    vec2 uv = coord/2.0 + 0.5;
    return texture(texLinearDepth, uv).r;
}

void main(void)
{
    //Sample textures
    vec4 texel = texture(texViewNormal, texcoord);
    float fragDepth = texture(texLinearDepth, texcoord).r;
    
    //Get fragment (pixel) data
    vec3 fragNormal = (texel.rgb - 0.5)*2.0;
    vec3 cameraRay = getCameraRay();
    vec3 fragPos = cameraRay * fragDepth; //In camera frame
    float reflectivity = texel.a;
    
    vec3 reflectedRay = normalize(reflect(cameraRay, normalize(fragNormal)));
    /*vec3 hitFragColor = vec3(1.0);
    
    for(int i=0; i<=100; ++i)
    {
        vec3 coord = fragPos + reflectedRay * i/100.0;
        float depth = getFragDepth(coord.xy);
        
        if(depth < coord.z)
        {
            hitFragColor = getFragColor(coord.xy);
            break;
        }
    }
    
    if(reflectivity > 0.0)
        fragColor = vec4(hitFragColor, 1.0);
    else*/
        fragColor = vec4(getFragColor(cameraRay.xy), 1.0);
}
