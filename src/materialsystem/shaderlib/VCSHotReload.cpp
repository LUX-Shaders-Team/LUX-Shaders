//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	21.09.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

// BaseShader
#include "BaseShader.h"
#include "VCSHotReload.h"

// Need this for String Mod Stuff
#include "filesystem.h"
#include "strtools.h"
#include <iomanip>
#include <sstream>
#include <string>

// Urgh pathing..
#include "../stdshaders/cpp_convars.h"

// Needed on the SDK, in TF2C this is included via one of the other Headers.
#include "utlbuffer.h"
#include "Color.h"

// NOTE: This must be the last include File in a .cpp File!
#include "tier0/memdbgon.h"

// Externs
CShaderReload g_ShaderReload;
bool g_bHotReloadEnabled = false;
bool g_bHotReloadCacheEnabled = false;
/*
	Overview for how this works:

	Pre-requisites:
		1. mat_queue_mode must be 0 (singlethreaded) - This Class can NOT account for multithreaded calls!
		2. Need MaterialSystem and FileSystem for checking VCS Times, Copying them and cleaning them up later
		3. Access to Shaderlib Files ( baseshader.cpp and shaderdll.cpp )

	Goal:
		1. When enabled, track which Materials desire to render what .vcs Files
		2. Track the "last edited" ( TimeStamp ) of these .vcs Files
		3. Once any of these Conditions is fullfilled..
			a. Batch File calls the Process with -hijack +'Pulse ConCommand'
			b. Pulse ConCommand is called manually
			c. After Engine_Post draws(?) TODO: Figure out if we can automate reload calls every few Seconds somewhere in stdshaders
		4. Check TimeStamps against recorded ones for all .vcs Files
		   If the TimeStamp changed, 
				1. Create a copy of the .vcs File under a different Name with an Index
				2. Keep track of the new Name ( store the Index )
				3. Throw all Material Names using this .vcs onto a List
				4. Reload the entire List of Materials
				5. Feed the actual IShaderShadow Function the new Name
				6. The Engine now loads what to it is a 'new' Shader ( The modified original Shader )
			else
				Do nothing

	Clean-up:
		1. Constructor of this Class *and* Deconstructor should call Cleanup Routine
		2. Go through all Files in shaders/fxc/ and nuke all Files with the indexed Namepattern
*/

// FIXME PRE-RELEASE: Throw to cpp_convars
int GetQueueMode()
{
	// This has to be in a Function or the ConVarRef won't init correctly
	static ConVarRef mat_queue_mode("mat_queue_mode");
	return mat_queue_mode.GetInt();
}

VCSReferences_t* CShaderReload::FindReference(const char* ccVCS)
{
	if(m_ShaderReferences.size() == 0)
	{
		return nullptr;
	}	
	else
	{
		for(size_t n = 0; n < m_ShaderReferences.size(); ++n)
		{
			if(V_stricmp(m_ShaderReferences[n].m_strVCS.c_str(), ccVCS) == 0)
				return &m_ShaderReferences[n];
		}
	}

	// Didn't find this Reference
	return nullptr;
}

MaterialReferences_t* CShaderReload::FindMaterialReference(VCSReferences_t* pReference, const char* ccMaterial)
{
	if(pReference->m_Materials.size() == 0)
	{
		return nullptr;
	}	
	else
	{
		for(size_t n = 0; n < pReference->m_Materials.size(); ++n)
		{
			// Simple Pointer comp.
			// This assumes all Shaders using the same .vcs Filename will also have the same const char*
			if(V_stricmp(pReference->m_Materials[n].m_MaterialName.c_str(), ccMaterial) == 0)
				return &pReference->m_Materials[n];
		}
	}

	// Didn't find this Reference
	return nullptr;
}

