// /!\ Shader designed for OpenGL43 backend /!\

#version 420 core
#extension GL_ARB_bindless_texture : require

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

out vec4 ge_FragColor;
out uint ge_FragDepth;
out vec3 ge_FragNormal;
out vec3 ge_FragPosition;

void main()
{
	ge_FragNormal = ge_Normal * 0.5 + vec3(0.5);

	if ( ge_HasTexture != 0.0 ) {
		ge_FragColor = ge_Color * texture2D( ge_Texture0, ge_TextureCoord.xy );
	} else {
		ge_FragColor = ge_Color;
	}
// 	ge_FragColor = ge_Color * mix( vec4(1.0), texture2D( ge_Texture0, ge_TextureCoord.xy ), ge_HasTexture );

// 		if ( ge_TextureCount > 1 ) {
// 			ge_FragNormal = ( texture2D( ge_Texture1, ge_TextureCoord.xy ).xyz * 2.0 - vec3(1.0) ) * tangentToWorld;
// 		}

	ge_FragDepth = uint( gl_FragCoord.z * 65535.0 );
	ge_FragPosition = ge_Position;
// 	ge_FragColor.a = 1.0;
}
