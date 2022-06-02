#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier :  enable

layout(binding = 1) uniform sampler2D ge_Textures[];


layout(location = 0) in VertexData {
	flat uint ge_TextureBase;
	flat uint ge_TextureCount;
	flat uint ge_TextureFlags;
	vec4 ge_Color;
	vec4 ge_TextureCoord;
	vec3 ge_Normal;
	vec3 ge_Position;
};

layout(location = 0) out vec4 ge_FragColor;
// layout(location = 1) out vec3 ge_FragNormal;
// layout(location = 2) out vec3 ge_FragPosition;

void main() {

	if ( ge_TextureCount > 0 && ge_TextureCoord.w >= 0.0 ) {
		ge_FragColor = ge_Color * texture( ge_Textures[ge_TextureBase + uint(ge_TextureCoord.w)], ge_TextureCoord.xy );
	} else {
		ge_FragColor = ge_Color;
	}

	vec3 norm = normalize( ge_Normal );
/*
	if ( ge_TextureFlags >> 1 == 1 ) {
		vec3 TextureNormal_tangentspace = normalize( texture( ge_Textures[ge_TextureBase + 1], ge_TextureCoord.xy ).rgb * 2.0 - 1.0 );
// 		norm *= TextureNormal_tangentspace;
		ge_FragColor.rgb = TextureNormal_tangentspace;
	}
*/
	vec3 lightDir = normalize( vec3( 3, 0, 10 ) - ge_Position );
	vec3 light = vec3(0.125) + max( dot( norm, lightDir ), 0.0 );
	ge_FragColor.rgb *= light;

// 	ge_FragNormal = ge_Normal * 0.5 + vec3(0.5);
// 	ge_FragPosition = ge_Position;
}
