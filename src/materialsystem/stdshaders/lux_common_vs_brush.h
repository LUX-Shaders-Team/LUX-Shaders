//
//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	07.12.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

#ifndef LUX_COMMON_VS_BRUSH_H_
#define LUX_COMMON_VS_BRUSH_H_

// mul3x3 and mul4x3 part of lux_common_vs_fxc.h

// Single Input Alignment
void AlignInputWithoutOffset(const float3 f3Input, out float3 f3Output)
{
	f3Output = mul3x3(f3Input, (const float3x3)cModel[0]);
}

// Single Input Alignment
void AlignInputWithOffset(const float3 f3Input, out float3 f3Output)
{
	f3Output = mul4x3(float4(f3Input, 1.0f), cModel[0]);
}

void DecompressVertexNormal(float4 f4InputNormal, out float3 f3OutputNormal)
{
	// Nothing to decompress on Brushes
	f3OutputNormal = f4InputNormal.xyz;
}

#endif //#ifndef LUX_COMMON_VS_BRUSH_H_
