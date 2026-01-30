//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	25.04.2024 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

#ifndef LUX_COMMON_VS_SWAY_H_
#define LUX_COMMON_VS_SWAY_H_

// Register Map
#include "lux_registermap_hlsl.h"
#include "lux_registermap_vs.h"

// The $Treesway code is ported from Valve's Alien Swarm
// Then adapated so that things look more standardised
// Such as variable, function names and function layouts.
// Additionally the comments have been adjusted to be more precise and less vulgar.

// Tree sway is a vertex animation process in which the vertices of a model get moved in various directions,
// based on external parameters such as wind. This works as we can move the position of a vertex before we tell the PixelShader where it has to draw.

// Treesway Mode 2 is a hack that is used for 'rectangular sheets of plastic/tarp attached at the four corners'
// It works by inverting the sway scale radius to be 1 at (0,0,0). The fall off is radially towards the edges of the model.
// The model is expected to be build lying in the xy plane. ( flat ), with its center at/as the origin.
// $TreeswayStrength should be 0 in the vmt.

#if (VERTEX_SWAY == 1 || VERTEX_SWAY == 2)

const float4 cMiscParams1 : register(LUX_VS_FLOAT_VERTEXSWAY_01);
const float4 cMiscParams2 : register(LUX_VS_FLOAT_VERTEXSWAY_02);
const float4 cMiscParams3 : register(LUX_VS_FLOAT_VERTEXSWAY_03);
const float4 cMiscParams4 : register(LUX_VS_FLOAT_VERTEXSWAY_04);
const float4 cMiscParams5 : register(LUX_VS_FLOAT_VERTEXSWAY_05);

#define g_f1Time					(cMiscParams1.x)
#define g_f1ScrumbleFalloffCurve	(cMiscParams1.y)
#define g_f1SwayFalloffCurve		(cMiscParams1.z)
#define g_f1ScrumbleSpeed			(cMiscParams1.w)

#define g_f1FastSwaySpeedScale		(cMiscParams2.x)
#define g_f1FastSwaySpeedScale2		(cMiscParams2.y)
#define g_f2WindDir					(cMiscParams2.zw)

// Previously:
/*
#define g_flHeight					(cMiscParams3.x)
#define g_flStartHeight				(cMiscParams3.y)
#define g_flRadius					(cMiscParams3.z)
#define g_flStartRadius				(cMiscParams3.w)
*/
#define g_f1Height_MUL				(cMiscParams3.x)
#define g_f1Height_RCP				(cMiscParams3.y)
#define g_f1Radial_MUL				(cMiscParams3.z)
#define g_f1Radial_RCP				(cMiscParams3.w)

#define g_f1SwaySpeed				(cMiscParams4.x)
#define g_f1SwayStrength			(cMiscParams4.y)
#define g_f1ScrumbleFrequency		(cMiscParams4.z)
#define g_f1ScrumbleStrength		(cMiscParams4.w)

#define g_f1WindIntensity			(cMiscParams5.x)
#define g_f1WindSpeedLerp			(cMiscParams5.y)

//	f1FastSwaySpeedScale
//	f1FastSwaySpeedScale2 = f1FastSwaySpeedScale * 2.14f
//	f1WindSpeedLerp = smoothstep(g_flWindSpeedLerpStart, g_flWindSpeedLerpEnd, flWindIntensity);
//	f1WindIntensity = length(f2WindDir);
//	f1Height_MUL = f1Height * f1StartHeight;
//	f1Height_RCP = 1.0f / ((1.0f - f1StartHeight) * f1Height);
//	f1Radial_MUL = f1Radius * f1StartRadius;
//	f1Radial_RCP = 1.0f / ((1.0f - f1StartRadius) * f1Radius);

