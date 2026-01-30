//===== Copyright 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
//===========================================================================//

#include "ShaderDLL.h"
#include "materialsystem/IShader.h"
#include "tier1/utlvector.h"
#include "tier0/dbg.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "materialsystem/materialsystem_config.h"
#include "ishadersystem.h"
#include "materialsystem/ishaderapi.h"
#include "shaderlib_cvar.h"
#include "mathlib/mathlib.h"
#include "tier1/tier1.h"
#include "Color.h"
#include "filesystem.h"
#include <tier2/tier2.h>
#include <vector>
#include <array>

// Needed for Parameter Spews
#include "BaseShader.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// The standard implementation of CShaderDLL
//-----------------------------------------------------------------------------
class CShaderDLL : public IShaderDLLInternal, public IShaderDLL
{
public:
	CShaderDLL();

	// methods of IShaderDLL
	virtual bool Connect( CreateInterfaceFn factory );
	virtual void Disconnect();
	virtual int ShaderCount() const;
	virtual IShader *GetShader( int nShader );

	// methods of IShaderDLLInternal
	virtual bool Connect( CreateInterfaceFn factory, bool bIsMaterialSystem );
	virtual void Disconnect( bool bIsMaterialSystem );
	virtual void InsertShader( IShader *pShader );

private:
	CUtlVector< IShader * >	m_ShaderList;
};


//-----------------------------------------------------------------------------
// Global interfaces/structures
//-----------------------------------------------------------------------------
IMaterialSystemHardwareConfig* g_pHardwareConfig;
const MaterialSystem_Config_t *g_pConfig;


//-----------------------------------------------------------------------------
// Interfaces/structures local to shaderlib
//-----------------------------------------------------------------------------
IShaderSystem* g_pSLShaderSystem;


// Pattern necessary because shaders register themselves in global constructors
static CShaderDLL *s_pShaderDLL;


//-----------------------------------------------------------------------------
// Global accessor
//-----------------------------------------------------------------------------
IShaderDLL *GetShaderDLL()
{
	// Pattern necessary because shaders register themselves in global constructors
	if ( !s_pShaderDLL )
	{
		s_pShaderDLL = new CShaderDLL;
	}

	return s_pShaderDLL;
}

IShaderDLLInternal *GetShaderDLLInternal()
{
	// Pattern necessary because shaders register themselves in global constructors
	if ( !s_pShaderDLL )
	{
		s_pShaderDLL = new CShaderDLL;
	}

	return static_cast<IShaderDLLInternal*>( s_pShaderDLL );
}

//-----------------------------------------------------------------------------
// Singleton interface
//-----------------------------------------------------------------------------
EXPOSE_INTERFACE_FN( (InstantiateInterfaceFn)GetShaderDLLInternal, IShaderDLLInternal, SHADER_DLL_INTERFACE_VERSION );

//-----------------------------------------------------------------------------
// Connect, disconnect...
//-----------------------------------------------------------------------------
CShaderDLL::CShaderDLL()
{
	MathLib_Init( 2.2f, 2.2f, 0.0f, 2.0f );
}


//-----------------------------------------------------------------------------
// Connect, disconnect...
//-----------------------------------------------------------------------------
bool CShaderDLL::Connect( CreateInterfaceFn factory, bool bIsMaterialSystem )
{
#ifdef NOLUX
	if ( !CommandLine()->HasParm( "-nolux" ) )
#else
	if ( CommandLine()->HasParm( "-nolux" ) )
#endif
    {
        return false;
    }

	g_pHardwareConfig =  (IMaterialSystemHardwareConfig*)factory( MATERIALSYSTEM_HARDWARECONFIG_INTERFACE_VERSION, NULL );
	g_pConfig = (const MaterialSystem_Config_t*)factory( MATERIALSYSTEM_CONFIG_VERSION, NULL );
	g_pSLShaderSystem =  (IShaderSystem*)factory( SHADERSYSTEM_INTERFACE_VERSION, NULL );

	if ( !bIsMaterialSystem )
	{
		ConnectTier1Libraries( &factory, 1 );
  		InitShaderLibCVars( factory );
	}

	g_bHammerPlusPlus = strstr(CommandLine()->GetParm(0), "hammerplusplus.exe");

	return ( g_pConfig != NULL ) && (g_pHardwareConfig != NULL) && ( g_pSLShaderSystem != NULL );
}

