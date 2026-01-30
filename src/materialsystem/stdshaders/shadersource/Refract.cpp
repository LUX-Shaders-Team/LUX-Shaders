//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	20.01.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../cpp_lux_shared.h"

// Includes for Shaderfiles...
#include "lux_refract_vs30.inc"
#include "lux_refract_ps30.inc"

// LUX Shaders will replace existing Shaders.
#ifdef REPLACE_SDK_SHADERS
DEFINE_FALLBACK_SHADER(SDK_Refract,			LUX_Refract)
DEFINE_FALLBACK_SHADER(SDK_Refract_DX90,	LUX_Refract)
DEFINE_FALLBACK_SHADER(SDK_Refract_DX80,	LUX_Refract)
DEFINE_FALLBACK_SHADER(SDK_Refract_DX60,	LUX_Refract)
#endif

#ifdef REPLACE_REFRACT
DEFINE_FALLBACK_SHADER(Refract,			LUX_Refract)
DEFINE_FALLBACK_SHADER(Refract_DX90,	LUX_Refract)
DEFINE_FALLBACK_SHADER(Refract_DX80,	LUX_Refract)
DEFINE_FALLBACK_SHADER(Refract_DX60,	LUX_Refract)
#endif

//==========================================================================//
// CommandBuffer Setup
//==========================================================================//
class RefractContext : public LUXPerMaterialContextData
{
public:
	ShrinkableCommandBuilder_t<5000> m_StaticCmds;
	CommandBuilder_t<1000> m_SemiStaticCmds;

	// Snapshot / Dynamic State
	BlendType_t m_nBlendType = BT_NONE;
	bool m_bIsFullyOpaque = false;

	// Everything related to constants

	RefractContext(CBaseShader* pShader)
		: m_SemiStaticCmds(pShader),
		m_StaticCmds(pShader)
	{
	}
};

//==========================================================================//
// Shader Start
//==========================================================================//
BEGIN_VS_SHADER(LUX_Refract, "Transparent Surface Shader with $Bumpmap based distortion.")
SHADER_INFO_GEOMETRY	("Brushes and Models.")
SHADER_INFO_USAGE		("A Material applied to a Model, that also has other Materials,\n"
						 "must specify the $MostlyPpaque Parameter in the QC File & $NoWritez 1 & $Model 1 in the VMT.\n"
						 "If the Model has a single Material, only $Model 1 is required.")
SHADER_INFO_LIMITATIONS	("The Shader is affected by the Order in which the Scene is rendered.\n"
						 "Depending on how it's used, z-sorting Issues *can* occur.\n"
						 "There are two Issues relating to this.\n"
						 "The Shader will use a Framebuffer Texture for Rendering, and that Texture contains only what's in View.\n"
						 "Due to this, it doesn't render Particles that are behind the Surface.\n"
						 "Only Opaque Materials are rendered in the Refract View.\n"
						 
						 "Refractions near the Border of the Texture may be incorrect.\nRefract Scales that are too large will more easily reach over the Texture Border.\n")
SHADER_INFO_PERFORMANCE	("Expensive, especially with Blur and/or lots of Overdraw.")
SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
SHADER_INFO_WEBLINKS	(WEBLINK_VDC
						"VDC Refract Shader Page: https://developer.valvesoftware.com/wiki/Refract")
SHADER_INFO_D3D			(LUX_SHADERINFO_SM30)

