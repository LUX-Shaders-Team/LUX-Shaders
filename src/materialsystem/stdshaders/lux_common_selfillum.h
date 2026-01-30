//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	21.02.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

#ifndef LUX_COMMON_SELFILLUM_H_
#define LUX_COMMON_SELFILLUM_H_

#if !PROJTEX // SelfIllum is not rendered under Projected Textures
	//==========================================================================//
	//	Constants
	//==========================================================================//
	#if SELFILLUM
		// by Defining this, it allows Shaders to move the Registers to something else
		#if !defined(MOVED_REGISTERS_SELFILLUM)
			const float4	cSelfIllumTint_Factor			: register(LUX_PS_FLOAT_SELFILLUM_FACTORS);
			#define			g_f3SelfIllumTint					(cSelfIllumTint_Factor.rgb)
			#define			g_f1SelfIllumMaskFactor				(cSelfIllumTint_Factor.w)
			const float4	cSelfIllumFresnel				: register(LUX_PS_FLOAT_SELFILLUM_FRESNEL);
			#define			g_f1SelfIllumFresnelScale			(cSelfIllumFresnel.x)
			#define			g_f1SelfIllumFresnelBias			(cSelfIllumFresnel.y)
			#define			g_f1SelfIllumFresnelExponent		(cSelfIllumFresnel.z)
			// .w free
		#endif

	//==========================================================================//
	//	Samplers
	//==========================================================================//

	#if !defined(MOVED_SAMPLERS_SELFILLUM)
		// $selfillummask or $selfillumtexture
		#if !SELFILLUM_ENVMAPMASK_ALPHA
			sampler Sampler_SelfIllum		: register(s13);
		#endif
	#endif

	//==========================================================================//
	//	Functions
	//==========================================================================//

	// Computes the Fresnel for SelfIllum
	float ComputeSelfIllumFresnel(float f1NdotV)
	{
		#if !defined(NO_SELFILLUMFRESNEL)
			// Consistent with Stock VLG Bump & Phong
			// NOTE: If SelfIllumFresnel is NOT enabled, Bias will be 1.0f and it will saturate.
			// Saves us a bool
			float f1Fresnel = pow(f1NdotV, g_f1SelfIllumFresnelExponent) * g_f1SelfIllumFresnelScale + g_f1SelfIllumFresnelBias;
				
			// Avoid >1
			return saturate(f1Fresnel);
		#else
			return 1.0f;
		#endif
	}
	#endif // Selfillum
#endif // !PROJTEX

#endif // LUX_COMMON_SELFILLUM_H_