void CShaderDLL::Disconnect( bool bIsMaterialSystem )
{
	if ( !bIsMaterialSystem )
	{
		ConVar_Unregister();
		DisconnectTier1Libraries();
	}

	g_pHardwareConfig = NULL;
	g_pConfig = NULL;
	g_pSLShaderSystem = NULL;
}

bool CShaderDLL::Connect( CreateInterfaceFn factory )
{
	return Connect( factory, false );
}

void CShaderDLL::Disconnect()
{
	Disconnect( false );
}


//-----------------------------------------------------------------------------
// Iterates over all shaders
//-----------------------------------------------------------------------------
int CShaderDLL::ShaderCount() const
{
	return m_ShaderList.Count();
}

IShader *CShaderDLL::GetShader( int nShader ) 
{
	if ( ( nShader < 0 ) || ( nShader >= m_ShaderList.Count() ) )
		return NULL;

	return m_ShaderList[nShader];
}


//-----------------------------------------------------------------------------
// Adds to the shader lists
//-----------------------------------------------------------------------------
void CShaderDLL::InsertShader( IShader *pShader )
{
	Assert( pShader );
	m_ShaderList.AddToTail( pShader );
}


//-----------------------------------------------------------------------------
//  When we find a "\n" we replace it with "  \n" for markdown formated strings 
//-----------------------------------------------------------------------------
static const char* ConvertEndLineToMardownEndLine(const char* pString)
{
	static char s_szBuffer[8192];
	std::size_t uiSrcLen = V_strlen(pString);
	std::size_t uiDstIndex = 0;

	for (size_t i = 0; i < uiSrcLen && uiDstIndex < sizeof(s_szBuffer) - 1; ++i)
	{
		if (pString[i] == '\n')
		{
			if (uiDstIndex + 3 >= sizeof(s_szBuffer))
				break;
			s_szBuffer[uiDstIndex++] = ' ';
			s_szBuffer[uiDstIndex++] = ' ';
			s_szBuffer[uiDstIndex++] = '\n';
		}
		else
		{
			s_szBuffer[uiDstIndex++] = pString[i];
		}
	}

	s_szBuffer[uiDstIndex] = '\0';
	return s_szBuffer;
}

#ifndef NOLUX
//-----------------------------------------------------------------------------
//  Unsuario2: Ideally we would like this Color data to in src/public, but we cannot
//-----------------------------------------------------------------------------
#define ColorGreen Color(20, 255, 20, 255)
#define ColorGreyBlue Color(100, 150, 175, 255)
#define ColorGrey Color(200, 200, 200, 255)
#define ColorGold Color(255, 191, 0, 255)

#define ColorParamName ColorGreen
#define ColorParamType ColorGreyBlue
#define ColorParamHelp ColorGrey

#define LUX_SHADER_PREFIX		"LUX_"
#define TF2C_SHADER_PREFIX		"TF2C_"
#define LUX_GITHUB_FILE_DUMP	"lux_github_wiki.md"
#define LUX_GITHUB_WIKI_DIR		"_shaderdump\\docs"

enum class HelperStringType
{
	Standard = 0,
	Internal = 1,
	Deprecated = 2
};

struct ShaderListCategory_t
{
	const char* pszName;
	const char* pszPrefix;
};

const ShaderListCategory_t s_ShaderCategories[] =
{
	{ "Other",		NULL },
	{ "LUX",		LUX_SHADER_PREFIX },
	{ "TF2C",		TF2C_SHADER_PREFIX },
};

struct ShaderListEntry_t
{
	const char* pszName;
	int nCategory;
};