BEGIN_SHADER_PARAMS
	SHADER_PARAM(NormalMap,		SHADER_PARAM_TYPE_TEXTURE,	"", "[RGB] Normal Map.\n[A] Mask for Refraction.")
	SHADER_PARAM(BumpFrame,		SHADER_PARAM_TYPE_INTEGER,	"", "Frame Number to be used with $NormalMap")
	SHADER_PARAM(BumpTransform, SHADER_PARAM_TYPE_MATRIX,	"", "Transforms the $NormalMap. Must include all Values!")

	SHADER_PARAM(NormalMap2,		SHADER_PARAM_TYPE_TEXTURE,	"", "[RGB] Normal Map.\n[A] Mask for Refraction.\nModulated with the $NormalMap.")
	SHADER_PARAM(BumpFrame2,		SHADER_PARAM_TYPE_INTEGER,	"", "Frame Number to be used with $NormalMap2")
	SHADER_PARAM(BumpTransform2,	SHADER_PARAM_TYPE_MATRIX,	"", "Transforms the $NormalMap2. Must include all Values!")

	SHADER_PARAM(RefractAmount,			SHADER_PARAM_TYPE_FLOAT,	"", "The Strength of the Refraction Effect.")
	SHADER_PARAM(RefractTint,			SHADER_PARAM_TYPE_COLOR,	"", "Tint for the Refraction Results. Note this does not apply to unrefracted Portions.")
	SHADER_PARAM(BlurAmount,			SHADER_PARAM_TYPE_BOOL,		"", "Enables Blur.\n0 = None\n1 = Blur")
	SHADER_PARAM(FadeOutOnSilhouette,	SHADER_PARAM_TYPE_BOOL,		"", "Enables Fresnel based Fade of the Refraction.")

	SHADER_PARAM(RefractTintTexture, SHADER_PARAM_TYPE_TEXTURE, "", "Tints the Refraction on a per-Texel basis.")
	SHADER_PARAM(RefractTintTextureFrame, SHADER_PARAM_TYPE_INTEGER, "0", "Frame Number to be used with $RefractTintTexture.")

	SHADER_PARAM(NoWriteZ, SHADER_PARAM_TYPE_INTEGER, "", "0 = Write Depth.\n1 = Don't write Depth.")
	SHADER_PARAM(Masked, SHADER_PARAM_TYPE_BOOL, "", "Mask the Result using Destination Alpha.")
	SHADER_PARAM(VertexColorModulate, SHADER_PARAM_TYPE_BOOL, "", "Use the Vertex Color to influence the Color of Refraction.\nAlpha will adjust the amount of Refraction.")
	SHADER_PARAM(ForceAlphaWrite, SHADER_PARAM_TYPE_BOOL, "", "Force the Material to write Alpha into the the Result.")

	// From Alien Swarm:
	SHADER_PARAM(MagnifyEnable, SHADER_PARAM_TYPE_BOOL, "", "Enable the magnification of the refracted $BaseTexture.")
	SHADER_PARAM(MagnifyCenter, SHADER_PARAM_TYPE_VEC2, "", "The refracted $BaseTexture is magnified around this Coordinate. 0..1")
	SHADER_PARAM(MagnifyScale, SHADER_PARAM_TYPE_FLOAT, "", "Magnification Factor.")

	// Portal2 Recreation:
	SHADER_PARAM(LocalRefract, SHADER_PARAM_TYPE_BOOL, "", "Applies a Parallax Mapping Effect to $BaseTexture.")
	SHADER_PARAM(LocalRefractDepth, SHADER_PARAM_TYPE_FLOAT, "", "The Distance away from the Surface that the $BaseTexture should be at.")

	// LUX Additions:
	// 
	// $FlipNormalZ is a mrFunreal and Cief. Suggestion.
	// Issue mentioned in Dead4Mods: Weapon Scopes with Refract need to use inverted Normals, so Cubemaps are oriented correctly.
	SHADER_PARAM(FlipNormal, SHADER_PARAM_TYPE_BOOL, "", "Flips the Normal Maps Z Direction. Useful for Weapon Scopes that need $EnvMap.")
	Declare_EnvironmentMapParameters()
	Declare_EnvMapMaskParameters()
	Declare_ParallaxCorrectionParameters()
	SHADER_PARAM(BumpMap, SHADER_PARAM_TYPE_TEXTURE, "", "Same as $NormalMap for this Shader.")
	SHADER_PARAM(BumpMap2, SHADER_PARAM_TYPE_TEXTURE, "", "Same as $NormalMap2 for this Shader.")

	// FIXME: FresnelReflection not actually implemented..
	// FIXME: $Time used somewhere?
	// FIXME: ForceAlphaWrite not actually implemented..
