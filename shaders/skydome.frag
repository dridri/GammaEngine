// /!\ Shader designed for OpenGL43 backend /!\

#version 430 core
#extension GL_ARB_explicit_uniform_location : require
#extension GL_ARB_bindless_texture : require

layout(location = 32) uniform float ge_Time;

const float g = 0.990;

const vec3 bR = vec3(5.266626e-006, 1.230601e-005, 3.004397e-005);
const vec3 bM = vec3(21.0e-6);

uniform vec3 v3CameraPos = vec3(0.0);
uniform vec3 v3SunPos = vec3(10000000.0, 5000000.0, 10000000.0);
uniform vec3 v3SunPos2 = vec3(5000000.0, -10000000.0, 10000000.0);
in vec3 vpos;
in vec3 sumR;
in vec3 sumM;
in sampler2D ge_RandTexture;
in vec3 v3SunColor;

vec3 sun_color = vec3(1.0);
vec3 sun_far_color = vec3(1.0);

vec4 ComputeClouds(vec3 pos, vec3 cam);

vec3 CalcSun( vec3 SunPosition )
{
	vec3 dir = vpos - v3CameraPos;
	float mu = dot(normalize(dir), normalize(SunPosition));
	float phaseR = (3.0 / (16.0 * 3.141528)) * (1.0 + mu * mu);
	float phaseM = (3.0 / (8.0 * 3.141528)) * ((1.0 - g * g) * (1.0 + mu * mu)) / ((2.0 + g * g) * pow(1.0 + g * g - 2.0 * g * mu, 1.5));
// 	sun_color = min( vec3(1.0), phaseM * sumM * 250.0 * vec3(1.0, 0.8, 1.0) );
// 	sun_color = min( vec3(1.0), phaseM * sumM * 250.0 ) * 0.25 + vec3( phaseR );
	sun_far_color = phaseR * sumR * bR;
// 	sun_color = min( vec3(1.0), 32.0 * v3SunColor * bM * vec3(1.0, 0.7, 0.9) );
	sun_color = min( vec3(1.0), 21.0 * v3SunColor * bM * vec3(1.0, 0.7, 0.9) );
	return 20.0 * (phaseR * sumR * bR * 2.0 + phaseM * sumM * bM);
}

void main()
{
	vec3 color = CalcSun( v3SunPos );
// 	color += CalcSun( v3SunPos2 );
	gl_FragColor = vec4(color, 1.0);
//	gl_FragColor.a = clamp(3.0 * (color.r + color.g + color.b), 0.0, 1.0);
	gl_FragColor.a = clamp(3.0*color.r + 2.0*color.g + 1.5*color.b, 0.4, 1.0);

	float fExposure = 6.0;
	gl_FragColor.rgb = vec3(1.0) - exp2( -fExposure * gl_FragColor.rgb );

	gl_FragColor.a = 1.0;

	vec4 clouds = ComputeClouds( vpos, v3CameraPos );
	clouds.rgb = clouds.rgb * 0.4 + vec3( ( clouds.r + clouds.g + clouds.b ) / 3.0 ) * 0.6;
	gl_FragColor.rgb = clouds.rgb * clouds.a + gl_FragColor.rgb * ( 1.0 - clouds.a );

//	gl_FragDepth = 1.0;
}















// uniform float cover = 220.0;
uniform float cover = 50.0;
uniform float sharpness = 0.97;
uniform float scale = 0.001;

const float speed = 1.0;
const float clouds_height = 3000.0;
const float clouds_thickness = 500.0;

#define CLOUDS_SAMPLES 2
#define COLOR_SAMPLES 1
#define CLOUDS_QUALITY 6

float noise3f(vec3 p, float amp){
	vec3 u = fract(p);
	vec3 ip = floor(p);
// 	u = u * u * (3.0 - 2.0 * u);
	u = u * u * u * ( 10.0 - 15.0 * u + 6.0 * u * u );

	float n = dot(ip, vec3(42.0, 148.0, 429.0));
	vec4 rand1 = vec4(n) + vec4(0.0, 148.0, 429.0, 148.0+429.0);
	vec4 rand2 = rand1 + vec4(42.0);

	rand1 = mod( rand1 * mod( rand1 * rand1 * 15731.0 + 789221.0, 2147483647.0 ) + 1376312589.0, 2147483647.0 );
	rand2 = mod( rand2 * mod( rand2 * rand2 * 15731.0 + 789221.0, 2147483647.0 ) + 1376312589.0, 2147483647.0 );

	rand1 = mix(rand1, rand2, u.x);
	rand1.xy = mix(rand1.xz, rand1.yw, u.y);
	return 1.0 - mix(rand1.x, rand1.y, u.z) * ( 1.0 / 1073741824.0 );
}

