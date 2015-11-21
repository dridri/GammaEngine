// /!\ Shader designed for OpenGL43 backend /!\

#version 420 core

layout(location = 0) in vec3 ge_VertexTexcoord;
layout(location = 1) in vec4 ge_VertexColor;
layout(location = 2) in vec3 ge_VertexNormal;
layout(location = 3) in vec3 ge_VertexPosition;
layout(location = 11) in vec4 ge_LightPosition;
layout(location = 12) in vec4 ge_LightDirection;
layout(location = 13) in vec4 ge_LightColor;
layout(location = 14) in vec4 ge_LightData;

layout (binding=0, std140) uniform ge_Matrices_0
{
	mat4 ge_ProjectionMatrix;
};

layout (binding=1, std140) uniform ge_Matrices_1
{
	mat4 ge_ViewMatrix;
};

flat out vec4 ge_Color;
out vec4 ge_FragCoord;
flat out vec3 ge_LightPos;
flat out vec3 ge_LightDir;
flat out float ge_LightAttenuation;
flat out float ge_LightInnerAngle;
flat out float ge_LightOuterAngle;

flat out vec4 ge_MainLightPos;

void main()
{
	vec4 pos;
	ge_LightPos = ge_LightPosition.xyz;
	ge_LightDir = ge_LightDirection.xyz;
	ge_Color = ge_LightColor;
	ge_LightAttenuation = ge_LightData.y;
	ge_LightInnerAngle = ge_LightData.z;
	ge_LightOuterAngle = ge_LightData.w;

	if ( ge_VertexTexcoord.x == 42.0 ) {
		pos = vec4(ge_VertexPosition, 1.0);
		ge_FragCoord = pos * 0.5 + 0.5;
	} else {
		pos = ge_ProjectionMatrix * ge_ViewMatrix * vec4(ge_LightPosition.xyz + ge_VertexPosition * ge_LightData.x, 1.0);
	}

	gl_Position = pos;

	ge_MainLightPos = ge_ProjectionMatrix * ge_ViewMatrix * vec4(1000.0, 500.0, 1000.0, 1.0);
}
