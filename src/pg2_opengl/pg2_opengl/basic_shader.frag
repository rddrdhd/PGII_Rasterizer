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

vec3 getNormalShade(vec3 normal){ return (normal + vec3(1,1,1)) / 2;}

vec2 c2s(vec3 normal) {
	float theta = acos(normal.z);
	float phi = atan(normal.y, normal.x);

	if (normal.y < 0){ phi += 2 * M_PI; }

	float u = phi / (2.0f * M_PI);
	float v = theta / M_PI;

	return vec2(u, v);
}

vec3 getIntegration(float alpha, float ct_o) {
	vec2 uv = vec2(ct_o, alpha);
	return texture(integration_map, uv).rgb;
}

vec3 getIrradiance() {
	vec2 uv = c2s(v_normal);
	// if(uv.x > uv.y){ return vec3(0,uv.x,0); }else{ return vec3(uv.y,0,uv.y);}
	vec3 tex_color = texture(irradiance_map, uv).rgb;
	// if(tex_color == vec3(0,0,0)){return vec3(1,0,0);}else{return vec3(0,1,0);}
	return tex_color ; 
}

vec3 getPrefEnv(float alpha) {
	float roughness = alpha * alpha;
	const float maxLevel = 6;
	
	vec3 omega_i_ws = reflect( -omega_o_ws ,  normalize(unified_normal_ws));
	vec2 uv = c2s(omega_i_ws);
	vec3 tex_color = (textureLod(prefilteredEnv_map, uv, roughness * maxLevel).rgb);
	// vec3 tex_color = texture(prefilteredEnv_map,uv).rgb; 
	return tex_color;
}

vec3 getNomalBumps() {
	vec3 tex_color = texture(normal_map, tex_coord).rgb;
	return tex_color;
}
vec3 getAlbedo(){
	vec3 tex_color = texture(albedo_map, tex_coord).rgb;
	return tex_color;
	} 
float Fresnell(float ct_o, float n1, float n2 ) {
	if (ct_o == 0) return 0;
	float f_0 = pow((n1-n2)/(n1+n2), 2);
	return f_0 + (1 - f_0) * pow(1 - ct_o, 5);
}

vec3 getRMA() {
	vec3 tex_color = texture(rma_map, tex_coord).rgb;
	return tex_color;
}
mat3 getTBNMatrix() {
	vec3 n = normalize(v_normal);
	//vec3 t = normalize(v_tangent - dot(v_tangent, n) * n);
	vec3 t= normalize(v_tangent);
	vec3 b = normalize(cross(n, t));
	return mat3(t, b, n);
}

//normala crossprodukt tangetna, dostanu bitangentu
//tbn * normala z textury
void main( void ) {
	vec3 rma = getRMA();
	// blue  = ambient occlusion = b
	// red = metalicity = r
	// green = roughness = g
	float metalicity = rma.r;
	float ambient_occ = rma.b;
	float roughness = rma.g;
	float alpha = pow(roughness,2); // <0,1> where 0 = mirror, 1 = dim
	vec3 albedo = getAlbedo(); // gray 
	vec3 normalBumps = getTBNMatrix()  * getNomalBumps();
	
	/* NORMAL shader */
	vec3 normal_shade  = getNormalShade(unified_normal_ws);

	/* PBR shader */
	float ct_o = dot(unified_normal_ws, omega_o_ws);
	float k_s = Fresnell(ct_o, 1.0f, 4.0); // 4.0 = IOR
	float k_d = (1 - k_s) * (1 - metalicity);

	vec3 sb = getIntegration(alpha, ct_o);
	vec3 Ld = albedo * getIrradiance(); 
	vec3 Lr = getPrefEnv(alpha);

	vec3 color =  k_d*Ld + (k_s*sb.x + sb.y) * Lr;
	FragColor = vec4(color * normalBumps ,1.0f);
	//FragColor = vec4(color.rgb, 1.0f ) * ambient_occ; krat getShadow( 0.001f, 10);
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
