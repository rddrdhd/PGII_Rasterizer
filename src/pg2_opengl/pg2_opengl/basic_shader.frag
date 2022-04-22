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
in vec3 omega_o;
in vec2 tex_coord;
in vec3 reflected_normal_ws;
flat in int mat_index;

// uniforms
uniform sampler2D irradiance_map;
uniform sampler2D prefilteredEnv_map;
uniform sampler2D integration_map;
//uniform sampler2D shadow_map;
//uniform mat4 MVN;

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
	float x = (log(alpha) + 7.0f) / 7.0f;
	vec2 uv = vec2(ct_o, x);
	return texture(integration_map, uv).rgb;
}
vec3 getIrradiance() {
	vec2 uv = c2s(unified_normal_ws);
	//if(uv.x > uv.y){ return vec3(0,uv.x,0); }else{ return vec3(uv.y,0,uv.y);}
	vec3 tex_color = texture(irradiance_map, uv).rgb;
	//if(tex_color == vec3(0,0,0)){return vec3(1,0,0);}else{return vec3(0,1,0);}
	return tex_color ; 
}

struct Material {
	vec3 diffuse;
	uint64_t tex_diffuse;

	vec3 rma;
	uint64_t tex_rma;

	vec3 norm;
	uint64_t tex_norm;
};

layout ( std430, binding = 0) readonly buffer Materials {
	Material materials[]; // only the last member can be unsized array
};

mat3 getTBN() {
	vec3 n = normalize(v_normal);
	vec3 t = normalize(v_tangent - dot(v_tangent, n) * n);
	vec3 b = normalize(cross(n, t));
	return mat3(t, b, n);
}

vec3 getPrefEnv(float roughness) {
	const float maxLevel = 6;
	
	//float x = (log(alpha) + 6.0f) / 6.0f;
	vec2 uv = c2s(reflected_normal_ws);
	
	//if(uv.x > uv.y){ return vec3(0,uv.x,0); }else{ return vec3(uv.y,0,uv.y);}
	vec3 tex_color = (textureLod(prefilteredEnv_map, uv, roughness * maxLevel).rgb);
	//vec3 tex_color = (textureLod(prefilteredEnv_map, uv, 2).rgb);
	//vec3 tex_color = texture(prefilteredEnv_map,uv).rgb;
	//if(tex_color == vec3(0,0,0)){return vec3(1,0,0);}else{return vec3(0,1,0);}
	return tex_color;
}

void main( void ) {

	//TESTING
	vec2 tex_coords = vec2(gl_FragCoord.x / 640.0f, gl_FragCoord.y /480.0f);
	FragColor = vec4(textureLod(prefilteredEnv_map, tex_coords,5.0f).xyz, 1.0f);

	float alpha = 0.1f; // reflectivity from Material

	vec3 normal_shade  = getNormalShade(v_normal);
	vec3 refl_shade  = getNormalShade(normalize(reflected_normal_ws)); // vypada legit
	vec3 blue_albedo = vec3(0,0,1); // jakoze modra
	vec3 irr =  getIrradiance() ;//L_S_d
	vec3 env = getPrefEnv(alpha);

	float ct_o = dot(unified_normal_ws, omega_o_es);
	vec3 integr = getIntegration(alpha, ct_o);
	vec3 albedo =vec3(0.5f,0.5f, 0.5f); // seda
	
	//vec3 env = getPrefEnv(0.3);//L_S_r
	//FragColor = vec4(albedo * irr.xyz, 1.0f);
	FragColor = vec4( integr.xyz, 1.0f );//  *getShadow( 0.001f, 10);
	//l_d_r = albedo * irradiance ?
}

/* mat3x3 TBN;

struct Material {
	vec3 diffuse;
	uvec2 tex_diffuse;

	vec3 rma;
	uvec2 tex_rma;

	vec3 norm;
	uvec2 tex_norm;
};

layout ( std430, binding = 0) readonly buffer Materials {
	Material materials[]; // only the last member can be unsized array
};


mat3 getTBN() {
	vec3 n = normalize(v_normal);
	vec3 t = normalize(v_tangent - dot(v_tangent, n) * n);
	vec3 b = normalize(cross(n, t));
	return mat3(t, b, n);
}

vec3 rotateVector(vec3 v, vec3 n) {
	vec3 o1 =  normalize((abs(n.x) > abs(n.z)) ? vec3(-n.y, n.x, 0.0f) : vec3(0.0f, -n.z, n.y));
	vec3 o2 = normalize(cross(o1, n));
	return mat3(o1,o2,n) * v;
}

vec3 getNormal_raw() {
	vec3 norm = materials[mat_index].norm.rgb;
	if (norm == vec3(1.0f,1.0f,1.0f)) {
		//return normalize( 2* texture(sampler2D(materials[mat_index].tex_norm), tex_coord).rgb - vec3(1,1,1));
		vec3 n_ls = normalize( 2* texture(sampler2D(materials[mat_index].tex_norm), tex_coord).rgb - vec3(1.0f,1.0f,1.0f));
		//vec3 n_ls = normalize(texture(sampler2D(materials[mat_index].tex_norm), tex_coord).rgb);
		return TBN * n_ls;
		return  normalize(TBN * n_ls);
	}
	return v_normal;
}

vec3 getNormal_unified() {
	return normalize((MVN * vec4(getNormal_raw().xyz, 0.0f)).xyz);
}

vec2 getUV(vec3 v) {
	float p = atan(v.y, v.x);
	float phi = (p < 0) ? p + 2 * M_PI : p;
	float theta = acos(v.z);

	return vec2(phi / (2 * M_PI), theta / M_PI);
}
vec3 getIrradiance() {
	vec2 uv = getUV(getNormal_raw());
	return texture(irradiance_map, uv).rgb;
}
vec3 getAlbedo() {
	vec3 result = materials[mat_index].diffuse.rgb;
	if (result == vec3(1,1,1)) {
		result = texture(sampler2D( materials[mat_index].tex_diffuse), -tex_coord).rgb;
	}
	return result;
}

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

