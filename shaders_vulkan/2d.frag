#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D ge_Textures[512];

layout(location = 0) in vec4 ge_Color;
layout(location = 1) in vec4 ge_TextureCoord;

layout(location = 0) out vec4 ge_FragColor;

void main() {
	ge_FragColor = ge_Color * texture( ge_Textures[0], ge_TextureCoord.xy );
}
