//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	20.11.2024 DMY
//	Last Change :	 30.01.2026 DMY
//
//	Purpose of this File :	Normal based Vertex Position Offset, rendered before the main Material
//
//==========================================================================//

// Commonly Shared Definitions, Defines and Data for all Shaders
#include "../../cpp_lux_shared.h"

// We need all of these
#include "MeshOutline.h"

// Includes for Shaderfiles...
#include "lux_meshoutline_vs30.inc"
#include "lux_meshoutline_ps30.inc"

void LuxOutlinePass_Draw(CBaseVSShader* pShader, IMaterialVar** params, IShaderDynamicAPI* pShaderAPI,
	IShaderShadow* pShaderShadow, Outline_Vars_t& info)
{
	bool bRenderOutline = (info.nOutlineEnable != -1) && pShader->GetBool(info.nOutlineEnable);

	if (!bRenderOutline)
		return;

	// Instantly abort ANY attempt to render a Flashlight
	if (bRenderOutline && pShader->HasFlashlight())
	{
		pShader->Draw(false);
	}
	else if (bRenderOutline)
	{
		bool bAlphaWrites = pShader->GetBool(info.nOutlineEnableAlphaWrites);
		bool bAdditive = pShader->GetBool(info.nOutlineAdditive);
		bool bDepthWrites = pShader->GetBool(info.nOutlineDepthWrite_Enable);
		bool bEnableFog = pShader->GetBool(info.nOutlineEnableFog);
		bool bCutoff = pShader->GetBool(info.nOutlineCutoffEnable);

		// This is unfinished.
		bool bTwoPass = false;

		// Never for Outline
		if (bAdditive)
			bAlphaWrites = false;

		//==========================================================================//
		// First Pass ( DepthWrite )
		//==========================================================================//
		if (bDepthWrites && bTwoPass && pShader->IsSnapshotting())
		{
			//==========================================================================//
			// General Rendering Setup
			//==========================================================================//

			// Multipass Shader, reset Shadow State
			pShaderShadow->SetDefaultState();

			// This handles : $IgnoreZ, $Decal, $Nocull, $Znearer, $Wireframe, $AllowAlphaToCoverage
			pShader->SetInitialShadowState();

			// Always Linear Values
			pShaderShadow->EnableSRGBWrite(true);

			// NO Color and Alpha
			pShaderShadow->EnableColorWrites(false);
			pShaderShadow->EnableAlphaWrites(false);

			//==========================================================================//
			// Vertex Shader - Vertex Format
			//==========================================================================//

			// We assume a Model with Compressed Normals
			unsigned int nFlags = VERTEX_POSITION | VERTEX_FORMAT_COMPRESSED | VERTEX_NORMAL;

			// Only one TexCoord, setting this for a minimum Vertex Format Size
			int nTexCoords = 1;

			// We don't need Tangents
			int nUserData = 0;

			// We don't need the TexCoord, but not asking for it causes a warning about a too narrow vertex format
			pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, NULL, nUserData);
			
			//==========================================================================//
			// Set Static Shaders
			//==========================================================================//
			DECLARE_STATIC_VERTEX_SHADER(lux_meshoutline_vs30);
			SET_STATIC_VERTEX_SHADER_COMBO(ENABLEFOG, bEnableFog || bAlphaWrites);
			SET_STATIC_VERTEX_SHADER_COMBO(CUTOFF, bCutoff);
			SET_STATIC_VERTEX_SHADER(lux_meshoutline_vs30);

			DECLARE_STATIC_PIXEL_SHADER(lux_meshoutline_ps30);
			SET_STATIC_PIXEL_SHADER_COMBO(ENABLEFOG, bEnableFog);
			SET_STATIC_PIXEL_SHADER_COMBO(WRITETOALPHA, bAlphaWrites);
			SET_STATIC_PIXEL_SHADER(lux_meshoutline_ps30);
		}

		//==========================================================================//
		// Entirely Dynamic Commands
		//==========================================================================//
		if (bDepthWrites && bTwoPass && pShader->IsDynamicState())
		{
			// Multipass Shader, reset Dynamic State
			pShaderAPI->SetDefaultState();

			// Prepare boolean array, yes we need to use BOOL
			BOOL BBools[REGISTER_BOOL_MAX] = { false };
			BBools[LUX_PS_BOOL_HEIGHTFOG] = pShader->WriteWaterFogToDestAlpha(bAlphaWrites);
			BBools[LUX_PS_BOOL_RADIALFOG] = pShader->HasRadialFog();
			BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = pShader->WriteDepthToDestAlpha(bAlphaWrites);
			pShaderAPI->SetBooleanPixelShaderConstant(REGISTER_BOOL_START, BBools, REGISTER_BOOL_MAX);

			// We allow the use of the Entity Origin Proxy
			// So get the .z of the OutlineCutoffHeight
			// If this is just a float than all 3 values of the Vector are the same and this won't be a problem
			float4 f4OutlineFactors;
			f4OutlineFactors.x = pShader->GetFloat(info.nOutlineDistance);
			f4OutlineFactors.y = pShader->GetFloat3(info.nOutlineCutoffHeight).z;
			f4OutlineFactors.z = pShader->GetFloat(info.nOutlineDepthWrite_Bias);
			pShaderAPI->SetVertexShaderConstant(50, f4OutlineFactors, 1);

			DECLARE_DYNAMIC_VERTEX_SHADER(lux_meshoutline_vs30);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, pShader->HasSkinning());
			SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSION, pShader->HasVertexCompression());
			SET_DYNAMIC_VERTEX_SHADER(lux_meshoutline_vs30);

			DECLARE_DYNAMIC_PIXEL_SHADER(lux_meshoutline_ps30);
			SET_DYNAMIC_PIXEL_SHADER(lux_meshoutline_ps30);
		}

		if(bDepthWrites && bTwoPass)
			pShader->Draw();

		//==========================================================================//
		// Second Pass ( Outline Color )
		//==========================================================================//
		if(pShader->IsSnapshotting())
		{
			//==========================================================================//
			// General Rendering Setup
			//==========================================================================//

			// Multipass Shader, reset Shadow State
			pShaderShadow->SetDefaultState();

			// This handles : $IgnoreZ, $Decal, $Nocull, $Znearer, $Wireframe, $AllowAlphaToCoverage
			pShader->SetInitialShadowState();

			// Always Linear Values
			pShaderShadow->EnableSRGBWrite(true);

			// Write Color and *maybe* Alpha
			pShaderShadow->EnableColorWrites(true);
			pShaderShadow->EnableAlphaWrites(bAlphaWrites);

			pShaderShadow->EnableCulling(false);

			if (!bDepthWrites && !bTwoPass)
				pShaderShadow->EnableDepthWrites(false);

//			if (!bTwoPass && !bDepthWrites)
//				pShaderShadow->EnableDepthWrites(false);

			//==========================================================================//
			// Vertex Shader - Vertex Format
			//==========================================================================//

			// We assume a Model with Compressed Normals
			unsigned int nFlags = VERTEX_POSITION | VERTEX_FORMAT_COMPRESSED | VERTEX_NORMAL;

			// Only one TexCoord, setting this for a minimum Vertex Format Size
			int nTexCoords = 1;

			// We don't need Tangents
			int nUserData = 0;

			// We don't need the TexCoord, but not asking for it causes a warning about a too narrow vertex format
			pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoords, NULL, nUserData);
		
			//==========================================================================//
			// Set Static Shaders
			//==========================================================================//
			DECLARE_STATIC_VERTEX_SHADER(lux_meshoutline_vs30);
			SET_STATIC_VERTEX_SHADER_COMBO(ENABLEFOG, bEnableFog || bAlphaWrites);
			SET_STATIC_VERTEX_SHADER_COMBO(CUTOFF, bCutoff);
			SET_STATIC_VERTEX_SHADER(lux_meshoutline_vs30);

			DECLARE_STATIC_PIXEL_SHADER(lux_meshoutline_ps30);
			SET_STATIC_PIXEL_SHADER_COMBO(ENABLEFOG, bEnableFog);
			SET_STATIC_PIXEL_SHADER_COMBO(WRITETOALPHA, bAlphaWrites);
			SET_STATIC_PIXEL_SHADER(lux_meshoutline_ps30);
		}

		//==========================================================================//
		// Entirely Dynamic Commands
		//==========================================================================//
		if(pShader->IsDynamicState())
		{
			// Multipass Shader, reset Dynamic State
			pShaderAPI->SetDefaultState();

			//==========================================================================//
			// Constant Registers
			//==========================================================================//

			// Prepare boolean array, yes we need to use BOOL
			BOOL BBools[REGISTER_BOOL_MAX] = { false };
			BBools[LUX_PS_BOOL_HEIGHTFOG] = pShader->WriteWaterFogToDestAlpha(bAlphaWrites);
			BBools[LUX_PS_BOOL_RADIALFOG] = pShader->HasRadialFog();
			BBools[LUX_PS_BOOL_DEPTHTODESTALPHA] = pShader->WriteDepthToDestAlpha(bAlphaWrites);
			pShaderAPI->SetBooleanPixelShaderConstant(REGISTER_BOOL_START, BBools, REGISTER_BOOL_MAX);

			// Set Pixel Shader Constants 
			float4 f4OutlineColor;
			f4OutlineColor.rgb = pShader->GetFloat3(info.nOutlineColor);
			f4OutlineColor.w = pShader->GetFloat(info.nOutlineCullPower);
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_000, f4OutlineColor);

			// Always need this on the Pixel Shader
			float4 f4CameraPos;
			pShaderAPI->GetWorldSpaceCameraPosition(f4CameraPos);
			f4CameraPos.w = pShader->GetFloat(info.nOutlineCullFactor);
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_001, f4CameraPos);

			// Always need this on the Pixel Shader
			float4 f4Factors;
			f4Factors.x = 0.0f;
			f4Factors.y = 0.0f;
			f4Factors.z = pShader->GetFloat(info.nOutlineDepthWrite_Bias);
			f4Factors.w = 0.0f;
			pShaderAPI->SetPixelShaderConstant(REGISTER_FLOAT_002, f4Factors);

			if (bEnableFog)
				pShaderAPI->SetPixelShaderFogParams(LUX_PS_FLOAT_FOGPARAMETERS);

			// We allow the use of the Entity Origin Proxy
			// So get the .z of the OutlineCutoffHeight
			// If this is just a float than all 3 values of the Vector are the same and this won't be a problem
			float4 f4OutlineFactors;
			f4OutlineFactors.x = pShader->GetFloat(info.nOutlineDistance);
			f4OutlineFactors.y = pShader->GetFloat3(info.nOutlineCutoffHeight).z;
			f4OutlineFactors.z = pShader->GetFloat(info.nOutlineDepthWrite_Bias);
			pShaderAPI->SetVertexShaderConstant(50, f4OutlineFactors, 1);

			// Always need Matrices for the Outline
			VMatrix matModel;
			pShaderAPI->GetMatrix(MATERIAL_MODEL, matModel[0]);

			VMatrix matView;
			pShaderAPI->GetMatrix(MATERIAL_VIEW, matView[0]);

			VMatrix matProj;
			pShaderAPI->GetMatrix(MATERIAL_PROJECTION, matProj[0]);

			// MV
			pShaderAPI->SetVertexShaderConstant(223, matView[0], 4);
			pShaderAPI->SetVertexShaderConstant(227, matProj[0], 4);
			// cModel Matrix already a thing

			// Origin of the Model
			float4 cModelOrigin = float4(matModel[3][0], matModel[3][1], matModel[3][2], 0.0f);

			// Camera Ray
			MatrixTranspose(matView, matView);
			float4 cViewForward = float4(matView[2][0], matView[2][1], matView[2][2], 0.0f);
			cViewForward.xyz *= -1.0f;

			float4 cPlanePos;
			cPlanePos.xyz = cModelOrigin.xyz + (cViewForward.xyz * pShader->GetFloat(info.nOutlineDepthWrite_Bias));
			
