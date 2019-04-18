// /!\ Shader designed for OpenGL43 backend /!\

#version 430
// #extension GL_ARB_bindless_texture : require

layout(location = 16 /* 17 18 .. 47 */) uniform sampler2D ge_Textures[32];

// flat in sampler2D ge_Textures[8];
in VertexData {
	flat uint ge_TextureBase;
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
		ge_FragColor = VertexIn.ge_Color * texture2D( ge_Textures[VertexIn.ge_TextureBase + 0], VertexIn.ge_TextureCoord.xy );
	} else {
		ge_FragColor = VertexIn.ge_Color;
	}

	ge_FragDepth = uint( gl_FragCoord.z * 65535.0 );
	ge_FragPosition = VertexIn.ge_Position;
}
