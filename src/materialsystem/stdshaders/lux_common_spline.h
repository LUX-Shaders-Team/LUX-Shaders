//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	08.12.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

#ifndef LUX_COMMON_SPLINE_H_
#define LUX_COMMON_SPLINE_H_

// "Derivative of catmull rom spline courtesy of calc"
float4 DCatmullRomSpline ( float4 a, float4 b, float4 c, float4 d, float t )
{
	return 0.5f * ( c - a + t * ( 2.0f * a - 5.0f * b + 4.0f * c - d + t * (3.0f * b - a - 3.0f * c + d ) ) 
						 + t * ( 2.0f * a - 5.0f * b + 4.0f * c - d + 2.0f * ( t * ( 3.0f * b - a - 3.0f * c + d ) ) ) );
}

float3 DCatmullRomSpline3 ( float3 a, float3 b, float3 c, float3 d, float t )
{
	return 0.5f * ( c - a + t * ( 2.0f * a - 5.0f * b + 4.0f * c - d + t * (3.0f * b - a - 3.0f * c + d ) ) 
				 + t * ( 2.0f * a - 5.0f * b + 4.0f * c - d + 2.0f * ( t * ( 3.0f * b - a - 3.0f * c + d ) ) ) );
}
    
float4 CatmullRomSpline( float4 a, float4 b, float4 c, float4 d, float t )
{
	return b + 0.5f * t * ( c - a + t * ( 2.0f * a - 5.0f * b + 4.0f * c - d + t * ( -a + 3.0f * b - 3.0f * c + d ) ) );
}

#endif // LUX_COMMON_SPLINE_H_