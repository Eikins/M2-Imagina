#version 330

uniform mat4 mat_ObjectToWorld;
uniform mat4 mat_WorldToView;
uniform mat4 mat_ViewToClip;

uniform sampler2D _HeightMap;

in vec3 position;
in vec2 texCoord0;

out vec3 v_osPos;
out vec2 v_texCoord0;

#define MAX_HEIGHT 2.0F

vec4 ObjectToWorld(in vec4 osVec)
{
    return mat_ObjectToWorld * osVec;
}

vec4 WorldToView(in vec4 wsVec)
{
    return mat_WorldToView * wsVec;
}

vec4 ViewToClip(in vec4 vsVec)
{
    return mat_ViewToClip * vsVec;
}

void main()
{
    v_texCoord0 = texCoord0;

    vec4 pos = vec4(position, 1.0);
    // Terrain height
    pos.y += texture2D(_HeightMap, texCoord0).r * MAX_HEIGHT;
    v_osPos = pos.xyz;

    pos = ObjectToWorld(pos);
    pos = WorldToView(pos);
    gl_Position = ViewToClip(pos);
}
