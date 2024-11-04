#version 410

struct ViewProjTransforms
{
    mat4 view_projection;
    mat4 view_projection_inverse;
};

layout (std140) uniform CameraViewProjTransforms
{
    ViewProjTransforms camera;
};

layout (std140) uniform LightViewProjTransforms
{
    ViewProjTransforms lights[4];
};

uniform int light_index;

uniform sampler2D depth_texture;
uniform sampler2D normal_texture;
uniform sampler2D shadow_texture;

uniform vec2 inverse_screen_resolution;

uniform vec3 camera_position;

uniform vec3 light_color;
uniform vec3 light_position;
uniform vec3 light_direction;
uniform float light_intensity;
uniform float light_angle_falloff;

layout (location = 0) out vec4 light_diffuse_contribution;
layout (location = 1) out vec4 light_specular_contribution;

const float PI = 3.1415926535898;
const float MinCosine = cos(PI * 0.2);
const float ShadowEpsilon = 0.0001;

void main() {
    vec2 shadowmap_texel_size = 1.0f / textureSize(shadow_texture, 0);

    vec2 gbuffer_texel = gl_FragCoord.xy * inverse_screen_resolution;

    float gbuffer_depth = texelFetch(depth_texture, ivec2(gl_FragCoord.xy), 0).r * 2.0 - 1.0;
    vec4 homogeneous_position = camera.view_projection_inverse * vec4(gbuffer_texel * 2.0 - 1.0, gbuffer_depth, 1.0);
    vec3 position = homogeneous_position.xyz / homogeneous_position.w;

    vec3 N = texture(normal_texture, gbuffer_texel).xyz * 2.0 - 1.0;
    vec3 L = normalize(light_position - position);
    vec3 V = normalize(camera_position - position);

    vec4 homogeneous_shadow_position = lights[light_index].view_projection * vec4(position, 1.0);
    vec3 shadow_position = homogeneous_shadow_position.xyz / homogeneous_shadow_position.w;
    float shadow = 9.0;
    for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
            float shadow_depth = texture(shadow_texture, shadow_position.xy * 0.5 + 0.5 + vec2(dx, dy) * shadowmap_texel_size, 0).r * 2.0 - 1.0;
            if (shadow_position.z > shadow_depth + ShadowEpsilon) {
                shadow -= 1.0;
            }
        }
    }
    shadow /= 9.0;

    float distance_falloff = dot((light_position - position), (light_position - position));
    float angle_falloff = (1.0f - MinCosine) / max(dot(-L, light_direction) - MinCosine, 0.0);

    float attenuation = distance_falloff * angle_falloff;
    vec3 col = shadow * light_intensity * light_color / attenuation;

    light_diffuse_contribution = vec4(col * max(0, dot(N, L)), 1.0);
    light_specular_contribution = vec4(col * max(0, dot(V, reflect(-L, N))), 1.0);
}
