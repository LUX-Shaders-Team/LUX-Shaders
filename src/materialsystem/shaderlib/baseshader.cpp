//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "BaseShader.h"
#include "ShaderDLL.h"
#include "tier0/dbg.h"
#include "shaderdll_global.h"
#include "ishadersystem.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/itexture.h"
#include "materialsystem/ishaderapi.h"
#include "materialsystem/materialsystem_config.h"
#include "cshader.h"
#include "mathlib/vmatrix.h"
#include "tier1/strtools.h"
#include "convar.h"
#include "tier0/vprof.h"
#include "../stdshaders/cpp_convars.h"
#include "../stdshaders/lux_registermap_cpp.h"
#include "../stdshaders/lux_registermap_vs.h"

// NOTE: This must be the last include File in a .cpp File!
#include "tier0/memdbgon.h"

// Externs
bool g_bSupressShaderWarnings = false;
bool g_bHammerPlusPlus = false;
bool g_bWaterAlienSwarmFogFactor = false;

#ifdef DEBUG
bool g_bDebugSpew = false;
bool g_bBreakPointShadow = false;
bool g_bBreakPointDynamic = false;

CON_COMMAND_F(lux_debug_shadowstate, "Prints the Shadow State of the specified Material(s).\nArgument: MaterialName\n", FCVAR_CHEAT)
{
	if (args.ArgC() == 1)
	{
		Msg("%s\n", "Use the Name of the Material as an Argument to this Command.");
		return;
	}

	if(!lux_general_shaderstatewrappers.GetBool())
	{
		Msg("%s\n", "shaderstatewrappers must be enabled for this Command.");
		return;
	}

	// Connect MatSys if it hasn't been
	if ( !g_pMaterialSystem )
		g_pMaterialSystem = LoadInterface<IMaterialSystem>(MATERIALSYSTEM_DLL_NAME, MATERIAL_SYSTEM_INTERFACE_VERSION );

	// Reload the previous KeyWord to set $Debug_True to false an all of them
	if ( g_pMaterialSystem && s_ShaderSpew.GetCachedKeyword()[0] )
	{
		g_bBreakPointDynamic = false;
		g_bBreakPointShadow = false;
		g_bDebugSpew = false;
		g_pMaterialSystem->ReloadMaterials( s_ShaderSpew.GetCachedKeyword() );
	}

	// Tell s_ShaderSpew what to do.
	// We need a fake Shadow State, and only want to spew Static State
	s_ProxyShaderShadow.SetFakeShadowState(true);
	s_ShaderSpew.SetSpewBehaviour(true, false, false, true);
	s_ShaderSpew.SetCachedKeyword( args[1] );

	// This will set $Debug_True on all Materials we reload
	if( g_pMaterialSystem )
	{
		g_bDebugSpew = true;
		g_pMaterialSystem->ReloadMaterials( args[1] );
	}
}

CON_COMMAND_F(lux_debug_dynamicstate, "Prints the Shadow State / Dynamic State of the specified Material(s).\nArgument: MaterialName\n", FCVAR_CHEAT)
{
	if (args.ArgC() == 1)
	{
		Msg("%s\n", "Use the Name of the Material as an Argument to this Command.");
		return;
	}
	
	if(!lux_general_shaderstatewrappers.GetBool())
	{
		Msg("%s\n", "shaderstatewrappers must be enabled for this Command.");
		return;
	}

	// Connect MatSys if it hasn't been
	if ( !g_pMaterialSystem )
		g_pMaterialSystem = LoadInterface<IMaterialSystem>(MATERIALSYSTEM_DLL_NAME, MATERIAL_SYSTEM_INTERFACE_VERSION );

	// Reload the previous KeyWord to set $Debug_True to false an all of them
	if ( g_pMaterialSystem && s_ShaderSpew.GetCachedKeyword()[0] )
	{
		g_bBreakPointDynamic = false;
		g_bBreakPointShadow = false;
		g_bDebugSpew = false;
		g_pMaterialSystem->ReloadMaterials( s_ShaderSpew.GetCachedKeyword() );
	}

	// Tell s_ShaderSpew what to do.
	// We need a fake Shadow State, and want to spew Static and Dynamic State
	s_ProxyShaderShadow.SetFakeShadowState(true);
	s_ShaderSpew.SetSpewBehaviour(true, true, false, true);
	s_ShaderSpew.SetCachedKeyword( args[1] );

	// This will set $Debug_True on all Materials we reload
	if ( g_pMaterialSystem )
	{
		g_bDebugSpew = true;
		g_pMaterialSystem->ReloadMaterials(args[1]);
	}
}

CON_COMMAND_F(lux_debug_breakpoint, "Forces an Assert in the BaseShader of the specified Material(s)\nArgument1: MaterialName\nArgument2: Shadow/Dynamic", FCVAR_CHEAT)
{
	if (args.ArgC() == 1)
	{
		Msg("%s\n", "Use the Name of the Material as an Argument to this Command, and the State at which to break.");
		return;
	}

	if (args.ArgC() == 2)
	{
		Msg("%s\n", "Add the State at which to break.");
		return;
	}

	// Connect MatSys if it hasn't been
	if ( !g_pMaterialSystem )
		g_pMaterialSystem = LoadInterface<IMaterialSystem>(MATERIALSYSTEM_DLL_NAME, MATERIAL_SYSTEM_INTERFACE_VERSION );

	// Reload the previous KeyWord to set $Debug_True to false an all of them
	if ( g_pMaterialSystem && s_ShaderSpew.GetCachedKeyword()[0] )
	{
		g_bBreakPointDynamic = false;
		g_bBreakPointShadow = false;
		g_bDebugSpew = false;
		g_pMaterialSystem->ReloadMaterials( s_ShaderSpew.GetCachedKeyword() );
	}

	// Figure out which one we want
	if(!V_stricmp(args[2], "Shadow"))
		g_bBreakPointShadow = true;
	else if(!V_stricmp(args[2], "Dynamic"))
		g_bBreakPointDynamic = true;
	else
	{
		Msg("%s %s\n", "Invalid State: ", args[2]);
		return;
	}

	if(g_pMaterialSystem)
	{
		s_ShaderSpew.SetCachedKeyword( args[1] );
		g_pMaterialSystem->ReloadMaterials(args[1]);
	}
	else
	{
		g_bBreakPointShadow = false;
		g_bBreakPointDynamic = false;
	}
}
#endif

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CBaseShader::CBaseShader()
{
	GetShaderDLL()->InsertShader( this );
}

