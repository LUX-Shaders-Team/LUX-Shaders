//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	22.05.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//	Purpose of this File :	'Infected' Shader for Models. Recreation of the L4D2 Shader
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../../cpp_lux_shared.h"

// Includes for Shaderfiles...
#include "lux_infected_vs30.inc"
#include "lux_infected_ps30.inc"

// Register Map for this Shader
#include "../../lux_infected_registermap.h"

// Used for random number generation on parameter init.
int RandomInteger(int min, int max)
{
	return min + rand() % (max - min + 1);
}

// No publicly available SDK_ Versions to replace.

#ifdef REPLACE_INFECTED
DEFINE_FALLBACK_SHADER(Infected, LUX_Infected)
#endif

//==========================================================================//
// CommandBuffer Setup
//==========================================================================//
class InfectedContext : public LUXPerMaterialContextData
{
public:
	ShrinkableCommandBuilder_t<5000> m_StaticCmds;
	CommandBuilder_t<1000> m_SemiStaticCmds;

	// Snapshot / Dynamic State
	BlendType_t m_nBlendType = BT_NONE;
	bool m_bIsFullyOpaque = false;

	InfectedContext(CBaseShader* pShader)
		: m_SemiStaticCmds(pShader),
		m_StaticCmds(pShader)
	{
	}
};

//==========================================================================//
// Shader Start
//==========================================================================//
BEGIN_VS_SHADER(LUX_Infected, "A Recreation of Left 4 Dead 2's Infected Shader.")
	SHADER_INFO_GEOMETRY	("Models. ( Static Props, Dynamic Props )")
	SHADER_INFO_USAGE		("Use with existing Left 4 Dead 2 Models, or create new ones using publicly available guides.")
	SHADER_INFO_LIMITATIONS	("Due to missing Model Instancing, Materials with identical Names will share the same randomly selected Variation.")
	SHADER_INFO_PERFORMANCE ("Pretty cheap but *could* get expensive when many many pile models are used in the same Scene.")
	SHADER_INFO_FALLBACK	("A DXLevel below 90 will cause a Fallback to the Wireframe Shader.")
	SHADER_INFO_WEBLINKS(WEBLINK_VDC
		"Sources used to create this Shader:\n"
		"VDC Infected Shader Page - Documents the Randomisation Mechanic:\n"
		"https://developer.valvesoftware.com/wiki/Infected_(shader)\n"
		"Bronwen Grimes, Valve Presentation - Shines some light on Pixel Shader Mathematics:\n"
		"https://steamcdn-a.akamaihd.net/apps/valve/2010/GDC10_ShaderTechniquesL4D2.pdf\n"
		"Alex Vlachos, Valve Presentation - Includes Ellipsoid Culling Code:\n"
		"https://alex.vlachos.com/graphics/Vlachos-GDC10-Left4Dead2Wounds.pdf\n"
		"MrFunreal's Guide - The best Documentation for how Infected Shader Textures work:\n"
		"https://steamcommunity.com/sharedfiles/filedetails/?id=1567031703\n"
		"Zappy's Source Filmmaker Guide - Default Values for Shader Parameters\n"
		"https://steamcommunity.com/sharedfiles/filedetails/?id=2162570606")

	SHADER_INFO_D3D(LUX_SHADERINFO_SM30)

// ShiroDkxtro2: I added some Comments here with Quotations
// These are Helper Strings from the TLS Binary
// ( I don't really consider that RE you can probably get these from Tools or something )
BEGIN_SHADER_PARAMS
	Declare_DetailTextureParameters()
	Declare_NormalTextureParameters()
	Declare_PhongParameters()

	// Parameters unique to Infected Shader start here.

	// Doesn't actually do anything according to VDC
	// Not sure what it would do..
//	SHADER_PARAM(CheapDiffuse, SHADER_PARAM_TYPE_BOOL, "", "")
	
	// Only causes rendering Issues when used in Left 4 Dead 2.
	// Interesting: Zappy's Guide lists "ShadowBuild" having the Parameters, but DepthWrite *doesn't*
//	SHADER_PARAM(RTTShadowBuild, SHADER_PARAM_TYPE_BOOL, "", "")
	
	// Not needed if this is just for rtt shadow build
	// "Dummy param to make rtt shadow build code work"
//	SHADER_PARAM(Translucent_Material, SHADER_PARAM_TYPE_STRING, "", "")
	
	// We don't have controlable SSAO Effects on the SDK.
	// This is a SFM Parameter apparently
//	SHADER_PARAM(AmbientOcclusion, SHADER_PARAM_TYPE_BOOL, "", "")
	
	// Left 4 Dead 1 Infected Shader Support
//	SHADER_PARAM(DisableVariation, SHADER_PARAM_TYPE_BOOL, "", "Last-Stand Parameter that intends to recreate Left 4 Dead 1's Infected Shader System.")

	// Not supporting $DisableVariation at the Moment so we don't need these.
	// "Enable shiny blood: Derive specmask from areas that are more red than others. Only when variation is disabled."
	// "HACK! Derive specmask from areas that are more red than others. Exponent for that math."