CON_COMMAND_F(lux_help_shaders, "Returns all Shader Names in the Custom Shader DLL. Usage: lux_help_shaders <category>", FCVAR_NONE)
{
	CUtlVector<ShaderListEntry_t> vecShaders;
	const int uiShaderCount = s_pShaderDLL->ShaderCount();
	for (int i = 0; i < uiShaderCount; i++)
	{
		if (IShader* pCur = s_pShaderDLL->GetShader(i))
		{
			const char* pszName = pCur->GetName();
			int nCategory = 0;

			for (int i = 1; i < V_ARRAYSIZE(s_ShaderCategories); i++)
			{
				if (V_strstr(pszName, s_ShaderCategories[i].pszPrefix))
				{
					nCategory = i;
					break;
				}
			}

			ShaderListEntry_t* pListEntry = vecShaders.AddToTailGetPtr();
			pListEntry->pszName = pszName;
			pListEntry->nCategory = nCategory;
		}
	}

	const char* pszPreferredCategory = args.Arg(1);
	int iPreferredCategory = -1;
	for (int i = 0; i < V_ARRAYSIZE(s_ShaderCategories); i++)
	{
		if (V_stristr(pszPreferredCategory, s_ShaderCategories[i].pszName))
		{
			iPreferredCategory = i;
			break;
		}
	}

	for (int i = 0; i < V_ARRAYSIZE(s_ShaderCategories); i++)
	{
		if (iPreferredCategory < 0 && i == 0)
		{
			continue;
		}

		if (iPreferredCategory >= 0 && iPreferredCategory != i)
		{
			continue;
		}

		Msg("\n---- %s Shaders ----\n", s_ShaderCategories[i].pszName);

		FOR_EACH_VEC(vecShaders, j)
		{
			ShaderListEntry_t* pListEntry = &vecShaders[j];
			if (pListEntry->nCategory == i)
			{
				Msg("\t%s\n", pListEntry->pszName);
			}
		}
	}

	Msg("\n");
}

