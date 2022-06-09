#version 410

uniform sampler2D water_texture;
uniform samplerCube skybox_texture;
uniform float time;
uniform vec3 camera_position;
uniform vec3 water_color_deep;
uniform vec3 water_color_shallow;

in VS_OUT {
	vec3 vertex;
	vec2 texcoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
} fs_in;

out vec4 frag_color;

void main()
{
	vec2 texScale = vec2(8.0, 4.0);
	vec2 normalSpeed = vec2(-0.05, 0.0);
	float normalTime = mod(time, 100.0);
	vec2 normalCoord0 = fs_in.texcoord * texScale + normalTime * normalSpeed;
	vec2 normalCoord1 = fs_in.texcoord * texScale * 2.0 + normalTime * normalSpeed * 4.0;
	vec2 normalCoord2 = fs_in.texcoord * texScale * 4.0 + normalTime * normalSpeed * 8.0;
	mat3 TBN = mat3(normalize(fs_in.tangent),
									normalize(fs_in.binormal),
									normalize(fs_in.normal));
	vec3 n = normalize(
		TBN * ((texture(water_texture, normalCoord0).xyz * 2.0 - 1.0) + 
		(texture(water_texture, normalCoord1).xyz * 2.0 - 1.0) + 
		(texture(water_texture, normalCoord2).xyz * 2.0 - 1.0))
	) * (gl_FrontFacing ? 1.0 : -1.0);

	vec3 V = normalize(camera_position - fs_in.vertex);
	vec3 R = reflect(-V, n);

	const float R0 = 0.020059;

	vec3 bump = texture(water_texture, fs_in.texcoord).rgb;

	float facing = 1.0 - max(dot(V, n), 0.0);
	float fresnel = R0 + (1.0 - R0) * pow(facing, 5.0);

	vec3 color_water = mix(water_color_deep, water_color_shallow, facing);
	vec3 reflection = texture(skybox_texture, R).rgb;
	vec3 refraction = texture(skybox_texture, refract(-V, n, gl_FrontFacing ? 1.0/1.33 : 1.33)).rgb;

	frag_color = vec4(color_water + reflection * fresnel + refraction * (1.0 - fresnel), 1.0);
}
