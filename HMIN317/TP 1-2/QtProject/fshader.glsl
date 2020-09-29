#version 330

uniform sampler2D _GrassTexMap;
uniform sampler2D _RockTexMap;
uniform sampler2D _SnowrockTexMap;

in vec3 v_osPos;
in vec2 v_texCoord0;

#define SNOW_HEIGHT 2.0F
#define ROCK_HEIGHT 1.5F
#define GRASS_HEIGHT 1.0F

void main()
{
    vec3 color;
    if (v_osPos.y > ROCK_HEIGHT)
    {
        vec3 rockColor = texture2D(_RockTexMap, v_texCoord0 * 10).xyz;
        vec3 snowrockColor = texture2D(_SnowrockTexMap, v_texCoord0 * 10).xyz;
        color = mix(rockColor, snowrockColor, (v_osPos.y - ROCK_HEIGHT) / (SNOW_HEIGHT - ROCK_HEIGHT));
    }
    else
    {
        vec3 grassColor = texture2D(_GrassTexMap, v_texCoord0 * 10).xyz;
        vec3 rockColor = texture2D(_RockTexMap, v_texCoord0 * 10).xyz;
        color = mix(grassColor, rockColor, (v_osPos.y - GRASS_HEIGHT) / (ROCK_HEIGHT - GRASS_HEIGHT));
    }

    gl_FragColor = vec4(color, 1.0);
}