float PRidged3(vec3 v, int terms)
{
	float res = 0.0;
	float prev = 1.0;
	float amp = 1.0;
	float maxAmp = 0.0;

	for(int i=0; i<terms; i++){
		float r = noise3f(v, amp);
		r = 1.0 - abs(r);
		r = r * r;
		r = r * 2.0;
		res += r * amp * prev;
		prev = r;
		v *= 2.0;
		maxAmp += amp * 3.0;
		amp *= 0.5;
	}

	return res / maxAmp;
}

float PNoise3Opt(vec3 v, vec3 mod, int terms, float fa, float fv)
{
	int i;
	float result = 0.0;
	float amp = 1.0;
	float maxAmp = 0.0;

	for(i=0; i<terms; i++){
		result += noise3f(v + mod, amp) * amp;
		v *= fv;
		maxAmp += amp;
		amp *= fa;
	}

	return result / maxAmp;
}

float PNoise3(vec3 v, int terms)
{
	return PNoise3Opt(v, vec3(ge_Time * speed * 0.1/* * amp*/, 0.0, 0.0), terms, 0.55, 2.0);
}

float CloudCover(vec3 ray)
{
	float n = PNoise3(ray * scale * 0.2, 2);
	n = clamp((n * 0.5 + 0.5) + 0.05, 0.0, 1.0);
	return n * cover;
}

float CloudExpCurve(float v, float Cover, float CloudSharpness)
{
	v *= 255.0;

	float c = v - Cover;
	c = max(c, 0.0);

	float CloudDensity = 1.0 - pow(CloudSharpness, c);
	return CloudDensity;
}

float CloudDensity(vec3 ray)
{
//	ray.x *= 4.0;
	float n = PNoise3(ray * scale, CLOUDS_QUALITY);// + PRidged3(ray * scale, 6);

//	Add granularity
	n += PNoise3(ray * scale * 50.0, 3) * 0.05;

	n = clamp(n * 0.5 + 0.5, 0.0, 1.0);

	return CloudExpCurve(n, CloudCover(ray), sharpness);
}

float CloudDensity2(vec3 ray)
{
	float n = PNoise3(ray * scale, 4);
	n = clamp(n * 0.5 + 0.5, 0.0, 1.0);

	return CloudExpCurve(n, CloudCover(ray + vec3(100.0, 0.0, 100.0)), sharpness + 0.005);
}

vec3 CloudColor(vec3 ray)
{
	vec3 color = vec3(0.0);
	float density = 0.0;

	vec3 dir = normalize(v3SunPos - ray);
	int i;
	for(i=0; i<COLOR_SAMPLES; i++)
	{
		ray += dir * (clouds_thickness / float(COLOR_SAMPLES));
		float ds = CloudDensity2(ray);
		density += ds;
		color += vec3(exp2(-1.0 * ds)) / float(COLOR_SAMPLES);
	}

	float color_power = 1.0 - clamp( 500.0 * density / clouds_thickness, 0.1, 0.75 );
// 	return color * dot(sun_color, vec3(1.0/3.0)) + color * sun_color * color_power;
	return color * dot(sun_color, vec3(1.0/3.0)) * (1.0 - color_power) + color * sun_color * color_power;
}

float CloudsAlpha(vec3 ray, vec3 end, vec3 dir, vec3 modifier, int it, float height, float thickness)
{
	float ray_step = 100.0;
	float t;
	float accum = 0.0;

	if((abs(dir.z) < 0.01 && ray.z < (height - thickness / 2.0))){
		return 0.0;
	}

	t = ((height - thickness / 2.0) - ray.z) / dir.z;
	ray += dir * t;

	t = ((height + thickness / 2.0) - end.z) / dir.z;
	end += dir * t;

	dir = normalize(end - ray);
// 	ray_step = distance(end, ray) / float(it);
	ray_step = (clouds_thickness) / float(it);

	int i;
	for(i=0; i<it; i++)
	{
		accum += CloudDensity(ray * modifier);
		if ( accum >= 1.0 ) {
			break;
		}
		ray += dir * ray_step * 1.0;
	}

	return accum;
}

