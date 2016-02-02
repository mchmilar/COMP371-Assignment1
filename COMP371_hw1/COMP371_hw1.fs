#version 130

in vec3 out_Color;
out vec4 frag_colour;	//final output color used to render the point

void main () {
	frag_colour = vec4 (out_Color, 1.0);
}