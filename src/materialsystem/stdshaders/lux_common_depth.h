//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	22.05.2024 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

#ifndef LUX_COMMON_DEPTH
#define LUX_COMMON_DEPTH

	// ShiroDkxtro2 :
	// The default Depth "Buffer" (Framebuffer Alpha) Range is 192 Units
	// However we never *actually* receive this as a readable value anywhere, as far as I'm concerned.
	// It is unclear to me how they ( The VDC Article below ) derived it. 
	// https://developer.valvesoftware.com/wiki/Depth_buffer
	//
	// We would usually get the Depth Buffer Range in a precomputed Value via OO_DESTALPHA_DEPTH_RANGE
	// When we want to override this Value we use a define. The thing is that its very easy to write bigger depth ranges using this method
	// However doing so will just outright break ALL PARTICLES as the default SpriteCard Shader will receive its Depth Range via a separate PSREG
	// More specifically, Particles become too transparent, only on Pixels that a custom Shader with the new Depth Range has written.
	// To fix this, we need to send the correct range via the register, under the condition of CUSTOM_DEPTH_RANGE
	// Which poses a minor challenge.. as SetDepthFeatheringPixelShaderConstant is closed source.
	// //
	// Determining what this Function does is relatively simple however.
	// It has a single Input, a Depth Scale Value.
	// Since it has to do with the Depth Range, it's Value is derivable from OO_DESTALPHA_DEPTH_RANGE
	// If the Range didn't match we would have the Particle Issue we are already having.
	// The Result we get is (DepthRange / Scale)
	// The original Assumumption was multiplication ( since it's called Scale ) but that causes a Missmatch.
	// This is probably done because the regular depth-range is an RCP Value
	// We want to scale down, not up.

#if defined(CUSTOM_DEPTH_RANGE)
	float SoftParticleDepth(float f1Depth)
	{
		return f1Depth * CUSTOM_DEPTH_RANGE;
	}
#else
	// You should have lux_common_ps_fxc.h included at this point, for the definition of OO_DESTALPHA_DEPTH_RANGE
	float SoftParticleDepth(float f1Depth)
	{
		return f1Depth * OO_DESTALPHA_DEPTH_RANGE;
	}
#endif

float DepthFeathering(sampler DepthSampler, const float4 f4ProjPos, float f1DepthRangeFactor)
{
	// This would usually just happen outside, but I moved it here
	float2 f2ScreenPos = f4ProjPos.xy / f4ProjPos.w;

	// The Depth in the Framebuffer alpha is already scaled by OO_DESTALPHA_DEPTH_RANGE or CUSTOM_DEPTH_RANGE
	// SoftParticleDepth will scale by the same value so the two have the same scale.
	float f1SceneDepth = tex2Dlod(DepthSampler, float4(f2ScreenPos, 0.0f, 0.0f)).a; // "PC uses dest alpha of the frame buffer"
	float f1SpriteDepth = SoftParticleDepth(f4ProjPos.z);

	// We are now comparing the scene depth from the perspective of the player,
	// to the Depth-Value of the particle/sprite we are on.
	// NOTE: Technically possible -- (f1SpriteDepth + f1HeightMap * Factor)
	// Could be a nice way to add some *actual* depth to some particles
	// "as the sprite approaches the edge of our compressed depth space, the math stops working.
	// So as the sprite approaches the far depth, smoothly remove feathering."
	// ????: Is this still a problem with CUSTOM_DEPTH_RANGE?
	// Since you can input some really whacky scale factors, make sure this never exceeds [0..1], saturate
	float f1FeatheredAlpha;
	f1FeatheredAlpha = abs(f1SceneDepth - f1SpriteDepth) * f1DepthRangeFactor;
	f1FeatheredAlpha = max(smoothstep(0.75f, 1.0f, f1SceneDepth), f1FeatheredAlpha);
	f1FeatheredAlpha = saturate(f1FeatheredAlpha);

	return f1FeatheredAlpha;
}

#endif // End of LUX_COMMON_DEPTH