//	SHADER_PARAM(ShinyBlood,		 SHADER_PARAM_TYPE_BOOL, "", "")
//	SHADER_PARAM(ShinyBloodExponent, SHADER_PARAM_TYPE_FLOAT, "17", "")

	// This is probably supposed to force Static Props to have StaticPropLighting instead of bumped Lighting 
	// Could support Model Lightmapping with this..
	SHADER_PARAM(StaticProp,		SHADER_PARAM_TYPE_BOOL, "", "Disables HW Skinning, nothing else, at the Moment")

	// The following Parameters are for debugging Purposes
	// For the Wound System we don't have.
	// However, I will allow hooking these by someone else and support Ellipsoid Culling.
	SHADER_PARAM(DebugEllipsoids,	SHADER_PARAM_TYPE_INTEGER, "", "Enables Debug Viewing for Ellipsoid Culling. ( Disables culling and tints 'culled areas' ).\n1 = Show Factor.\n2 = Show Vertex Culling Factor.\n")
	SHADER_PARAM(EllipsoidCenter,	SHADER_PARAM_TYPE_VEC3, "", "Center of the Ellipsoid ( Ellipsoid Origin ).")
	SHADER_PARAM(EllipsoidUp,		SHADER_PARAM_TYPE_VEC3, "", "Up Vector for the Ellipsoid. Should be ")
	SHADER_PARAM(EllipsoidLookAt,	SHADER_PARAM_TYPE_VEC3, "", "Position that the Ellipsoid ends on ( Apoapsis of the Equator )")
	SHADER_PARAM(EllipsoidScale,	SHADER_PARAM_TYPE_VEC3, "", "3D Scale of the Ellipsoid. ( Determines Size )")
	SHADER_PARAM(EllipsoidCenter2,	SHADER_PARAM_TYPE_VEC3, "", "See first Ellipsoid Parameters.")
	SHADER_PARAM(EllipsoidUp2,		SHADER_PARAM_TYPE_VEC3, "", "See first Ellipsoid Parameters.")
	SHADER_PARAM(EllipsoidLookAt2,	SHADER_PARAM_TYPE_VEC3, "", "See first Ellipsoid Parameters.")
	SHADER_PARAM(EllipsoidScale2,	SHADER_PARAM_TYPE_VEC3, "", "See first Ellipsoid Parameters.")

	// Not sure what CullType is supposed to be?
	// Stop first Ellipsoid from culling? Coloration? Some other Vertex Factors?
//	SHADER_PARAM(Ellipsoid2CullType, SHADER_PARAM_TYPE_INTEGER, "", "")
	
	// WoundCutOutTexture is applied via Ellipsoid Axis
	// Bias 
	SHADER_PARAM(Wounded, SHADER_PARAM_TYPE_BOOL, "", "Enables Ellipsoid Wound Culling.")
	SHADER_PARAM(WoundCutOutTexture, SHADER_PARAM_TYPE_TEXTURE, "", "[Greyscale] Factor for Wound Culling. Bright Values will be culled.\n$CutOutTextureBias can be used to influence when the culling takes Place.")
	SHADER_PARAM(CutOutTextureBias, SHADER_PARAM_TYPE_FLOAT, "", "Leveling Operation, determines when to cull Pixels from $WoundCutOutTexture")
	SHADER_PARAM(CutOutDecalFalloff, SHADER_PARAM_TYPE_FLOAT, "", "Unknown Purpose. Does Nothing.")
	SHADER_PARAM(CutOutDecalMappingScale, SHADER_PARAM_TYPE_FLOAT, "", "Scale for the CutOutTexture.")

	// 'New' LUX Parameters to be used with the Culling System:
	SHADER_PARAM(WoundComboTexture,	SHADER_PARAM_TYPE_BOOL, "", "Set to 1 when $WoundCutOutTexture contains Bullet and Slash Wound Types.\nThis allows $EllipsoidWoundType and Ellipsoid2WoundType to be used to determine which Type of Wound to apply.")
	SHADER_PARAM(EllipsoidWoundType, SHADER_PARAM_TYPE_BOOL, "", "0 = Bullet Wound, 1 = Slash Wound.")
	SHADER_PARAM(Ellipsoid2WoundType, SHADER_PARAM_TYPE_BOOL, "", "0 = Bullet Wound, 1 = Slash Wound.")

	SHADER_PARAM(GradientTexture, SHADER_PARAM_TYPE_TEXTURE, "", "Two-Dimensional Color Palette for Skin and Cloth Color.\nUpper half contains Skin Color, lower half contains Cloth Color.\nThis Texture cannot be higher than 16 Pixels.")

	// Blood
	SHADER_PARAM(BloodColor,	 SHADER_PARAM_TYPE_COLOR, "", "Color of the Blood. Please, have some self-restraint; Do not use white Blood.")
	SHADER_PARAM(BloodSpecBoost, SHADER_PARAM_TYPE_FLOAT, "", "$PhongBoost for Blood.")
	SHADER_PARAM(BloodMaskRange, SHADER_PARAM_TYPE_VEC2, "", "Min and Max for Blood Smoothstep.")

	// "Controls whether the Material should appear burning. It needs to be in a different VMT with a '_burning' suffix."
	// Not for LUX
	SHADER_PARAM(Burning, SHADER_PARAM_TYPE_BOOL, "", "Enables Burning Texture.")
	SHADER_PARAM(BurnDetailTexture, SHADER_PARAM_TYPE_TEXTURE, "", "[RGB] Textur to apply over the Model when burning. Works like DetailBlendMode 5 on other Shaders.")
	SHADER_PARAM(BurnStrength, SHADER_PARAM_TYPE_FLOAT, "", "How much Burn to apply. Lower Values mean less Burning.")

	// LUX Specific:
	// I don't know if this is animated in Left 4 Dead 2,
	// I'm making it possible just in case.
	SHADER_PARAM(BurnDetailTextureFrame, SHADER_PARAM_TYPE_INTEGER, "", "Frame Number for $BurnDetailTexture.")

	// Eye Factors
	SHADER_PARAM(EyeGlow,					SHADER_PARAM_TYPE_BOOL, "", "Causes lower half of the Red Channel in the $BaseTexture to be used for Retroreflectivity.")
	SHADER_PARAM(EyeGlowColor,				SHADER_PARAM_TYPE_COLOR, "", "$EyeGlow Tint.")
	SHADER_PARAM(EyeGlowFlashlightBoost,	SHADER_PARAM_TYPE_FLOAT, "", "Amplifies $EyeGlow for projected Textures. ( Flashlight is a projected Texture )")

	// Phong Exponents
	SHADER_PARAM(DefaultPhongExponent,	SHADER_PARAM_TYPE_FLOAT, "", "$PhongExponent for unmasked Parts.")
	SHADER_PARAM(SkinPhongExponent,		SHADER_PARAM_TYPE_FLOAT, "", "$PhongExponent for Skin. ( Blue Channel )")
	SHADER_PARAM(BloodPhongExponent,	SHADER_PARAM_TYPE_FLOAT, "", "$PhongExponent for Blood. ( Green Channel )")
	SHADER_PARAM(DetailPhongExponent,	SHADER_PARAM_TYPE_FLOAT, "", "$PhongExponent for $Detail.")

	// Debug Parameters
	SHADER_PARAM(SheetIndex,		SHADER_PARAM_TYPE_INTEGER, "",	"Debugging Parameter - Forces a specific Index for the $BaseTexture.")
	SHADER_PARAM(ColorTintGradient, SHADER_PARAM_TYPE_INTEGER, "",	"Debugging Parameter - Forces a specific horizontal Gradient-Slice for Color ( lower Gradient Texture ) to be used.")
	SHADER_PARAM(SkinTintGradient,	SHADER_PARAM_TYPE_INTEGER, "",	"Debugging Parameter - Forces a specific horizontal Gradient-Slice for Skin ( upper Gradient Texture ) to be used.")

	SHADER_PARAM(Random1, SHADER_PARAM_TYPE_FLOAT, "", "(INTERNAL PARAMETER) Used to store current Randomisation")
	SHADER_PARAM(Random2, SHADER_PARAM_TYPE_FLOAT, "", "(INTERNAL PARAMETER) Used to store current Randomisation")
	SHADER_PARAM(Random3, SHADER_PARAM_TYPE_FLOAT, "", "(INTERNAL PARAMETER) Used to store current Randomisation")
	SHADER_PARAM(Random4, SHADER_PARAM_TYPE_FLOAT, "", "(INTERNAL PARAMETER) Used to store current Randomisation")

	SHADER_PARAM(UsesBumpMap, SHADER_PARAM_TYPE_BOOL, "", "(INTERNAL PARAMETER) keeps track of whether or not $BumpMap is VMT Defined or not.")
