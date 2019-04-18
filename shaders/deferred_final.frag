// /!\ Shader designed for OpenGL43 backend /!\

#version 430 core

uniform sampler2D ge_Texture0;
uniform usampler2D ge_Texture1;
uniform sampler2D ge_Texture2;
uniform sampler2D ge_Texture3;

flat in vec4 ge_Color;
in vec4 ge_FragCoord;
flat in vec3 ge_LightPos;
flat in vec3 ge_LightDir;
flat in float ge_LightAttenuation;
flat in float ge_LightInnerAngle;
flat in float ge_LightOuterAngle;
out vec4 ge_FragColor;


float Attenuate( float d, float kc, float kl, float kq )
{
	if ( kq == 0.0 ) {
		return 1.0;
	}
	float ret = 1.0 / ( kc + kl * d + kq * pow( d, 2.0 ) );
	return ( ret - kq ) * 1.1;
}


vec2 VR_Distort( vec2 coords )
{
	vec2 ret = coords;
	vec2 ofs = vec2(1920/4, 1080/2);
	if ( coords.x >= 1920/2 ) {
		ofs.x = 3 * 1920/4;
	}
	vec2 offset = coords - ofs;
	float r2 = offset.x * offset.x + offset.y * offset.y;
	float r = sqrt(r2);
	float k1 = 1.95304;
	k1 = k1 / ((1920.0 / 4.0)*(1920.0 / 4.0) * 16);

	ret = ofs + 1/(1 - k1 * r*r) * offset;

	return ret;
}


void main()
{
	vec2 screenSize = textureSize( ge_Texture0, 0 ).xy;
	vec2 texcoords = gl_FragCoord.xy / screenSize;
// 	vec2 texcoords = VR_Distort( gl_FragCoord.xy ) / screenSize;
	float depth = float(texture(ge_Texture1, texcoords.st).r) / 65535.0;
	gl_FragDepth = depth;

	if ( ge_LightAttenuation < -1.0 ) {
		ge_FragColor = ge_Color * texture2D( ge_Texture0, texcoords );
	} else if ( ge_LightAttenuation < 0.0 ) {
		vec4 scene_color = texture2D( ge_Texture0, texcoords );
		vec3 normal = texture2D( ge_Texture2, texcoords ).xyz * 2.0 - vec3(1.0);
		vec3 position = texture2D( ge_Texture3, texcoords ).xyz;

		float dist = distance( position, ge_LightPos.xyz );
		vec3 L = normalize(ge_LightPos.xyz - position);
		float lambertTerm = max(dot(normal, L), 0.0);
		if ( lambertTerm >= 0.0 ) {
			ge_FragColor.rgb = ge_Color.a * ge_Color.rgb * scene_color.rgb * lambertTerm;// * Attenuate( dist, 1.0, 0.0, 0.0f );
			ge_FragColor.a = scene_color.a;
		}
	} else {
		if ( depth > gl_FragCoord.z ) {
			discard;
		}
		vec4 scene_color = texture2D( ge_Texture0, texcoords );
		vec3 normal = texture2D( ge_Texture2, texcoords ).xyz * 2.0 - vec3(1.0);
		vec3 position = texture2D( ge_Texture3, texcoords ).xyz;

		float dist = distance( position, ge_LightPos.xyz );
		vec3 L = normalize(ge_LightPos.xyz - position);
		float lambertTerm = max(dot(normal, L), 0.0);
		if ( lambertTerm >= 0.0 ) {
			if ( ge_LightInnerAngle >= 360.0 ) {
				ge_FragColor.rgb = ge_Color.a * ge_Color.rgb * scene_color.rgb * lambertTerm * Attenuate( dist, 1.0, 0.0, ge_LightAttenuation );
			} else {
				float cos_cur_angle = dot(-L, ge_LightDir.xyz);
				float spot = clamp((cos_cur_angle - ge_LightOuterAngle) / ge_LightInnerAngle, 0.0, 1.0);
				ge_FragColor.rgb = ge_Color.a * ge_Color.rgb * scene_color.rgb * lambertTerm * spot * Attenuate( dist, 1.0, 0.0, ge_LightAttenuation );
				if ( dist <= 0.1 ) {
					ge_FragColor.rgb += 2.0 * ge_Color.rgb;
				}
			}
			ge_FragColor.a = scene_color.a;
		}
// 		ge_FragColor += vec4(0.1, 0.0, 0.0, 1.0);
	}
}

/*
void test()
{
	gl_FragColor = ge_Color * texture2D( ge_Texture0, gl_FragCoord.xy / textureSize( ge_Texture0, 0 ).xy );
	return;

	gl_FragColor.rgb = texture2D( ge_Texture2, ge_FragCoord.xy ).rgb;
	gl_FragColor.a = 1.0;
// 	return;

	gl_FragColor.rgb = texture2D( ge_Texture3, ge_FragCoord.xy ).rgb / 32.0;
	gl_FragColor.a = 1.0;
	return;

	float depth = float(texture2D(ge_Texture1, ge_FragCoord.st).r) / 65535.0;

	float zNear = 0.01;
	float zFar = 100.0;
	float base_depth = (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
	gl_FragColor = vec4(base_depth, base_depth, base_depth, 1.0);
	gl_FragColor.a = 1.0;
}
*/
