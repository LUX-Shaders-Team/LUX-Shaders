//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef CSHADER_H
#define CSHADER_H

#ifdef _WIN32		   
#pragma once
#endif

// uncomment this if you want to build for nv3x
//#define NV3X 1

// This is what all shaders include.
// CBaseShader will become CShader in this file.
#include "materialsystem/ishaderapi.h"
#include "utlvector.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/imaterial.h"
#include "BaseShader.h"

#include "materialsystem/itexture.h"

// Included for convenience because they are used in a bunch of shaders
#include "materialsystem/imesh.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "materialsystem/materialsystem_config.h"
#include "shaderlib/ShaderDLL.h"

// make "local variable is initialized but not referenced" warnings errors for combo checking macros
#pragma warning ( error : 4189 )

//-----------------------------------------------------------------------------
// Global interfaces
//-----------------------------------------------------------------------------
extern IMaterialSystemHardwareConfig *g_pHardwareConfig;
extern const MaterialSystem_Config_t *g_pConfig;
static const bool g_shaderConfigDumpEnable = false;

// These Macros have to exist for Functions that are called by Multithreaded Systems
#define SET_FLAGS( _flag )		SetFlag(params, _flag)
#define SET_FLAGS2( _flag )		SetFlag2(params, _flag)
#define IS_FLAG_SET( _flag )	HasFlag(params, _flag)
#define IS_FLAG2_SET( _flag )	HasFlag2(params, _flag)
#define IS_FLAG_DEFINED( _flag ) IsFlagDefined(params, _flag)
#define IS_FLAG2_DEFINED( _flag ) IsFlag2Defined(params, _flag)
#define CLEAR_FLAGS( _flag )	ClearFlag(params, _flag)
#define CLEAR_FLAGS2( _flag )	ClearFlag2(params, _flag)

