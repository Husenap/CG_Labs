#version 410

layout (location = 0) in vec3 vertex;
layout (location = 2) in vec3 texcoord;

uniform mat4 vertex_model_to_world;
uniform mat4 normal_model_to_world;
uniform mat4 vertex_world_to_clip;
uniform float time;

out VS_OUT {
	vec3 vertex;
	vec2 texcoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
} vs_out;

float wave(in vec2 position, in vec2 direction, in float amplitude, in float frequency, in float phase, in float sharpness, in float time){
	float omega = dot(position, direction) * frequency + time * phase;
	return amplitude *
		pow(sin(omega) * 0.5 + 0.5, sharpness);
}
float waveD(in vec2 position, in vec2 direction, in float amplitude, in float frequency, in float phase, in float sharpness, in float time, in float component){
	float omega = dot(position, direction) * frequency + phase * time; 
	return 0.5 * sharpness * frequency * amplitude *
		pow(sin(omega) * 0.5 + 0.5, sharpness - 1) *
		cos(omega) * component;
}
float waveDx(in vec2 position, in vec2 direction, in float amplitude, in float frequency, in float phase, in float sharpness, in float time){
	return waveD(position, direction, amplitude, frequency, phase, sharpness, time, direction.x);
}
float waveDz(in vec2 position, in vec2 direction, in float amplitude, in float frequency, in float phase, in float sharpness, in float time){
	return waveD(position, direction, amplitude, frequency, phase, sharpness, time, direction.y);
}

void applyWave(inout vec3 vertex, inout vec3 normal, in vec2 direction, in float amplitude, in float frequency, in float phase, in float sharpness, in float time){
	vertex.y += wave(vertex.xz, direction, amplitude, frequency, phase, sharpness, time);
	normal.x -= waveDx(vertex.xz, direction, amplitude, frequency, phase, sharpness, time);
	normal.z -= waveDz(vertex.xz, direction, amplitude, frequency, phase, sharpness, time);
}
void applyWaves(inout vec3 vertex, inout vec3 normal){
	applyWave(vertex, normal, vec2(-1.0, 0.0), 1.0, 0.2, 0.5, 2.0, time);
	applyWave(vertex, normal, vec2(-0.7, 0.7), 0.5, 0.4, 1.3, 2.0, time);
}

void main()
{
	vec3 displaced_vertex = vertex;
	vec3 normal = vec3(0.0, 1.0, 0.0);
	applyWaves(displaced_vertex, normal);
	vec3 tangent = normalize(vec3(1.0, -normal.x, 0.0));
	vec3 binormal = normalize(vec3(0.0, -normal.z, 1.0));
	normal = normalize(normal);

	vs_out.vertex = vec3(vertex_model_to_world * vec4(displaced_vertex, 1.0));
	vs_out.texcoord = texcoord.xy;
	vs_out.normal = vec3(normal_model_to_world * vec4(normal, 0.0));
	vs_out.tangent = vec3(normal_model_to_world * vec4(tangent, 0.0));
	vs_out.binormal = vec3(normal_model_to_world * vec4(binormal, 0.0));

	gl_Position = vertex_world_to_clip * vertex_model_to_world * vec4(displaced_vertex, 1.0);
}
