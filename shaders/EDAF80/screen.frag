#version 410

uniform sampler2D diffuse_texture;
uniform int has_diffuse_texture;

uniform float time;
uniform float flash;

in VS_OUT {
    vec2 texcoord;
} fs_in;

out vec4 frag_color;

float hash12(vec2 p) {
	vec3 p3  = fract(vec3(p.xyx) * .1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}
float onOff(float a, float b, float c) {
	return step(c, sin(time + a*cos(time*b)));
}
float ramp(float y, float start, float end) {
	float inside = step(start,y) - step(end,y);
	float fact = (y-start)/(end-start)*inside;
	return (1.-fact) * inside;
}
float stripes(vec2 uv) {
	float noi = hash12(uv*vec2(0.5,1.) + vec2(1.,3.));
	return ramp(mod(uv.y*4. + time/2.+sin(time + sin(time*0.63)),1.),0.5,0.6)*noi;
}
vec2 screenDistort(vec2 uv) {
	uv -= vec2(.5,.5);
	uv = uv*1.2*(1./1.2+2.*uv.x*uv.x*uv.y*uv.y);
	uv += vec2(.5,.5);
	return uv;
}

void main() {
    if (has_diffuse_texture != 0) {
        vec2 uv = fs_in.texcoord;
        uv = screenDistort(uv);

       	float vigAmt = 3.+.3*sin(time + 5.*cos(time*5.));
        float vignette = (1.-vigAmt*(uv.y-.5)*(uv.y-.5))*(1.-vigAmt*(uv.x-.5)*(uv.x-.5));


        vec2 p = mod(uv * 32.0, vec2(1.0)) * 2.0 - 1.0;
        float f = pow(1.0-max(abs(p.x),abs(p.y)), 0.4545);

        vec3 col = texture(diffuse_texture, uv).rgb;
        col *= f;
        col += stripes(uv) * 0.1;
        col += hash12(uv*2000.0+time) * 0.2;
        // col *= vignette;
        col *= (12.+mod(uv.y*30.+time,1.))/19.;

        col = mix(col, vec3(1.0), flash*flash);

        frag_color = vec4(col, 1.0);
    } else {
        frag_color = vec4(1.0);
    }
}