END_SHADER_PARAMS

void RefractShaderFlags()
{
	// Materials not using RenderTargets should not have the translucent Flag,
	// and they don't need a Framebuffer Texture.
	// To check for this we can see if there is a $BaseTexture and if it has _rt_
	// This assumes all Rendertargetse have the _rt_ Identifier
	// It's particularly important to consider this so Brushes with Features like $LocalRefract,
	// don't cause the maps to leak during a compile. ( Translucent Brushes don't seal the Map )
	bool bRendertarget = true;
	if (IsDefined(BaseTexture))
	{
		// Note that this Code runs during Param Init,
		// We don't have a Texture Value on the Parameter, just grab the String directly.
		const char* ccTextureName = GetString(BaseTexture);

		// Don't know if strstr is appropriate here
		if (!strstr(ccTextureName, "_rt_"))
			bRendertarget = false;
	}

	if (bRendertarget)
	{
		SetFlag(MATERIAL_VAR_TRANSLUCENT);
		SetFlag2(MATERIAL_VAR2_NEEDS_POWER_OF_TWO_FRAME_BUFFER_TEXTURE);
	}

	// Always need Tangents for BumpMap WorldSpace Conversion
	SetFlag2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);

	// Stock Shader doesn't set this Flag for some Reason.
	// In Theory that means it can't animate models.
	// I set it here because it should logically be set.
	if (HasFlag(MATERIAL_VAR_MODEL))
		SetFlag2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);
	
	// Never gets lit by Projected Textures
	// It should? Stock-Consistency: Not doing it
	SetBool(ReceiveProjectedTextures, false);
}

SHADER_INIT_PARAMS()
{
	RefractShaderFlags();

	DefaultFloat(EnvMapSaturation, 1.0f);
	DefaultFloat(FresnelReflection, 1.0f);

	// 'Parallax' Offset should never be 0.0f ( Default is 0.0f on Alien Swarm Reactive Drop )
	// I'm setting it to a small Value so there is at least some Offset going on when someone turns this Feature on..
	DefaultFloat(LocalRefractDepth, 0.1f);

	// Support for $BumpMap and $BumpMap2
	// Deep-Down we actually use $NormalMap.. and $NormalMap2
	// NOTE: Do not set $NormalMap to $BumpMap if there already IS a $NormalMap
	// Some Water Overlays specify a DuDv Map similar to the Water Shader itself!!
	if (!IsDefined(NormalMap) && IsDefined(BumpMap))
		SetString(NormalMap, GetString(BumpMap));

	if (!IsDefined(NormalMap2) && IsDefined(BumpMap2))
		SetString(NormalMap2, GetString(BumpMap2));

	// Stock-Consistency: Not lit by Projected Textures
	SetBool(ReceiveProjectedTextures, false);
}

SHADER_FALLBACK
{
	if (g_pHardwareConfig->GetDXSupportLevel() < 90)
	{
		Warning("Game run at DXLevel < 90 \n");
		return "Wireframe";
	}

	return 0;
}

SHADER_INIT
{
	LoadTexture(BaseTexture, TEXTUREFLAGS_SRGB);
	LoadBumpMap(NormalMap);
	LoadBumpMap(NormalMap2);

	LoadTexture(RefractTintTexture, TEXTUREFLAGS_SRGB);
	LoadTexture(EnvMapMask);

	// LDR sRGB loadflag?
	LoadCubeMap(EnvMap);

	// Need to load these two in case $BumpFrame or $BumpFrame2 are linked to these Textures
	LoadBumpMap(BumpMap);
	LoadBumpMap(BumpMap2);	

	// According to Ficool2 ( aka Engine Code knowledge we shouldn't have or need ),
	// Parameters not set after Shader Init, are automatically initialised by the internal Shader System.
	// Now the Mapbase Implementation just used this Parameter, $EnvMapParallax to determine whether or not the Feature should be on
	// I will make a blend between VDC and Mapbase here because checking Parameter Types for whether it's not a VECTOR after setting INT is cursed
	if(IsDefined(EnvMapParallaxOBB1) && !GetBool(EnvMapParallax))
		DefaultBool(EnvMapParallax, true);
}