vec4 HighClouds(vec3 ray, vec3 end, vec3 dir)
{
	float t;
	vec4 accum = vec4(0.0);
/*
	t = ((1000.0 - clouds_thickness / 2.0) - ray.z) / dir.z;
	ray += dir * t;

//	float d = CloudDensity(ray * 1.0 * vec3(1.0, 1.5, 1.0));
//	accum.a += d * 0.15 * float(CLOUDS_SAMPLES);
	//accum.rgb = CloudColor(ray);
	//accum.rgb = clamp(accum.rgb, vec3(0.15), vec3(1.0));

	accum.a = CloudsAlpha(ray, end, dir, 1.0 * vec3(1.0, 1.0, 1.0), 2, 800.0, clouds_thickness);
*/

	t = ((clouds_height + clouds_thickness * 6.0 - clouds_thickness / 2.0) - ray.z) / dir.z;
	ray += dir * t;

	accum.rgb = 1.25 * CloudColor(ray);
	accum.a = CloudsAlpha(ray, end, dir, 1.0 * vec3(1.0, 1.0, 1.0), 2, clouds_height + clouds_thickness * 6.0, clouds_thickness);

	accum.a = clamp(accum.a, 0.0, 1.0);
	return accum;
}

float Thunder(vec3 ray, vec3 end, vec3 dir)
{
	float ret = 0.0;

//	ret = PRidged3(ray * 0.025, 2);
	ret = clamp(PNoise3Opt(ray * 0.001, vec3(0.0), 3, 0.55, 2.8), 0.0, 1.0);
	float f = PNoise3Opt(vec3(cos(ge_Time) * 0.25, 0.0, 0.0), vec3(ge_Time, 0.0, 0.0), 3, 1.0, 8.0) * 0.5 + 0.5;
	ret *= clamp(exp(f) - 1.5, 0.0, 1.0);
	return ret;
}

vec4 ComputeClouds(vec3 pos, vec3 cam)
{
	vec3 ray = cam + vec3(ge_Time * speed * 0.0, 0.0, 0.0);
	vec3 end = pos + vec3(ge_Time * speed * 0.0, 0.0, 0.0);
	vec3 dir = normalize(end - ray);

	if(end.z < 10000.0 || distance(pos.xy, cam.xy) > 500000.0 ){
		return vec4(0.0);
	}

	float t = ((clouds_height - clouds_thickness / 2.0) - ray.z) / dir.z;
	ray += dir * t;

//	vec4 high = HighClouds(ray, end, dir);

	vec4 accum = vec4(0.0);

	accum.a = CloudsAlpha(ray, end, dir, vec3(1.0), CLOUDS_SAMPLES, clouds_height, clouds_thickness);
	if ( accum.a == 0.0 ) {
		return vec4( 0.0 );
	}
	accum.a = clamp(accum.a, 0.0, 1.0);

	accum.rgb = CloudColor(ray);
	accum.rgb = clamp(accum.rgb, vec3(0.05), vec3(1.0));
/*
//	accum = vec4(0.0);

//	accum += high;
	accum.rgb *= (accum.a);
	accum.rgb += high.rgb * (1.0 - accum.a);
	accum.a += high.a;
*/

// 	if(cover <= 50){
// 		accum.rgb = mix(accum.rgb, vec3(2.0, 2.0, 3.0), Thunder(ray, end, dir));
// 	}

	accum.a *= 1.0 - clamp(distance(cam, ray) / 100000.0, 0.0, 1.0);
// 	accum.rgb += sun_color * clamp(distance(cam, ray) / 100000.0, 0.0, 1.0);
// 	accum.rgb += mix( accum.rgb, sun_far_color * 0.0000000001, clamp(distance(cam, ray) / 100000.0, 0.0, 1.0) );
	return accum;
}
