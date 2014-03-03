#version 120
/*
    :copyright: 2011 by Florian Boesch <pyalot@gmail.com>.
    :license: GNU AGPL3, see LICENSE for more details.
*/

uniform sampler2D source, texNormal;
uniform float distance_factor, depth_power;
uniform float normal_factor, normal_power;
uniform vec2 viewport, axis;

void main(void)
{
    vec2 off = (axis*1.0)/viewport;

    vec4 da = texture2D(texNormal, gl_TexCoord[0].xy);
    vec4 db = texture2D(texNormal, gl_TexCoord[0].xy+off);
    vec4 dc = texture2D(texNormal, gl_TexCoord[0].xy-off);
    da.w = (da.w-1.0)/999999.0;
    db.w = (db.w-1.0)/999999.0;
    dc.w = (dc.w-1.0)/999999.0;

    float fdb = 1.0-smoothstep(0.0, 1.0, distance_factor*abs(db.w-da.w));
    float fdc = 1.0-smoothstep(0.0, 1.0, distance_factor*abs(dc.w-da.w));
    float fnb = max(0.0, pow(dot(da.xyz, db.xyz)*normal_factor, normal_power));
    float fnc = max(0.0, pow(dot(da.xyz, dc.xyz)*normal_factor, normal_power));

    vec4 a = texture2D(source, gl_TexCoord[0].xy);
    vec4 b = texture2D(source, gl_TexCoord[0].xy+off)*1.0*fdb*fnb;
    vec4 c = texture2D(source, gl_TexCoord[0].xy-off)*1.0*fdc*fnc;
    vec4 result = a+b+c;
    result /= (1.0 + 1.0*fdb*fnb + 1.0*fdc*fnc);
    gl_FragColor = result;
}