// Virtual Void Override for Context Data
RefractContext* CreateMaterialContextData() override
{
	return new RefractContext(this);
}

SHADER_DRAW
{
	// Get Context Data. BaseShader handles creation for us, using the CreateMaterialContextData() virtual
	auto* pContextData = GetMaterialContextData<RefractContext>(pContextDataPtr);
//	auto& StaticCmds = pContextData->m_StaticCmds;
	auto& SemiStaticCmds = pContextData->m_SemiStaticCmds;

	bool bIsModel	= HasFlag(MATERIAL_VAR_MODEL);
	bool bIsMasked	= GetBool(Masked);
	bool bIsBlurred = !bIsMasked && GetBool(BlurAmount); // Masked + this is a skipped Combo
	bool bWriteZ	= !GetBool(NoWriteZ);

	// NormalMaps
	bool bHasNormalMap	= IsTextureLoaded(NormalMap);
	bool bHasNormalMap2 = IsTextureLoaded(NormalMap2);

	// Other Textures
	bool bHasBaseTexture		= IsTextureLoaded(BaseTexture);
	bool bHasRefractTintTexture = !bIsMasked && IsTextureLoaded(RefractTintTexture); // Masked + this is a skipped Combo

	// $EnvMap and related
	bool bHasEnvMap		= IsTextureLoaded(EnvMap);
	bool bHasEnvMapMask = bHasEnvMap && IsTextureLoaded(EnvMapMask);
	bool bPCC = bHasEnvMap && GetBool(EnvMapParallax);

	//==========================================================================//
	// Pre-Snapshot Context Data Variables
	//==========================================================================//
	if (IsSnapshottingCommands())
	{
		int nTextureWithOpacity = BaseTexture;
		bool bIsBaseTexture = true;

		// "If envmap is not specified, the alpha channel is the translucency
		// (If envmap *is* specified, alpha channel is the reflection amount)"
		// $EnvMapMask frees up Normal Alpha for translucency
		// ( not really, The Shader loves using the $NormalMap Alpha for *Everything* )
		if (bHasNormalMap && !bHasEnvMap || bHasEnvMapMask)
		{
			nTextureWithOpacity = NormalMap;
			bIsBaseTexture = false;
		}
		
		pContextData->m_nBlendType = ComputeBlendType(nTextureWithOpacity, bIsBaseTexture);
		pContextData->m_bIsFullyOpaque = IsFullyOpaque(pContextData->m_nBlendType);

		// $Masked uses the 'RefractSampler' ( $BaseTexture ) Alpha as the Output
		// In this Case, Alpha of the $BaseTexture ( or Depth ) will act as the Mask for Transparency
		// This is always using inverse translucency, which doesn't relate to any BT_ Mode!
		if (bIsMasked)
		{
			pContextData->m_nBlendType = BT_NONE;
			pContextData->m_bIsFullyOpaque = false;
		}
	}

	//==========================================================================//
	// Static Snapshot of Shader Setup
	//==========================================================================//
	if (IsSnapshotting())
	{
		// Purpose : Int to tell the Shader what Mask to use.
		// 0 = Nothing
		// 1 = $EnvMap - Mask determined through abs(0||1 - Mask)
		// 2 = $EnvMap + $EnvMapMask
		// 3 = $EnvMap + PCC
		// 4 = $EnvMap + PCC + $EnvMapMask
		int nEnvMapMode = bHasEnvMap + bHasEnvMapMask + 2 * bPCC;

		//==========================================================================//
		// General Rendering Setup
		//==========================================================================//

		// This handles : $IgnoreZ, $Decal, $Nocull, $Znearer, $Wireframe, $AllowAlphaToCoverage
		// Source : OrangeBox/ASW Code which have BaseShader.cpp
		SetInitialShadowState();


		// NOTE: Not the same as $Translucent
		if (bIsMasked)
			EnableAlphaBlending(SHADER_BLEND_ONE_MINUS_SRC_ALPHA, SHADER_BLEND_SRC_ALPHA);
		else
		{
			// Everything Transparency is packed into this Function
			EnableTransparency(pContextData->m_nBlendType);		
		}

		// This is now handled in the flashlight sampler setup
		// pShader->DefaultFog(); 

		// Need AlphaWrites for Particle Depth if allowed
		// NOTE: Stock Shader sets DepthToDestAlpha to bIsFullyOpaque && WriteZ, but AlphaWrites to just bIsFullyOpaque
		// In other Words, we only have AlphaWrites *with* bWriteZ
		pShaderShadow->EnableAlphaWrites(pContextData->m_bIsFullyOpaque && bWriteZ);

		// Weird Name, what it actually means : We output linear Values
		pShaderShadow->EnableSRGBWrite(true);

		// Enables DepthWrites by default, $NoWriteZ overwrites this behaviour
		pShaderShadow->EnableDepthWrites(bWriteZ);

		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//

		// Always asking for the Normal
		unsigned int nFlags = VERTEX_POSITION | VERTEX_NORMAL;
		int nTexCoords = 1;
		int nUserDataSize = 0;

		// This Shader supports compression
		// Well yes, but Brushes don't get it.
		// Just setting this is fine though.
		nFlags |= VERTEX_FORMAT_COMPRESSED;

		// Tangents come through a different Stream for Models
		// Uncompressed Models get it through the TANGENT Stream ( vUserData )
		if (bIsModel)
		{
			nUserDataSize = 4;
		}
		else
		{
			nFlags |= VERTEX_TANGENT_SPACE;
		}

		// Is this not needed for Hammers Texture Shaded Polygons View?
		// HasFlag(MATERIAL_VAR_VERTEXCOLOR)
		if (GetBool(VertexColorModulate))
			nFlags |= VERTEX_COLOR;

		pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, NULL, nUserDataSize);

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//

		// s0 - $BaseTexture or Framebuffer. Always sRGB
		EnableSampler(SHADER_SAMPLER0, true);
		
		// s1 - $NormalMap. never sRGB
		EnableSampler(SHADER_SAMPLER1, false);

		// s2 - $NormalMap2. never sRGB
		EnableSampler(bHasNormalMap2, SHADER_SAMPLER2, false);

		// s3 - $RefractTintTexture. Always sRGB
		EnableSampler(bHasRefractTintTexture, SHADER_SAMPLER3, true);

		// s4 - $EnvMap. Only sRGB in LDR
		EnableSampler(bHasEnvMap, SHADER_SAMPLER4, !IsHDREnabled());

		// s5 - $EnvMapMask. Not sRGB ( Consistent with VLG )
		EnableSampler(bHasEnvMapMask, SAMPLER_ENVMAPMASK, false);

		//==========================================================================//
		// Declare Static Shaders
		//==========================================================================//
		DECLARE_STATIC_VERTEX_SHADER(lux_refract_vs30);
		SET_STATIC_VERTEX_SHADER_COMBO(BRUSH, !bIsModel);
		SET_STATIC_VERTEX_SHADER_COMBO(VERTEXCOLORS, GetBool(VertexColorModulate));
		SET_STATIC_VERTEX_SHADER_COMBO(SECONDTEXCOORD, bHasNormalMap2);
		SET_STATIC_VERTEX_SHADER(lux_refract_vs30);

		DECLARE_STATIC_PIXEL_SHADER(lux_refract_ps30);
		SET_STATIC_PIXEL_SHADER_COMBO(MASKED, bIsMasked);
		SET_STATIC_PIXEL_SHADER_COMBO(SILHOUETTEFADE, !bIsMasked && GetBool(FadeOutOnSilhouette)); // Masked + this is a skipped Combo
		SET_STATIC_PIXEL_SHADER_COMBO(REFRACTTINTTEXTURE, bHasRefractTintTexture);
		SET_STATIC_PIXEL_SHADER_COMBO(BLUR, bIsBlurred);
		SET_STATIC_PIXEL_SHADER_COMBO(ENVMAPMODE, nEnvMapMode);
		SET_STATIC_PIXEL_SHADER_COMBO(SECONDARYNORMAL, bHasNormalMap2);
		SET_STATIC_PIXEL_SHADER_COMBO(MAGNIFY, GetBool(MagnifyEnable));
		SET_STATIC_PIXEL_SHADER_COMBO(LOCALREFRACT, GetBool(LocalRefract));
		SET_STATIC_PIXEL_SHADER(lux_refract_ps30);
	}

	//==========================================================================//
	// Post-Snapshot Context Data Static Commands
	//==========================================================================//
	if (IsSnapshottingCommands())
	{
		// Set the Buffer back to its original ( Empty ) State
		/*
		StaticCmds.Reset();

		//==========================================================================//
		// Bind Standard Textures
		//==========================================================================//

		// Instruct the Buffer to set an End Point
		StaticCmds.End();
		*/
	}

	//==========================================================================//
	// Pre-Dynamic Context Data Semi-Static Commands
	//==========================================================================//
	SEMI_STATIC_COMMANDS
	{
		// Set the Buffer back to its original ( Empty ) State
		SemiStaticCmds.Reset(this);

		//==========================================================================//
		// Bind Textures
		//==========================================================================//

		// s0 - $BaseTexture or Framebuffer
		if (bHasBaseTexture)
			SemiStaticCmds.BindTexture(SHADER_SAMPLER0, BaseTexture, Frame);
		else
			SemiStaticCmds.BindTexture(SHADER_SAMPLER0, TEXTURE_FRAME_BUFFER_FULL_TEXTURE_0);

		// s1 - $NormalMap or $BumpMap
		if(bHasNormalMap)
			SemiStaticCmds.BindTexture(SHADER_SAMPLER1, NormalMap, BumpFrame);
		else
			SemiStaticCmds.BindTexture(SHADER_SAMPLER1, TEXTURE_NORMALMAP_FLAT);

		// s2 - $NormalMap2 or $BumpMap2
		if(bHasNormalMap2)
			SemiStaticCmds.BindTexture(SHADER_SAMPLER2, NormalMap2, BumpFrame2);

		// s3 - $RefractTintTexture
		if(bHasRefractTintTexture)
			SemiStaticCmds.BindTexture(SHADER_SAMPLER3, RefractTintTexture, RefractTintTextureFrame);

		// s5 - $EnvMapMask
		if(bHasEnvMapMask)
			SemiStaticCmds.BindTexture(SAMPLER_ENVMAPMASK, EnvMapMask, EnvMapMaskFrame);

		// s14 - $EnvMap
		// Always Dynamic

		//==========================================================================//
		// Setup Constant Registers
		//==========================================================================//

		// Always
		SemiStaticCmds.SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, BumpTransform);

		// Only with $NormalMap2
		// Stock does it always..
		if(bHasNormalMap2)
			SemiStaticCmds.SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_02, BumpTransform2);

		// c1 - Feature suggested by 'Dana Cief' and 'MrFunreal' independently.
		// Refract with Cubemaps is sometimes used on Weapon Scopes ( In L4D2 Workshop Addons for Example )
		// This simulates a 'Viewing through the Scope' Effect, however Cubemaps obviously Reflect the outside, not the Inside.
		// So what Modders do is invert the Normals of the Glass on the Scope, inverting the Cubemap to how it's supposed to be.
		// This exposes the Effect as a Parameter, obsoleting the crude Hack.
		float4 f4NormalDirFlip = GetBool(FlipNormal) ? -1.0f : 1.0f;
		SemiStaticCmds.SetPixelShaderConstant(REGISTER_FLOAT_001, f4NormalDirFlip);

		// c2
		// Stock-Consistency: GammaToLinear $RefractTint
		float4 f4RefractParams = 0.0f;
		f4RefractParams.rgb = GetFloat3(RefractTint);
		f4RefractParams.rgb = GammaToLinearTint(f4RefractParams.rgb);
		f4RefractParams.a = GetFloat(RefractAmount);
		SemiStaticCmds.SetPixelShaderConstant(REGISTER_FLOAT_002, f4RefractParams);

		if(bIsBlurred)
		{
			// Stock-Consistency: Magic Numbers for BlurFraction and HalfBlurFraction.
			float4 f4BlurParams = 0.0f;
			f4BlurParams.x = 1.0f / 512.0f;
			f4BlurParams.y = 0.5f * f4BlurParams.x;
			// .zw Free
			SemiStaticCmds.SetPixelShaderConstant(REGISTER_FLOAT_003, f4BlurParams);
		}

		if(GetBool(MagnifyEnable))
		{
			// Consistent with ASW
			float4 f4MagnificationParams = 0.0f;
			f4MagnificationParams.xy = GetFloat2(MagnifyCenter);
			f4MagnificationParams.z = GetFloat(MagnifyScale);

			// Avoid Zero Division..
			if(f4MagnificationParams.z != 0.0f)
				f4MagnificationParams.z = 1.0f / f4MagnificationParams.z; // Inverse Value

			// .zw Free
			SemiStaticCmds.SetPixelShaderConstant(REGISTER_FLOAT_004, f4MagnificationParams);
		}

		if(GetBool(LocalRefract))
		{
			float4 f4ParallaxParams = 0.0f;
			f4ParallaxParams.x = GetFloat(LocalRefractDepth);

			if(bHasBaseTexture)
			{
				// BUGBUG:
				// Aspect Ratio Code consistent with Alien Swarm Reactive Drop
				// However this is a dangerous Pointer in my Experience.
				// For some Reason the Game might try to randomly render after Textures have been unloaded
				// If no one complains this is fine.
				// This crashed with $EnvMap and $Detail on LUX, before I removed that Feature.
				// NOTE: The if(pTexture) here won't fix that crash because it'll be a dangling Pointer
				ITexture* pTexture = GetTexture(BaseTexture);
				if(pTexture)
					f4ParallaxParams.y = float(pTexture->GetActualHeight()) / float(pTexture->GetActualWidth());
			}
			else if(pShaderAPI)
			{				
				// This isn't considered on ASRD for some Reason..
				int nWidth, nHeight;
				pShaderAPI->GetStandardTextureDimensions(&nWidth, &nHeight, TEXTURE_FRAME_BUFFER_FULL_TEXTURE_0);
				f4ParallaxParams.y = float(nHeight) / float(nWidth);
			}

			// .zw Free
			SemiStaticCmds.SetPixelShaderConstant(REGISTER_FLOAT_005, f4ParallaxParams);
		}
		
		// c11
		// Always, for Radial Fog
		SemiStaticCmds.SetPixelShaderConstant_EyePos(LUX_PS_FLOAT_CAMERAPOSITION);

		// c12
		// Always Fog!
		SemiStaticCmds.SetPixelShaderConstant_FogParams(LUX_PS_FLOAT_FOGPARAMETERS);

		// c37, c38, c39, c40
		if (bHasEnvMap)
		{
			// $EnvMapTint, $EnvMapLightScale
			float4 f4EnvMapTint_LightScale;
			f4EnvMapTint_LightScale.xyz = GetFloat3(EnvMapTint);
			f4EnvMapTint_LightScale.w = 0.0f; // No LightScale.

			// Stock Consistency - Convert from Gamma to Linear
			f4EnvMapTint_LightScale.rgb = GammaToLinearTint(f4EnvMapTint_LightScale.rgb);
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_TINT, f4EnvMapTint_LightScale);

			// $EnvMapSaturation, $EnvMapContrast
			float4 f4EnvMapSaturation_Contrast;
			f4EnvMapSaturation_Contrast.rgb = GetFloat3(EnvMapSaturation); // Yes, this *is* a float3 Parameter.
			f4EnvMapSaturation_Contrast.w = GetFloat(EnvMapContrast);
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_FACTORS, f4EnvMapSaturation_Contrast);

			float4 f4EnvMapControls = 0.0f;
