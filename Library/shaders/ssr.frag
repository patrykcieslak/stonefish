/*    
    Copyright (c) 2019 Patryk Cieslak. All rights reserved.

    This file is a part of Stonefish.

    Stonefish is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Stonefish is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#version 330

in vec2 texcoord;
out vec4 fragColor;

uniform sampler2D texColor;               //Scene color
uniform sampler2D texViewNormal;          //Surface normal in view space + reflection strength
uniform sampler2D texLinearDepth;         //Depth of front faces
uniform sampler2D texLinearBackfaceDepth; //Depth of back faces
uniform vec2 viewportSize;              //Dimensions of viewport
uniform vec2 invViewportSize;           //One over dimensions of viewport
uniform vec4 projInfo;
uniform mat4 P;                         //Camera projection matrix
uniform mat4 invP;                      //Inverse of camera projection matrix
uniform float near;                     //Camera near plane
uniform float far;                      //Camera far plane
uniform int maxIterations;              // maximum ray iterations
uniform int maxBinarySearchIterations;  // maximum binary search refinement iterations
uniform float pixelZSize;               // Z size in camera space of a pixel in the depth buffer
uniform float pixelStride;              // number of pixels per ray step close to camera
uniform float pixelStrideZCutoff;       // ray origin Z at this distance will have a pixel stride of 1.0
uniform float maxRayDistance;           // maximum distance of a ray
uniform float screenEdgeFadeStart;      // distance to screen edge that ray hits will start to fade (0.0 -> 1.0)
uniform float eyeFadeStart;             // ray direction's Z that ray hits will start to fade (0.0 -> 1.0)
uniform float eyeFadeEnd;               // ray direction's Z that ray hits will be cut (0.0 -> 1.0)

vec3 positionFromDepth(vec2 uv, float depth)
{
    return vec3((uv * projInfo.xy + projInfo.zw) * depth, -depth);
}

void swapIfBigger(inout float aa, inout float bb)
{
    if(aa > bb)
    {
        float tmp = aa;
        aa = bb;
        bb = tmp;
    }
}

bool rayIntersectsDepthBF(float zA, float zB, vec2 uv)
{
    float cameraZ = -texture(texLinearDepth, uv).r;
    float backZ = -texture(texLinearBackfaceDepth, uv).r;
    return zB <= cameraZ && zA >= backZ - pixelZSize;
}

float distanceSquared(vec2 a, vec2 b)
{
    a -= b;
    return dot(a, a);
}

// Trace a ray in screenspace from rayOrigin (in camera space) pointing in rayDirection (in camera space)
// using jitter to offset the ray based on (jitter * _PixelStride).
//
// Returns true if the ray hits a pixel in the depth buffer
// and outputs the hitPixel (in UV space), the hitPoint (in camera space) and the number
// of iterations it took to get there.
//
// Based on Morgan McGuire & Mike Mara's GLSL implementation:
// http://casual-effects.blogspot.com/2014/08/screen-space-ray-tracing.html
bool traceScreenSpaceRay(vec3 rayOrigin,
                         vec3 rayDirection,
                         float jitter,
                         out vec2 hitPixel,
                         out vec3 hitPoint,
                         out int iterationCount,
                         bool debugHalf)
{
    // Clip to the near plane
    float rayLength = ((rayOrigin.z + rayDirection.z * maxRayDistance) > -near) ? (-near - rayOrigin.z)/rayDirection.z : maxRayDistance;
    vec3 rayEnd = rayOrigin + rayDirection * rayLength;
    
    // Project into homogeneous clip space
    vec4 H0 = P * vec4(rayOrigin, 1.0);
    vec4 H1 = P * vec4(rayEnd, 1.0);
    
    // The interpolated homogeneous version of the camera-space points
    float k0 = 1.0 / H0.w;
    float k1 = 1.0 / H1.w;
    vec3 Q0 = rayOrigin * k0;
    vec3 Q1 = rayEnd * k1;
    
    // Screen-space endpoints
    vec2 P0 = H0.xy * k0;
    vec2 P1 = H1.xy * k1;
    
    // If the line is degenerate, make it cover at least one pixel
    // to avoid handling zero-pixel extent as a special case later
    P1 += (distanceSquared(P1, P0) < 0.0001) ? 0.01 : 0.0;
    vec2 delta = P1 - P0;
    
    // Permute so that the primary iteration is in x to collapse
    // all quadrant-specific DDA cases later
    bool permute = false;
    if(abs(delta.x) < abs(delta.y))
    {
        // This is a more-vertical line
        permute = true;
        delta = delta.yx;
        P0 = P0.yx;
        P1 = P1.yx;
    }
    float stepDir = sign(delta.x);
    float invdx = stepDir / delta.x;
    
    // Track the derivatives of Q and k
    vec3  dQ = (Q1 - Q0) * invdx;
    float dk = (k1 - k0) * invdx;
    vec2  dP = vec2(stepDir, delta.y * invdx);
    
    // Calculate pixel stride based on distance of ray origin from camera.
    // Since perspective means distant objects will be smaller in screen space
    // we can use this to have higher quality reflections for far away objects
    // while still using a large pixel stride for near objects (and increase performance)
    // this also helps mitigate artifacts on distant reflections when we use a large
    // pixel stride.
    float strideScaler = 1.0 - min(1.0, -rayOrigin.z/pixelStrideZCutoff);
    float pxStride = 1.0 + strideScaler * pixelStride;
    
    // Scale derivatives by the desired pixel stride and then
    // offset the starting values by the jitter fraction
    dP *= pxStride; dQ *= pxStride; dk *= pxStride;
    P0 += dP * jitter; Q0 += dQ * jitter; k0 += dk * jitter;
    
    int i;
    float zA = 0.0;
    float zB = 0.0;
    
    // Track ray step and derivatives in a vec4 to parallelize
    vec4 pqk = vec4(P0, Q0.z, k0);
    vec4 dPQK = vec4(dP, dQ.z, dk);
    bool intersect = false;
    
    for(i = 0; i < maxIterations && intersect == false; ++i)
    {
        pqk += dPQK;
        
        zA = zB;
        zB = (dPQK.z * 0.5 + pqk.z) / (dPQK.w * 0.5 + pqk.w);
        swapIfBigger(zB, zA);
        
        hitPixel = permute ? pqk.yx : pqk.xy;
        hitPixel *= invViewportSize;
        
        intersect = rayIntersectsDepthBF(zA, zB, hitPixel);
    }
    
    // Binary search refinement
    if(pxStride > 1.0 && intersect)
    {
        pqk -= dPQK;
        dPQK /= pxStride;
        
        float originalStride = pxStride * 0.5;
        float stride = originalStride;
        
        zA = pqk.z / pqk.w;
        zB = zA;
        
        for(int j = 0; j<maxBinarySearchIterations; ++j)
        {
            pqk += dPQK * stride;
            
            zA = zB;
            zB = (dPQK.z * -0.5 + pqk.z) / (dPQK.w * -0.5 + pqk.w);
            swapIfBigger( zB, zA);
            
            hitPixel = permute ? pqk.yx : pqk.xy;
            hitPixel *= invViewportSize;
            
            originalStride *= 0.5;
            stride = rayIntersectsDepthBF( zA, zB, hitPixel) ? -originalStride : originalStride;
        }
    }
    
    Q0.xy += dQ.xy * i;
    Q0.z = pqk.z;
    hitPoint = Q0 / pqk.w;
    iterationCount = i;
    
    return intersect;
}

float calculateAlphaForIntersection(bool intersect,
                                    int iterationCount,
                                    float reflectionStrength,
                                    vec2 hitPixel,
                                    vec3 hitPoint,
                                    vec3 vsRayOrigin,
                                    vec3 vsRayDirection)
{
    float alpha = min(1.0, reflectionStrength);
    
    // Fade ray hits that approach the maximum iterations
    alpha *= 1.0 - float(iterationCount)/float(maxIterations);
    
    // Fade ray hits that approach the screen edge
    float screenFade = screenEdgeFadeStart;
    vec2 hitPixelNDC = (hitPixel * 2.0 - 1.0);
    float maxDimension = min(1.0, max(abs(hitPixelNDC.x), abs(hitPixelNDC.y)));
    alpha *= 1.0 - (max(0.0, maxDimension - screenFade)/(1.0 - screenFade));
    
    // Fade ray hits base on how much they face the camera
    float fadeStart = eyeFadeStart;
    float fadeEnd = eyeFadeEnd;
    swapIfBigger(fadeStart, fadeEnd);
    
    float eyeDirection = clamp(vsRayDirection.z, fadeStart, fadeEnd);
    alpha *= 1.0 - ((eyeDirection - fadeStart) / (fadeEnd - fadeStart));
    
    // Fade ray hits based on distance from ray origin
    alpha *= 1.0 - clamp( distance( vsRayOrigin, hitPoint) / maxRayDistance, 0.0, 1.0);
    
    alpha *= float(intersect);
    
    return alpha;
}

//Schlick's approximation to Fresnel function (assuming equal IOR for all wavelengths)
// R0 = ((n1-n2)/(n1+n2))^2
float fresnelSchlick(float cosTheta, float R0)
{
	return R0 + (1.0 - R0) * pow(1.0 - cosTheta, 5.0);
}

void main(void)
{
    //Get view space normal
    vec4 normalReflection = texture(texViewNormal, texcoord);
    vec3 vsNormal = normalize(normalReflection.xyz * 2.0 - 1.0); //View space normal
    float reflectionStrength = normalReflection.w; //Reflectivity
    
    //If pixel belongs to sky there is no reflection! --> ADD REFLECTION OF THE SKY!!!!
    if(dot(vsNormal, vsNormal) < 0.01 || reflectionStrength < 0.01) 
        fragColor = vec4(texture(texColor, texcoord).rgb, 0.0);
    else
    {    
        //Get eye space position
        float depth = texture(texLinearDepth, texcoord).r;
        vec3 vsRayOrigin = positionFromDepth(texcoord, depth); //Origin of reflected ray
        
        //Calculate reflected direction
        vec3 vsRayDirection = normalize(reflect(normalize(vsRayOrigin), vsNormal)); //Direction of reflected ray
        
        //Calculate jitter
        float c = (gl_FragCoord.x + gl_FragCoord.y) * 0.25;
        float jitter = fract(c);
        
        //Trace ray
        vec2 hitPixel;
        vec3 hitPoint;
        int iterationCount;
        bool intersect = traceScreenSpaceRay(vsRayOrigin, vsRayDirection, jitter, hitPixel, hitPoint, iterationCount, texcoord.x > 0.5);
        float alpha = calculateAlphaForIntersection(intersect, iterationCount, reflectionStrength, hitPixel, hitPoint, vsRayOrigin, vsRayDirection);
        alpha *= fresnelSchlick(max(vsNormal.z, 0.0), 0.02);

        //Add sky fallback or underwater background fallback
        hitPixel = mix(texcoord, hitPixel, float(intersect));
        fragColor = vec4(mix(texture(texColor, texcoord).rgb, texture(texColor, hitPixel).rgb, alpha), alpha);
        //fragColor = vec4(texture(texColor, hitPixel).rgb, alpha * reflectionStrength);
        //fragColor = vec4(vec3(alpha * reflectionStrength), 1.0);
        //fragColor = vec4(100*(vsNormal*0.5+0.5), 1.0);
        //fragColor = vec4(vec3(-vsRayOrigin.z), 1.0);
        //fragColor = vec4(normalReflection.xyz, 1.0);
        //fragColor = vec4(texture(texLinearDepth, texcoord).rrr, 1.0);
        //fragColor = vec4(texture(texColor, hitPixel).rgb, 1.0);
        //fragColor = vec4(100*vec3(reflectionStrength), 1.0);
        //fragColor = vec4(vec3(gl_FragCoord.z), 1.0);
        //fragColor = vec4(vec3(float(iterationCount)/maxIterations), 1.0);
        //fragColor = vec4(100*vec3(depth), 1.0);
        //fragColor = vec4(texture(texColor, texcoord).rgb, 1.0);
    }
}