CON_COMMAND_F(lux_help, "Enter Shader Name and retrieve Information about it.", FCVAR_NONE)
{
	if (args.ArgC() == 1)
	{
		Msg("%s\n", "Enter the Name of a Shader to spew Information about it");
		return;
	}

	// The Shaderlist only contains Shaders from our DLL
	// In which only Shaders with LUX_ Prefix are real Shaders
	// To indicate a Fallback Shader,
	// I made the Description for Fallback Shaders be "FALLBACK"
	// If the Description is "FALLBACK", GetFallback() will return the real ShaderName
	// NOTE: We can't use GetFallbackShader() since it requires Params and we don't have those
	const char* cFinalName = nullptr;
	bool bFoundShader = false;
	for (int nShader = 0; nShader < s_pShaderDLL->ShaderCount(); nShader++)
	{
		// GetShader returns an IShader*
		// But it is actually storing a CBaseShader* which Inherits from IShader
		// So we can use Custom Funcs here
		CBaseShader* Cur = (CBaseShader*)s_pShaderDLL->GetShader(nShader);
		if (Cur && !V_stricmp(Cur->GetName(), args[1]))
		{
			bFoundShader = true;
			if (!V_stricmp(Cur->GetDescription(), "FALLBACK"))
			{
				cFinalName = Cur->GetFallback();
			}
			else
			{
				cFinalName = args[1];
			}
			break;
		}
	}

	// No Shader no Fun
	if (!bFoundShader)
	{
		Msg("Shader %s not found.\n", args[1]);
		return;
	}

	// Search for the real Shader
	for (int nShader = 0; nShader < s_pShaderDLL->ShaderCount(); nShader++)
	{
		CBaseShader* Cur = (CBaseShader*)s_pShaderDLL->GetShader(nShader);
		if (Cur)
		{
			if (!V_stricmp(Cur->GetName(), cFinalName))
			{
				ConColorMsg(ColorGold, "//=====================================\n");
				ConColorMsg(ColorGold, "// Shader: %s\n", Cur->GetName());
				ConColorMsg(ColorGold, "//=====================================\n");

				ConColorMsg(ColorGold, "[Description]\n");
				ConColorMsg(ColorGrey, "%s\n\n", Cur->GetDescription());

				ConColorMsg(ColorGold, "[Supported Geometry]\n");
				ConColorMsg(ColorGrey, "%s\n\n", Cur->GetSupportedGeometry());

				ConColorMsg(ColorGold, "[Usage]\n");
				ConColorMsg(ColorGrey, "%s\n\n", Cur->GetUsage());

				ConColorMsg(ColorGold, "[Limitations]\n");
				ConColorMsg(ColorGrey, "%s\n\n", Cur->GetLimitations());

				ConColorMsg(ColorGold, "[Performance]\n");
				ConColorMsg(ColorGrey, "%s\n\n", Cur->GetPerformance());

				ConColorMsg(ColorGold, "[Fallbacks]\n");
				ConColorMsg(ColorGrey, "%s\n\n", Cur->GetFallback());

				ConColorMsg(ColorGold, "[See Also]\n");
				ConColorMsg(ColorGrey, "%s\n\n", Cur->GetWebLinks());

				ConColorMsg(ColorGold, "[D3D Information]\n");
				ConColorMsg(ColorGrey, "%s\n\n", Cur->GetD3DInfo());
        
				ConColorMsg(ColorGold, "//=====================================\n");
				ConColorMsg(ColorGold, "// Parameters\n");
				ConColorMsg(ColorGold, "//=====================================\n");

				// IShader has a different GetNumParams() that only returns BaseParam Count
				// So reinterpret as a CBaseShader and use that GetNumParams()
				for (int nParam = 0; nParam < Cur->GetNumParams(); nParam++)
				{
					ConColorMsg(ColorParamName, "%s ", Cur->GetParamName(nParam));
					switch (Cur->GetParamType(nParam))
					{
						case SHADER_PARAM_TYPE_TEXTURE:
							ConColorMsg(ColorParamType, "[Texture]\n");
							break;
						case SHADER_PARAM_TYPE_INTEGER:
							ConColorMsg(ColorParamType, "[Integer]\n");
							break;
						case SHADER_PARAM_TYPE_COLOR:
							ConColorMsg(ColorParamType, "[Vector | Color]\n");
							break;
						case SHADER_PARAM_TYPE_VEC2:
							ConColorMsg(ColorParamType, "[Vector | Vec2 | Float]\n");
							break;
						case SHADER_PARAM_TYPE_VEC3:
							ConColorMsg(ColorParamType, "[Vector | Vec3 | Float]\n");
							break;
						case SHADER_PARAM_TYPE_VEC4:
							ConColorMsg(ColorParamType, "[Vector | Vec4 | Float]\n");
							break;
						case SHADER_PARAM_TYPE_ENVMAP:
							ConColorMsg(ColorParamType, "[DEPRECATED EnvMap TYPE]\n");
							break;
						case SHADER_PARAM_TYPE_FLOAT:
							ConColorMsg(ColorParamType, "[Float]\n");
							break;
						case SHADER_PARAM_TYPE_BOOL:
							ConColorMsg(ColorParamType, "[Boolean]\n");
							break;
						case SHADER_PARAM_TYPE_FOURCC:
							ConColorMsg(ColorParamType, "[FourCC]\n");
							break;
						case SHADER_PARAM_TYPE_MATRIX:
							ConColorMsg(ColorParamType, "[Matrix]\n");
							break;
						case SHADER_PARAM_TYPE_MATERIAL:
							ConColorMsg(ColorParamType, "[Material]\n");
							break;
						case SHADER_PARAM_TYPE_STRING:
							ConColorMsg(ColorParamType, "[String]\n");
							break;
						case SHADER_PARAM_TYPE_MATRIX4X2:
							ConColorMsg(ColorParamType, "[Matrix4x2]\n");
							break;
						default: break;
					}

					ConColorMsg(ColorParamHelp, "%s\n\n", Cur->GetParamHelp(nParam));
				}
			}
		}
	}
}