//-----------------------------------------------------------------------------
// Default Shader Parameters included with every Shader
//-----------------------------------------------------------------------------
// Update the BaseShader.h enum if you add/remove something from here.
// ShiroDkxtro2: All new Parameters are below $sRGBTint to avoid any potential Issues.
static ShaderParamInfo_t s_StandardParams[NUM_SHADER_MATERIAL_VARS] =
{
	// $Flags contain MaterialVarFlags_t, see imaterial.h for the enum
	// Used for IS_FLAG_SET
	{
		"$Flags",
		"(INTERNAL PARAMETER) Used for things like $BaseAlphaEnvMapMask, $HalfLambert. etc.",
		SHADER_PARAM_TYPE_INTEGER,
		"0",
		SHADER_PARAM_NOT_EDITABLE
	},

	// Used for IS_FLAG_DEFINED, the difference to IS_FLAG_SET is, whether these are actually present in the VMT File
	{
		"$Flags_Defined",
		"(INTERNAL PARAMETER) Stores whether a Flag is defined, used for IS_FLAG_DEFINED, which is different from IS_FLAG_SET.",
		SHADER_PARAM_TYPE_INTEGER,
		"0",
		SHADER_PARAM_NOT_EDITABLE
	},

	// Flags2 contain MaterialVarFlags2_t, see imaterial.h for the enum
	{
		"$Flags2",
		"(INTERNAL PARAMETER) Used by the Shader to indicate certain requirements, like Bumped Lightmaps or Tangent Data.",
		SHADER_PARAM_TYPE_INTEGER,
		"0",
		SHADER_PARAM_NOT_EDITABLE
	},

	// Used for IS_FLAG_DEFINED2, the difference to IS_FLAG_SET2 is, whether these are actually present in the VMT File
	{
		"$Flags_Defined2",
		"(INTERNAL PARAMETER) Stores whether a Flag2 is defined, used for IS_FLAG_DEFINED2, which is different from IS_FLAG_SET2.",
		SHADER_PARAM_TYPE_INTEGER,
		"0",
		SHADER_PARAM_NOT_EDITABLE
	},

	// $Color, the main tinting Parameter
	{
		"$Color",
		"First Color-Tint, Multiplier for (usually) the BaseTexture.",
		SHADER_PARAM_TYPE_COLOR,
		"[1 1 1]",
		0
	},

	// $Alpha, Used to scale transparency
	{
		"$Alpha",
		"Alpha Modulation Var. Usually not setable by VMT's. Entities can write to this using their Alpha Value.",
		SHADER_PARAM_TYPE_FLOAT,
		"1.0",
		0
	},

	// Default Albedo/Diffuse Texture for most Shaders
	{
		"$BaseTexture",
		"[RGB] Albedo / Diffuse Texture of the Surface.\n[A] Mask for $BaseAlphaEnvMapMask, $SelfIllum, $BaseMapAlphaPhongMask, etc.",
		SHADER_PARAM_TYPE_TEXTURE,
		"shadertest/BaseTexture",
		0
	},

	// Frame Parameter for $BaseTexture
	{
		"$Frame",
		"Frame Number for $BaseTexture.",
		SHADER_PARAM_TYPE_INTEGER,
		"0",
		0
	},

	// Transform Parameter for $BaseTexture
	{
		"$BaseTextureTransform",
		"Transforms for $BaseTexture. Must include all Values!",
		SHADER_PARAM_TYPE_MATRIX,
		"center .5 .5 scale 1 1 rotate 0 translate 0 0",
		0
	},

	// Not currently known if this is still used anywhere
	{
		"$FlashlightTexture",
		"(DEPRECATED).",
		SHADER_PARAM_TYPE_TEXTURE,
		"effects/flashlight001",
		SHADER_PARAM_NOT_EDITABLE
	},

	// Not currently known if this is still used anywhere
	{
		"$FlashlightTextureFrame",
		"(DEPRECATED).",
		SHADER_PARAM_TYPE_INTEGER,
		"0",
		SHADER_PARAM_NOT_EDITABLE
	},

	// Alternative Tint Parameter to $Color, automatically set to the RenderColor on a Prop
	// If you use this on your Shader you should support $NoTint/$AllowDiffuseModulation to disable RenderColor influence
	{
		"$Color2",
		"Second Color-Tint, Multiplier for (usually) the BaseTexture. Entities (Prop_Dynamic for Example) set this Parameter with the RenderColor Keyvalue.",
		SHADER_PARAM_TYPE_COLOR,
		"[1 1 1]",
		0
	},

	// Yet another alternative Tint Parameter. 
	// Usually only used when g_pHardwareConfig->UsesSRGBCorrectBlending() is true
	{
		"$sRGBTint",
		"Secret third Color-Tint, Multiplier for (usually) the BaseTexture. Only allowed when UsesSRGBCorrectBlending() is true.",
		SHADER_PARAM_TYPE_COLOR,
		"[1 1 1]",
		0
	},

	// $Alpha is often hooked by Proxies or not writeable.
	// This can always be written to,
	// although it may not have the same Behaviour as $Alpha and acts more like a writeable multiplier
	{
		"$Alpha2",
		"Compared to $Alpha ( Sometimes used by the System or Proxies ), $Alpha2 can be used to modify Alpha.",
		SHADER_PARAM_TYPE_FLOAT,
		"0",
		0
	},

	// $AlphaTestReference was previously a Parameter on various Shaders
	// Since it's very closely related to $AlphaTest, it should be a default Parameter.
	// It also makes our C++ Code nicer because now we don't have like 20 different instances of it across multiple Shaders.
	// And won't have to pass on AlphaTestReference in Structs and Functions.
	{
		"$AlphaTestReference",
		"Specifies the threshold Alpha Value at which the Surface should be transparent instead of opaque.\n"
		"Most Shaders use the GEQUAl AlphaFunc.. Where Alpha Values smaller than the AlphaTestReference make the Surface transparent.",
		SHADER_PARAM_TYPE_FLOAT,
		"0",
		0
	},

	// Handy Parameter to stop a Shader from applying projected Textures
	{
		"$ReceiveProjectedTextures",
		"Whether or not the Material should be able to receive/render projected Textures.\nSome Shaders will force this, usually when they are incapable of receiving them.",
		SHADER_PARAM_TYPE_BOOL,
		"0",
		0
	},

	// Allows for an unfinished Stock Feature where N.L is not considered
	{
		"$ProjectedTextureNoLambert",
		"Material using this Parameter will not have a N.L Term for Projected Textures. Allows Shadows to render onto the backfaces. This is only really useful for flat Geometry with $NoCull.",
		SHADER_PARAM_TYPE_BOOL,
		"0",
		0
	},

	// Handy Parameter to stop a Shader from applying $Color2
	{
		"$AllowDiffuseModulation",
		"Whether or not $Color2 (And thus RenderColor) should affect the Material.",
		SHADER_PARAM_TYPE_BOOL,
		"1",
		0
	},

	// Handy Parameter to stop a Shader from applying $Color2
	{
		"$NoTint",
		"Whether or not $Color2 (And thus RenderColor) should affect the Material. Same as $AllowDiffuseModulation, exists for CS:GO Material Support.",
		SHADER_PARAM_TYPE_BOOL,
		"0",
		0
	},

	// There is an existing Parameter called $No_Draw that disables drawing.
	// It appears you cannot set it via a Proxy or in Client Code for that matter.
	// This makes it very difficult to dynamically not-draw a specific Material on a Model with several Materials.
	// I made this Parameter after this specific Issue arised on TF2C.
	{
		"$DynamicNoDraw",
		"Whether the Material *shouldn't* render. This is an alternate Version to the $No_Draw Parameter.",
		SHADER_PARAM_TYPE_BOOL,
		"0",
		0
	},

	// Parameter for debugging SPECIFIC Materials.
	// Use lux_debug_material to specify a Keyword foudn in the Materials Name.
	// Then set a breakpoint in an if-Statement that checks the boolean Value of this Parameter
	// 
	// Especially useful if you are trying to debug a Model-Material with unknown VMT Name.
	// For example, if it's a Chair Model, try "Chair" as the Keyword.
	// Reload all "Chair" Materials ( mat_reloadmaterial Chair ).
	// If you set a breakpoint ( and the Keyword matches ), the attached Debugger will stop the Program in your if-Statement.
	{
		"$Debug_True",
		"Whether or not the Keyword from the lux_debug_material ConVar matched with the MaterialName. Debugging Parameter.",
		SHADER_PARAM_TYPE_BOOL,
		"0",
		SHADER_PARAM_NOT_EDITABLE
	},
};

//-----------------------------------------------------------------------------
// Default Shader Parameter Interface
//-----------------------------------------------------------------------------
// ShiroDkxtro2: This replaces the virtual void, from BaseShader.h
// There are multiple virtual voids with this Name..
// IShader has one for Example. 
int CBaseShader::GetNumParams( ) const
{ 
	return NUM_SHADER_MATERIAL_VARS; 
}

char const* CBaseShader::GetParamName( int nParamIndex ) const
{
	Assert( nParamIndex < NUM_SHADER_MATERIAL_VARS );
	return s_StandardParams[nParamIndex].m_pName;
}

const char *CBaseShader::GetParamHelp( int nParamIndex ) const
{
	Assert( nParamIndex < NUM_SHADER_MATERIAL_VARS );
	return s_StandardParams[nParamIndex].m_pHelp;
}

