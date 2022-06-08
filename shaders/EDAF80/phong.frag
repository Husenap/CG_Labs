#version 410

uniform int use_normal_mapping;
uniform vec3 light_position;
uniform vec3 camera_position;
uniform vec3 diffuse_colour;
uniform vec3 specular_colour;
uniform vec3 ambient_colour;
uniform float shininess_value;

uniform samplerCube skybox_texture;
uniform sampler2D albedo_texture;
uniform sampler2D normal_texture;
uniform sampler2D roughness_texture;

float saturate(float f){
	return clamp(f, 0.0, 1.0);
}

in VS_OUT {
	vec3 vertex;
	vec3 normal;
	vec2 texcoord;
	vec3 tangent;
	vec3 binormal;
} fs_in;

out vec4 frag_color;

void main() {
	vec3 n = normalize(fs_in.normal);

	if(use_normal_mapping != 0){
		vec3 normalColor = texture(normal_texture, fs_in.texcoord).rgb * 2.0 - 1.0;
		mat3 TBN = mat3(normalize(fs_in.tangent),
										normalize(fs_in.binormal),
										normalize(fs_in.normal));
		n = normalize(TBN * normalColor);
	}

	vec3 L = normalize(light_position - fs_in.vertex);
	vec3 V = normalize(camera_position - fs_in.vertex);

	float shininess = 1.0 / (texture(roughness_texture, fs_in.texcoord).r / shininess_value);

	float diffuse =  saturate(dot(n, L) * 0.75 + 0.25);
	float specular =  pow(saturate(dot(V, reflect(-L, n))), shininess);

	vec3 albedoColor = texture(albedo_texture, fs_in.texcoord).rgb;
	vec3 skyboxColor = textureLod(skybox_texture, reflect(-V, n), mix(0.0, 9.0, saturate(1.0 - shininess/50.0))).rgb;

	frag_color = vec4(albedoColor, 1.0) * vec4(
			ambient_colour * skyboxColor +
			diffuse_colour * diffuse +
			specular_colour * specular,
			1.0);
}