VCSReferences_t* CShaderReload::CreateReference(const char* ccVCS)
{
	m_ShaderReferences.emplace_back();
	VCSReferences_t* pResult = &m_ShaderReferences.back();

	pResult->m_strVCS = ccVCS;
	pResult->m_strIndexed = ccVCS; // No Index

	// Create the full Name and Path to the .vcs File
	V_sprintf_safe(m_cOriginalFilePath, "%s%s%s", m_cShadersPath, ccVCS, ".vcs");

	// Store this so we don't have to use sprintf all the Time
	pResult->m_strFullPath = m_cOriginalFilePath;

	// Get the Filetime
	pResult->m_LastFileTime = g_pFullFileSystem->GetFileTime(m_cOriginalFilePath, "GAME");

	return pResult;
}

MaterialReferences_t* CShaderReload::CreateMaterialReference(VCSReferences_t* pReference, const char* ccMaterial)
{
	pReference->m_Materials.emplace_back();
	MaterialReferences_t* pResult = &pReference->m_Materials.back();

	pResult->m_MaterialName = ccMaterial;

	return pResult;
}

bool CShaderReload::IndexShader(VCSReferences_t* pShader)
{
	pShader->m_nLastLoadedIndex++;

	// Read the File to a Buffer
	// Whether the .vcs is compressed or not doesn't matter at all
	CUtlBuffer utlBuffer;
	if (!g_pFullFileSystem->ReadFile(pShader->m_strFullPath.c_str(), "GAME", utlBuffer))
    {
        Warning("%s%s\n", "CShaderReload::IndexShader() Failed to read .vcs File: ", pShader->m_strFullPath.c_str());
        return false;
    }

	// Make the Full FilePath to the NEW File
	// We expect: shaders/fxc/originalname_000temp.vcs
	V_sprintf_safe(m_cIndexedFilePath, "%s%s_%03dtemp.vcs", m_cShadersPath, pShader->m_strVCS.c_str(), pShader->m_nLastLoadedIndex);

	// Store the File we just loaded under the NEW Name
	if (!g_pFullFileSystem->WriteFile(m_cIndexedFilePath, "GAME", utlBuffer))
	{
		Warning("%s%s\n", "CShaderReload::IndexShader() Failed to write .vcs File: ", m_cIndexedFilePath);
        return false;
    }

	// We have to do the same Thing again but without the Shaders Path..
	// We expect: originalname_000temp ( no Suffix and no FilePath )
	V_sprintf_safe(m_cIndexedFilePath, "%s_%03dtemp", pShader->m_strVCS.c_str(), pShader->m_nLastLoadedIndex);
	pShader->m_strIndexed = m_cIndexedFilePath; // Store that

	// Everything worked!
	return true;
}

bool CShaderReload::FileTimeChanged(VCSReferences_t* pShader)
{
	// Get the current Timestamp
	long CurFileTime = g_pFullFileSystem->GetFileTime(pShader->m_strFullPath.c_str(), "GAME");

	// If the Current FileTime is not the same as the Last FileTime
	if(CurFileTime != pShader->m_LastFileTime)
		return true;
	else
		return false;
}

void CShaderReload::UpdateFileTime(VCSReferences_t* pShader)
{
	// Get the current Timestamp
	pShader->m_LastFileTime = g_pFullFileSystem->GetFileTime(pShader->m_strFullPath.c_str(), "GAME");
}