ShaderParamType_t CBaseShader::GetParamType( int nParamIndex ) const
{
	Assert( nParamIndex < NUM_SHADER_MATERIAL_VARS );
	return s_StandardParams[nParamIndex].m_Type;
}

const char *CBaseShader::GetParamDefault( int nParamIndex ) const
{
	Assert( nParamIndex < NUM_SHADER_MATERIAL_VARS );
	return s_StandardParams[nParamIndex].m_pDefaultValue;
}

int CBaseShader::GetParamFlags( int nParamIndex ) const
{
	Assert( nParamIndex < NUM_SHADER_MATERIAL_VARS );
	return s_StandardParams[nParamIndex].m_nFlags;
}

//-----------------------------------------------------------------------------
// Called upon by the System. Forwards to a Shaders PARAM_INIT Section
//-----------------------------------------------------------------------------
void CBaseShader::InitShaderParams(IMaterialVar** ppParams, const char *pMaterialName)
{
	// 'Re-entrancy check'
	Assert( !m_ppParams );
	m_ppParams = ppParams;

	// Set $AllowDiffuseModulation to true for all Shaders
	// ( Default Value )
	if (!IsDefined(AllowDiffuseModulation))
		SetBool(AllowDiffuseModulation, true);

	// Projected Textures can be received unless $ReceiveProjectedTextures is set to 0
	if (!IsDefined(ReceiveProjectedTextures))
		SetBool(ReceiveProjectedTextures, true);

	// lux_debug_..
	#ifdef DEBUG
	if(g_bDebugSpew || g_bBreakPointShadow || g_bBreakPointDynamic)
		SetBool(Debug_True, true);
	#endif

	// Set $Alpha2 to 1.0f for all Shaders
	// ( Defeault Value )
	// ShiroDkxtro2: Yikes! Proxies that use this Parameter will not define it.
	// This goes back to the TextureTransform Proxy Problem!!
	// Unfortunately there is nothing I can do about that!
	if (!IsDefined(Alpha2))
		SetFloat(Alpha2, 1.0f);

	// Call the Shaders PARAM_INIT
	OnInitShaderParams( ppParams, pMaterialName );

	// Reset so we don't pass invalid Data
	m_ppParams = NULL;
}

//-----------------------------------------------------------------------------
// Called upon by the System. Forwards to a Shaders SHADER_INIT Section
//-----------------------------------------------------------------------------
void CBaseShader::InitShaderInstance( IMaterialVar** ppParams, IShaderInit *pShaderInit, const char *pMaterialName, const char *pTextureGroupName )
{
	// 'Re-entrancy check'
	Assert( !m_ppParams );

	m_ppParams = ppParams;
	m_pShaderInit = pShaderInit;
	m_pTextureGroupName = pTextureGroupName;

	// Call the Shaders SHADER_INIT
	OnInitShaderInstance( ppParams, pShaderInit, pMaterialName );

	// Reset so we don't pass invalid Data
	m_pTextureGroupName = NULL;
	m_ppParams = NULL;
	m_pShaderInit = NULL;
}

//-----------------------------------------------------------------------------
// Called upon by the System. Forwards to a Shaders SHADER_INIT Section
//-----------------------------------------------------------------------------
void CBaseShader::DrawElements( IMaterialVar **ppParams, int nModulationFlags,
	IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI, VertexCompressionType_t vertexCompression, CBasePerMaterialContextData **pContextDataPtr )
{
	VPROF("CBaseShader::DrawElements");

	// 'Re-entrancy check'
	Assert( !m_ppParams );

	m_ppParams = ppParams;

	bool bProxies = lux_general_shaderstatewrappers.GetBool();
	if(bProxies)
	{
		// Can use these as IShaderShadow and IShaderDynamicAPI as they inherit from them
		m_pShaderShadow = s_ProxyShaderShadow.GetProxy(pShaderShadow);
		m_pShaderAPI = s_ProxyShaderAPI.GetProxy(pShaderAPI);
	}
	else
	{
		m_pShaderShadow = pShaderShadow;
		m_pShaderAPI = pShaderAPI;
	}

	m_nModulationFlags = nModulationFlags;
	m_pMeshBuilder = pShaderAPI ? pShaderAPI->GetVertexModifyBuilder() : NULL;
	m_nVertexCompression = vertexCompression;

#ifdef DEBUG
	bool bDebug = GetBool(Debug_True);

	// BreakPoint if desired.
	if(bDebug && ((g_bBreakPointShadow && pShaderShadow) || (g_bBreakPointDynamic && pShaderAPI)))
	{
		// Reset so this doesn't trigger on every Material
		g_bBreakPointShadow = false;
		g_bBreakPointDynamic = false;

		// You may now step through the Shader, specific to the specified Material
		DebuggerBreakIfDebugging();
	}

	// Only Spew when bDebugSpew and $Debug_True
	s_ShaderSpew.Start(CurrentShaderName(), CurrentMaterialName(), bDebug && g_bDebugSpew);
#endif

	// Send the MaterialName towards the vcs Reload Class
	if(g_bHotReloadEnabled)
		g_ShaderReload.SetMaterialName(CurrentMaterialName());

	if (IsSnapshotting())
	{
		// Set up the shadow state
		SetInitialShadowState();
	}
	
	UpdateMaterialContextData(pContextDataPtr);

	// Don't Draw anything if there's an undesired projected Texture,
	// else call the Shaders SHADER_DRAW
	if (UsingFlashlight() && !GetBool(ReceiveProjectedTextures))
		Draw(false);
	else
		OnDrawElements(ppParams, m_pShaderShadow, m_pShaderAPI, vertexCompression, pContextDataPtr);

	if (m_pMaterialContextData)
	{
		m_pMaterialContextData->m_bSnapshottingCommands = false;
	}

	// Reset so we don't pass invalid Data
	m_nModulationFlags = 0;
	m_nVertexCompression = VERTEX_COMPRESSION_INVALID;
	m_ppParams = NULL;
	m_pShaderAPI = NULL;
	m_pShaderShadow = NULL;
	m_pMeshBuilder = NULL;

	// Reset the Proxies.
	// Ensures that the next Draw() doesn't get dangling Pointers
	if(bProxies)
	{
		s_ProxyShaderShadow.ResetShaderShadow();
		s_ProxyShaderAPI.ResetShaderAPI();	
	}

	
#ifdef DEBUG
	// End Point for ShaderSpew, this is where printing happens
	s_ShaderSpew.End();

	// After rendering the Material once, reset s_ShaderSpew
	// Note that we only record during REAL pShaderShadow, so NOT that here
	if (s_ShaderSpew.SpewSpecificMaterial() && g_bDebugSpew && bDebug && !pShaderShadow)
	{
		g_bDebugSpew = false;
		s_ShaderSpew.AllowSpew(false);
	}
#endif
}

//-----------------------------------------------------------------------------
// Sets the default Values for the Shadow State
//-----------------------------------------------------------------------------
void CBaseShader::SetInitialShadowState( )
{
	// Reset the ShaderShadow State
	m_pShaderShadow->SetDefaultState();

	// Some Flags ( $IgnoreZ, $Decal, $NoCull, etc )
	// have Behaviours not handled by the Shader itself
	// They are handled here.
	int nFlags = GetInt(Flags);
	if (nFlags & MATERIAL_VAR_IGNOREZ)
	{
		m_pShaderShadow->EnableDepthTest( false );
		m_pShaderShadow->EnableDepthWrites( false );
	}

	// Decals don't need DepthWrites, the Surface they are drawn onto, already have done that.
	if (nFlags & MATERIAL_VAR_DECAL)
	{
		// ShiroDkxtro2: From my own Experience with D3D9,
		// it does not appear to have dedicated Offset Functionality
		// OpenGL *does*, and I don't know about Vulkan.
		// Very curious to see how this was done.
		m_pShaderShadow->EnablePolyOffset( SHADER_POLYOFFSET_DECAL );
		m_pShaderShadow->EnableDepthWrites( false );
	}

	if (nFlags & MATERIAL_VAR_NOCULL)
	{
		m_pShaderShadow->EnableCulling( false );
	}

	if (nFlags & MATERIAL_VAR_ZNEARER)
	{
		m_pShaderShadow->DepthFunc( SHADER_DEPTHFUNC_NEARER );
	}

	if (nFlags & MATERIAL_VAR_WIREFRAME)
	{
		m_pShaderShadow->PolyMode( SHADER_POLYMODEFACE_FRONT_AND_BACK, SHADER_POLYMODE_LINE );
	}

	if (nFlags & MATERIAL_VAR_ALLOWALPHATOCOVERAGE)
	{
		// "Force the bit on and then check against alpha blend and test states in CShaderShadowDX8::ComputeAggregateShadowState()"
		m_pShaderShadow->EnableAlphaToCoverage( true );
	}
}