END_SHADER_PARAMS

void InfectedShaderFlags()
{
	// Always needed for Lighting
	SetFlag2(MATERIAL_VAR2_LIGHTING_VERTEX_LIT);


	if (!GetBool(StaticProp))
	{
		// Skinning ( Animation Support )
		SetFlag2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);
	}

	// Ambient Cubes
	SetFlag2(MATERIAL_VAR2_NEEDS_BAKED_LIGHTING_SNAPSHOTS);

	// This Shader uses Per-Pixel Lighting
	SetFlag2(MATERIAL_VAR2_DIFFUSE_BUMPMAPPED_MODEL);

	// Always need Tangents for Per-Pixel Lighting
	SetFlag2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);
}

SHADER_INIT_PARAMS()
{
	InfectedShaderFlags();

	// Always set to something.
//	if (!GetBool(StaticProp))
	{
		if (!IsDefined(BumpMap))
			SetString(BumpMap, "...");
		else
			SetBool(UsesBumpMap, true);	
	}

	// Detail Texture Defaults
	DefaultFloat(DetailBlendFactor, 1.0f);	// Default Value is supposed to be 1.0f
	DefaultFloat(DetailScale, 4.0f);		// Default Value is supposed to be 4.0f

	// PhongFresnelRanges need this or Fresnel will be 0.0f
	DefaultFloat3(PhongFresnelRanges, 0.0f, 0.5f, 1.0f);

	// ShiroDkxtro2 Instruction Reduction :
	// On the Shader we'd do < $PhongExponentTexture.x * 149 + 1 >
	// On SDK2013MP this would be < $PhongExponentTexture.x * $PhongExponentFactor + 1 >
	// But we will do instead < $PhongExponentTexture.x * $PhongExponentFactor + $PhongExponent >
	// Without $PhongExponentTexture, this will just end up < $PhongExponent > on the Shader
	//
	// Stock Consistency : Override to $PhongExponent when its anything other than 0
	//
	// We use DefaultFloat's because maybe someone wants to do some really whacky stuff by combining Parameters 
	if (IsDefined(PhongExponent) && GetFloat(PhongExponent) > 0.0f)
	{
		DefaultFloat(PhongExponentFactor, 0.0f);
	}
	else if (IsDefined(PhongExponentFactor) && GetFloat(PhongExponentFactor) > 0.0f)
	{
		DefaultFloat(PhongExponent, 1.0f);
	}
	else
	{
		// Default Value is supposed to be ... Well in SDK2013mp its 0...
		// It replaces the *149 of the calculation so that is what its default value SHOULD be
		DefaultFloat(PhongExponentFactor, 149.0f);

		// Default Value is supposed to be 5.0f
		DefaultFloat(PhongExponent, 5.0f);
	}

	// Randomisation
	// Gradient Textures are at max 256x16
	// To avoid Lookups falling inbetween two Colors, the V Coordinate gets a 1/32f Offset
	static const float FixupOffset = 1.0f / 32.0f;
	static const float Offset = 1.0f / 16.0f;
	bool bForceVariation = lux_infected_forcerandomisation.GetBool();
	
	// Cloth and Skin Randomisation
	if (!IsDefined(SheetIndex) || bForceVariation)
	{
		// One of the four Corners
		SetFloat(Random3, 0.5f * (float)RandomInteger(0, 1));
		SetFloat(Random4, 0.5f * (float)RandomInteger(0, 1));
	}
	else
	{
		// Not so Random offsets
		// "Pick a specific corner of the basetexture to be used, as opposed to randomly picking one.
		// Those corners would be the four variants in the Red and Green channels." - VDC
		// Since this is an int, we have to remap it..
		// However it doesn't appear to be documented, which Index picks what Corner
		// So I'm guessing here, this likely looks incorrect.
		float2 Corner = 0.0f;
		switch (GetInt(SheetIndex))
		{
			// 0, 0
			// x, 0
			case 0:
				Corner = float2(0.0f, 0.0f);
				break;

			// 0, 0
			// 0, x
			case 1:
				Corner = float2(1.0f, 0.0f);
				break;

			// x, 0
			// 0, 0
			case 2:
				Corner = float2(0.0f, 1.0f);
				break;

			// 0, x
			// 0, 0
			case 3:
				Corner = float2(1.0f, 1.0f);
				break;

			// Same as case 0
			default:
				Corner = float2(0.0f, 0.0f);
				break;

		}

		SetFloat(Random3, 0.5f * Corner.x);
		SetFloat(Random4, 0.5f * Corner.y);
	}

	// Cloth Index
	if (!IsDefined(ColorTintGradient) || bForceVariation)
	{
		SetFloat(Random1, FixupOffset + (float)RandomInteger(0, 7) * Offset + 0.5f);
	}
	else
	{
		// Not so Random offsets
		float Index = (float)GetInt(ColorTintGradient);
		SetFloat(Random1, FixupOffset + Index * Offset + 0.5f);
	}

	// Skin Index
	if (!IsDefined(SkinTintGradient) || bForceVariation)
	{
		SetFloat(Random2, FixupOffset + (float)RandomInteger(0, 7) * Offset);
	}
	else
	{
		// Not so Random offsets
		float Index = (float)GetInt(SkinTintGradient);
		SetFloat(Random2, FixupOffset + Index * Offset);	// Skin Index
	}

	// Same Default Values as L4D2 below. Thank you Zappy for listing these in your Source Filmmaker Guide!
	DefaultFloat3(BloodColor, 1.0f, 0.0f, 0.0f);
	DefaultFloat2(BloodMaskRange, 0.0f, 1.0f);
	DefaultFloat(DefaultPhongExponent, 5.0f);
	DefaultFloat(SkinPhongExponent, 5.0f);
	DefaultFloat(DetailPhongExponent, 5.0f);
	DefaultFloat(BloodPhongExponent, 5.0f);
	DefaultFloat(BloodSpecBoost, 1.0f);
	DefaultFloat(EyeGlowFlashlightBoost, 100.0f);
	DefaultFloat(CutOutDecalMappingScale, 1.0f);
	DefaultFloat(PhongBoost, 1.0f);

//	DefaultFloat(ShinyBloodExponent, 17.0f);
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
	LoadTexture(BaseTexture);

	// Both, for the Frame Parameter
	LoadTexture(BumpMap);
	LoadTexture(NormalTexture);

	LoadTexture(GradientTexture);
	LoadTexture(Detail);

	// ShiroDkxtro2: I couldn't find a VMT with a WoundCutOutTexture defined
	// My Assumption is that in L4D2 it set's in Code.
	// I did find the Texture shown in the GDC Talk.
	// I'm setting the Default here to that so the Wound Culling System can be used when mounting L4D2 Assets.
	// NOTE: To *actually* use this Texture, $Wounded has to be set.
	if (!IsDefined(WoundCutOutTexture))
	{
		SetString(WoundCutOutTexture, "models/infected/common/body_wounds/wound_combined_cull_texture");

		// It doesn't appear this Texture is flagged with ClampS and ClampT
		// I'm adding it manually.
		LoadTexture(WoundCutOutTexture, (TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT)); 
	}
	else
		LoadTexture(WoundCutOutTexture, (TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT));

	LoadTexture(BurnDetailTexture);
}

