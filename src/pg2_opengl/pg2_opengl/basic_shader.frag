#version 460 core
#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64 : require 

#define M_PI 3.1415926535897932384626433832795

in vec3 v_normal;
in vec3 v_tangent;
in vec3 unified_normal_ws;
in vec3 unified_position_ws;
//in vec3 position_lcs;

layout ( location = 0 ) out vec4 FragColor;

in vec3 omega_o_es;
in vec2 tex_coord;
in vec3 cam_pos;
flat in int mat_index;

// uniforms
uniform sampler2D irradiance_map;
uniform sampler2D prefilteredEnv_map;
uniform sampler2D integration_map;
//uniform sampler2D shadow_map;

struct Material
{
	vec3 diffuse; // (1,1,1) or albedo
	uvec2 tex_diffuse; // albedo texture
	vec3 rma; // (1,1,1) or (roughness, metalness, 1)
	uvec2 tex_rma; // rma texture
	vec3 normal; // (1,1,1) or (0,0,1)
	uvec2 tex_normal; // bump texture
};

layout ( std430, binding = 0 ) readonly buffer Materials{
	Material materials[];
};

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
	vec3 omega_o = - normalize( cam_pos.xyz - unified_position_ws );
	vec3 omega_i = reflect( -omega_o ,  normalize(normal));
	return textureLod(prefilteredEnv_map, c2s(omega_i), roughness * maxLevel).rgb;
}


float Fresnell(float ct_o, float n1, float n2 ) {
	if (ct_o == 0) return 0;
	float f_0 = pow((n1-n2)/(n1+n2), 2);
	return f_0 + (1 - f_0) * pow(1 - ct_o, 5);
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
	// BINDLESS TEXTURES
	
	vec3 albedo_bt = materials[mat_index].diffuse.rgb * texture( sampler2D( materials[mat_index].tex_diffuse ),tex_coord ).rgb;
	vec3 rma_bt = materials[mat_index].rma.rgb * texture(sampler2D(materials[mat_index].tex_rma), tex_coord).rgb;
	vec3 normal_map_bt = materials[mat_index].normal.rgb * texture(sampler2D(materials[mat_index].tex_normal), tex_coord).rgb;

	vec3 rma = rma_bt;
	vec3 albedo =albedo_bt;
	float ambient_occlusion = rma.b;
	float metalic = rma.g;
	float roughness = rma.r;
	float alpha = pow(roughness, 2);
	vec3 local_normal = getTBNMatrix() * normalize( vec3(normal_map_bt)*(2.0f - vec3( 1.0f )) );
	vec3 omega_o_ws =  normalize( cam_pos.xyz - local_normal );

	
	float cosinus_theta_o = dot(unified_normal_ws, omega_o_ws);
	

	float Fo = Fresnell(cosinus_theta_o, 1.0f, 4.0); // 4.0 = IOR
	float Fd = (1 - Fo) * (1 - metalic);
	vec3 sb = getIntegration(cosinus_theta_o, roughness);

	vec3 Ld = albedo * getIrradiance(local_normal); 
	vec3 Lr = getPrefEnv(roughness, local_normal);

	vec3 color = (Fd * Ld + (Fo*sb.x + sb.y) * Lr) * ambient_occlusion;
	
	vec3 toned_color = tonemapping(color, 1.5f, 2.2f);
	vec3 final_color = toned_color.xyz ;
	return final_color;
}


void main( void ) {
	vec3 color;
	

	color = getPBRShader(); 

	FragColor = vec4(color, 1.0f);
}
	