void CShaderReload::CleanIndexedShaderFiles()
{
	// Make sure we have the Filesystem
	Init();

	// Make sure this is valid.
	if (!g_pFullFileSystem)
		return;

	char cShadersFilter[MAX_PATH];

	// Construct the full Path to shaders/fxc/ with wildcard
	V_sprintf_safe(cShadersFilter, "%s*.vcs", m_cShadersPath);

	// All Files with the _%03dtemp.vcs Suffix
	std::vector<std::string> TempFileNames;

	// Find all Files in this Directory
	// m_pShadersPath is based on the gameinfo.txt, so this should always be absolute Paths
	FileFindHandle_t h;
	const char *pFileName = g_pFullFileSystem->FindFirstEx(cShadersFilter, NULL, &h);
	for (; pFileName; pFileName = g_pFullFileSystem->FindNext( h ))
	{
		// Skip special Files
		if (!V_stricmp(pFileName, "..") || !V_stricmp(pFileName, "."))
			continue;

		// Handy Stock Function that checks for Suffix
		// We formatted it so the Index precedes the temp Suffix, making it easier to look for this Pattern
		if(V_strEndsWith(pFileName, "temp.vcs"))
		{
			TempFileNames.emplace_back();
			TempFileNames.back() = pFileName; // Store the name
		}
	}
	g_pFullFileSystem->FindClose( h );

	// No Temporary Files to clean up, leave.
	if(TempFileNames.size() == 0)
		return;

	// Nuke all Files that have the temp.vcs Suffix
	for(size_t n = 0; n < TempFileNames.size(); ++n)
	{
		V_sprintf_safe(m_cIndexedFilePath, "%s%s", m_cShadersPath, TempFileNames[n].c_str());

		// Make sure it exists, then remove it
		if(g_pFullFileSystem->FileExists(m_cIndexedFilePath, "GAME"))
			g_pFullFileSystem->RemoveFile(m_cIndexedFilePath, "GAME");
	}
}

void CShaderReload::SetMaterialName(const char* ccMaterialName)
{
	m_ccCurrentMaterialName = ccMaterialName;
}

void CShaderReload::Init()
{
	// Connect FileSystem and MaterialSystem if it hasn't been already
	if (!g_pFullFileSystem)
		g_pFullFileSystem = LoadInterface<IFileSystem>(FILESYSTEM_DLL_NAME, FILESYSTEM_INTERFACE_VERSION);

	if(!g_pMaterialSystem)
		g_pMaterialSystem = LoadInterface<IMaterialSystem>(MATERIALSYSTEM_DLL_NAME, MATERIAL_SYSTEM_INTERFACE_VERSION);

	if(m_bInitialised)
		return;

	// Find the gameinfo path
	// Then navigate to shaders/fxc/
	char szGamePath[MAX_PATH];

	// We look for Gameinfo.txt here to get the path to the Mod Directory where the Shaders are
	g_pFullFileSystem->RelativePathToFullPath( "gameinfo.txt", "MOD", szGamePath, sizeof( szGamePath ) );
	char *pChar = strchr(szGamePath, ';');
	if (pChar)
	    *pChar = '\0';

	// Strip gameinfo.txt
	V_StripFilename(szGamePath);

	// This is probably not necessary
	V_StripTrailingSlash(szGamePath);

	// Append /shaders/fxc/
	V_sprintf_safe(m_cShadersPath, "%s%s", szGamePath, "\\shaders\\fxc\\");
	m_bInitialised = true;
}

const char* CShaderReload::GetResolvedFileName(const char* ccVCS)
{
	if(!g_bHotReloadEnabled)
		return ccVCS;

	// Find this Shader Reference
	VCSReferences_t* pReference = FindReference(ccVCS);
	if(!pReference)
		pReference = CreateReference(ccVCS);

	// Find Material Reference if caching allowed
	// Didn't find it? Make it
	// Found it? Increase RefCount
	if(g_bHotReloadCacheEnabled)
	{
		MaterialReferences_t* pMatRef = FindMaterialReference(pReference, m_ccCurrentMaterialName);
		if(!pMatRef)
			pMatRef = CreateMaterialReference(pReference, m_ccCurrentMaterialName);
		else
			pMatRef->nReferences++;
	}

	// If the File has changed..
	// ( This is kinda terrible, it will recheck Filetimes for every Material trying to make a Snapshot.. )
	// That's a LOT of Filesystem calls.. 
	// UnlitGeneric ( with 836+ Materials and 4 References each ) will take FOREVER
	if(FileTimeChanged(pReference))
	{
		// Will update m_strIndexed as well
		// Only update Filetime if it worked
		if(IndexShader(pReference))
			UpdateFileTime(pReference);
	}

	// Returns the indexed Name..
	// Except if there is no indexed Name, in that case it returns ccVCS instead
	return pReference->m_strIndexed.c_str();
}

