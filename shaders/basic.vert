// /!\ Shader designed for OpenGL43 backend /!\

#version 420 core
#extension GL_ARB_shader_draw_parameters : require
#extension GL_ARB_bindless_texture : require

#define geTextureValid(x) ( any( notEqual( ge_Textures[ ( ge_TextureBase & 0xFFFF ) + x ].xy, uvec2(0) ) ) )
#define geTexture2D(x) sampler2D( ge_Textures[ ( ge_TextureBase & 0xFFFF ) + x ].xy )
#define geTexture3D(x) sampler3D( ge_Textures[ ( ge_TextureBase & 0xFFFF ) + x ].xy )

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
	uvec4 ge_Textures[512];
};

flat out sampler2D ge_Texture0;
// flat out sampler2D ge_Texture1;
flat out uint ge_Texture0Base;
flat out uint ge_TextureCount;
flat out float ge_HasTexture;
out vec4 ge_Color;
out vec3 ge_TextureCoord;
out vec3 ge_Normal;
out vec3 ge_Position;
// out mat3 tangentToWorld;

void main()
{
// 	ge_HasTexture = ( ge_Textures[ ge_TextureBase + 0 ].xy != uvec2(0) ) ? 1.0 : 0.0;
	ge_Texture0Base = ge_TextureBase & 0xFFFF;
	ge_TextureCount = ge_TextureBase >> 16;
	ge_HasTexture = ( ge_TextureCount > 0 ) ? 1.0 : 0.0;
// 	if ( all( equal( ge_Textures[ ( ge_TextureBase & 0xFFFF ) + 0 ].xy, uvec2(0) ) ) ) {
	if ( !geTextureValid(0) ) {
		ge_HasTexture = 0.0;
	}
	if ( ge_HasTexture != 0.0 ) {
		ge_Texture0 = geTexture2D(0);
	}
	ge_Color = ge_VertexColor;
	ge_TextureCoord = ge_VertexTexcoord.xyz;
// 	ge_Normal = ge_VertexNormal.xyz;

	mat3 normalMatrix = transpose( inverse( mat3( ge_ObjectMatrix ) ) );
	ge_Normal = normalize( normalMatrix * ge_VertexNormal.xyz );

// 	if ( ge_TextureCount > 1 ) {
// 		ge_Texture1 = geTexture2D(1);
// 		mat3 normalMatrix = transpose( inverse( mat3( ge_ViewMatrix * ge_ObjectMatrix ) ) );
// 		vec3 Normal = normalize( normalMatrix * ge_VertexNormal.xyz );
// 		vec3 Tangent = normalize( normalMatrix[0]) ; 
// 		vec3 Binormal = normalize( normalMatrix[1] );
// 		tangentToWorld = mat3(Tangent.x, Binormal.x, Normal.x,
// 							  Tangent.y, Binormal.y, Normal.y,
// 							  Tangent.z, Binormal.z, Normal.z);
// 	}

	gl_Position = ge_ProjectionMatrix * ge_ViewMatrix * ge_ObjectMatrix * vec4(ge_VertexPosition.xyz, 1.0);
// 	ge_Position = ( ge_ViewMatrix * ge_ObjectMatrix * vec4(ge_VertexPosition, 1.0) ).xyz;
	ge_Position = (ge_ObjectMatrix * vec4(ge_VertexPosition.xyz, 1.0)).xyz;
}
