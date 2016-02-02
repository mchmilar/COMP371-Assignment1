#version 130

uniform mat4 view_matrix, model_matrix, proj_matrix;

in  vec3 in_Position;		//vertex position
out vec3 out_Color;

void main () {
	mat4 CTM = proj_matrix * view_matrix * model_matrix;
	gl_Position = CTM * vec4 (in_Position, 1.0);

	out_Color = vec3 (0.8,0.2,gl_Position.z);
}