//-----------------------------------------------------------------------------
// 'Draws a Snapshot', makes the Snapshot when Snapshotting, otherwise Draws.
//-----------------------------------------------------------------------------
void CBaseShader::Draw( bool bMakeActualDrawCall )
{
	// If we are spewing Information about Dynamic State, IsSnapshotting will be true.
	// So I'm checking this again m_pShaderAPI to make sure we don't go down the wrong path.
	if (!m_pShaderAPI && IsSnapshotting())
	{
		// Turn off Transparency if we're asked to....
		// ShiroDkxtro2: Is there a ConVar that can set g_pConfig->bNoTransparency ?
		if (g_pConfig->bNoTransparency && !HasFlag(MATERIAL_VAR_NO_DEBUG_OVERRIDE))
		{
			m_pShaderShadow->EnableDepthWrites( true );
 			m_pShaderShadow->EnableBlending( false );
		}

		GetShaderSystem()->TakeSnapshot();

#ifdef DEBUG
		s_ShaderSpew.LogDraw_ShadowState();
#endif
	}
	else
	{
		// Set Half-Pixel Offset Fix Constant before rendering
		SetHPOFixConstant();

		// ShiroDkxtro2: $DynamicNoDraw is set to 0 by Default
		// If it's set to 1 by a Proxy or via Client IMaterialVar*, this will fail and not draw the Material. Dynamically.
		GetShaderSystem()->DrawSnapshot(bMakeActualDrawCall && !GetBool(DynamicNoDraw));

#ifdef DEBUG
		// We only go down the Snapshot or Dynamic Path
		// In case of Dynamic State we also need to log the fake ShadowState
		s_ShaderSpew.LogDraw_ShadowState();
		s_ShaderSpew.LogDraw_DynamicState();
#endif
	}
}

//-----------------------------------------------------------------------------
// Finds a particular Parameter	(works because the lowest parameters match the shader)
//-----------------------------------------------------------------------------
int CBaseShader::FindParamIndex( const char *pName ) const
{
	int nNumParams = GetNumParams();
	for( int i = 0; i < nNumParams; i++ )
	{
		if( Q_strnicmp( GetParamName( i ), pName, 64 ) == 0 )
		{
			return i;
		}
	}
	return -1;
}

//-----------------------------------------------------------------------------
// 'Are we using graphics?'
//-----------------------------------------------------------------------------
bool CBaseShader::IsUsingGraphics()
{
	return GetShaderSystem()->IsUsingGraphics();
}

//-----------------------------------------------------------------------------
// "Are we using graphics?" Again? Urgh
//-----------------------------------------------------------------------------
bool CBaseShader::CanUseEditorMaterials()
{
	return GetShaderSystem()->CanUseEditorMaterials();
}

//-----------------------------------------------------------------------------
// "Gets the builder..."
//-----------------------------------------------------------------------------
CMeshBuilder* CBaseShader::MeshBuilder()
{
	return m_pMeshBuilder;
}

//-----------------------------------------------------------------------------
// Loads a Texture
// ShiroDkxtro2: Both SDK2007 and ASW don't have 'nAdditionalCreationFlags' for 'm_pShaderInit->LoadTexture()'
// It was added later, *but* SDK2013's ishaderapi.h gives us a virtual Function, that tells us how to use it
//-----------------------------------------------------------------------------
void CBaseShader::LoadTexture(int nTextureVar, int nAdditionalCreationFlags)
{
	if ((!m_ppParams) || (nTextureVar == -1))
		return;

	IMaterialVar* pNameVar = m_ppParams[nTextureVar];
	if( pNameVar && pNameVar->IsDefined() )
	{
		m_pShaderInit->LoadTexture(pNameVar, m_pTextureGroupName, nAdditionalCreationFlags);
	}
}

//-----------------------------------------------------------------------------
// Loads a BumpMap
//-----------------------------------------------------------------------------
void CBaseShader::LoadBumpMap( int nTextureVar )
{
	if ((!m_ppParams) || (nTextureVar == -1))
		return;

	IMaterialVar* pNameVar = m_ppParams[nTextureVar];
	if( pNameVar && pNameVar->IsDefined() )
	{
		m_pShaderInit->LoadBumpMap( pNameVar, m_pTextureGroupName );
	}
}

//-----------------------------------------------------------------------------
// Loads a Cubemap
// ShiroDkxtro2: Both SDK2007 and ASW don't have 'nAdditionalCreationFlags' for 'm_pShaderInit->LoadCubeMap()'
// It was added later, *but* SDK2013's ishaderapi.h gives us a virtual Function, that tells us how to use it
//-----------------------------------------------------------------------------
void CBaseShader::LoadCubeMap(int nTextureVar, int nAdditionalCreationFlags)
{
	if ((!m_ppParams) || (nTextureVar == -1))
		return;

	IMaterialVar* pNameVar = m_ppParams[nTextureVar];
	if( pNameVar && pNameVar->IsDefined() )
	{
		m_pShaderInit->LoadCubeMap(m_ppParams, pNameVar, nAdditionalCreationFlags);
	}
}

ShaderAPITextureHandle_t CBaseShader::GetShaderAPITextureBindHandle( int nTextureVar, int nFrameVar, int nTextureChannel )
{
	if(!(m_pShaderAPI && m_pShaderShadow))
		Assert( !IsSnapshotting() );

	Assert( nTextureVar != -1 );
	Assert ( m_ppParams );

	IMaterialVar* pTextureVar = m_ppParams[nTextureVar];
	IMaterialVar* pFrameVar = (nFrameVar != -1) ? m_ppParams[nFrameVar] : NULL;
	int nFrame = pFrameVar ? pFrameVar->GetIntValue() : 0;
	return GetShaderSystem()->GetShaderAPITextureBindHandle( pTextureVar->GetTextureValue(), nFrame, nTextureChannel );
}

//-----------------------------------------------------------------------------
// "Four different flavors of BindTexture(), handling the two-sampler
// case as well as ITexture* versus textureVar forms"
//-----------------------------------------------------------------------------
void CBaseShader::BindTexture( Sampler_t sampler1, int nTextureVar, int nFrameVar /* = -1 */ )
{
	if(!(m_pShaderAPI && m_pShaderShadow))
		Assert(!IsSnapshotting());

	Assert(nTextureVar != -1);
	Assert(m_ppParams);

	IMaterialVar* pTextureVar = m_ppParams[nTextureVar];
	IMaterialVar* pFrameVar = (nFrameVar != -1) ? m_ppParams[nFrameVar] : NULL;
	if (pTextureVar)
	{
		int nFrame = pFrameVar ? pFrameVar->GetIntValue() : 0;

		ITexture* pTexture = pTextureVar->GetTextureValue();

		if (pTexture)
		{
#ifdef DEBUG
			s_ShaderSpew.LogBindTexture((int)sampler1, pTextureVar->GetName());
#endif
			GetShaderSystem()->BindTexture(sampler1, pTexture, nFrame);
		}
		else
			Warning("Invalid Texture Reference from ITexture* from Material %s\n", m_ppParams[Flags]->GetOwningMaterial()->GetName());
	}
}

