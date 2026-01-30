//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	21.09.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

#ifndef VCSHOTRELOAD_H
#define VCSHOTRELOAD_H

#ifdef _WIN32		   
#pragma once
#endif

#include <string>
#include <vector>

struct MaterialReferences_t
{
	std::string m_MaterialName = "";
	int nReferences = 1; // Always at least one Reference otherwise the Struct wouldn't exist
};

struct VCSReferences_t
{
	// VCS Name used to be a cc ( making the struct 128 bytes )
	// but some Shaders ( Screenspace_General and CSS ) pass on vcs Name from a Parameter
	// Aka the const char* will get nuked when the Shader is done rendering, corrupting the VCS Name HERE as well
	// string turns it into a copy of the Name instead, so it solves the issue but makes the struct 160 bytes..
	std::string m_strVCS = "";
	std::string m_strIndexed = "";
	std::string m_strFullPath = "";
	std::vector<MaterialReferences_t> m_Materials;
	long m_LastFileTime = 0.0;
	int m_nLastLoadedIndex = 0;
};

class CShaderReload
{
private:
	bool m_bInitialised = false;

	// Path to /shaders/fxc/ ( Mod Directory )
	char m_cShadersPath[MAX_PATH];

	char m_cOriginalFilePath[MAX_PATH];	// ../shaders/fxc/OriginalName.vcs
	char m_cIndexedFilePath[MAX_PATH];	// ../shaders/fxc/OriginalName_000temp.vcs

	// All Shader References kept track of
	std::vector<VCSReferences_t> m_ShaderReferences;
	VCSReferences_t* m_pLastShader = NULL;

	// Find a Reference in the List of References
	VCSReferences_t* FindReference(const char* ccVCS);
	MaterialReferences_t* FindMaterialReference(VCSReferences_t* pReference, const char* ccMaterial);

	// Create a Reference
	VCSReferences_t* CreateReference(const char* ccVCS);
	MaterialReferences_t* CreateMaterialReference(VCSReferences_t* pReference, const char* ccMaterial);

	// Copies a Shader under a new Name ( indexed )
	// Format: OriginalName_Temp000.
	// LightmappedGeneric_ps30 becomes LightmappedGeneric_ps30_001temp, 002temp, 003temp..
	bool IndexShader(VCSReferences_t* pShader);

	// Checks the File on Disk for the recorded Timestamp
	bool FileTimeChanged(VCSReferences_t* pShader);

	// Updates the TimeStamp of the Reference
	void UpdateFileTime(VCSReferences_t* pShader);

	// Current MaterialName, set by CBaseShader.
	const char* m_ccCurrentMaterialName = nullptr;

public:
	// Goes through shaders/fxc/ and removes any indexed .vcs Files *clearly* created by CShaderReload
	// ( by checking for 000temp.vcs )
	// This should be called when connecting the dll ( shaderdll.cpp's connect Function )
	void CleanIndexedShaderFiles();

	// Set the MaterialName of the currently rendering Material
	// Set by CBaseShader's Draw Function!
	// Used internally to know what Materials to reload when a VCS-File Changes
	void SetMaterialName(const char* ccMaterialName);

	// Initializes global and local Variables required for this Class to work
	void Init();

	// Used by IShaderShadow Proxy, asks for the new (indexed) Name
	const char* GetResolvedFileName(const char* ccVCS);

	// Functions for ConCommand Interactions
	void CacheCurrentMaterials();
	void ReadReferenceList(const char* ccShader);
	void ClearReferenceList();
	void ClearMaterialReferenceList();
	void ReloadPulse();

	~CShaderReload()
	{
		CleanIndexedShaderFiles();
	}
};

extern bool g_bHotReloadEnabled;
extern bool g_bHotReloadCacheEnabled;
extern CShaderReload g_ShaderReload;

#endif // SHADERSPEW_H