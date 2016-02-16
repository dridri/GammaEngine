// /!\ Shader designed for OpenGL43 backend /!\

#version 420 core
#extension GL_ARB_bindless_texture : require
/*
flat in sampler2D ge_Texture0;
flat in sampler2D ge_Texture1;
flat in int ge_Texture0Base;
flat in float ge_HasTexture;
flat in uint ge_TextureCount;
in vec4 ge_Color;
in vec3 ge_TextureCoord;
in vec3 ge_Normal;
in vec3 ge_Position;
// in mat3 tangentToWorld;
*/

flat in sampler2D ge_Textures[8];

in VertexData {
	flat uint ge_Texture0Base;
	flat uint ge_TextureCount;
	vec4 ge_Color;
	vec3 ge_TextureCoord;
	vec3 ge_Normal;
	vec3 ge_Position;
} VertexIn;

out vec4 ge_FragColor;
out uint ge_FragDepth;
out vec3 ge_FragNormal;
out vec3 ge_FragPosition;

void main()
{
	ge_FragNormal = VertexIn.ge_Normal * 0.5 + vec3(0.5);

	if ( VertexIn.ge_TextureCount > 0 ) {
		ge_FragColor = VertexIn.ge_Color;
		ge_FragColor = VertexIn.ge_Color * texture2D( ge_Textures[0], VertexIn.ge_TextureCoord.xy );
	} else {
		ge_FragColor = VertexIn.ge_Color;
	}

	ge_FragDepth = uint( gl_FragCoord.z * 65535.0 );
	ge_FragPosition = VertexIn.ge_Position;
// 	ge_FragColor.a = 1.0;
}
