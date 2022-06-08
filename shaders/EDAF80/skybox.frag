#version 410

uniform samplerCube skybox_texture;

in VS_OUT {
	vec3 normal;
} fs_in;

out vec4 frag_color;

void main()
{
	frag_color = texture(skybox_texture, normalize(fs_in.normal));
}
