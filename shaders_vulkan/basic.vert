#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 ge_VertexTexcoord;
layout(location = 1) in vec4 ge_VertexColor;
layout(location = 2) in vec4 ge_VertexNormal;
layout(location = 3) in vec4 ge_VertexPosition;
layout(location = 4 /* 5 6 7 */) in mat4 ge_ObjectMatrix;
layout(location = 8) in uvec4 ge_ObjectTexture;


layout(binding = 0) uniform Buffers {
	mat4 ge_ProjectionMatrix;
	mat4 ge_ViewMatrix;
};


layout(location = 0) out VertexData {
	flat uint ge_TextureBase;
	flat uint ge_TextureCount;
	flat uint ge_TextureFlags;
	vec4 ge_Color;
	vec4 ge_TextureCoord;
	vec3 ge_Normal;
	vec3 ge_Position;
};


void main() {
// 	mat3 normalMatrix = transpose( inverse( mat3( ge_ObjectMatrix ) ) );
	mat3 normalMatrix = mat3( ge_ObjectMatrix );
	ge_Normal = normalize( normalMatrix * ge_VertexNormal.xyz );

	ge_TextureBase = ge_ObjectTexture.x;
	ge_TextureCount = ge_ObjectTexture.y;
	ge_TextureFlags = ge_ObjectTexture.z;
	ge_Color = ge_VertexColor;
	ge_TextureCoord = ge_VertexTexcoord;

	ge_Position.xyz = (ge_ObjectMatrix * vec4(ge_VertexPosition.xyz, 1.0)).xyz;
	gl_Position = ge_ProjectionMatrix * ge_ViewMatrix * vec4(ge_Position.xyz, 1.0);

	gl_Position.y = -gl_Position.y;
}
