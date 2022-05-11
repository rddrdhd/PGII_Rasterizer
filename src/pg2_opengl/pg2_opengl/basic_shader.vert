#version 460 core

/* vertex attributes */

layout ( location = 0 ) in vec4 in_position;
layout ( location = 1 ) in vec3 in_normal;
layout ( location = 2 ) in vec3 in_tangent;
layout ( location = 3 ) in vec2 in_texcoord;
layout ( location = 4 ) in int in_material_index; 

/* uniform variables */

uniform mat4 MVn;
uniform mat4 MVP;
//uniform mat4 MV;
uniform mat4 M;
//uniform mat4 MLP;

uniform vec3 camera_pos;

/* out variables */

out vec3 v_normal;
out vec3 v_tangent;
out vec3 unified_normal_ws;
out vec3 unified_position_ws;
out vec3 cam_pos;
//out vec3 position_lcs;
out vec3 omega_o_es;
out vec2 tex_coord;

flat out int mat_index;

void main( void )
{
	gl_Position = MVP * in_position; // model-space -> clip-space

	v_normal = normalize(in_normal);
	v_tangent = normalize(in_tangent);

	vec4 tmp = MVn * vec4( v_normal.xyz, 1.0f ); 
	unified_normal_ws = normalize( tmp.xyz / tmp.w );

	vec4 hit_es = M * in_position;
	unified_position_ws = hit_es.xyz / hit_es.w;
	vec3 omega_i_ws = normalize( camera_pos.xyz - unified_position_ws );
	

	tex_coord = vec2(in_texcoord.x, 1.0f - in_texcoord.y);
	mat_index = in_material_index;
	cam_pos = camera_pos;

	//SHADOW
	//vec4 tmp2 = MLP * vec4( in_position.xyz, 1.0f );
	//position_lcs = tmp2.xyz / tmp2.w;
}
