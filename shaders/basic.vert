// /!\ Shader designed for OpenGL43 backend /!\

#version 420 core
#extension GL_ARB_shader_draw_parameters : require
#extension GL_ARB_bindless_texture : require

#define geTextureValid(x) ( any( notEqual( ge_TextureHandlers[ ( ge_TextureBase & 0xFFFF ) + x ].xy, uvec2(0) ) ) )
#define geTexture2D(x) sampler2D( ge_TextureHandlers[ ( ge_TextureBase & 0xFFFF ) + x ].xy )
#define geTexture3D(x) sampler3D( ge_TextureHandlers[ ( ge_TextureBase & 0xFFFF ) + x ].xy )

layout(location = 0) in vec4 ge_VertexTexcoord;
layout(location = 1) in vec4 ge_VertexColor;
layout(location = 2) in vec4 ge_VertexNormal;
layout(location = 3) in vec4 ge_VertexPosition;
layout(location = 7 /* 8 9 10 */) in mat4 ge_ObjectMatrix;
layout(location = 11) in uint ge_TextureBase;

layout (binding=0, std140) uniform ge_Matrices_0
{
	mat4 ge_ProjectionMatrix;
};

layout (binding=1, std140) uniform ge_Matrices_1
{
	mat4 ge_ViewMatrix;
};

layout (binding=2, std140) uniform ge_Textures_0
{
	uvec4 ge_TextureHandlers[256];
};


flat out sampler2D ge_Textures[8];

out VertexData {
	flat uint ge_Texture0Base;
	flat uint ge_TextureCount;
	vec4 ge_Color;
	vec3 ge_TextureCoord;
	vec3 ge_Normal;
	vec3 ge_Position;
} VertexOut;

void main()
{

	VertexOut.ge_Texture0Base = ge_TextureBase & 0xFFFF;
	VertexOut.ge_TextureCount = ge_TextureBase >> 16;
	for ( uint i = 0; i < VertexOut.ge_TextureCount; i++ ) {
		ge_Textures[i] = geTexture2D(i);
	}

	VertexOut.ge_Color = ge_VertexColor;
	VertexOut.ge_TextureCoord = ge_VertexTexcoord.xyz;

	mat3 normalMatrix = transpose( inverse( mat3( ge_ObjectMatrix ) ) );
	VertexOut.ge_Normal = normalize( normalMatrix * ge_VertexNormal.xyz );


	gl_Position = ge_ProjectionMatrix * ge_ViewMatrix * ge_ObjectMatrix * vec4(ge_VertexPosition.xyz, 1.0);
// 	ge_Position = ( ge_ViewMatrix * ge_ObjectMatrix * vec4(ge_VertexPosition, 1.0) ).xyz;
	VertexOut.ge_Position = (ge_ObjectMatrix * vec4(ge_VertexPosition.xyz, 1.0)).xyz;
}
