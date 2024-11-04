#version 410

in VS_OUT {
	vec2 texcoord;
} fs_in;

out vec4 frag_color;

uniform vec3 color;

void main() {
	vec2 p = fs_in.texcoord * 2.0 - 1.0;
	float f = pow(1.0-max(abs(p.x),abs(p.y)), 0.4545)+0.25;
	frag_color = vec4(color*f, 1.0);
}
