//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	26.01.2026 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

#ifndef LUX_COMMON_INTRO_H_
#define LUX_COMMON_INTRO_H_

// Unchanged Stock Functions from vortwarp_vs20_helper.h
// This is used for the EP1 Intro Effect

float Sine(float min, float max, float t)
{
	return (sin(t) * 0.5f + 0.5f) * (max - min) + min;
}

float3 QuadraticBezier(float3 A, float3 B, float3 C, float t)
{
	return lerp(lerp(A, B, t), lerp(B, C, t), t);
}

float3 CubicBezier(float3 A, float3 B, float3 C, float3 D, float t)
{
	return QuadraticBezier(lerp(A, B, t), lerp(B, C, t), lerp(C, D, t), t);
}

// Simplified this Function
// Now returns modified WorldPos instead of InOut
float3 WorldSpaceVertexProcess(const float3 f3ModelOrigin, const float3 f3WorldPos, float f1Time)
{
	f1Time = saturate(1.0f - f1Time);
	f1Time *= f1Time;
	f1Time *= f1Time;
	f1Time *= f1Time;

	// end
	float3 A = float3( 0.0f, 0.0f, 1.0f );
	float3 B = float3( 1.0f, 1.0f, 1.0f );
	float3 C = float3( 0.0f, 0.0f, 1.0f );
	float3 D = float3( 0.0f, 0.0f, 1.0f );
	// start
	
	// "about 72 inches tall"
	float t = f3WorldPos.z * (1.0f / (72.0f));
	t = saturate(t);
	float3 f3WorldPosDelta = (f3WorldPos - f3ModelOrigin) * CubicBezier(A, B, C, D, t);
	f3WorldPosDelta.z += Sine(0.0f, 10.0, f3WorldPos.z);
	return lerp(f3WorldPos, f3WorldPosDelta + f3ModelOrigin, f1Time);
}

#endif //#ifndef LUX_COMMON_INTRO_H_
