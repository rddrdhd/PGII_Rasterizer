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
uniform mat4 MV;
uniform mat4 M;
//uniform mat4 MLP;

uniform vec3 camera_pos;

/* out variables */

out vec3 v_normal;
out vec3 v_tangent;
out vec3 unified_normal_ws;
//out vec3 position_lcs;
out vec3 omega_o_es;
out vec3 reflected_normal_ws;
out vec2 tex_coord;

flat out int mat_index;

void main( void )
{
	gl_Position = MVP * in_position; // model-space -> clip-space

	v_normal = normalize(in_normal);
	v_tangent = normalize(in_tangent);

	vec4 tmp = MVn * vec4( v_normal.xyz, 1.0f ); 
	unified_normal_ws = normalize( tmp.xyz / tmp.w );

	vec4 hit_es = MV * in_position;
	vec3 omega_i_es = normalize( hit_es.xyz / hit_es.w );
	omega_o_es = -omega_i_es;

	if (dot(unified_normal_ws, omega_i_es) > 0.0f) {
		unified_normal_ws *= -1.0f;
	}

	vec4 hit_ws = M * in_position;
	vec3 omega_o_ws = normalize(camera_pos - vec3(hit_ws.xyz/hit_ws.w));
	reflected_normal_ws = reflect( -omega_o_ws ,  normalize(unified_normal_ws));

	tex_coord = in_texcoord;
	mat_index = in_material_index;

	//SHADOW
	//vec4 tmp3 = MLP * vec4( in_position.xyz, 1.0f );
	//position_lcs = tmp3.xyz / tmp3.w;
}