//			f4EnvMapControls.x = bBaseAlphaEnvMapMask;
//			f4EnvMapControls.y = bNormalMapAlphaEnvMapMask;
			f4EnvMapControls.z = GetBool(EnvMapMaskFlip);
			SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_CONTROLS, f4EnvMapControls);

			// $EnvMapFresnelMinMaxExp
			// Not on this Shader
			// SemiStaticCmds.SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_FRESNEL, f4EnvMapFresnelRanges);
		}

		// Set an Endpoint
		SemiStaticCmds.End();
	}

	//==========================================================================//
	// Entirely Dynamic Commands
	//==========================================================================//
	if(IsDynamicState())
	{
		// s14 - $EnvMap
		BindTexture(bHasEnvMap, SAMPLER_ENVMAPTEXTURE, EnvMap, EnvMapFrame);

		// Set this for $Alpha and $Alpha2 Support
		SetModulationConstant(false, false);

		// c19
		// Gamma Constants for EnvMaps
		if(bHasEnvMap)
			SetLuminanceGammaConstant(LUX_PS_FLOAT_LUMINANCE_GAMMA);

		// Prepare boolean array, yes we need to use BOOL
		BOOL BBools[REGISTER_BOOL_MAX] = { false };

		// b13, b14, b15
		BBools[LUX_PS_BOOL_HEIGHTFOG] = WriteWaterFogToDestAlpha(pContextData->m_bIsFullyOpaque);
		BBools[LUX_PS_BOOL_RADIALFOG] = HasRadialFog();
		BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = WriteDepthToDestAlpha(pContextData->m_bIsFullyOpaque);

		// Always set Boolean registers
		pShaderAPI->SetBooleanPixelShaderConstant(REGISTER_BOOL_START, BBools, REGISTER_BOOL_MAX);

		//==================================================================================================
		// Set Dynamic Shaders
		//==================================================================================================
		DECLARE_DYNAMIC_VERTEX_SHADER(lux_refract_vs30);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, bIsModel && HasSkinning());
		SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSION, bIsModel && HasVertexCompression());
		SET_DYNAMIC_VERTEX_SHADER(lux_refract_vs30);

		DECLARE_DYNAMIC_PIXEL_SHADER(lux_refract_ps30);
		SET_DYNAMIC_PIXEL_SHADER(lux_refract_ps30);