// Virtual Void Override for Context Data
InfectedContext* CreateMaterialContextData() override
{
	return new InfectedContext(this);
}

SHADER_DRAW
{
	// Get Context Data. BaseShader handles creation for us, using the CreateMaterialContextData() virtual
	auto* pContextData = GetMaterialContextData<InfectedContext>(pContextDataPtr);
//	auto& StaticCmds = pContextData->m_StaticCmds;
	auto& SemiStaticCmds = pContextData->m_SemiStaticCmds;

	// Terrific
	bool bProjectedTexture = HasFlashlight();

	bool bHasBaseTexture = IsTextureLoaded(BaseTexture);
	bool bHasNormalTexture = GetBool(UsesBumpMap) && IsTextureLoaded(BumpMap);
	bool bWounded = GetBool(Wounded);
	bool bWoundCutOutTexture = bWounded && IsTextureLoaded(WoundCutOutTexture);
	bool bGradientTexture = IsTextureLoaded(GradientTexture);
	bool bDetailTexture = IsTextureLoaded(Detail);

	bool bBurning = GetBool(Burning);
	bool bBurnTexture = bBurning && IsTextureLoaded(BurnDetailTexture);

	bool bHalfLambert = HasFlag(MATERIAL_VAR_HALFLAMBERT);
	bool bPhong = GetBool(Phong);

	// Hammer probably wants this for Texture Shaded Polygons
	bool bVertexColors = HasFlag(MATERIAL_VAR_VERTEXCOLOR);

	//==========================================================================//
	// Pre-Snapshot Context Data Variables
	//==========================================================================//
	if (IsSnapshottingCommands())
	{
		/*
		pContextData->m_nBlendType = ComputeBlendType(BaseTexture, true, Detail, GetInt(DetailBlendMode));
		pContextData->m_bIsFullyOpaque = IsFullyOpaque(pContextData->m_nBlendType);
		*/

		// Can't do transparency through alternative means.
		if (bProjectedTexture)
			pContextData->m_nBlendType = BT_ADD;
		else
			pContextData->m_nBlendType = BT_NONE;

		pContextData->m_bIsFullyOpaque = true;
	}

	//==========================================================================//
	// Static Snapshot of Shader Setup
	//==========================================================================//
	if (IsSnapshotting())
	{
		//==========================================================================//
		// General Rendering Setup Shenanigans
		//==========================================================================//

		// This handles : $IgnoreZ, $Decal, $Nocull, $Znearer, $Wireframe, $AllowAlphaToCoverage
		SetInitialShadowState();

		// Everything Transparency is packed into this Function
		EnableTransparency(pContextData->m_nBlendType);

		// We always need this
		pShaderShadow->EnableAlphaWrites(pContextData->m_bIsFullyOpaque);

		// Weird name, what it actually means : We output linear values
		pShaderShadow->EnableSRGBWrite(true);

		//==========================================================================//
		// Vertex Shader - Vertex Format
		//==========================================================================//
		
		// We always want the Normal, even when we don't use it.
		// Otherwise we might get some Issues with too-thin Vertex Formats
		unsigned int nFlags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;

		// Not with Phong and Normal?
		// This seems a bit too generic
		if (bVertexColors)
			nFlags |= VERTEX_COLOR;

		// Always just one..
		int nTexCoords = 1;

		// Morphing isn't enabled, so we can set this to 1.
		/*
		if (HasFlag(MATERIAL_VAR_DECAL) && bFastVertexTextures)
			nTexCoords = 3;
		*/
		
		// No Second TexCoord, but I keep this for Consistency Reasons.
		// Wish we had a second UV though.. Would blow the Doors wide open for new Features.
		int nTexCoordDim[3] = { 2, 0, 3 };

		// On Stock Shaders vUserData is defined on the TANGENT Stream.
		// After some testing on an unrelated Shader,
		// it appears that 4 is the Size of the TANGENT Stream
		// What we get is TangentS + Binormal Sign, in an uncompressed Format.
		// 
		// *However*, usually Tangents come from the NORMAL Stream, in a compressed Format.
		// In Dynamic State we determine which one to actually use using the 'vertexCompression' Variable.
		// When a VMT sets $SoftwareSkin, it forces uncompressed Data.
		// FaceFlexes force the uncompressed Data..
		// So unfortunately, we can't add $SoftwareSkin as a Parameter to check whether we even need UserDataSize of 4
		// Some Geometry ran on this Shader might also just never set that Parameter.
		// We pretty much have to force a UserDataSize of 4 here. Hopefully this gets optimised somewhere in the Engine.
		int nUserDataSize = (bProjectedTexture || bHasNormalTexture || bPhong) ? 4 : 0;

		pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, nTexCoordDim, nUserDataSize);

		//==========================================================================//
		// Sampler Setup
		//==========================================================================//

		// s0 - $BaseTexture
		// We always have a BaseTexture, it contains masking Information
		// So don't make it sRGB!
		EnableSampler(SHADER_SAMPLER0, false);

		// s1 - $BumpMap
		// Never sRGB
		EnableSampler(SHADER_SAMPLER1, false);

		// s2 - $WoundCutOutTexture
		// Never sRGB ( These Textures are greyscale )
		EnableSampler(bWoundCutOutTexture, SHADER_SAMPLER2, false);

		// s3 - $GradientTexture
		// Should(?) be sRGB
		EnableSampler(bGradientTexture, SHADER_SAMPLER3, true);

		// s4 - $Detail
		// Not sRGB, we treat this as BlendMode 0
		EnableSampler(bDetailTexture, SHADER_SAMPLER4, false);

		// s5 - BurnDetailTexture
		// Assuming BlendMode 6 Application so make it sRGBRead
		EnableSampler(bBurnTexture, SHADER_SAMPLER5, true);

		// s13, s14, s15 - Projected Texture Samplers
		// Handles Flashlight Samplers and Fog State
		SetupFlashlightSamplers();

		//==========================================================================//
		// Set Static Shaders
		//==========================================================================//

		DECLARE_STATIC_VERTEX_SHADER(lux_infected_vs30);
		SET_STATIC_VERTEX_SHADER_COMBO(PROJTEX, bProjectedTexture);
		SET_STATIC_VERTEX_SHADER_COMBO(DETAILTEXTURE_UV, bDetailTexture);
		SET_STATIC_VERTEX_SHADER_COMBO(VERTEXCOLORS, bVertexColors);
		SET_STATIC_VERTEX_SHADER(lux_infected_vs30);

		DECLARE_STATIC_PIXEL_SHADER(lux_infected_ps30);
		SET_STATIC_PIXEL_SHADER_COMBO(PROJTEX, bProjectedTexture);
		SET_STATIC_PIXEL_SHADER_COMBO(BURNING, !bProjectedTexture && bBurnTexture);
		SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, bDetailTexture);
		SET_STATIC_PIXEL_SHADER_COMBO(EYEGLOW, GetBool(EyeGlow));
		SET_STATIC_PIXEL_SHADER_COMBO(VERTEXCOLORS, bVertexColors);
		SET_STATIC_PIXEL_SHADER_COMBO(ELLIPSOIDDEBUG, GetBool(DebugEllipsoids));
		SET_STATIC_PIXEL_SHADER(lux_infected_ps30);
	}

	//==========================================================================//
	// Post-Snapshot Context Data Static Commands
	//==========================================================================//
	if (IsSnapshottingCommands())
	{
		// Set the Buffer back to its original ( Empty ) State
//		StaticCmds.Reset();

		// Instruct the Buffer to set an End Point
//		StaticCmds.End();

		// Set the Buffer back to its original ( Empty ) State
		SemiStaticCmds.Reset(this);

		// Instruct the Buffer to set an End Point
		SemiStaticCmds.End();
	}

	//==========================================================================//
	// Pre-Dynamic Context Data Semi-Static Commands
	//==========================================================================//
	if (MaterialVarsChanged())
	{
		// Set the Buffer back to its original ( Empty ) State
		SemiStaticCmds.Reset(this);

		//==========================================================================//
		// Bind StandardTextures
		//==========================================================================//

		// s1 - $BumpMap
		if (!bHasNormalTexture)
			SemiStaticCmds.BindTexture(SHADER_SAMPLER1, TEXTURE_NORMALMAP_FLAT);

		if (GetInt(DebugEllipsoids) == 2)
			SemiStaticCmds.BindTexture(SHADER_SAMPLER2, TEXTURE_BLACK);

		//==========================================================================//
		// Bind Textures
		//==========================================================================//

		// s0 - $BaseTexture
		if (bHasBaseTexture)
			SemiStaticCmds.BindTexture(SHADER_SAMPLER0, BaseTexture, Frame);

		// s1 - $BumpMap
		if (bHasNormalTexture)
			SemiStaticCmds.BindTexture(SHADER_SAMPLER1, BumpMap, BumpFrame);

		// s2 - $WoundCutOutTexture
		if (bWoundCutOutTexture && GetInt(DebugEllipsoids) != 2)
			SemiStaticCmds.BindTexture(SHADER_SAMPLER2, WoundCutOutTexture, -1);

		// s3 - $GradientTexture
		if (bGradientTexture)
			SemiStaticCmds.BindTexture(SHADER_SAMPLER3, GradientTexture, -1);

		// s4 - $Detail
		if (bDetailTexture)
			SemiStaticCmds.BindTexture(SHADER_SAMPLER4, Detail, DetailFrame);

		// s5 - $BurnDetailTexture
		if (bBurnTexture)
			SemiStaticCmds.BindTexture(SHADER_SAMPLER5, BurnDetailTexture, BurnDetailTextureFrame); // No Frame Parameter? That's odd

		//==========================================================================//
		// Setup Constant Registers
		//==========================================================================//

		// c0
		float4 cRandomisation;
		cRandomisation.x = GetFloat(Random1);
		cRandomisation.y = GetFloat(Random2);
		cRandomisation.z = GetFloat(Random3);
		cRandomisation.w = GetFloat(Random4);
		SemiStaticCmds.SetPixelShaderConstant(INFECTED_RANDOMISATION, cRandomisation);

		// c1 - $BloodColor
		float4 cBloodControls1;
		cBloodControls1.rgb = GetFloat3(BloodColor);

		// Eyeballed to L4D2.
		// Without this, Blood is too saturated
		cBloodControls1.rgb = GammaToLinearTint(cBloodControls1.rgb);
		cBloodControls1.w = GetFloat(BloodPhongExponent);
		SemiStaticCmds.SetPixelShaderConstant(INFECTED_BLOODCONTROLS1, cBloodControls1);

		// c2 - $BloodMaskRange, $BloodPhongExponent, $BloodSpecBoost
		float4 cBloodControls2;
		cBloodControls2.xy = GetFloat2(BloodMaskRange);
		cBloodControls2.z = GetFloat(BloodSpecBoost);
		cBloodControls2.w = GetFloat(SkinPhongExponent);
		SemiStaticCmds.SetPixelShaderConstant(INFECTED_BLOODCONTROLS2, cBloodControls2);

		// c3 - $PhongTint, $PhongBoost
		float4 cPhongControls1;
		cPhongControls1.xyz = GetFloat3(PhongTint);
		cPhongControls1.w = GetFloat(PhongBoost);
		SemiStaticCmds.SetPixelShaderConstant(INFECTED_PHONGCONTROLS1, cPhongControls1);

		// c4 - $PhongFresnelRanges, $DefaultPhongExponent
		float4 cPhongControls2;
		cPhongControls2.xyz = GetFloat3(PhongFresnelRanges);
		cPhongControls2.x = (cPhongControls2.y - cPhongControls2.x) * 2;
		cPhongControls2.z = (cPhongControls2.z - cPhongControls2.y) * 2;
		cPhongControls2.w = GetFloat(DefaultPhongExponent);
		SemiStaticCmds.SetPixelShaderConstant(INFECTED_PHONGCONTROLS2, cPhongControls2);

		// c5 - $EyeGlowColor, $EyeGlowFlashlightBoost
		float4 cEyeGlowControls;
		cEyeGlowControls.rgb = GetFloat3(EyeGlowColor) * (bProjectedTexture ? GetFloat(EyeGlowFlashlightBoost) : 1.0f);
		cEyeGlowControls.w = GetFloat(DetailPhongExponent);
		SemiStaticCmds.SetPixelShaderConstant(INFECTED_EYEGLOWCONTROLS, cEyeGlowControls);

		// c6 - $DetailTint, $DetailBlendFactor
		float4 cDetailControls;
		cDetailControls.xyz = GetFloat3(DetailTint);
		cDetailControls.w = GetFloat(DetailBlendFactor);
		SemiStaticCmds.SetPixelShaderConstant(INFECTED_DETAILCONTROLS, cDetailControls);

		// c7 - c22+ Handled in Dynamic State

		// Texture Coordinates
		SemiStaticCmds.SetVertexShaderTextureTransform(LUX_VS_TEXTURETRANSFORM_01, BaseTextureTransform);

		if (bDetailTexture)
		{
			if(HasTransform(true, DetailTextureTransform))
				SemiStaticCmds.SetVertexShaderTextureScaledTransform(LUX_VS_TEXTURETRANSFORM_02, DetailTextureTransform, DetailScale);
			else
				SemiStaticCmds.SetVertexShaderTextureScaledTransform(LUX_VS_TEXTURETRANSFORM_02, BaseTextureTransform, DetailScale);
		}

		// Vertex Shader Registers
		// First Ellipsoid
		Vector v1_Center;
		params[EllipsoidCenter]->GetVecValue(v1_Center.Base(), 3);

		Vector v1_Target;
		params[EllipsoidLookAt]->GetVecValue(v1_Target.Base(), 3);

		Vector v1_Scale;
		params[EllipsoidScale]->GetVecValue(v1_Scale.Base(), 3);

		Vector v1_Up;
		params[EllipsoidUp]->GetVecValue(v1_Up.Base(), 3);

		Vector v1_Forward;
		v1_Forward = v1_Target - v1_Center;
		v1_Forward.NormalizeInPlace();

		Vector v1_Right;
		CrossProduct(v1_Forward, v1_Up, v1_Right);
		v1_Right.NormalizeInPlace();
	
		// Right is the first Axis, 0
		// Up is the second Axis, 1
		// Forward is the third Axis, 2
		// NOTE: Vlachos GDC Talk says
		// "Ellipsoid basis is the orthonormal basis of the ellipsoid divided by the per-axis ellipsoid size"
		// So we have to *divide* here
		v1_Right	/= v1_Scale.x != 0.0000f ? v1_Scale.x : 1.0f;
		v1_Up		/= v1_Scale.y != 0.0000f ? v1_Scale.y : 1.0f;
		v1_Forward	/= v1_Scale.z != 0.0000f ? v1_Scale.z : 1.0f;

		// Package
		float4 f4Ellipsoid1_Center	= float4(v1_Center.x,	v1_Center.y,	v1_Center.z,	0.0f);
		float4 f4Ellipsoid1_Right	= float4(v1_Right.x,	v1_Right.y,		v1_Right.z,		0.0f);
		float4 f4Ellipsoid1_Up		= float4(v1_Up.x,		v1_Up.y,		v1_Up.z,		0.0f);
		float4 f4Ellipsoid1_Forward = float4(v1_Forward.x,	v1_Forward.y,	v1_Forward.z,	0.0f);

		// Send off
		SemiStaticCmds.SetVertexShaderConstant(LUX_VS_FLOAT_SET1_0, f4Ellipsoid1_Center, 1);
		SemiStaticCmds.SetVertexShaderConstant(LUX_VS_FLOAT_SET1_1, f4Ellipsoid1_Right, 1);
		SemiStaticCmds.SetVertexShaderConstant(LUX_VS_FLOAT_SET1_2, f4Ellipsoid1_Up, 1);
		SemiStaticCmds.SetVertexShaderConstant(LUX_VS_FLOAT_SET1_3, f4Ellipsoid1_Forward, 1);

		// Vertex Shader Registers
		// Second Ellipsoid ( Same Way as the first )
		Vector v2_Center;
		params[EllipsoidCenter2]->GetVecValue(v2_Center.Base(), 3);

		Vector v2_Target;
		params[EllipsoidLookAt2]->GetVecValue(v2_Target.Base(), 3);

		Vector v2_Scale;
		params[EllipsoidScale2]->GetVecValue(v2_Scale.Base(), 3);

		Vector v2_Up;
		params[EllipsoidUp2]->GetVecValue(v2_Up.Base(), 3);

		Vector v2_Forward;
		v2_Forward = v2_Target - v2_Center;
		v2_Forward.NormalizeInPlace();

		Vector v2_Right;
		CrossProduct(v2_Forward, v2_Up, v2_Right);
		v2_Right.NormalizeInPlace();

		// Treating this like ClipSpace
		// Right is the first Axis, 0
		// Up is the second Axis, 1
		// Forward is the third Axis, 2
		v2_Right	/= v2_Scale.x != 0.0000f ? v2_Scale.x : 1.0f;
		v2_Up		/= v2_Scale.y != 0.0000f ? v2_Scale.y : 1.0f;
		v2_Forward	/= v2_Scale.z != 0.0000f ? v2_Scale.z : 1.0f;

		// Package
		float4 f4Ellipsoid2_Center	= float4(v2_Center.x,	v2_Center.y,	v2_Center.z,	0.0f);
		float4 f4Ellipsoid2_Right	= float4(v2_Right.x,	v2_Right.y,		v2_Right.z,		0.0f);
		float4 f4Ellipsoid2_Up		= float4(v2_Up.x,		v2_Up.y,		v2_Up.z,		0.0f);
		float4 f4Ellipsoid2_Forward = float4(v2_Forward.x,	v2_Forward.y,	v2_Forward.z,	0.0f);

		// Send off
		SemiStaticCmds.SetVertexShaderConstant(LUX_VS_FLOAT_SET1_4, f4Ellipsoid2_Center, 1);
		SemiStaticCmds.SetVertexShaderConstant(LUX_VS_FLOAT_SET1_5, f4Ellipsoid2_Right, 1);
		SemiStaticCmds.SetVertexShaderConstant(LUX_VS_FLOAT_SET1_6, f4Ellipsoid2_Up, 1);
		SemiStaticCmds.SetVertexShaderConstant(LUX_VS_FLOAT_SET1_7, f4Ellipsoid2_Forward, 1);

		// Instruct the Buffer to set an End Point
		SemiStaticCmds.End();
	}

	//==========================================================================//
	// Entirely Dynamic Commands
	//==========================================================================//
	if (IsDynamicState())
	{
		//==========================================================================//
		// Bind Textures
		//==========================================================================//

		// Binds Textures and sends Flashlight Constants
		// Returns bFlashlightShadows
		bool bFlashlightShadows = SetupFlashlight();

		//==========================================================================//
		// Setup Constant Registers
		//==========================================================================//

		bool bComboTexture = GetBool(WoundComboTexture);

		// ScaleFactors for the WoundCutOutTexture
		float4 Reg7;
		Reg7.x = (bComboTexture ? 0.5f : 1.0f);	// Ellipsoid1 U
		Reg7.y = 1.0f;							// Ellipsoid1 V
		Reg7.z = (bComboTexture ? 0.5f : 1.0f);	// Ellipsoid2 U
		Reg7.w = 1.0f;							// Ellipsoid2 V
		pShaderAPI->SetPixelShaderConstant(INFECTED_CUTOUTCONTROLS1, Reg7);

		float4 Reg8;
		Reg8.x = bBurnTexture ? GetFloat(BurnStrength) : 0.0f; // Needs to be fully dynamic!
		Reg8.y = GetFloat(CutOutTextureBias);
		Reg8.z = (bComboTexture && GetBool(EllipsoidWoundType)) ? 0.5f : 0.0f;	// Applying 0.5f UV Offset when the Wound is a Slash
		Reg8.w = (bComboTexture && GetBool(Ellipsoid2WoundType)) ? 0.5f : 0.0f;	// Only works with Combo Textures!
		pShaderAPI->SetPixelShaderConstant(INFECTED_CUTOUTCONTROLS2, Reg8);

		// c9 is in SemiStaticCmds

		float4 Reg10;
		Reg10.x = GetFloat(CutOutDecalMappingScale);
		Reg10.y = GetFloat(CutOutDecalMappingScale);
		Reg10.z = 0.0f;
		Reg10.w = 0.0f;
		pShaderAPI->SetPixelShaderConstant(INFECTED_CUTOUTCONTROLS3, Reg10);

		// Need this for $Alpha/$Alpha2 and WaterFogFactorType
		SetModulationConstant(false, false);

		// c11 - Camera Position
		SetPixelShaderCameraPosition(LUX_PS_FLOAT_CAMERAPOSITION);

		// c12 - Fog Params
		pShaderAPI->SetPixelShaderFogParams(LUX_PS_FLOAT_FOGPARAMETERS);

		if (!bProjectedTexture)
		{
			pShaderAPI->SetPixelShaderStateAmbientLightCube(REGISTER_FLOAT_013);
			pShaderAPI->CommitPixelShaderLighting(REGISTER_FLOAT_020);
		}

		// Prepare boolean array, yes we need to use BOOL
		BOOL BBools[REGISTER_BOOL_MAX] = { false };

		// Bumped Lighting wants this
		BBools[LUX_PS_BOOL_HALFLAMBERT] = bHalfLambert;

		BBools[LUX_PS_BOOL_HEIGHTFOG] = WriteWaterFogToDestAlpha(pContextData->m_bIsFullyOpaque);
		BBools[LUX_PS_BOOL_RADIALFOG] = HasRadialFog();
		BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = WriteDepthToDestAlpha(pContextData->m_bIsFullyOpaque);

		// Always set Boolean registers
		pShaderAPI->SetBooleanPixelShaderConstant(REGISTER_BOOL_START, BBools, REGISTER_BOOL_MAX);

		//==================================================================================================
		// Set Dynamic Shaders
		//==================================================================================================

		// Determine how many Ellipsoids we want
		// 0 = none
		// 1 = Ellipsoid 1
		// 2 = Ellipsoid 2
		// 3 = Ellipsoid 1 and Ellipsoid 2
		int nEllipsoidCulling = 0;
		if (bWounded)
		{
			// Can't use float3 abs() Template because of gcc.
			// Thanks gcc, very helpful
			float3 Scale1 = GetFloat3(EllipsoidScale);
			Scale1.x = abs(Scale1.x);
			Scale1.y = abs(Scale1.y);
			Scale1.z = abs(Scale1.z);

			float3 Scale2 = GetFloat3(EllipsoidScale2);
			Scale2.x = abs(Scale2.x);
			Scale2.y = abs(Scale2.y);
			Scale2.z = abs(Scale2.z);

			// Not sure how this would be handled in L4D2
			// I'm assuming here that there is only an ellipsoid to cull from
			// *if* the Scale of the Ellipsoid is non-zero
			if ((Scale1.x + Scale1.y + Scale1.z) > 0.0f)
				nEllipsoidCulling += 1;

			if ((Scale2.x + Scale2.y + Scale2.z) > 0.0f)
				nEllipsoidCulling += 2;
		}

		DECLARE_DYNAMIC_VERTEX_SHADER(lux_infected_vs30);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, HasSkinning());
		SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSION, HasVertexCompression());
		SET_DYNAMIC_VERTEX_SHADER_COMBO(ELLIPSOIDCULLING, nEllipsoidCulling);
		SET_DYNAMIC_VERTEX_SHADER(lux_infected_vs30);

		// LightState is always fully Dynamic, and we always need it.
		LightState_t LightState;
		pShaderAPI->GetDX9LightState(&LightState);

		DECLARE_DYNAMIC_PIXEL_SHADER(lux_infected_ps30);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(PROJTEXSHADOWS, bProjectedTexture && bFlashlightShadows);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(ELLIPSOIDCULLING, nEllipsoidCulling);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(NUM_LIGHTS_COMBO, !bProjectedTexture ? LightState.m_nNumLights : 0);
		SET_DYNAMIC_PIXEL_SHADER(lux_infected_ps30);

//		pShaderAPI->ExecuteCommandBuffer(StaticCmds.Base());
		pShaderAPI->ExecuteCommandBuffer(SemiStaticCmds.Base());
	}

	Draw();
}
END_SHADER