//			pShaderAPI->SetVertexShaderConstant(231, cViewForward);
			pShaderAPI->SetVertexShaderConstant(232, cPlanePos);

			// 2D Vector facing away from the Camera
			cModelOrigin.z = 0.0f; // No .z - We want a 2D Vector facing away from the Camera

			// Origin of the Camera we are looking from
			Vector4D CameraOrigin(f4CameraPos.x, f4CameraPos.y, f4CameraPos.z, 0.0f);
			CameraOrigin.z = 0.0f; // No .z - We want a 2D Vector facing away from the Camera


			Vector4D vModelOrogin(cModelOrigin.x, cModelOrigin.y, cModelOrigin.z, 0.0f);
			Vector4D ToModel(0.0f, 0.0f, 0.0f, 0.0f);
			ToModel.AsVector3D() = vModelOrogin.AsVector3D() - CameraOrigin.AsVector3D();
			ToModel.AsVector3D().NormalizeInPlace();

			pShaderAPI->SetVertexShaderConstant(231, ToModel.Base());

			//==================================================================================================
			// Set Dynamic Shaders
			//==================================================================================================
			DECLARE_DYNAMIC_VERTEX_SHADER(lux_meshoutline_vs30);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, pShader->HasSkinning());
			SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSION, pShader->HasVertexCompression());
			SET_DYNAMIC_VERTEX_SHADER(lux_meshoutline_vs30);

			DECLARE_DYNAMIC_PIXEL_SHADER(lux_meshoutline_ps30);
			SET_DYNAMIC_PIXEL_SHADER(lux_meshoutline_ps30);
		}

		pShader->Draw();

		// Reset because there will be a new pass after this
		if (pShaderShadow)
			pShaderShadow->SetDefaultState();

		if (pShaderAPI)
			pShaderAPI->SetDefaultState();
	}
}