//		pShaderAPI->ExecuteCommandBuffer(StaticCmds.Base());
		pShaderAPI->ExecuteCommandBuffer(SemiStaticCmds.Base());
	}

	// We are not done here!
	// The Command Buffer blocks us from using ConVars.
	// Let's overwrite the Constants now. This is fine since we'd only ever use any of these for debugging.
	//==========================================================================//
	// ConVars
	//==========================================================================//
	if(IsDynamicState())
	{
#ifdef DEBUG_FULLBRIGHT2 
		if (mat_fullbright.GetInt() == 2 && !HasFlag(MATERIAL_VAR_NO_DEBUG_OVERRIDE))
			BindTexture(SAMPLER_BASETEXTURE, TEXTURE_GREY);
#endif

#ifdef LUX_DEBUGCONVARS
		if (bHasNormalMap && lux_disablefast_normalmap.GetBool())
		{
			BindTexture(SHADER_SAMPLER1, TEXTURE_NORMALMAP_FLAT);
			BindTexture(SHADER_SAMPLER2, TEXTURE_NORMALMAP_FLAT);
		}

		if (bHasEnvMap && lux_disablefast_envmap.GetBool())
		{
			float4 f4EnvMapTint_LightScale;
			f4EnvMapTint_LightScale = 0.0f;
			pShaderAPI->SetPixelShaderConstant(LUX_PS_FLOAT_ENVMAP_TINT, f4EnvMapTint_LightScale);
		}
	}
#endif

	Draw();
}
END_SHADER