void CShaderReload::CacheCurrentMaterials()
{
	// Make sure we init'd
	Init();

	bool bPreviousCache = g_bHotReloadCacheEnabled;
	g_bHotReloadCacheEnabled = true;

	// Reload everything so we can also log ALL of them
	g_pMaterialSystem->ReloadMaterials(NULL);

	// Reset to the earlier State
	g_bHotReloadCacheEnabled = bPreviousCache;
}

void CShaderReload::ReadReferenceList(const char* ccShader)
{
	// Make sure we init'd
	Init();

	for(size_t n = 0; n < m_ShaderReferences.size(); ++n)
	{
		// Skip Shaders that don't match
		if(V_stricmp(ccShader, m_ShaderReferences[n].m_strVCS.c_str()) == 0)
		{
			ConColorMsg(Color(100, 100, 200, 255), "==================================================\n");
			ConColorMsg(Color(100, 100, 200, 255), "VCS References for: %s\n", m_ShaderReferences[n].m_strVCS.c_str());
			ConColorMsg(Color(100, 100, 200, 255), "Last Change: %ld\n", m_ShaderReferences[n].m_LastFileTime);
			ConColorMsg(Color(100, 100, 200, 255), "==================================================\n");

			for(size_t n2 = 0; n2 < m_ShaderReferences[n].m_Materials.size(); ++n2)
			{
				const char* ccMaterialName = m_ShaderReferences[n].m_Materials[n2].m_MaterialName.c_str();
				int nReferences = m_ShaderReferences[n].m_Materials[n2].nReferences;
				ConColorMsg(Color(255, 255, 0, 255), "[%03d] %s\n", nReferences, ccMaterialName);
			}
		}
	}
}

void CShaderReload::ClearReferenceList()
{
	// Make sure we init'd
	Init();

	m_ShaderReferences.clear();
}

void CShaderReload::ClearMaterialReferenceList()
{
	if(m_ShaderReferences.size() == 0)
		return;

	for(size_t n = 0; n < m_ShaderReferences.size(); ++n)
	{
		m_ShaderReferences[n].m_Materials.clear();
	}
}

void CShaderReload::ReloadPulse()
{
	if(!g_bHotReloadEnabled)
		return;

	// Only ever do this with a single thread
	if(GetQueueMode() != 0)
		return; // Silent for this one

	// Make sure we init'd
	Init();

	if(m_ShaderReferences.size() == 0)
		return;

	// Big List of all the Materials we have to reload
	// NOTE: Pointer to the Strings
	std::vector<std::string*> ReloadList;

	// else: Check which Files updated
	for(size_t n = 0; n < m_ShaderReferences.size(); ++n)
	{
		// Get CurTime
		long ShadersCurTime = g_pFullFileSystem->GetFileTime(m_ShaderReferences[n].m_strFullPath.c_str(), "GAME");

		if(m_ShaderReferences[n].m_LastFileTime != ShadersCurTime)
		{
			Msg("Shader changed: %s\n", m_ShaderReferences[n].m_strVCS.c_str());
			for(size_t nMaterial = 0; nMaterial < m_ShaderReferences[n].m_Materials.size(); ++nMaterial)
			{
				ReloadList.push_back(&m_ShaderReferences[n].m_Materials[nMaterial].m_MaterialName);
			}
		}
	}

	if(ReloadList.size() != 0)
	{
		// Don't want any Warnings
		g_bSupressShaderWarnings = true;

		// We already cached the Materials ( otherwise we wouldn't have this List.. )
		// So load them once to actually reload the VCS Files
		for(size_t n = 0; n < ReloadList.size(); ++n)
		{
			#if 1
				IMaterial* pMaterial = g_pMaterialSystem->FindMaterial(ReloadList[n]->c_str(), NULL, false);
				if(pMaterial)
					pMaterial->Refresh();
			#else
				// This will reload Keywords as well.. so brick004 will reload brick004a, b and c
				g_pMaterialSystem->ReloadMaterials(ReloadList[n]->c_str());

				// Do it again ( yes ) because some Shaders might haven't updated yet
				g_pMaterialSystem->ReloadMaterials(ReloadList[n]->c_str());
			#endif
		}

		// Allow Warnings again
		g_bSupressShaderWarnings = false;
	}

	// Update Filetimes
	// Should happen automatically
	/*
	for(size_t n = 0; n < m_ShaderReferences.size(); ++n)
	{
		// Construct the full FilePath from the .vcs Name
		m_ShaderReferences[n].m_LastFileTime = g_pFullFileSystem->GetFileTime(m_ShaderReferences[n].m_strFullPath.c_str(), "GAME");
	}
	*/
}