/*
void CBaseShader::BindTexture( Sampler_t sampler1, Sampler_t sampler2, int nTextureVar, int nFrameVar)
{
	Assert( !IsSnapshotting() );
	Assert( nTextureVar != -1 );
	Assert ( m_ppParams );

	IMaterialVar* pTextureVar = m_ppParams[nTextureVar];
	IMaterialVar* pFrameVar = (nFrameVar != -1) ? m_ppParams[nFrameVar] : NULL;
	if (pTextureVar)
	{
		int nFrame = pFrameVar ? pFrameVar->GetIntValue() : 0;

		if ( sampler2 == -1 )
		{
			GetShaderSystem()->BindTexture( sampler1, pTextureVar->GetTextureValue(), nFrame );
		}
		else
		{
			GetShaderSystem()->BindTexture( sampler1, sampler2, pTextureVar->GetTextureValue(), nFrame );
		}
	}
}
*/

void CBaseShader::BindTexture( Sampler_t sampler1, ITexture *pTexture, int nFrame )
{
	if(!(m_pShaderAPI && m_pShaderShadow))			
		Assert(!IsSnapshotting());

	Assert(m_ppParams);

	if (pTexture)
	{
#ifdef DEBUG
		s_ShaderSpew.LogBindTexture((int)sampler1, pTexture);
#endif
		GetShaderSystem()->BindTexture(sampler1, pTexture, nFrame);
	}
	else
		Warning("Invalid Texture Reference from ITexture* from Material %s\n", m_ppParams[Flags]->GetOwningMaterial()->GetName());
}

/*
void CBaseShader::BindTexture( Sampler_t sampler1, Sampler_t sampler2, ITexture *pTexture, int nFrame )
{
	Assert( !IsSnapshotting() );

	if ( sampler2 == -1 )
	{
		GetShaderSystem()->BindTexture( sampler1, pTexture, nFrame );
	}
	else
	{
		GetShaderSystem()->BindTexture( sampler1, sampler2, pTexture, nFrame );
	}
}
*/

//-----------------------------------------------------------------------------
// "Does the texture store translucency in its alpha channel?"
// ShiroDkxtro2: Slightly modified Version of this Function
//-----------------------------------------------------------------------------
bool CBaseShader::TextureIsTranslucent( int textureVar, bool isBaseTexture )
{
	if (textureVar < 0)
		return false;

	IMaterialVar** params = m_ppParams;
	if (params[textureVar]->GetType() == MATERIAL_VAR_TYPE_TEXTURE)
	{
		if (!isBaseTexture)
		{
			return GetTexture(textureVar)->IsTranslucent();
		}
		else
		{
			// Override translucency settings if this flag is set.
			if (HasFlag(MATERIAL_VAR_OPAQUETEXTURE))
				return false;

			// Check if we are using BaseTexture Alpha for something other than translucency.
			// ShiroDkxtro2: This does not consider other Parameters like BaseMapAlphaPhongMask, or BlendTintByBaseAlpha..
			if (!HasFlag(MATERIAL_VAR_SELFILLUM) && !HasFlag(MATERIAL_VAR_BASEALPHAENVMAPMASK))
			{
				// Check if the Material has $Translucent or $AlphaTest.
				if (HasFlag(MATERIAL_VAR_TRANSLUCENT) || HasFlag(MATERIAL_VAR_ALPHATEST))
				{
					// Make sure the Texture has an Alpha Channel.
					// ( This is potentially unreliable )
					return GetTexture(textureVar)->IsTranslucent();
				}
			}
		}
	}

	return false;
}


//-----------------------------------------------------------------------------
//
// "Helper methods for color modulation"
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// "Are we alpha or color modulating?"
//-----------------------------------------------------------------------------
bool CBaseShader::IsAlphaModulating()
{
	return (m_nModulationFlags & SHADER_USING_ALPHA_MODULATION) != 0;
}

bool CBaseShader::IsColorModulating()
{
	return (m_nModulationFlags & SHADER_USING_COLOR_MODULATION) != 0;
}

void CBaseShader::GetColorParameter( IMaterialVar **params, float *pColorOut ) const
{
	float flColor2[3];
	params[Color1]->GetVecValue( pColorOut, 3 );
	params[Color2]->GetVecValue( flColor2, 3 );

	pColorOut[0] *= flColor2[0];
	pColorOut[1] *= flColor2[1];
	pColorOut[2] *= flColor2[2];

	if ( g_pHardwareConfig->UsesSRGBCorrectBlending() )
	{
		float flSRGBTint[3];
		params[sRGBTint]->GetVecValue( flSRGBTint, 3 );
		
		pColorOut[0] *= flSRGBTint[0];
		pColorOut[1] *= flSRGBTint[1];
		pColorOut[2] *= flSRGBTint[2];
	}
}

