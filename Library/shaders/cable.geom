/*    
    Copyright (c) 2025 Patryk Cieslak. All rights reserved.

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

layout(lines) in;
layout(triangle_strip, max_vertices = 18) out;

in VS_OUT {
    float coord;
    vec3 localTangent;
} gs_in[];

uniform mat4 MVP;
uniform mat4 M;
uniform mat3 N;
uniform mat3 MV;
uniform float FC;
uniform float cableRadius;

out vec3 normal;
out vec4 fragPos;
out vec3 eyeSpaceNormal;
out float logz;
out vec2 texCoord;
out mat3 TBN;

void main() {
    vec3 p0 = gl_in[0].gl_Position.xyz;
    vec3 p1 = gl_in[1].gl_Position.xyz;

    // Calculate local coordinate frames at both ends
    vec3 t0 = gs_in[0].localTangent;
    vec3 t1 = gs_in[1].localTangent;
    
    vec3 up1 = vec3(0.0, 1.0, 0.0);
    if (abs(dot(t0, up1)) > 0.99) {
        up1 = vec3(1.0, 0.0, 0.0);
    }
    vec3 right1 = normalize(cross(t0, up1));
    up1 = normalize(cross(right1, t0));

    vec3 up2 = vec3(0.0, 1.0, 0.0);
    if (abs(dot(t1, up2)) > 0.99) {
        up2 = vec3(1.0, 0.0, 0.0);
    }
    vec3 right2 = normalize(cross(t1, up2));
    up2 = normalize(cross(right2, t1));

    // Emit vertices around the cable
    int segments = 8; // Number of segments around the cable
    for (int i = 0; i <= segments; ++i) {
        float angle = float(i) / float(segments) * 2.0 * 3.14159265;
        vec3 offset1 = cableRadius * (cos(angle) * right1 + sin(angle) * up1);
        vec3 offset2 = cableRadius * (cos(angle) * right2 + sin(angle) * up2);

        // Emit vertex 1 and set varyings
        vec3 vt = p0 + offset1;
        normal = normalize(N * offset1);
        eyeSpaceNormal = normalize(MV * offset1);
        vec3 tangent = normalize(N * t0);
        vec3 bitangent = cross(normal, tangent);
        TBN = mat3(tangent, bitangent, normal);
        fragPos = M * vec4(vt, 1.0);
        gl_Position = MVP * vec4(vt, 1.0);
        gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * 2.0 * FC - 1.0;
        logz = 1.0 + gl_Position.w;
        texCoord = vec2(float(i) / float(segments), gs_in[0].coord);
        EmitVertex();

        // Emit vertex 2 and set varyings
        vt = p1 + offset2;
        normal = normalize(N * offset2);
        eyeSpaceNormal = normalize(MV * offset2);
        tangent = normalize(N * t1);
        bitangent = cross(normal, tangent);
        TBN = mat3(tangent, bitangent, normal);
        fragPos = M * vec4(vt, 1.0);
        gl_Position = MVP * vec4(vt, 1.0);
        gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * 2.0 * FC - 1.0;
        logz = 1.0 + gl_Position.w;
        texCoord = vec2(float(i) / float(segments), gs_in[1].coord);
        EmitVertex();
    }
    EndPrimitive();
}