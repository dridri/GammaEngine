// /!\ Shader designed for OpenGL43 backend /!\

#version 420 core
#extension GL_ARB_bindless_texture : require

layout(triangles) in;
layout (triangle_strip, max_vertices=3) out;

in VertexData {
	flat sampler2D ge_Texture0;
	flat uint ge_Texture0Base;
	flat uint ge_TextureCount;
	flat float ge_HasTexture;
	vec4 ge_Color;
	vec3 ge_TextureCoord;
	vec3 ge_Normal;
	vec3 ge_Position;
} VertexIn[3];

out VertexData {
	flat sampler2D ge_Texture0;
	flat uint ge_Texture0Base;
	flat uint ge_TextureCount;
	flat float ge_HasTexture;
	vec4 ge_Color;
	vec3 ge_TextureCoord;
	vec3 ge_Normal;
	vec3 ge_Position;
} VertexOut;


void main()
{
	for ( int i = 0; i < gl_in.length(); i++ ) {
		gl_Position = gl_in[i].gl_Position;
		VertexOut.ge_Texture0 = VertexIn[i].ge_Texture0;
		VertexOut.ge_Texture0Base = VertexIn[i].ge_Texture0Base;
		VertexOut.ge_TextureCount = VertexIn[i].ge_TextureCount;
		VertexOut.ge_HasTexture = VertexIn[i].ge_HasTexture;
		VertexOut.ge_Color = VertexIn[i].ge_Color;
		VertexOut.ge_TextureCoord = VertexIn[i].ge_TextureCoord;
		VertexOut.ge_Normal = VertexIn[i].ge_Normal;
		VertexOut.ge_Position = VertexIn[i].ge_Position;
		gl_Position.x = gl_Position.x - 10.0;
// 		VertexOut.ge_Position.x = VertexIn[i].ge_Position.x * 0.5 - 1.0;
		EmitVertex();
	}
}