CON_COMMAND_F(lux_vcshotreloads_toggle, "Enable/Disable .vcs reloading.\n", FCVAR_CHEAT)
{
	g_bHotReloadEnabled = !g_bHotReloadEnabled;
	if(g_bHotReloadEnabled)
	{
		// Only ever do this with a single thread
		if(GetQueueMode() != 0)
		{
			Msg("%s\n", "CShaderReload Functions may only be used with mat_queue_mode 0.");
			g_bHotReloadEnabled = false;
			return;
		}

		// Required
		g_ShaderReload.Init();

		// Force enable state wrappers
		lux_general_shaderstatewrappers.SetValue(true);

		// Enable the Feature(s)
		g_bHotReloadEnabled = true;
		g_bHotReloadCacheEnabled = true; // Makes it somewhat slower since we search potentially hundreds of References

		g_ShaderReload.CacheCurrentMaterials();
	}
	else
	{
		// Disable the Feature
		g_bHotReloadEnabled = false;
		g_bHotReloadCacheEnabled = false;

		// Nuke the Cache
		g_ShaderReload.ClearReferenceList();

		Msg("%s\n", ".vcs reloads are now disabled.\n");
	}
}

/*
CON_COMMAND_F(lux_vcshotreloads_cache,
"Reloads all Materials. All .vcs Names will be cached together with Material-Names that referenced them\n"
"Once this is done, you can use lux_vcshotreloads_pulse.\n", FCVAR_CHEAT)
{
	// Only ever do this with a single thread
	if(GetQueueMode() != 0)
	{
		Msg("%s\n", "CShaderReload Functions may only be used with mat_queue_mode 0.");
		return;
	}
	
	// Clear any existing Cache for Materials
	g_ShaderReload.ClearMaterialReferenceList();

	// Dump Everything into the Reference List
	g_ShaderReload.CacheCurrentMaterials();
}
*/

CON_COMMAND_F(lux_vcshotreloads_pulse,
"Calls for a check on Filetimes. If a .vcs File changed ( from the List of cached .vcs's ),\n"
"all Materials using that .vcs File will be reloaded.\n", FCVAR_CHEAT)
{
	if(!g_bHotReloadEnabled)
	{
		Msg("%s\n", "vcshotreloads have not been enabled. Use lux_vcshotreloads_toggle.\n");
		return;
	}

	// Only ever do this with a single thread
	if(GetQueueMode() != 0)
	{
		Msg("%s\n", "CShaderReload Functions can only be used with mat_queue_mode 0.");
		return;
	}

	g_ShaderReload.ReloadPulse();
}

CON_COMMAND_F(lux_vcshotreloads_read, "Debug ConCommand to read the List of VCS References found\n", FCVAR_CHEAT)
{
	// Only ever do this with a single thread
	if(GetQueueMode() != 0)
	{
		Msg("%s\n", "CShaderReload Functions may only be used with mat_queue_mode 0.");
		return;
	}

	if (args.ArgC() == 1)
	{
		Msg("%s\n", "Use the Name of the Material or a Keyword as the Argument to this Command.");
		return;
	}

	g_ShaderReload.ReadReferenceList(args[1]);
}

CON_COMMAND_F(lux_vcshotreloads_clear, "Debug ConCommand to read the List of VCS References found\n", FCVAR_CHEAT)
{
	// Only ever do this with a single thread
	if(GetQueueMode() != 0)
	{
		Msg("%s\n", "CShaderReload Functions may only be used with mat_queue_mode 0.");
		return;
	}

	g_ShaderReload.ClearReferenceList();
}