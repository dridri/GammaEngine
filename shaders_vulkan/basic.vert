#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 ge_VertexTexcoord;
layout(location = 1) in vec4 ge_VertexColor;
layout(location = 2) in vec4 ge_VertexNormal;
layout(location = 3) in vec4 ge_VertexPosition;
// layout(location = 7 /* 8 9 10 */) in mat4 ge_ObjectMatrix;


layout(binding = 0) uniform Buffers {
	mat4 ge_ProjectionMatrix;
	mat4 ge_ViewMatrix;
};


layout(location = 0) out vec3 fragColor;

void main() {
	gl_Position = ge_ProjectionMatrix * ge_ViewMatrix * vec4( ge_VertexPosition.xyz, 1.0 );
	gl_Position.y = -gl_Position.y;
	fragColor = ge_VertexColor.rgb;
}