float3 ComputeSway(float3 f3vPosition)
{
	// Model root position is the translation component of the model to world matrix
	float3 f3ModelRoot = float3(cModel[0][3].x, cModel[0][3].y, cModel[0][3].z);

	// Transform the wind direction into model space
	float3 f3WindDirAndIntensityOS = mul((float3x3)cModel[0], float3(g_f2WindDir, 0.0f));

	// Previously :
	// float f1SwayScaleHeight = saturate((f3vPosition.z - f1Height * f1StartHeight) /
	// ((1.0 - f1StartHeight) * f1Height));
	// Now, as its faster, compute these two per Material :
	// float f1Height_MUL = f1Height * f1StartHeight;
	// float f1Height_RCP = 1.0f / ((1.0f - f1StartHeight) * f1Height);
//	float f1SwayScaleHeight = saturate((f3vPosition.z - g_flHeight * g_flStartHeight) / ((1.0 - g_flStartHeight) * g_flHeight));
	float f1SwayScaleHeight = saturate((f3vPosition.z - g_f1Height_MUL) * g_f1Height_RCP);
	
	// Previously :
	// float f1SwayScaleRadius = saturate(length((f3vPosition.xy) - f1Radius * f1StartRadius) /
	// ((1.0 - f1StartRadius) * f1Radius));
	// Now, as its faster, compute these two per Material :
	// float f1Radial_MUL = f1Radius * f1StartRadius;
	// float f1Radial_RCP = 1.0f / ((1.0f - f1StartRadius) * f1Radius);
//	float f1SwayScaleRadius = saturate(length((f3vPosition.xy) - g_flRadius * g_flStartRadius) / ((1.0 - g_flStartRadius) * g_flRadius));
	float f1SwayScaleRadius = saturate(length((f3vPosition.xy) - g_f1Radial_MUL) * g_f1Radial_RCP);

#if (VERTEX_SWAY == 1)
	// Used to turn off branch sway and scrumble below the minimum sway height
//	float f1HeightThreshold = step(0, f3vPosition.z - g_flHeight * g_flStartHeight);
	float f1HeightThreshold = step(0, f3vPosition.z - g_f1Height_MUL);
#else // Previously TREESWAY == 2
	// Works better for hanging vines
//	float f1HeightThreshold = step(f3vPosition.z - g_flHeight * g_flStartHeight, 0);
	float f1HeightThreshold = step(f3vPosition.z - g_f1Height_MUL, 0);
#endif

	
	static const float f1WindOffsetScale = 19;
	float f1WindTimeOffset = dot(f3ModelRoot.xyz, float3(1.0f, 1.0f, 1.0f)) * f1WindOffsetScale;
	float f1SlowSwayTime = (g_f1Time + f1WindTimeOffset) * g_f1SwaySpeed;


	// lerp between slow and fast sines based on wind speed
	// Previously :
	// --float f1WindIntensity = length(f2WindDir);-- Defined way above
	// float f1WindSpeedLerp = smoothstep( f1WindSpeedLerpStart, f1WindSpeedLerpEnd, f1WindIntensity );
	// float4 f4BunchOfSines = sin(float4(1.0, 2.31, f1FastSwaySpeedScale, 2.14 * f1FastSwaySpeedScale) * f1SlowSwayTime.xxxx);
	// f4BunchOfSines.xy = lerp(f4BunchOfSines.xy, f4BunchOfSines.zw, f1SpeedLerp);
	//
	// Now with less GPU Instructions, thanks to moving things to the C++ portion of the Shader :
	float4 f4BunchOfSines	  = sin(float4(1.0f, 2.31f, g_f1FastSwaySpeedScale, g_f1FastSwaySpeedScale2) * f1SlowSwayTime.xxxx);
			f4BunchOfSines.xy = lerp(f4BunchOfSines.xy, f4BunchOfSines.zw, g_f1WindSpeedLerp);

	// The eventual Offset by which we move the Position of the Vertex
	float f1SwayScaleTrunk = g_f1SwayStrength * pow(f1SwayScaleHeight, g_f1SwayFalloffCurve);

	float3 f3PositionOffset = f3WindDirAndIntensityOS * f1SwayScaleTrunk * (f4BunchOfSines.x + 0.1f);

	// From ASW :
	//	Scale branch motion based on how orthogonal they are
	//	This is what I want to compute:
	//	float flOrthoBranchScale = 1.0 - abs( dot( normalize( vWindDirAndIntensityOS.xyz ), float3( normalize( vPositionOS.xy ), 0 ) ) );
	//	Some NV hardware (7800) will do bad things when normalizing a 0 length vector. Instead, I'm doing the dot product unnormalized
	//	and divide by the length of the vectors, making sure to avoid divide by 0.
	//	float f1OrthoBranchScale = abs(dot(f3WindDirAndIntensityOS.xyz, float3(f3vPosition.xy, 0)));
	//	f1OrthoBranchScale = 1.0 - saturate(f1OrthoBranchScale / (max(length(f3WindDirAndIntensityOS.xyz), 0.0001) * max(length(f3vPosition.xy), 0.0001)));
	// Now :
	// I don't think this is any longer a hardware issue. On any relatively modern hardware anyways.
	// But using the other Code for consistency now
#if (VERTEX_SWAY == 1)
//	float f1OrthoBranchScale = 1.0 - abs(dot(normalize(f3WindDirAndIntensityOS.xyz), float3(normalize(f3vPosition.xy), 0)));
	float f1OrthoBranchScale = abs(dot(f3WindDirAndIntensityOS, float3(f3vPosition.xy, 0)));
	f1OrthoBranchScale = 1.0f - saturate(f1OrthoBranchScale / (max(length(f3WindDirAndIntensityOS.xyz), 0.0001f) * max(length(f3vPosition.xy), 0.0001f)));

	float f1SwayScaleBranches = g_f1SwayStrength * f1OrthoBranchScale * f1SwayScaleRadius * f1HeightThreshold;

	// Previously it would set ( f1SwayScaleBranches = 0 ) when TREESWAY == 2
	// So now we just don't do all of these unless SWAY == 1
	f3PositionOffset.xyz += f3WindDirAndIntensityOS * f1SwayScaleBranches * (f4BunchOfSines.y + 0.4f);
#endif

	float f1ScrumbleScale = pow(f1SwayScaleRadius, g_f1ScrumbleFalloffCurve) * g_f1ScrumbleStrength * f1HeightThreshold;
	float3 f3ScrumbleScale = f1ScrumbleScale.xxx;

#if (VERTEX_SWAY == 2)
	f3ScrumbleScale *= float3(0.5f, 0.5f, 1.0f);
#endif

	// ShiroDkxtro2 : Doesn't this just give the direction in which the vertex is located?
	float3 f3SwayPosOS = normalize(f3vPosition.xyz) * g_f1ScrumbleFrequency;

	// Previously sin(f1ScrumbleSpeed * g_f1Time + ...)
	// Now sin(f1ScrumbleSpeed + ...)
	// Simple precompute
	f3PositionOffset.xyz += g_f1WindIntensity * (f3ScrumbleScale.xyz * sin(g_f1ScrumbleSpeed.xxx * g_f1Time.xxx + f3SwayPosOS.yzx + f1WindTimeOffset.xxx));

	return f3vPosition.xyz + f3PositionOffset.xyz;
}
#endif

#endif // LUX_COMMON_VS_SWAY_H_