CON_COMMAND_F(lux_dump_shaderparamhelp, "Enter Shader Name and Name of the Parameter to retrieve its Description", FCVAR_NONE)
{
	for (int nShader = 0; nShader < s_pShaderDLL->ShaderCount(); nShader++)
	{
		IShader* Cur = s_pShaderDLL->GetShader(nShader);
		if (Cur)
		{
			if (!V_stricmp(Cur->GetName(), args[1]))
			{
				// Loop through Parameters
				bool bFoundParam = false;
				for (int nParam = 0; nParam < Cur->GetNumParams(); nParam++)
				{
					// Only if the Name Matches
					if (!V_stricmp(Cur->GetParamName(nParam), args[2]))
					{
						bFoundParam = true;
						Msg("Parameter: %s\nDescription: %s\n", args[2], Cur->GetParamHelp(nParam));
						break;
					}
				}

				if (!bFoundParam)
					Msg("Did not find Parameter %s for Shader %s\n", args[2], args[1]);
			}
		}
	}
}

//-----------------------------------------------------------------------------
//  Unusuario2: I really dont recommend touching anything here unless you REALLY
//  know what are you doing, BEFORE making ANY changes, 
//	please read: https://www.markdownguide.org/basic-syntax/
//-----------------------------------------------------------------------------
static const char *WikiGetParamTypeHelperString( HelperStringType eParamType )
{
	switch ( eParamType )
	{
	default:
		// None
	case HelperStringType::Standard:
		return "**Parameters:**  \n";
	case HelperStringType::Internal:
		return "**Internal Parameters:**  \n";
	case HelperStringType::Deprecated:
		return "**Deprecated Parameters:**  \n";
	}
}

static bool WikiIsInfoStringMatchingParamType( HelperStringType eParamType, const char *pParamHelpShader )
{
	switch ( eParamType )
	{
	default:
		// None
	case HelperStringType::Standard:
		return !V_strstr( pParamHelpShader, "(INTERNAL PARAMETER)" ) && !V_strstr( pParamHelpShader, "(DEPRECATED)" );
	case HelperStringType::Internal:
		return V_strstr( pParamHelpShader, "(INTERNAL PARAMETER)" );
	case HelperStringType::Deprecated:
		return V_strstr( pParamHelpShader, "(DEPRECATED)" );
	}
}

