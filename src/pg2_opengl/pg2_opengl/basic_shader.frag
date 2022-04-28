#version 460 core
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64 : require 

#define M_PI 3.1415926535897932384626433832795

in vec3 v_normal;
in vec3 v_tangent;
in vec3 unified_normal_ws;
//in vec3 position_lcs;

layout ( location = 0 ) out vec4 FragColor;

mat3x3 TBN;
in vec3 omega_o_es;
in vec3 omega_o_ws;
in vec2 tex_coord;
flat in int mat_index;

// uniforms
uniform sampler2D irradiance_map;
uniform sampler2D prefilteredEnv_map;
uniform sampler2D integration_map;
//uniform sampler2D shadow_map;
uniform sampler2D rma_map; 
uniform sampler2D normal_map; 
uniform sampler2D albedo_map;

vec3 normalToColorSpace(vec3 normal){ return (normal + vec3(1,1,1)) / 2;}

vec2 c2s(vec3 normal) {
	float theta = acos(normal.z);
	float phi = atan(normal.y, normal.x);

	if (normal.y < 0){ phi += 2 * M_PI; }

	float u = phi / (2.0f * M_PI);
	float v = theta / M_PI;

	return vec2(u, v);
}

vec3 getIntegration(float ct_o, float roughness) {
	return texture(integration_map, vec2(ct_o, roughness)).rgb;
}

vec3 getIrradiance(vec3 normal) {
	return texture(irradiance_map, c2s(normal)).rgb;
}

vec3 getPrefEnv(float roughness, vec3 normal) {
	const float maxLevel = 6;
	vec3 omega_i_ws = reflect( omega_o_ws ,  normalize(normal));
	return textureLod(prefilteredEnv_map, c2s(omega_i_ws), roughness * maxLevel).rgb;
}

vec3 getLocalNormal() {
	return texture(normal_map, tex_coord).bgr;
}

vec3 getAlbedo(){
	return texture(albedo_map, tex_coord).rgb;
}

float Fresnell(float ct_o, float n1, float n2 ) {
	if (ct_o == 0) return 0;
	float f_0 = pow((n1-n2)/(n1+n2), 2);
	return f_0 + (1 - f_0) * pow(1 - ct_o, 5);
}

vec3 getRMA() {
	return texture(rma_map, tex_coord).rgb;
}

mat3 getTBNMatrix() {
	vec3 n = normalize(unified_normal_ws);
	vec3 t = normalize(v_tangent - dot(v_tangent, n) * n);
	vec3 b = normalize(cross(n, t));
	return transpose(mat3(t, b, n));
}

vec3 tonemapping(vec3 color, float gamma , float exposure){
	color = pow(color, vec3(1.0f)/gamma);
	color *= exposure;
	color = color/(color+vec3(1));
	return color;
}

vec3 getPBRShader(){
	vec3 rma = getRMA();
	vec3 albedo = getAlbedo();  
	float ambient_occlusion = rma.b;
	float metalic = rma.g;
	float roughness = rma.r;
	float alpha = pow(roughness, 2);
	vec3 local_normal = getTBNMatrix() * (2*getLocalNormal() - vec3(1.0f));

	if (dot(local_normal, omega_o_ws) < 0.0f) {
		local_normal *= -1.0f;
	}
	
	float cosinus_theta_o = dot(local_normal, omega_o_ws);
	

	float Fo = Fresnell(cosinus_theta_o, 1.0f, 4.0); // 4.0 = IOR
	float Fd = (1 - Fo) * (1 - metalic);
	vec3 sb = getIntegration(cosinus_theta_o,roughness);

	vec3 Ld = Fd*albedo * getIrradiance(local_normal); 
	vec3 Lr = getPrefEnv(roughness, local_normal);

	vec3 color =  Ld + (Fo*sb.x + sb.y) * Lr;
	vec3 toned_color = tonemapping(color, 1.5f, 2.2f);
	vec3 final_color = toned_color.xyz * ambient_occlusion;

	return final_color;
}


void main( void ) {

	
	//vec3 final_color = normalToColorSpace(unified_normal_ws);
	vec3 final_color = getPBRShader();


	FragColor = vec4(final_color, 1.0f);

}

/*
float getShadow(float bias, const int r) {
	vec2 shadow_texel_size = 1.0f / textureSize(shadow_map, 0);
	float shadow = 0.0f;

	for (int y = -r; y <= r; ++y) {
		for (int x = -r; x <= r; ++x) {
			vec2 a_tc = (position_lcs.xy + vec2(1.0f)) * 0.5f;
			a_tc += vec2(x, y) * shadow_texel_size;
			float depth = texture(shadow_map, a_tc).r * 2.0f - 1.0f;
			shadow += (depth + bias >= position_lcs.z) ? 1.0f : 0.25f;
		}
	}
	shadow *= 1.0f / ((2 * r + 1) * (2 * r + 1));

	return shadow;
}*/
	//vec3 n_ls = normalize((2*normalBumps)-vec3(1.0f)); // slide 73
	