//-----------------------------------------------------------------------------
// "Figure out a better way to do this?"
//-----------------------------------------------------------------------------
int CBaseShader::ComputeModulationFlags( IMaterialVar** params, IShaderDynamicAPI* pShaderAPI )
{
	int mod = 0;
	if ( GetAlpha(params) < 1.0f )
	{
		mod |= SHADER_USING_ALPHA_MODULATION;
	}

	float color[3];
	GetColorParameter( params, color );

	if ((color[0] != 1.0) || (color[1] != 1.0) || (color[2] != 1.0))
	{
		mod |= SHADER_USING_COLOR_MODULATION;
	}

	// ShiroDkxtro2: Made this compliant with multithreaded calls to this Function
	if(IsSnapshotting() && IS_FLAG2_SET(MATERIAL_VAR2_USE_FLASHLIGHT) || pShaderAPI->InFlashlightMode())
	{
		mod |= SHADER_USING_FLASHLIGHT;
	}
	
	if (IsSnapshotting() && IS_FLAG2_SET(MATERIAL_VAR2_USE_EDITOR) || pShaderAPI->InEditorMode())
	{
		mod |= SHADER_USING_EDITOR;
	}

	if(IS_FLAG2_SET( MATERIAL_VAR2_USE_FIXED_FUNCTION_BAKED_LIGHTING ) )
	{
		AssertOnce(IS_FLAG2_SET( MATERIAL_VAR2_NEEDS_BAKED_LIGHTING_SNAPSHOTS ) );
		if(IS_FLAG2_SET( MATERIAL_VAR2_NEEDS_BAKED_LIGHTING_SNAPSHOTS ) )
		{
			mod |= SHADER_USING_FIXED_FUNCTION_BAKED_LIGHTING;
		}
	}

	return mod;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
bool CBaseShader::NeedsPowerOfTwoFrameBufferTexture( IMaterialVar **params, bool bCheckSpecificToThisFrame ) const 
{ 
	// This is a Virtual Function called by Client Code ( Particle System to name an Example ) 
	// So use the manual Method or we get a crash since m_ppParams is nullptr
	return HasFlag2(params, MATERIAL_VAR2_NEEDS_POWER_OF_TWO_FRAME_BUFFER_TEXTURE);
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
bool CBaseShader::NeedsFullFrameBufferTexture( IMaterialVar **params, bool bCheckSpecificToThisFrame ) const 
{ 
	// This is a Virtual Function called by Client Code ( Particle System to name an Example ) 
	// So use the manual Method or we get a crash since m_ppParams is nullptr
	return (params[Flags2]->GetIntValue() & MATERIAL_VAR2_NEEDS_FULL_FRAME_BUFFER_TEXTURE) != 0;
}

//-----------------------------------------------------------------------------
// ShiroDkxtro2: This is kinda pointless..
//-----------------------------------------------------------------------------
bool CBaseShader::IsTranslucent( IMaterialVar **params ) const
{
	// This is a Virtual Function called by Client Code ( Particle System to name an Example ) 
	// So use the manual Method or we get a crash since m_ppParams is nullptr
	return (params[Flags]->GetIntValue() & MATERIAL_VAR_TRANSLUCENT) != 0;
}

//-----------------------------------------------------------------------------
// Returns the translucency...
//-----------------------------------------------------------------------------
float CBaseShader::GetAlpha( IMaterialVar** ppParams )
{
	if ( !ppParams )
	{
		ppParams = m_ppParams;
	}

	// When would this EVER be the Case?
	if (!ppParams)
		return 1.0f;

	if (ppParams[Flags]->GetIntValue() & MATERIAL_VAR_NOALPHAMOD )
		return 1.0f;

	float f1Alpha1 = ppParams[Alpha]->GetFloatValue();
//	return clamp(f1Alpha1, 0.0f, 1.0f);

	// Support for second Alpha Parameter! Yay!
	float f1Alpha2 = ppParams[Alpha2]->GetFloatValue();
	return clamp( f1Alpha1 * f1Alpha2, 0.0f, 1.0f );
}

//-----------------------------------------------------------------------------
// "Sets the color + transparency"
// ShiroDkxtro2: Is this old Fixed Function Pipeline Stuff?
//-----------------------------------------------------------------------------
void CBaseShader::SetColorState( int colorVar, bool setAlpha )
{
	if(!(m_pShaderAPI && m_pShaderShadow))
		Assert( !IsSnapshotting() );

	if ( !m_ppParams )
		return;

	// Use tint instead of color if it was specified...
	IMaterialVar* pColorVar = (colorVar != -1) ? m_ppParams[colorVar] : 0;

	float color[4] = { 1.0, 1.0, 1.0, 1.0 };
	if (pColorVar)
	{
		if (pColorVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
		{
			pColorVar->GetVecValue( color, 3 );
		}
		else
		{
			color[0] = color[1] = color[2] = pColorVar->GetFloatValue();
		}
		
		if ( !g_pHardwareConfig->SupportsPixelShaders_1_4() )		// Clamp 0..1 for ps_1_1 and below
		{
			color[0] = clamp( color[0], 0.0f, 1.0f );
			color[1] = clamp( color[1], 0.0f, 1.0f );
			color[2] = clamp( color[2], 0.0f, 1.0f );
		}
		else if ( !g_pHardwareConfig->SupportsPixelShaders_2_0() ) 	// Clamp 0..8 for ps_1_4
		{
			color[0] = clamp( color[0], 0.0f, 8.0f );
			color[1] = clamp( color[1], 0.0f, 8.0f );
			color[2] = clamp( color[2], 0.0f, 8.0f );
		}
	}
	ApplyColor2Factor( color );
	color[3] = setAlpha ? GetAlpha() : 1.0f;
	m_pShaderAPI->Color4fv( color );	
}

// ShiroDkxtro2: Is this old Fixed Function Pipeline Stuff?
void CBaseShader::SetModulationShadowState( int tintVar )
{
	// Have have no control over the tint var...
	bool doModulation = (tintVar != -1);

	// We activate color modulating when we're alpha or color modulating
	doModulation = doModulation || IsAlphaModulating() || IsColorModulating();

	m_pShaderShadow->EnableConstantColor( doModulation );
}

// ShiroDkxtro2: Is this old Fixed Function Pipeline Stuff?
void CBaseShader::SetModulationDynamicState( int tintVar )
{
	if (tintVar != -1)
	{
		SetColorState( tintVar, true );
	}
	else
	{
		SetColorState( Color1, true );
	}
}

// Used by ComputeModulationColor below
void CBaseShader::ApplyColor2Factor( float *pColorOut ) const // (*pColorOut) *= Color2
{
	IMaterialVar* pColor2Var = m_ppParams[Color2];
	if (pColor2Var->GetType() == MATERIAL_VAR_TYPE_VECTOR)
	{
		float flColor2[3];
		pColor2Var->GetVecValue( flColor2, 3 );
		
		pColorOut[0] *= flColor2[0];
		pColorOut[1] *= flColor2[1];
		pColorOut[2] *= flColor2[2];
	}
	if ( g_pHardwareConfig->UsesSRGBCorrectBlending() )
	{
		IMaterialVar* pSRGBVar = m_ppParams[sRGBTint];
		if (pSRGBVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
		{
			float flSRGB[3];
			pSRGBVar->GetVecValue( flSRGB, 3 );
			
			pColorOut[0] *= flSRGB[0];
			pColorOut[1] *= flSRGB[1];
			pColorOut[2] *= flSRGB[2];
		}
	}
}

void CBaseShader::ComputeModulationColor( float* color )
{
	if(!(m_pShaderAPI && m_pShaderShadow))
		Assert( !IsSnapshotting() );

	if (!m_ppParams)          
		return;

	IMaterialVar* pColorVar = m_ppParams[Color1];
	if (pColorVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
	{
		pColorVar->GetVecValue( color, 3 );
	}
	else
	{
		color[0] = color[1] = color[2] = pColorVar->GetFloatValue();
	}

	ApplyColor2Factor( color );

	// ShiroDkxtro2: bShowDiffuse can be set using the mat_diffuse ConVar.
	if( !g_pConfig->bShowDiffuse )
	{
		color[0] = color[1] = color[2] = 0.0f;
	}
	if( mat_fullbright.GetInt() == 2 )
	{
		color[0] = color[1] = color[2] = 1.0f;
	}
	color[3] = GetAlpha();
}

//-----------------------------------------------------------------------------
// "Helper methods for alpha blending...."
//-----------------------------------------------------------------------------
void CBaseShader::EnableAlphaBlending( ShaderBlendFactor_t src, ShaderBlendFactor_t dst )
{
	Assert( IsSnapshotting() );
	m_pShaderShadow->EnableBlending( true );
	m_pShaderShadow->BlendFunc( src, dst );
	m_pShaderShadow->EnableDepthWrites(false);
}

void CBaseShader::DisableAlphaBlending()
{
	Assert( IsSnapshotting() );
	m_pShaderShadow->EnableBlending( false );
}

void CBaseShader::SetNormalBlendingShadowState( int textureVar, bool isBaseTexture )
{
	Assert( IsSnapshotting() );

	// Either we've got a constant modulation
	bool isTranslucent = IsAlphaModulating();

	// Or we've got a vertex alpha
	isTranslucent = isTranslucent || (CurrentMaterialVarFlags() & MATERIAL_VAR_VERTEXALPHA);

	// Or we've got a texture alpha
	isTranslucent = isTranslucent || ( TextureIsTranslucent( textureVar, isBaseTexture ) &&
		                               !(CurrentMaterialVarFlags() & MATERIAL_VAR_ALPHATEST ) );

	if (isTranslucent)
	{
		EnableAlphaBlending( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
	}
	else
	{
		DisableAlphaBlending();
	}
}

//ConVar mat_debug_flashlight_only( "mat_debug_flashlight_only", "0" );
void CBaseShader::SetAdditiveBlendingShadowState( int textureVar, bool isBaseTexture )
{
	Assert( IsSnapshotting() );

	// Either we've got a constant modulation
	bool isTranslucent = IsAlphaModulating();

	// Or we've got a vertex alpha
	isTranslucent = isTranslucent || (CurrentMaterialVarFlags() & MATERIAL_VAR_VERTEXALPHA);

	// Or we've got a texture alpha
	isTranslucent = isTranslucent || ( TextureIsTranslucent( textureVar, isBaseTexture ) &&
		                               !(CurrentMaterialVarFlags() & MATERIAL_VAR_ALPHATEST ) );

	/*
	if ( mat_debug_flashlight_only.GetBool() )
	{
		if (isTranslucent)
		{
			EnableAlphaBlending( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA);
			//m_pShaderShadow->EnableAlphaTest( true );
			//m_pShaderShadow->AlphaFunc( SHADER_ALPHAFUNC_GREATER, 0.99f );
		}
		else
		{
			EnableAlphaBlending( SHADER_BLEND_ONE, SHADER_BLEND_ZERO);
		}
	}
	else
	*/
	{
		if (isTranslucent)
		{
			EnableAlphaBlending( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE );
		}
		else
		{
			EnableAlphaBlending( SHADER_BLEND_ONE, SHADER_BLEND_ONE );
		}
	}
}

void CBaseShader::SetDefaultBlendingShadowState( int textureVar, bool isBaseTexture ) 
{
	if ( CurrentMaterialVarFlags() & MATERIAL_VAR_ADDITIVE )
	{
		SetAdditiveBlendingShadowState( textureVar, isBaseTexture );
	}
	else
	{
		SetNormalBlendingShadowState( textureVar, isBaseTexture );
	}
}

void CBaseShader::SetBlendingShadowState( BlendType_t nMode )
{
	switch ( nMode )
	{
		case BT_NONE:
			DisableAlphaBlending();
			break;

		case BT_BLEND:
			EnableAlphaBlending( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
			break;

		case BT_ADD:
			EnableAlphaBlending( SHADER_BLEND_ONE, SHADER_BLEND_ONE );
			break;

		case BT_BLENDADD:
			EnableAlphaBlending( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE );
			break;
	}
}

//-----------------------------------------------------------------------------
// Loads the identity transform into a matrix
//-----------------------------------------------------------------------------
void CBaseShader::LoadIdentity( MaterialMatrixMode_t matrixMode )
{
	if(!(m_pShaderAPI && m_pShaderShadow))
		Assert( !IsSnapshotting() );

	m_pShaderAPI->MatrixMode( matrixMode );
	m_pShaderAPI->LoadIdentity( );
}

//-----------------------------------------------------------------------------
// Loads the camera to world transform into a matrix
//-----------------------------------------------------------------------------
void CBaseShader::LoadCameraToWorldTransform( MaterialMatrixMode_t matrixMode )
{
	m_pShaderAPI->MatrixMode( matrixMode );
	m_pShaderAPI->LoadCameraToWorld();
}

void CBaseShader::LoadCameraSpaceSphereMapTransform( MaterialMatrixMode_t matrixMode )
{
	static float mat[4][4] = 
	{
		{ 0.5f,  0.0f, 0.0f, 0.0f },
		{ 0.0f, -0.5f, 0.0f, 0.0f },
		{ 0.0f,  0.0f, 0.0f, 0.0f },
		{ 0.5f, -0.5f, 0.0f, 1.0f },
	};

	m_pShaderAPI->MatrixMode( matrixMode );
	m_pShaderAPI->LoadMatrix( (float*)mat );
}

//-----------------------------------------------------------------------------
// Helper methods for fog
//-----------------------------------------------------------------------------
void CBaseShader::FogToOOOverbright( void )
{
	Assert( IsSnapshotting() );
	if (( CurrentMaterialVarFlags() & MATERIAL_VAR_NOFOG ) == 0)
	{
		m_pShaderShadow->FogMode( SHADER_FOGMODE_OO_OVERBRIGHT );
	}
	else
	{
		m_pShaderShadow->FogMode( SHADER_FOGMODE_DISABLED );
	}
}

void CBaseShader::FogToWhite( void )
{
	Assert( IsSnapshotting() );
	if (( CurrentMaterialVarFlags() & MATERIAL_VAR_NOFOG ) == 0)
	{
		m_pShaderShadow->FogMode( SHADER_FOGMODE_WHITE );
	}
	else
	{
		m_pShaderShadow->FogMode( SHADER_FOGMODE_DISABLED );
	}
}
void CBaseShader::FogToBlack( void )
{
	Assert( IsSnapshotting() );
	if (( CurrentMaterialVarFlags() & MATERIAL_VAR_NOFOG ) == 0)
	{
		m_pShaderShadow->FogMode( SHADER_FOGMODE_BLACK );
	}
	else
	{
		m_pShaderShadow->FogMode( SHADER_FOGMODE_DISABLED );
	}
}

void CBaseShader::FogToGrey( void )
{
	Assert( IsSnapshotting() );
	if (( CurrentMaterialVarFlags() & MATERIAL_VAR_NOFOG ) == 0)
	{
		m_pShaderShadow->FogMode( SHADER_FOGMODE_GREY );
	}
	else
	{
		m_pShaderShadow->FogMode( SHADER_FOGMODE_DISABLED );
	}
}

void CBaseShader::FogToFogColor( void )
{
	Assert( IsSnapshotting() );
	if (( CurrentMaterialVarFlags() & MATERIAL_VAR_NOFOG ) == 0)
	{
		m_pShaderShadow->FogMode( SHADER_FOGMODE_FOGCOLOR );
	}
	else
	{
		m_pShaderShadow->FogMode( SHADER_FOGMODE_DISABLED );
	}
}

void CBaseShader::DisableFog( void )
{
	Assert( IsSnapshotting() );
	m_pShaderShadow->FogMode( SHADER_FOGMODE_DISABLED );
}

void CBaseShader::DefaultFog( void )
{
	if ( CurrentMaterialVarFlags() & MATERIAL_VAR_ADDITIVE )
	{
		FogToBlack();
	}
	else
	{
		FogToFogColor();
	}
}

bool CBaseShader::UsingFlashlight() const
{
	if( IsSnapshotting() )
	{
		return HasFlag2(MATERIAL_VAR2_USE_FLASHLIGHT);
	}
	else
	{
		return m_pShaderAPI->InFlashlightMode();
	}
}

bool CBaseShader::UsingEditor() const
{
	if( IsSnapshotting() )
	{
		return HasFlag2( MATERIAL_VAR2_USE_EDITOR );
	}
	else
	{
		return m_pShaderAPI->InEditorMode();
	}
}

// Code from InevitablyDivinity ( slightly modified )
const char* CBaseShader::CurrentMaterialName() const
{
	// Get the Reference to the VMT itself
	IMaterial* pMaterial = m_ppParams[Flags]->GetOwningMaterial();

	// If the Reference is not bogus, returns the local Path and Name of the Material
	if (pMaterial)
	{
		return pMaterial->GetName();
	}
	else
		return " -- Invalid Material Reference -- ";
}

const char* CBaseShader::CurrentShaderName() const
{
	// Get the Reference to the VMT itself
	IMaterial* pMaterial = m_ppParams[Flags]->GetOwningMaterial();

	// If the Reference is not bogus, returns the local Path and Name of the Material
	if (pMaterial)
	{
		return pMaterial->GetShaderName();
	}
	else
		return " -- Invalid Material Reference -- ";
}

//========================//
// Parameter Helpers
//========================//

//==============//
// Get Values
//==============//
float2 CBaseShader::GetFloat2(const int var)
{
	float2 f2Result;
	m_ppParams[var]->GetVecValue(f2Result, 2);
	return f2Result;
}

float3 CBaseShader::GetFloat3(const int var)
{
	float3 f3Result;
	m_ppParams[var]->GetVecValue(f3Result, 3);
	return f3Result;
}

float4 CBaseShader::GetFloat4(const int var)
{
	float4 f4Result;
	m_ppParams[var]->GetVecValue(f4Result, 4);
	return f4Result;
}
// No Overloads for non floatx

const char* CBaseShader::GetString(const int var)
{
	return m_ppParams[var]->GetStringValue();
}

ITexture* CBaseShader::GetTexture(const int var)
{
	return m_ppParams[var]->GetTextureValue();
}

const char* CBaseShader::GetTextureName(const int var)
{
	// NOTE: You would have to use this function in param or shader init
	// Where there is no ITexture Value for a Texture Parameter
	// In param and shader init they are dealth with as String Parameters
	return m_ppParams[var]->GetStringValue();
	//	return m_ppParams[var]->GetTextureValue()->GetName();
}

const char* CBaseShader::GetTextureName(const ITexture* pTexture) // overload
{
	return pTexture->GetName();
}

//==============//
// Set Values
//==============//
void CBaseShader::SetString(const int var, const char* string)
{
	m_ppParams[var]->SetStringValue(string);
}

void CBaseShader::SetUndefined(const int var)
{
	m_ppParams[var]->SetUndefined();
}

void CBaseShader::SetBool(const int var, const bool value)
{
	m_ppParams[var]->SetIntValue(value);
}

void CBaseShader::SetInt(const int var, const int value)
{
	m_ppParams[var]->SetIntValue(value);
}

void CBaseShader::SetFloat(const int var, const float value)
{
	m_ppParams[var]->SetFloatValue(value);
}

// floatx interface
void CBaseShader::SetFloat2(const int var, const float2& value)
{
	m_ppParams[var]->SetVecValue(value, 2);
}

void CBaseShader::SetFloat3(const int var, const float3& value)
{
	m_ppParams[var]->SetVecValue(value, 3);
}

void CBaseShader::SetFloat4(const int var, const float4& value)
{
	m_ppParams[var]->SetVecValue(value, 4);
}

// Overloads for non floatx
void CBaseShader::SetFloat2(const int var, const float x, const float y)
{
	float value[2] = { x, y };
	m_ppParams[var]->SetVecValue(value, 2);
}

void CBaseShader::SetFloat3(const int var, const float x, const float y, const float z)
{
	float value[3] = { x, y, z };
	m_ppParams[var]->SetVecValue(value, 3);
}

void CBaseShader::SetFloat4(const int var, const float x, const float y, const float z, const float w)
{
	float value[4] = { x, y, z, w };
	m_ppParams[var]->SetVecValue(value, 4);
}

//==============//
// Defaults
//==============//
void CBaseShader::DefaultBool(const int var, const bool value)
{
	if (!IsDefined(var))
		m_ppParams[var]->SetIntValue(value);
}

void CBaseShader::DefaultInt(const int var, const int value)
{
	if (!IsDefined(var))
		m_ppParams[var]->SetIntValue(value);
}

void CBaseShader::DefaultFloat(const int var, const float value)
{
	if (!IsDefined(var))
		m_ppParams[var]->SetFloatValue(value);
}

// floatx interface
void CBaseShader::DefaultFloat2(const int var, const float2& value)
{
	if (!IsDefined(var))
		SetFloat2(var, value);
}

void CBaseShader::DefaultFloat3(const int var, const float3& value)
{
	if (!IsDefined(var))
		SetFloat3(var, value);
}

void CBaseShader::DefaultFloat4(const int var, const float4& value)
{
	if (!IsDefined(var))
		SetFloat4(var, value);
}

// Overloads for non floatx
void CBaseShader::DefaultFloat2(const int var, const float x, const float y)
{
	if (!IsDefined(var))
		SetFloat2(var, x, y);
}

void CBaseShader::DefaultFloat3(const int var, const float x, const float y, const float z)
{
	if (!IsDefined(var))
		SetFloat3(var, x, y, z);
}

void CBaseShader::DefaultFloat4(const int var, const float x, const float y, const float z, const float w)
{
	if (!IsDefined(var))
		SetFloat4(var, x, y, z, w);
}

//========================//
// Shader Helpers
//========================//
void CBaseShader::EnableSampler(const Sampler_t nSampler, const bool bSRGBRead)
{
	m_pShaderShadow->EnableTexture(nSampler, true);
	m_pShaderShadow->EnableSRGBRead(nSampler, bSRGBRead);
}

void CBaseShader::EnableSampler(const bool bCheck, const Sampler_t nSampler, const bool bSRGBRead)
{
	// Check first, this is here to make Shaders easier to read
	if (bCheck)
	{
		m_pShaderShadow->EnableTexture(nSampler, true);
		m_pShaderShadow->EnableSRGBRead(nSampler, bSRGBRead);
	}
}

// Already a CBaseShader Function
/*
void CBaseShader::BindTexture(Sampler_t nSampler, const int var, const int framevar)
{
	BindTexture(nSampler, var, framevar);
}

void CBaseShader::BindTexture(Sampler_t nSampler, ITexture* var, const int framevar)
{
	BindTexture(nSampler, var, framevar);
}
*/

void CBaseShader::BindTexture(const bool bCheck, Sampler_t nSampler, ITexture* var, const int framevar)
{
	if (bCheck)
	{
		BindTexture(nSampler, var, framevar);
	}
}

void CBaseShader::BindTexture(const bool bCheck, Sampler_t nSampler, const int var, const int framevar)
{
	if (bCheck)
	{
		BindTexture(nSampler, var, framevar);
	}
}

void CBaseShader::BindTexture(const bool bCheck, Sampler_t nSampler, const int var,
	const int framevar, StandardTextureId_t StandardTexture)
{
	if (bCheck)
	{
		BindTexture(nSampler, var, framevar);
	}
	else
	{
		m_pShaderAPI->BindStandardTexture(nSampler, StandardTexture);
	}
}

void CBaseShader::BindTexture(const bool bCheck, Sampler_t nSampler, StandardTextureId_t StandardTexture)
{
	if (bCheck)
	{
		m_pShaderAPI->BindStandardTexture(nSampler, StandardTexture);
	}
}

void CBaseShader::BindTexture(Sampler_t nSampler, StandardTextureId_t StandardTexture)
{
	m_pShaderAPI->BindStandardTexture(nSampler, StandardTexture);
}

void CBaseShader::UpdateMaterialContextData(CBasePerMaterialContextData** ppContextData)
{
	if (!ppContextData)
		return;

	LUXPerMaterialContextData* pContextData = reinterpret_cast<LUXPerMaterialContextData*>(*ppContextData);
	bool bNeedsRegenStaticCmds = !pContextData || m_pShaderShadow;
	if (!pContextData)
	{
		pContextData = CreateMaterialContextData();
		*ppContextData = pContextData;
	}

	m_pMaterialContextData = pContextData;

	if (m_pMaterialContextData)
	{
		m_pMaterialContextData->m_bSnapshottingCommands = bNeedsRegenStaticCmds;
	}
}

void CBaseShader::SetHPOFixConstant()
{
	#if 0 
	float4 cHPOFix = 0.0f;
	if(lux_general_fixdx9hpo.GetBool())
	{
		// NOTE: In DX9 the Viewport Size must be the same as the Rendertarget being rendered to
		// Using this here is an Assumption that the Viewport Size is the same as the actual d3d9 Viewport Size set for the Rendertarget
		// GetRenderTargetEX(0) will randomly return nullptrs ( Quality Code ) so we can't use that for RT Res.
		// GetBackBufferDimensions() will be wrong we can't use that since the Rendertarget we render to might be smaller or larger.
		ShaderViewport_t CurrentViewport;
		m_pShaderAPI->GetViewports(&CurrentViewport, 1);

		// According to the public Sources ( see common vs ) 1.0f is the value supposed to be used for the divide
		// Half Pixel offset turning into Full Pixel Offset for some Reason.
		// See also https://learn.microsoft.com/en-us/windows/win32/direct3d9/directly-mapping-texels-to-pixels
		// However this Code here DOES NOT work. Specifically for HUD Elements we had incorrect Viewport Sizes for some Reason.
		// Since HUD's and all kinds of People love to use UnlitGeneric I can't just make this optional to Screenspace Shaders..
		// Presumably the Viewport Values are not always updated correctly depending on the Rendertarget that are being rendered to
		// NOTE: I commented the hlsl Code in lux_common_vs_fxc.h until someone can determine how to fix this
		// If this gets fixed, update the Description of lux_general_fixdx9hpo too while you're at it. Thanks!
		cHPOFix.x = -1.0f / (float)CurrentViewport.m_nWidth;
		cHPOFix.y = 1.0f / (float)CurrentViewport.m_nHeight;
	}
	m_pShaderAPI->SetVertexShaderConstant(LUX_VS_FLOAT_HPOFIX, cHPOFix);
	#endif
}

bool IsHDREnabled()
{
	return g_pHardwareConfig->GetHDRType() != HDR_TYPE_NONE;
}

bool IsHDRTexture(const ITexture* pTexture)
{
	if (!pTexture)
		return false;

	return IsHDRImageFormat(pTexture->GetImageFormat());
}