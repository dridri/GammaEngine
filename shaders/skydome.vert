// /!\ Shader designed for OpenGL43 backend /!\

#version 430 core
#extension GL_ARB_explicit_uniform_location : require
#extension GL_ARB_shader_draw_parameters : require
#extension GL_ARB_bindless_texture : require

#define geTexture2D(x) sampler2D( ge_Textures[ ge_TextureBase + x ].xy )
#define geTexture3D(x) sampler3D( ge_Textures[ ge_TextureBase + x ].xy )

layout(location = 0) in vec4 ge_VertexTexcoord;
layout(location = 1) in vec4 ge_VertexColor;
layout(location = 2) in vec4 ge_VertexNormal;
layout(location = 3) in vec4 ge_VertexPosition;
layout(location = 7 /* 8 9 10 */) in mat4 ge_ObjectMatrix;
layout(location = 11) in int ge_TextureBase;

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
	uvec4 ge_Textures[256];
};


uniform vec3 v3CameraPos = vec3(0.0);
uniform vec3 v3SunPos = vec3(10000000.0, 5000000.0, 10000000.0);
uniform vec3 v3SunPos2 = vec3(5000000.0, -10000000.0, 10000000.0);
uniform float Hr = 8800.0;
uniform float Hm = 2400.0;

#define NUM_SAMPLES 8
#define NUM_SAMPLES_LIGHT 8

const float Re = 6350000.0;
const float Ra = 6420000.0;

const vec3 bR = vec3(5.266626e-006, 1.230601e-005, 3.004397e-005);
const vec3 bM = vec3(21.0e-6);

out vec3 vpos;
out vec3 sumR;
out vec3 sumM;

out sampler2D ge_RandTexture;
out vec3 v3SunColor;

void CalcSun( vec3 SunPosition, vec3 curr, float hr, float hm, float opticalDepthR, float opticalDepthM );

vec3 t0;
vec3 t1;
bool intersect( vec3 orig, vec3 dir, float r )
{
	float a = dot(dir, dir);
	float b = 2.0 * dot(dir, orig);
	float c = dot(orig, orig) - r*r;
	float delta = b*b - 4.0*a*c;
	if ( delta >= 0.0 ) {
		t0 = orig + ( ( -b - sqrt(delta) ) / (2.0 * a) ) * dir;
		t1 = orig + ( ( -b + sqrt(delta) ) / (2.0 * a) ) * dir;
		return true;
	}
	return false;
}

void main()
{
	int i, j;
	vec3 color = vec3(0.0);
	float mu = 0.0;
	float phaseR = 0.0;
	float phaseM = 0.0;
	float height = v3CameraPos.z;
	sumR = vec3(0.0, 0.0, 0.0);
	sumM = vec3(0.0);

	ge_RandTexture = geTexture2D( 0 );

	vec3 start = v3CameraPos;
	vpos = ge_VertexPosition.xyz;

	vec3 end = vpos;
	vec3 dir = (end - start);
	vec3 segment = dir / float(NUM_SAMPLES);
	vec3 curr = start;

	float opticalDepthR = 0.0;
	float opticalDepthM = 0.0;
	float hm, hr;

	hr = exp(-(0.0) / Hr) * length(segment);
	hm = exp(-(0.0) / Hm) * length(segment);
	CalcSun( normalize( v3SunPos ), start, hr, hm, hr, hm );
	v3SunColor = sumM;
	sumM = vec3(0.0);
	sumR = vec3(0.0);

	for(i=0; i<NUM_SAMPLES; i++){
		//height = length(curr)/* - Re*/ + Hr;
		height = curr.z + Hr;

		hr = exp(-(height) / Hr) * length(segment);
		hm = exp(-(height) / Hm) * length(segment);
		opticalDepthR += hr;
		opticalDepthM += hm;

		CalcSun( normalize( v3SunPos ), curr, hr, hm, opticalDepthR, opticalDepthM );
// 		CalcSun( normalize( v3SunPos2 ), curr, hr, hm, opticalDepthR, opticalDepthM );
/*
		float opticalDepthLightR = 0.0;
		float opticalDepthLightM = 0.0;
		vec3 lightStart = curr;
		vec3 lightDir = normalize(v3SunPos);
		intersect(lightStart + vec3(0.0, 0.0, 6350000.0), lightDir, Ra);
		t1.z -= 6350000.0;
		vec3 lightSegment = (t1 - lightStart) / float(NUM_SAMPLES_LIGHT);
		vec3 lightCurr = lightStart;
		for(j=0; j<NUM_SAMPLES_LIGHT; j++){
			//float lightHeight = length(lightCurr) - Re + Hr;
			float lightHeight = lightCurr.z;// + Hr;
			//if(lightHeight < 0)break;
			opticalDepthLightR += exp(-lightHeight / Hr) * length(lightSegment);
			opticalDepthLightM += exp(-lightHeight / Hm) * length(lightSegment);
			lightCurr += lightSegment;
		}
		if(j == NUM_SAMPLES_LIGHT){
			vec3 tau = bR * (opticalDepthR + opticalDepthLightR) + bM * 1.1 * (opticalDepthM + opticalDepthLightM);
			vec3 attenuation = exp(-tau);
			sumR += vec3(hr) * attenuation;
			sumM += vec3(hm) * attenuation;
		}
*/
		curr = curr + segment;
	}
	gl_Position = ge_ProjectionMatrix * ge_ViewMatrix * vec4( ge_VertexPosition.xyz, 1.0 );
}


void CalcSun( vec3 SunPosition, vec3 curr, float hr, float hm, float opticalDepthR, float opticalDepthM )
{
	int j;
	float opticalDepthLightR = 0.0;
	float opticalDepthLightM = 0.0;
	vec3 lightStart = curr;
	vec3 lightDir = normalize( SunPosition );
	intersect(lightStart + vec3(0.0, 0.0, 6350000.0), lightDir, Ra);
	t1.z -= 6350000.0;
	vec3 lightSegment = (t1 - lightStart) / float(NUM_SAMPLES_LIGHT);
	vec3 lightCurr = lightStart;
	for(j=0; j<NUM_SAMPLES_LIGHT; j++){
		//float lightHeight = length(lightCurr) - Re + Hr;
		float lightHeight = lightCurr.z;// + Hr;
		//if(lightHeight < 0)break;
		opticalDepthLightR += exp(-lightHeight / Hr) * length(lightSegment);
		opticalDepthLightM += exp(-lightHeight / Hm) * length(lightSegment);
		lightCurr += lightSegment;
	}
	if(j == NUM_SAMPLES_LIGHT){
		vec3 tau = bR * (opticalDepthR + opticalDepthLightR) + bM * 1.1 * (opticalDepthM + opticalDepthLightM);
		vec3 attenuation = exp(-tau);
		sumR += vec3(hr) * attenuation;
		sumM += vec3(hm) * attenuation;
	}
}