CON_COMMAND_F(lux_generate_github_wiki, "Generates a wiki in markdown format. (mainly used in the LUX github repo).", FCVAR_NONE)
{
	if(!g_pFullFileSystem)
		g_pFullFileSystem = LoadInterface<IFileSystem>(FILESYSTEM_DLL_NAME, FILESYSTEM_INTERFACE_VERSION);

	if (!g_pFullFileSystem)
	{
		Warning("Could not create g_pFullFileSystem!\n");
		return;
	}

	const int uiShaderCount = s_pShaderDLL->ShaderCount();

	// Make a list of the LUX_ shaders
	std::vector<std::array<char, 128>> LuxShaderList;
	LuxShaderList.reserve(uiShaderCount);
	{
		Msg("Generating LUX shader list... ");
		for (int i = 0; i < uiShaderCount; ++i)
		{
			IShader* Cur = s_pShaderDLL->GetShader(i);
			if (Cur)
			{
				const char* pTemp = Cur->GetName();
				if (V_strstr(pTemp, LUX_SHADER_PREFIX))
				{
					std::array<char, 128> shaderName{};
					V_strncpy(shaderName.data(), pTemp, static_cast<int>(shaderName.size() - 1));
					shaderName.at(shaderName.size() - 1) = '\0';
					LuxShaderList.push_back(shaderName);
				}
			}
		}
		Msg("done\n");
	}

	char szLuxDumpDir[MAX_PATH];
	{
		char szGamePath[MAX_PATH];
		g_pFullFileSystem->GetSearchPath_safe("GAME", false, szGamePath);
		V_sprintf_safe(szLuxDumpDir, "%s%s", strtok(szGamePath, ";"), LUX_GITHUB_WIKI_DIR);
		g_pFullFileSystem->CreateDirHierarchy(szLuxDumpDir);
	}

	// Unusuario2, Base of the wiki.
	{
		char szLuxBaseWiki[MAX_PATH];
		V_sprintf_safe(szLuxBaseWiki, "%s\\%s", szLuxDumpDir, LUX_GITHUB_FILE_DUMP);

		FILE* pLuxGithubWikiDump = fopen(szLuxBaseWiki, "w");
		if (!pLuxGithubWikiDump)
		{
			Warning("Command lux_generate_github_wiki failed! The dump file could not be generated at: %s\n", szLuxBaseWiki);
			return;
		}

		// Header of the WIKI.
		fprintf(pLuxGithubWikiDump, "# LUX Shader Wiki.  \n");
		fprintf(pLuxGithubWikiDump, "A quick reference for LUX Source Engine shaders descriptions and their parameters. To generate the wiki type in the console \"**lux_generate_github_wiki**\".  \n");
		fprintf(pLuxGithubWikiDump, "\n---  \n");
		fprintf(pLuxGithubWikiDump, "## Shader Input data types:  \n");
		fprintf(pLuxGithubWikiDump, "| Data Type | Description | Example |  \n");
		fprintf(pLuxGithubWikiDump, "|------------|--------------|----------|  \n");
		fprintf(pLuxGithubWikiDump, "| **[Texture]** | Expects a relative path to the compiled texture. | \"shadertest/lux/default_color\" |  \n");
		fprintf(pLuxGithubWikiDump, "| **[Boolean]** | Expects a true (1) or false (0). | \"0\" |  \n");
		fprintf(pLuxGithubWikiDump, "| **[Integer]** | Expects a negative, positive, and whole number. | \"1\" |  \n");
		fprintf(pLuxGithubWikiDump, "| **[Float]** | Expects a negative or positive number with decimals. | \"1.1\" | \n");
		fprintf(pLuxGithubWikiDump, "| **[String]** | Expects a sequence of characters. | \"_rt_WaterReflection\" | \n");
		fprintf(pLuxGithubWikiDump, "| **[Material]** | Expects a string containing the path to the `.vmt` file. | \"shadertest/lux/default_color.vmt\" | \n");
		fprintf(pLuxGithubWikiDump, "| **[Vector (Color type)]** | Expects a vector where the first value is **R**, second **G**, and third **B**.(e.g., `[0-1, 0-1, 0-1]` *(Float)* or `{0-255, 0-255, 0-255}` *(Integer)*) | `[0-1, 0-1, 0-1]` | \n");
		fprintf(pLuxGithubWikiDump, "| **[Vector (Vec2 type, float)]** | Expects a vector where there are only two values. | \"[x, y]\" | \n");
		fprintf(pLuxGithubWikiDump, "| **[Vector (Vec3 type, float)]** | Expects a vector where there are only three values. | \"[x, y, z]\" | \n");
		fprintf(pLuxGithubWikiDump, "| **[Vector (Vec4 type, float)]** | Expects a vector where there are only four values. | \"[x, y, z, w]\" | \n");
		fprintf(pLuxGithubWikiDump, "| **[Matrix]** | Expects a complete string in the following format:<br> "
			" - **Format string**: \"center **[0-1] [0-1]** scale **[int] [int]** rotate **[0-360]** translate **[int] [int]**\"<br>"
			" - **center**: Defines the point of rotation (only useful if rotate is being used).<br>"
			" - **scale**: Fits the texture into the material the given number of times. For example, `2 1` is a 50 percent scale on X, keeping Y unchanged.<br>"
			" - **rotate**: Rotates the texture counter-clockwise in degrees. Accepts any number, including negatives.<br>"
			" - **translate**: Shifts the texture by the given numbers. `.5` shifts halfway; `1` shifts once completely over (same as no shift).<br>"
			" | \"center .5 .5 scale 1 1 rotate 0 translate 0 0\" |  \n");
		fprintf(pLuxGithubWikiDump, "| **[Matrix4x2]** | Legacy data input. | - |  \n");
		fprintf(pLuxGithubWikiDump, "| **[FourCC]** | Legacy data input. | - |  \n");
		fprintf(pLuxGithubWikiDump, "\n---  \n");
		fflush(pLuxGithubWikiDump);

		// Generate the shader list index.
		fprintf(pLuxGithubWikiDump, "## LUX Shader List:  \n");
		for (const auto& ShaderName : LuxShaderList)
		{
			fprintf(pLuxGithubWikiDump, "- [%s](%s)  \n", ShaderName.data(), ShaderName.data());
		}
		fprintf(pLuxGithubWikiDump, "\n---  \n");
		fclose(pLuxGithubWikiDump);
	}

	Msg("Generating the shader description... ");
	for (int nShader = 0; nShader < uiShaderCount; ++nShader)
	{
		CBaseShader* Cur = (CBaseShader*)s_pShaderDLL->GetShader(nShader);
		if (Cur)
		{
			for(const auto &ShaderName : LuxShaderList)
			{
				if (!V_stricmp(Cur->GetName(), ShaderName.data()))
				{
					char szShaderWikiDumpPath[MAX_PATH];
					V_sprintf_safe(szShaderWikiDumpPath, "%s\\%s.md", szLuxDumpDir, ShaderName.data());

					// Unusuario2 TODO: use CUltBuffer/tier1 file stuff instead of C funtions! 
					FILE* pLuxGithubWikiDump = fopen(szShaderWikiDumpPath, "w");
					if (!pLuxGithubWikiDump)
					{
						Warning("The shader (%s) dump file could not be generated at: %s\n", ShaderName.data(), szShaderWikiDumpPath);
						continue;
					}

					fprintf(pLuxGithubWikiDump, "## Shader -> %s  \n", Cur->GetName());

					fprintf(pLuxGithubWikiDump, "**Description:**  \n");
					fprintf(pLuxGithubWikiDump, "%s  \n\n", ConvertEndLineToMardownEndLine(Cur->GetDescription()));

					fprintf(pLuxGithubWikiDump, "**Supported Geometry:**  \n");
					fprintf(pLuxGithubWikiDump, "%s  \n\n", ConvertEndLineToMardownEndLine(Cur->GetSupportedGeometry()));

					fprintf(pLuxGithubWikiDump, "**Usage:**   \n");
					fprintf(pLuxGithubWikiDump, "%s  \n\n", ConvertEndLineToMardownEndLine(Cur->GetUsage()));

					fprintf(pLuxGithubWikiDump, "**Limitations:**  \n");
					fprintf(pLuxGithubWikiDump, "%s  \n\n", ConvertEndLineToMardownEndLine(Cur->GetLimitations()));

					fprintf(pLuxGithubWikiDump, "**Performance:**  \n");
					fprintf(pLuxGithubWikiDump, "%s  \n\n", ConvertEndLineToMardownEndLine(Cur->GetPerformance()));

					fprintf(pLuxGithubWikiDump, "**Fallbacks:**  \n");
					fprintf(pLuxGithubWikiDump, "%s  \n\n", ConvertEndLineToMardownEndLine(Cur->GetFallback()));

					fprintf(pLuxGithubWikiDump, "**See Also:**  \n");
					fprintf(pLuxGithubWikiDump, "%s  \n\n", ConvertEndLineToMardownEndLine(Cur->GetWebLinks()));

					fprintf(pLuxGithubWikiDump, "**D3D Information:**  \n");
					fprintf(pLuxGithubWikiDump, "%s  \n\n", ConvertEndLineToMardownEndLine(Cur->GetD3DInfo()));

					// Unusuario2, dump all the parameters, a bit overkill the for loop =)
					for(HelperStringType eParamType = HelperStringType::Standard; eParamType <= HelperStringType::Deprecated; eParamType = static_cast<HelperStringType>((int)eParamType + 1))
					{
						// Unusuario2, What type of helper string are we writting?
						fprintf(pLuxGithubWikiDump, WikiGetParamTypeHelperString( eParamType ) );

						for (int nParam = 0; nParam < Cur->GetNumParams(); ++nParam)
						{
							const char* pParamHelpShader = ConvertEndLineToMardownEndLine(Cur->GetParamHelp(nParam));
							if ( WikiIsInfoStringMatchingParamType( eParamType, pParamHelpShader ) )
							{
								fprintf(pLuxGithubWikiDump, "- **`%s`** ", Cur->GetParamName(nParam));
								switch (Cur->GetParamType(nParam))
								{
								case SHADER_PARAM_TYPE_TEXTURE:
									fprintf(pLuxGithubWikiDump, "&nbsp;&nbsp;&nbsp;&nbsp;[Texture]  \n");
									break;
								case SHADER_PARAM_TYPE_INTEGER:
									fprintf(pLuxGithubWikiDump, "&nbsp;&nbsp;&nbsp;&nbsp;[Integer]  \n");
									break;
								case SHADER_PARAM_TYPE_COLOR:
									fprintf(pLuxGithubWikiDump, "&nbsp;&nbsp;&nbsp;&nbsp;[Vector | Color]  \n");
									break;
								case SHADER_PARAM_TYPE_VEC2:
									fprintf(pLuxGithubWikiDump, "&nbsp;&nbsp;&nbsp;&nbsp;[Vector | Vec2 | Float]  \n");
									break;
								case SHADER_PARAM_TYPE_VEC3:
									fprintf(pLuxGithubWikiDump, "&nbsp;&nbsp;&nbsp;&nbsp;[Vector | Vec3 | Float]  \n");
									break;
								case SHADER_PARAM_TYPE_VEC4:
									fprintf(pLuxGithubWikiDump, "&nbsp;&nbsp;&nbsp;&nbsp;[Vector | Vec4 | Float]  \n");
									break;
								case SHADER_PARAM_TYPE_ENVMAP:
									fprintf(pLuxGithubWikiDump, "&nbsp;&nbsp;&nbsp;&nbsp;[DEPRECATED EnvMap TYPE]  \n");
									break;
								case SHADER_PARAM_TYPE_FLOAT:
									fprintf(pLuxGithubWikiDump, "&nbsp;&nbsp;&nbsp;&nbsp;[Float]  \n");
									break;
								case SHADER_PARAM_TYPE_BOOL:
									fprintf(pLuxGithubWikiDump, "&nbsp;&nbsp;&nbsp;&nbsp;[Boolean]  \n");
									break;
								case SHADER_PARAM_TYPE_FOURCC:
									fprintf(pLuxGithubWikiDump, "&nbsp;&nbsp;&nbsp;&nbsp;[FourCC]  \n");
									break;
								case SHADER_PARAM_TYPE_MATRIX:
									fprintf(pLuxGithubWikiDump, "&nbsp;&nbsp;&nbsp;&nbsp;[Matrix]  \n");
									break;
								case SHADER_PARAM_TYPE_MATERIAL:
									fprintf(pLuxGithubWikiDump, "&nbsp;&nbsp;&nbsp;&nbsp;[Material]  \n");
									break;
								case SHADER_PARAM_TYPE_STRING:
									fprintf(pLuxGithubWikiDump, "&nbsp;&nbsp;&nbsp;&nbsp;[String]  \n");
									break;
								case SHADER_PARAM_TYPE_MATRIX4X2:
									fprintf(pLuxGithubWikiDump, "&nbsp;&nbsp;&nbsp;&nbsp;[Matrix4x2]  \n");
									break;
								default: break;
								}
								fprintf(pLuxGithubWikiDump, "%s\n\n", pParamHelpShader);
								fflush(pLuxGithubWikiDump);

							} // end if statement
						} // end for loop param dump 
					} // end for loop helper string type

					fprintf(pLuxGithubWikiDump, "\n---  \n");
					fclose(pLuxGithubWikiDump);
					Msg("Done writting wiki dump of: %s\n", ShaderName.data());
				} // end for loop shadername
			}
		}
	}

	Msg("done\n");
	Msg("Github Wiki file dump at: %s", szLuxDumpDir);
}


#endif // NOLUX