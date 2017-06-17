#version 330 core
#define NUM_SAMPLES (9)
#define FAR_PLANE_Z (-100.0)
#define NUM_SPIRAL_TURNS (7)

out vec4 fragcolor;
uniform sampler2D texPosition;
uniform sampler2D texNormal;
uniform sampler2D texRandom;
uniform float radius;
uniform float projScale;
uniform float bias;
uniform float intensityDivR6; //intensity/radius^6
uniform vec2 viewportSize;

float radius2 = radius * radius;
//float invRadius2 = 1.0/radius2;

// Used for packing Z into the GB channels
float CSZToKey(float z)
{
    return clamp(z * (1.0 / FAR_PLANE_Z), 0.0, 1.0);
}

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

// Returns a unit vector and a screen-space radius for the tap on a unit disk (the caller should scale by the actual disk radius) */
vec2 tapLocation(int sampleNumber, float spinAngle, out float ssR)
{
    // Radius relative to ssR
    float alpha = (float(sampleNumber) + 0.5) * (1.0 / float(NUM_SAMPLES));
    float angle = alpha * (float(NUM_SPIRAL_TURNS) * 6.28) + spinAngle;
    
    ssR = alpha;
    return vec2(cos(angle), sin(angle));
}

// Read the camera-space position of the point at screen-space pixel ssP + unitOffset * ssR.  Assumes length(unitOffset) == 1 */
vec3 getOffsetPosition(ivec2 ssC, vec2 unitOffset, float ssR)
{
    ivec2 ssP = ivec2(ssR * unitOffset) + ssC;
    vec2 ssPCoord = (2.0 * vec2(ssP) + vec2(1.0))/(2.0 * viewportSize);
    return texture(texPosition, ssPCoord).xyz;
}

/* Compute the occlusion due to sample with index \a i about the pixel at \a ssC that corresponds
 to camera-space point \a C with unit normal \a n_C, using maximum screen-space sampling radius \a ssDiskRadius
 
 Four versions of the falloff function are implemented below */
float sampleAO(ivec2 ssC, vec3 C, vec3 n_C, float ssDiskRadius, int tapIndex, float randomPatternRotationAngle)
{
    // Offset on the unit disk, spun for this pixel
    float ssR = 0.0;
    vec2 unitOffset = tapLocation(tapIndex, randomPatternRotationAngle, ssR);
    ssR *= ssDiskRadius;
    
    // The occluding point in camera space
    vec3 Q = getOffsetPosition(ssC, unitOffset, ssR);
    
    vec3 v = Q - C;
    
    float vv = dot(v, v);
    float vn = dot(v, n_C);
    
    const float epsilon = 0.005;
    
    // A: From the HPG12 paper
    // Note large epsilon to avoid overdarkening within cracks
    //return float(vv < radius2) * max((vn - bias) / (epsilon + vv), 0.0) * radius2 * 0.6;
    
    // B: Smoother transition to zero (lowers contrast, smoothing out corners). [Recommended]
    float f = max(radius2 - vv, 0.0);
    return f * f * f * max((vn - bias) / (epsilon + vv), 0.0);
    
    // C: Medium contrast (which looks better at high radii), no division.  Note that the
    // contribution still falls off with radius^2, but we've adjusted the rate in a way that is
    // more computationally efficient and happens to be aesthetically pleasing.
    //return 4.0 * max(1.0 - vv * invRadius2, 0.0) * max(vn - bias, 0.0);
    
    // D: Low contrast, no division operation
    // return 2.0 * float(vv < radius * radius) * max(vn - bias, 0.0);
}

void main()
{
    // Pixel being shaded
    ivec2 ssC = ivec2(gl_FragCoord.xy);
    vec2 ssCCoord = (2.0 * vec2(ssC) + vec2(1.0))/(2.0 * viewportSize);
    
    // World space point being shaded
    vec3 C = texture(texPosition, ssCCoord).xyz;
    packKey(CSZToKey(C.z), fragcolor.gb);
    
    // Hash function used in the HPG12 AlchemyAO paper
    vec3 random = texture(texRandom, ssCCoord * viewportSize/vec2(64.0)).xyz;
    float randomPatternRotationAngle = float((201.0 * random.x + ssC.x * ssC.y) * 11.0);
    //float randomPatternRotationAngle = float((3.0 * (ssC.x + ssC.y) + ssC.x * ssC.y) * 10.0);
    
    // Reconstruct normals from positions. These will lead to 1-pixel black lines
    // at depth discontinuities, however the blur will wipe those out so they are not visible
    // in the final image.
    vec4 ND = texture(texNormal, ssCCoord);
    vec3 n_C = ND.xyz;
    
    // Choose the screen-space sample radius
    // proportional to the projected area of the sphere
    float ssDiskRadius = -projScale * radius / C.z;
    
    float sum = 0.0;
    
    for (int i = 0; i < NUM_SAMPLES; ++i)
        sum += sampleAO(ssC, C, n_C, ssDiskRadius, i, randomPatternRotationAngle);
    
    float A = max(0.0, 1.0 - sum * intensityDivR6 * (5.0 / float(NUM_SAMPLES)));
    
    // Bilateral box-filter over a quad for free, respecting depth edges
    // (the difference that this makes is subtle)
    if (abs(dFdx(C.z)) < 0.02)
        A -= dFdx(A) * (mod(ssC.x, 2) - 0.5);
    
    if (abs(dFdy(C.z)) < 0.02)
        A -= dFdy(A) * (mod(ssC.y, 2) - 0.5);
    
    //gl_FragColor = vec4(cos(randomPatternRotationAngle), sin(randomPatternRotationAngle), 0.0, 1.0);
    fragcolor.r = 1.0 - A;
}

