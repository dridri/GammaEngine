#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 ge_VertexTexcoord;
layout(location = 1) in vec4 ge_VertexColor;
layout(location = 2) in vec4 ge_VertexNormal;
layout(location = 3) in vec4 ge_VertexPosition;


layout(binding = 0) uniform Buffers {
	mat4 ge_ProjectionMatrix;
	mat4 ge_ViewMatrix;
};

layout(location = 0) out vec4 ge_Color;
layout(location = 1) out vec4 ge_TextureCoord;

void main() {
	ge_Color = ge_VertexColor;
	ge_TextureCoord = ge_VertexTexcoord;

	gl_Position = ge_ProjectionMatrix * ge_ViewMatrix * vec4(ge_VertexPosition.xyz, 1.0);
	gl_Position.y = -gl_Position.y;
}