#define __BEGIN_SHADER_INTERNAL(_baseclass, name, help, flags) \
	namespace name \
	{\
		typedef _baseclass CBaseClass;\
		static const char *s_HelpString = help; \
		static const char *s_Name = #name; \
		static int s_nFlags = flags; \
		class CShaderParam;\
		static CUtlVector<CShaderParam *> s_ShaderParams;\
		static CShaderParam *s_pShaderParamOverrides[NUM_SHADER_MATERIAL_VARS];\
		class CShaderParam\
		{\
		public:\
			CShaderParam( ShaderMaterialVars_t var, ShaderParamType_t type, const char *pDefaultParam, const char *pHelp, int nFlags )\
			{\
				m_Info.m_pName = "override";\
				m_Info.m_Type = type;\
				m_Info.m_pDefaultValue = pDefaultParam;\
				m_Info.m_pHelp = pHelp;\
				m_Info.m_nFlags = nFlags;\
				AssertMsg( !s_pShaderParamOverrides[var], ( "Shader parameter override duplicately defined!" ) );\
				s_pShaderParamOverrides[var] = this;\
				m_Index = var;\
			}\
			CShaderParam( const char *pName, ShaderParamType_t type, const char *pDefaultParam, const char *pHelp, int nFlags )\
			{\
				m_Info.m_pName = pName;\
				m_Info.m_Type = type;\
				m_Info.m_pDefaultValue = pDefaultParam;\
				m_Info.m_pHelp = pHelp;\
				m_Info.m_nFlags = nFlags;\
				m_Index = NUM_SHADER_MATERIAL_VARS + s_ShaderParams.Count();\
				s_ShaderParams.AddToTail( this );\
			}\
			operator int()	\
			{\
				return m_Index;\
			}\
			const char *GetName()\
			{\
				return m_Info.m_pName;\
			}\
			ShaderParamType_t GetType()\
			{\
				return m_Info.m_Type;\
			}\
			const char *GetDefault()\
			{\
				return m_Info.m_pDefaultValue;\
			}\
			int GetFlags() const\
			{\
				return m_Info.m_nFlags;\
			}\
			const char *GetHelp()\
			{\
				return m_Info.m_pHelp;\
			}\
		private:\
			ShaderParamInfo_t m_Info; \
			int m_Index;\
		};\
		/* Can't create and set default Values for these in the Namespace. */\
		/* A class has a constructor that allows us to overwrite a default Value. */\
		class CShaderInfo\
		{\
			public:\
			const char *m_Geometry;\
			const char *m_Usage;\
			const char *m_Limitations;\
			const char *m_Performance;\
			const char *m_Fallback;\
			const char *m_WebLinks;\
			const char *m_D3DInfo;\
			CShaderInfo()\
			{\
				/* This will indicate to those reading the spew output that there is no Info for this Shader. */\
				m_Geometry = "No Info.";\
				m_Usage = m_Geometry;\
				m_Limitations = m_Geometry;\
				m_Performance = m_Geometry;\
				m_Fallback = m_Geometry;\
				m_WebLinks = m_Geometry;\
				m_D3DInfo = m_Geometry;

#define BEGIN_SHADER(name,help)	__BEGIN_SHADER_INTERNAL( CBaseShader, name, help, 0 )
#define BEGIN_SHADER_FLAGS(name,help,flags)	__BEGIN_SHADER_INTERNAL( CBaseShader, name, help, flags )

// "Helper macro for vertex shaders"
#define BEGIN_VS_SHADER_FLAGS(_name, _help, _flags)	__BEGIN_SHADER_INTERNAL( CBaseVSShader, _name, _help, _flags )
#define BEGIN_VS_SHADER(_name,_help)	__BEGIN_SHADER_INTERNAL( CBaseVSShader, _name, _help, 0 )

#define SHADER_INFO_GEOMETRY(geo)\
	m_Geometry = geo;

#define SHADER_INFO_USAGE(usage)\
	m_Usage = usage;

#define SHADER_INFO_LIMITATIONS(limitations)\
	m_Limitations = limitations;

#define SHADER_INFO_PERFORMANCE(performance)\
	m_Performance = performance;

#define SHADER_INFO_FALLBACK(fallbackinfo)\
	m_Fallback = fallbackinfo;

#define SHADER_INFO_WEBLINKS(weblinks)\
	m_WebLinks = weblinks;

#define SHADER_INFO_D3D(d3dinfo)\
	m_D3DInfo = d3dinfo;

#define SHADER_INFO_FALLBACKSHADER(fallback)\
	m_Fallback = #fallback;

// Closing brackets for the CShaderInfo Class and it's Constructor.
#define BEGIN_SHADER_PARAMS }};

#define SHADER_PARAM( param, paramtype, paramdefault, paramhelp ) \
	static CShaderParam param( "$" #param, paramtype, paramdefault, paramhelp, 0 );

#define SHADER_PARAM_FLAGS( param, paramtype, paramdefault, paramhelp, flags ) \
	static CShaderParam param( "$" #param, paramtype, paramdefault, paramhelp, flags );

#define SHADER_PARAM_OVERRIDE( param, paramtype, paramdefault, paramhelp, flags ) \
	static CShaderParam param( (ShaderMaterialVars_t) ::param, paramtype, paramdefault, paramhelp, flags );

	// regarding the macro above: the "::" was added to the first argument in order to disambiguate it for GCC.
	// for example, in cloak.cpp, this usage appears:
	// 		SHADER_PARAM_OVERRIDE( Color, SHADER_PARAM_TYPE_COLOR, "{255 255 255}", "unused", SHADER_PARAM_NOT_EDITABLE )
	// which in turn tries to ask the compiler to instantiate an object like so:
	// 		static CShaderParam Color( (ShaderMaterialVars_t)Color, SHADER_PARAM_TYPE_COLOR, "{255 255 255}", "unused", SHADER_PARAM_NOT_EDITABLE )
	// and GCC thinks that the reference to Color in the arg list is actually a reference to the object we're in the middle of making.
	// and you get --> error: invalid cast from type ‘Cloak_DX90::CShaderParam’ to type ‘ShaderMaterialVars_t’
	// Resolved: add the "::" so compiler knows that reference is to the enum, not to the name of the object being made.
	
#define END_SHADER_PARAMS \
	class CShader : public CBaseClass\
	{\
	public:\
		/* The ShaderInfo Class will automatically construct itself with our Data. */\
		CShaderInfo ShaderInfo;
			
#define SHADER_INIT_PARAMS()\
	virtual void OnInitShaderParams( IMaterialVar **params, const char *pMaterialName )

#define SHADER_FALLBACK\
	/* Can't set m_ppParams without detouring the Function. */\
	virtual char const* GetFallbackShader( IMaterialVar** params ) const\
	{\
		/* m_ppParams is not set since this isn't a CBaseShader Function. So set it now. */\
		m_ppParams = params;\
		char const* pResult = GetRealFallbackShader();\
		m_ppParams = NULL;\
		return pResult;\
	}\
	char const* GetRealFallbackShader() const

#define SHADER_INIT						\
	char const* GetName() const			\
	{									\
		return s_Name;					\
	}									\
	int GetFlags() const				\
	{									\
		return s_nFlags;				\
	}									\
	int GetNumParams() const			\
	{\
		return CBaseClass::GetNumParams() + s_ShaderParams.Count();\
	}\
	char const* GetParamName( int param ) const \
	{\
		int nBaseClassParamCount = CBaseClass::GetNumParams();	\
		if (param < nBaseClassParamCount)					\
			return CBaseClass::GetParamName(param);			\
		else												\
			return s_ShaderParams[param - nBaseClassParamCount]->GetName();	\
	}\
	char const* GetParamHelp( int param ) const \
	{\
		int nBaseClassParamCount = CBaseClass::GetNumParams();	\
		if (param < nBaseClassParamCount)						\
		{														\
			if ( !s_pShaderParamOverrides[param] )				\
				return CBaseClass::GetParamHelp( param );		\
			else												\
				return s_pShaderParamOverrides[param]->GetHelp(); \
		}														\
		else													\
			return s_ShaderParams[param - nBaseClassParamCount]->GetHelp();		\
	}\
	ShaderParamType_t GetParamType( int param ) const \
	{\
		int nBaseClassParamCount = CBaseClass::GetNumParams();	\
		if (param < nBaseClassParamCount)				\
			return CBaseClass::GetParamType( param ); \
		else \
			return s_ShaderParams[param - nBaseClassParamCount]->GetType();		\
	}\
	char const* GetParamDefault( int param ) const \
	{\
		int nBaseClassParamCount = CBaseClass::GetNumParams();	\
		if (param < nBaseClassParamCount)						\
		{														\
			if ( !s_pShaderParamOverrides[param] )				\
				return CBaseClass::GetParamDefault( param );	\
			else												\
				return s_pShaderParamOverrides[param]->GetDefault(); \
		}														\
		else													\
			return s_ShaderParams[param - nBaseClassParamCount]->GetDefault();	\
	}\
	int GetParamFlags( int param ) const \
	{\
		int nBaseClassParamCount = CBaseClass::GetNumParams();	\
		if (param < nBaseClassParamCount)						\
		{														\
			if ( !s_pShaderParamOverrides[param] )				\
				return CBaseClass::GetParamFlags( param );		\
			else												\
				return s_pShaderParamOverrides[param]->GetFlags(); \
		}														\
		else													\
			return s_ShaderParams[param - nBaseClassParamCount]->GetFlags(); \
	}\
	/* Virtual Functions introduced by LUX */\
	char const* GetDescription()\
	{\
		return s_HelpString;\
	}\
	char const* GetSupportedGeometry()\
	{\
		return ShaderInfo.m_Geometry;\
	}\
	char const* GetUsage()\
	{\
		return ShaderInfo.m_Usage;\
	}\
	char const* GetLimitations()\
	{\
		return ShaderInfo.m_Limitations;\
	}\
	char const* GetPerformance()\
	{\
		return ShaderInfo.m_Performance;\
	}\
	char const* GetFallback()\
	{\
		return ShaderInfo.m_Fallback;\
	}\
	char const* GetWebLinks()\
	{\
		return ShaderInfo.m_WebLinks;\
	}\
	char const* GetD3DInfo()\
	{\
		return ShaderInfo.m_D3DInfo;\
	}\
	/* Original Virtual void */\
	void OnInitShaderInstance( IMaterialVar **params, IShaderInit *pShaderInit, const char *pMaterialName )

#define SHADER_DRAW \
	void OnDrawElements( IMaterialVar **params, IShaderShadow* pShaderShadow, IShaderDynamicAPI* pShaderAPI, VertexCompressionType_t vertexCompression, CBasePerMaterialContextData **pContextDataPtr )

// Snapshot State
#define SHADOW_STATE if (pShaderShadow)

// For use with Context Data
#define STATIC_COMMANDS if(bNeedsRegenStaticCmds)

// For use with Context Data
#define SEMI_STATIC_COMMANDS if(pShaderAPI && pContextData->m_bMaterialVarsChanged)

#define DYNAMIC_STATE if (pShaderAPI)

#define END_SHADER }; \
	static CShader s_ShaderInstance;\
} // namespace

//-----------------------------------------------------------------------------
// Used to easily define a shader which *always* falls back
// ShiroDkxtro2: I added two Things here. the "FALLBACK" here allows
// ShaderDLL.cpp to detect whether or not something *is* a Fallback Shader
// The SHADER_INFO_FALLBACKSHADER allows it to go to the dead-end Shader
//-----------------------------------------------------------------------------
#define DEFINE_FALLBACK_SHADER( _shadername, _fallbackshadername )	\
	BEGIN_SHADER( _shadername, "FALLBACK" )\
	SHADER_INFO_FALLBACKSHADER(_fallbackshadername)\
	BEGIN_SHADER_PARAMS	\
	END_SHADER_PARAMS \
	SHADER_FALLBACK { return #_fallbackshadername; }	\
	SHADER_INIT {} \
	SHADER_DRAW {} \
	END_SHADER

//-----------------------------------------------------------------------------
// Used to easily define a shader which inherits from another shader
//-----------------------------------------------------------------------------

// A dumbed-down version which does what I need now which works
// This version doesn't allow you to do chain *anything* down to the base class
#define BEGIN_INHERITED_SHADER_FLAGS( _name, _base, _help, _flags ) \
	namespace _base\
	{\
		namespace _name\
		{\
			static const char *s_Name = #_name; \
			static const char *s_HelpString = _help;\
			static int s_nFlags = _flags;\
			class CShader : public _base::CShader\
			{\
			public:\
				char const* GetName() const			\
				{									\
					return s_Name;					\
				}									\
				int GetFlags() const				\
				{									\
					return s_nFlags;				\
				}

#define BEGIN_INHERITED_SHADER( _name, _base, _help ) BEGIN_INHERITED_SHADER_FLAGS( _name, _base, _help, 0 )
#define END_INHERITED_SHADER END_SHADER }

// psh ## shader is used here to generate a warning if you don't ever call SET_DYNAMIC_PIXEL_SHADER
#define DECLARE_DYNAMIC_PIXEL_SHADER( shader ) \
	int declaredynpixshader_ ## shader ## _missingcurlybraces = 0; \
	NOTE_UNUSED( declaredynpixshader_ ## shader ## _missingcurlybraces ); \
	shader ## _Dynamic_Index _pshIndex; \
	int psh ## shader = 0

// vsh ## shader is used here to generate a warning if you don't ever call SET_DYNAMIC_VERTEX_SHADER
#define DECLARE_DYNAMIC_VERTEX_SHADER( shader ) \
	int declaredynvertshader_ ## shader ## _missingcurlybraces = 0; \
	NOTE_UNUSED( declaredynvertshader_ ## shader ## _missingcurlybraces ); \
	shader ## _Dynamic_Index _vshIndex; \
	int vsh ## shader = 0


// psh ## shader is used here to generate a warning if you don't ever call SET_STATIC_PIXEL_SHADER
#define DECLARE_STATIC_PIXEL_SHADER( shader ) \
	int declarestaticpixshader_ ## shader ## _missingcurlybraces = 0; \
	NOTE_UNUSED( declarestaticpixshader_ ## shader ## _missingcurlybraces ); \
	shader ## _Static_Index _pshIndex; \
	int psh ## shader = 0

// vsh ## shader is used here to generate a warning if you don't ever call SET_STATIC_VERTEX_SHADER
#define DECLARE_STATIC_VERTEX_SHADER( shader ) \
	int declarestaticvertshader_ ## shader ## _missingcurlybraces = 0; \
	NOTE_UNUSED( declarestaticvertshader_ ## shader ## _missingcurlybraces ); \
	shader ## _Static_Index _vshIndex; \
	int vsh ## shader = 0


// psh_forgot_to_set_dynamic_ ## var is used to make sure that you set all
// all combos.  If you don't, you will get an undefined variable used error 
// in the SET_DYNAMIC_PIXEL_SHADER block.
#define SET_DYNAMIC_PIXEL_SHADER_COMBO( var, val ) \
	int dynpixshadercombo_ ## var ## _missingcurlybraces = 0; \
	NOTE_UNUSED( dynpixshadercombo_ ## var ## _missingcurlybraces ); \
	_pshIndex.Set ## var( ( val ) );  if(g_shaderConfigDumpEnable){printf("\n   PS dyn  var %s = %d (%s)", #var, (int) val, #val );}; \
	int psh_forgot_to_set_dynamic_ ## var = 0

// vsh_forgot_to_set_dynamic_ ## var is used to make sure that you set all
// all combos.  If you don't, you will get an undefined variable used error 
// in the SET_DYNAMIC_VERTEX_SHADER block.
#define SET_DYNAMIC_VERTEX_SHADER_COMBO( var, val ) \
	int dynvertshadercombo_ ## var ## _missingcurlybraces = 0; \
	NOTE_UNUSED( dynvertshadercombo_ ## var ## _missingcurlybraces ); \
	_vshIndex.Set ## var( ( val ) );  if(g_shaderConfigDumpEnable){printf("\n   VS dyn  var %s = %d (%s)", #var, (int) val, #val );}; \
	int vsh_forgot_to_set_dynamic_ ## var = 0


// psh_forgot_to_set_static_ ## var is used to make sure that you set all
// all combos.  If you don't, you will get an undefined variable used error 
// in the SET_STATIC_PIXEL_SHADER block.
#define SET_STATIC_PIXEL_SHADER_COMBO( var, val ) \
	int staticpixshadercombo_ ## var ## _missingcurlybraces = 0; \
	NOTE_UNUSED( staticpixshadercombo_ ## var ## _missingcurlybraces ); \
	_pshIndex.Set ## var( ( val ) ); if(g_shaderConfigDumpEnable){printf("\n   PS stat var %s = %d (%s)", #var, (int) val, #val );}; \
	int psh_forgot_to_set_static_ ## var = 0

// vsh_forgot_to_set_static_ ## var is used to make sure that you set all
// all combos.  If you don't, you will get an undefined variable used error 
// in the SET_STATIC_VERTEX_SHADER block.
#define SET_STATIC_VERTEX_SHADER_COMBO( var, val ) \
	int staticvertshadercombo_ ## var ## _missingcurlybraces = 0; \
	NOTE_UNUSED( staticvertshadercombo_ ## var ## _missingcurlybraces ); \
	_vshIndex.Set ## var( ( val ) ); if(g_shaderConfigDumpEnable){printf("\n   VS stat var %s = %d (%s)", #var, (int) val, #val );}; \
	int vsh_forgot_to_set_static_ ## var = 0


// psh_testAllCombos adds up all of the psh_forgot_to_set_dynamic_ ## var's from 
// SET_DYNAMIC_PIXEL_SHADER_COMBO so that an error is generated if they aren't set.
// psh_testAllCombos is set to itself to avoid an unused variable warning.
// psh ## shader being set to itself ensures that DECLARE_DYNAMIC_PIXEL_SHADER 
// was called for this particular shader.
#define SET_DYNAMIC_PIXEL_SHADER( shader ) \
	int dynamicpixshader_ ## shader ## _missingcurlybraces = 0; \
	NOTE_UNUSED( dynamicpixshader_ ## shader ## _missingcurlybraces ); \
	int psh_testAllCombos = shaderDynamicTest_ ## shader; \
	NOTE_UNUSED( psh_testAllCombos ); \
	NOTE_UNUSED( psh ## shader ); \
	pShaderAPI->SetPixelShaderIndex( _pshIndex.GetIndex() )

#define SET_DYNAMIC_PIXEL_SHADER_CMD( cmdstream, shader ) \
	int dynamicpixshader_ ## shader ## _missingcurlybraces = 0; \
	NOTE_UNUSED( dynamicpixshader_ ## shader ## _missingcurlybraces ); \
	int psh_testAllCombos = shaderDynamicTest_ ## shader; \
	NOTE_UNUSED( psh_testAllCombos ); \
	NOTE_UNUSED( psh ## shader ); \
	cmdstream.SetPixelShaderIndex( _pshIndex.GetIndex() )


// vsh_testAllCombos adds up all of the vsh_forgot_to_set_dynamic_ ## var's from 
// SET_DYNAMIC_VERTEX_SHADER_COMBO so that an error is generated if they aren't set.
// vsh_testAllCombos is set to itself to avoid an unused variable warning.
// vsh ## shader being set to itself ensures that DECLARE_DYNAMIC_VERTEX_SHADER 
// was called for this particular shader.
#define SET_DYNAMIC_VERTEX_SHADER( shader ) \
	int dynamicvertshader_ ## shader ## _missingcurlybraces = 0; \
	NOTE_UNUSED( dynamicvertshader_ ## shader ## _missingcurlybraces ); \
	int vsh_testAllCombos = shaderDynamicTest_ ## shader; \
	NOTE_UNUSED( vsh_testAllCombos ); \
	NOTE_UNUSED( vsh ## shader ); \
	pShaderAPI->SetVertexShaderIndex( _vshIndex.GetIndex() )

#define SET_DYNAMIC_VERTEX_SHADER_CMD( cmdstream, shader ) \
	int dynamicvertshader_ ## shader ## _missingcurlybraces = 0; \
	NOTE_UNUSED( dynamicvertshader_ ## shader ## _missingcurlybraces ); \
	int vsh_testAllCombos = shaderDynamicTest_ ## shader; \
	NOTE_UNUSED( vsh_testAllCombos ); \
	NOTE_UNUSED( vsh ## shader ); \
	cmdstream.SetVertexShaderIndex( _vshIndex.GetIndex() )


// psh_testAllCombos adds up all of the psh_forgot_to_set_static_ ## var's from 
// SET_STATIC_PIXEL_SHADER_COMBO so that an error is generated if they aren't set.
// psh_testAllCombos is set to itself to avoid an unused variable warning.
// psh ## shader being set to itself ensures that DECLARE_STATIC_PIXEL_SHADER 
// was called for this particular shader.
#define SET_STATIC_PIXEL_SHADER( shader ) \
	int staticpixshader_ ## shader ## _missingcurlybraces = 0; \
	NOTE_UNUSED( staticpixshader_ ## shader ## _missingcurlybraces ); \
	int psh_testAllCombos = shaderStaticTest_ ## shader; \
	NOTE_UNUSED( psh_testAllCombos ); \
	NOTE_UNUSED( psh ## shader ); \
	pShaderShadow->SetPixelShader( #shader, _pshIndex.GetIndex() )

// vsh_testAllCombos adds up all of the vsh_forgot_to_set_static_ ## var's from 
// SET_STATIC_VERTEX_SHADER_COMBO so that an error is generated if they aren't set.
// vsh_testAllCombos is set to itself to avoid an unused variable warning.
// vsh ## shader being set to itself ensures that DECLARE_STATIC_VERTEX_SHADER 
// was called for this particular shader.
#define SET_STATIC_VERTEX_SHADER( shader ) \
	int staticvertshader_ ## shader ## _missingcurlybraces = 0; \
	NOTE_UNUSED( staticvertshader_ ## shader ## _missingcurlybraces ); \
	int vsh_testAllCombos = shaderStaticTest_ ## shader; \
	NOTE_UNUSED( vsh_testAllCombos ); \
	NOTE_UNUSED( vsh ## shader ); \
	pShaderShadow->SetVertexShader( #shader, _vshIndex.GetIndex() )

#endif // CSHADER_H
