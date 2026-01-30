//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	17.09.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
//	Purpose of this File :	'Wrapper' for IShaderDynamicAPI
//
//==========================================================================//

#include "ProxyShaderAPI.h"

// NOTE: This must be the last include File in a .cpp File!
#include "tier0/memdbgon.h"

// extern
CProxyShaderDynamicAPI s_ProxyShaderAPI;

void CProxyShaderDynamicAPI::SetViewports(int nCount, const ShaderViewport_t* pViewports)
{
	m_pShaderAPI->SetViewports(nCount, pViewports);
}

int CProxyShaderDynamicAPI::GetViewports(ShaderViewport_t* pViewports, int nMax) const
{
	return m_pShaderAPI->GetViewports(pViewports, nMax);
}

double CProxyShaderDynamicAPI::CurrentTime() const
{
	return m_pShaderAPI->CurrentTime();
}

void CProxyShaderDynamicAPI::GetLightmapDimensions(int* w, int* h)
{
	m_pShaderAPI->GetLightmapDimensions(w, h);
}

MaterialFogMode_t CProxyShaderDynamicAPI::GetSceneFogMode()
{
	return m_pShaderAPI->GetSceneFogMode();
}

void CProxyShaderDynamicAPI::GetSceneFogColor(unsigned char* rgb)
{
	m_pShaderAPI->GetSceneFogColor(rgb);
}

void CProxyShaderDynamicAPI::MatrixMode(MaterialMatrixMode_t matrixMode)
{
m_pShaderAPI->MatrixMode(matrixMode);
}
void CProxyShaderDynamicAPI::PushMatrix()
{
	m_pShaderAPI->PushMatrix();
}

void CProxyShaderDynamicAPI::PopMatrix()
{
	m_pShaderAPI->PopMatrix();
}

void CProxyShaderDynamicAPI::LoadMatrix(float* m)
{
	m_pShaderAPI->LoadMatrix(m);
}

void CProxyShaderDynamicAPI::MultMatrix(float* m)
{
	m_pShaderAPI->MultMatrix(m);
}

void CProxyShaderDynamicAPI::MultMatrixLocal(float* m)
{
	m_pShaderAPI->MultMatrixLocal(m);
}

void CProxyShaderDynamicAPI::GetMatrix(MaterialMatrixMode_t matrixMode, float* dst)
{
	m_pShaderAPI->GetMatrix(matrixMode, dst);
}

void CProxyShaderDynamicAPI::LoadIdentity(void)
{
	m_pShaderAPI->LoadIdentity();
}

void CProxyShaderDynamicAPI::LoadCameraToWorld(void)
{
	m_pShaderAPI->LoadCameraToWorld();
}

void CProxyShaderDynamicAPI::Ortho(double left, double right, double bottom, double top, double zNear, double zFar)
{
	m_pShaderAPI->Ortho(left, right, bottom, top, zNear, zFar);
}

void CProxyShaderDynamicAPI::PerspectiveX(double fovx, double aspect, double zNear, double zFar)
{
	m_pShaderAPI->PerspectiveX(fovx, aspect, zNear, zFar);
}

void CProxyShaderDynamicAPI::PickMatrix(int x, int y, int width, int height)
{
	m_pShaderAPI->PickMatrix(x, y, width, height);
}

void CProxyShaderDynamicAPI::Rotate(float angle, float x, float y, float z)
{
	m_pShaderAPI->Rotate(angle, x, y, z);
}

void CProxyShaderDynamicAPI::Translate(float x, float y, float z)
{
	m_pShaderAPI->Translate(x, y, z);
}

void CProxyShaderDynamicAPI::Scale(float x, float y, float z)
{
	m_pShaderAPI->Scale(x, y, z);
}

void CProxyShaderDynamicAPI::ScaleXY(float x, float y)
{
	m_pShaderAPI->ScaleXY(x, y);
}

void CProxyShaderDynamicAPI::Color3f(float r, float g, float b)
{
	m_pShaderAPI->Color3f(r, g, b);
}

void CProxyShaderDynamicAPI::Color3fv(float const* pColor)
{
	m_pShaderAPI->Color3fv(pColor);
}

void CProxyShaderDynamicAPI::Color4f(float r, float g, float b, float a)
{
	m_pShaderAPI->Color4f(r, g, b, a);
}

void CProxyShaderDynamicAPI::Color4fv(float const* pColor)
{
	m_pShaderAPI->Color4fv(pColor);
}

void CProxyShaderDynamicAPI::Color3ub(unsigned char r, unsigned char g, unsigned char b)
{
	m_pShaderAPI->Color3ub(r, g, b);
}

void CProxyShaderDynamicAPI::Color3ubv(unsigned char const* pColor)
{
	m_pShaderAPI->Color3ubv(pColor);
}

void CProxyShaderDynamicAPI::Color4ub(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
	m_pShaderAPI->Color4ub(r, g, b, a);
}

void CProxyShaderDynamicAPI::Color4ubv(unsigned char const* pColor)
{
	m_pShaderAPI->Color4ubv(pColor);
}

void CProxyShaderDynamicAPI::SetVertexShaderConstant(int var, float const* pVec, int numConst, bool bForce)
{
	s_ShaderSpew.LogVertexShaderRegisterF(var, pVec, numConst);
	m_pShaderAPI->SetVertexShaderConstant(var, pVec, numConst, bForce);
}

void CProxyShaderDynamicAPI::SetPixelShaderConstant(int var, float const* pVec, int numConst, bool bForce)
{
	s_ShaderSpew.LogPixelShaderRegisterF(var, pVec, numConst, bForce);
	m_pShaderAPI->SetPixelShaderConstant(var, pVec, numConst, bForce);
}

void CProxyShaderDynamicAPI::SetDefaultState()
{
	s_ShaderSpew.ResetDynamicState();
	m_pShaderAPI->SetDefaultState();
}

void CProxyShaderDynamicAPI::GetWorldSpaceCameraPosition(float* pPos) const
{
	m_pShaderAPI->GetWorldSpaceCameraPosition(pPos);
}

int CProxyShaderDynamicAPI::GetCurrentNumBones(void) const
{
	return m_pShaderAPI->GetCurrentNumBones();
}

int CProxyShaderDynamicAPI::GetCurrentLightCombo(void) const
{
	return m_pShaderAPI->GetCurrentLightCombo();
}

MaterialFogMode_t CProxyShaderDynamicAPI::GetCurrentFogType(void) const 
{
	return m_pShaderAPI->GetCurrentFogType();
}

void CProxyShaderDynamicAPI::SetTextureTransformDimension(TextureStage_t textureStage, int dimension, bool projected)
{
	m_pShaderAPI->SetTextureTransformDimension(textureStage, dimension, projected);
}

void CProxyShaderDynamicAPI::DisableTextureTransform(TextureStage_t textureStage)
{
	m_pShaderAPI->DisableTextureTransform(textureStage);
}

void CProxyShaderDynamicAPI::SetBumpEnvMatrix(TextureStage_t textureStage, float m00, float m01, float m10, float m11)
{
	m_pShaderAPI->SetBumpEnvMatrix(textureStage, m00, m01, m10, m11);
}

void CProxyShaderDynamicAPI::SetVertexShaderIndex(int vshIndex)
{
	m_pShaderAPI->SetVertexShaderIndex(vshIndex);
}

void CProxyShaderDynamicAPI::SetPixelShaderIndex(int pshIndex)
{
	m_pShaderAPI->SetPixelShaderIndex(pshIndex);
}

void CProxyShaderDynamicAPI::GetBackBufferDimensions(int& width, int& height) const
{
	m_pShaderAPI->GetBackBufferDimensions(width, height);
}

int CProxyShaderDynamicAPI::GetMaxLights(void) const
{
	return m_pShaderAPI->GetMaxLights();
}

const LightDesc_t& CProxyShaderDynamicAPI::GetLight(int lightNum) const
{
	return m_pShaderAPI->GetLight(lightNum);
}

void CProxyShaderDynamicAPI::SetPixelShaderFogParams(int reg)
{
	s_ShaderSpew.LogPixelShaderRegister(reg, 1, "Fog Parameters");
	m_pShaderAPI->SetPixelShaderFogParams(reg);
}

void CProxyShaderDynamicAPI::SetVertexShaderStateAmbientLightCube()
{
	s_ShaderSpew.LogVertexShaderRegister(21, 6, "Ambient Cube");
	m_pShaderAPI->SetVertexShaderStateAmbientLightCube();
}

void CProxyShaderDynamicAPI::SetPixelShaderStateAmbientLightCube(int pshReg, bool bForceToBlack)
{
	s_ShaderSpew.LogPixelShaderRegister(pshReg, 6, "Ambient Cube");
	m_pShaderAPI->SetPixelShaderStateAmbientLightCube(pshReg, bForceToBlack);
}


void CProxyShaderDynamicAPI::CommitPixelShaderLighting(int pshReg)
{
	s_ShaderSpew.LogPixelShaderRegister(pshReg, 6, "Light Data");
	m_pShaderAPI->CommitPixelShaderLighting(pshReg);
}

CMeshBuilder* CProxyShaderDynamicAPI::GetVertexModifyBuilder()
{
	return m_pShaderAPI->GetVertexModifyBuilder();
}

bool CProxyShaderDynamicAPI::InFlashlightMode() const
{
	return m_pShaderAPI->InFlashlightMode();
}

const FlashlightState_t& CProxyShaderDynamicAPI::GetFlashlightState(VMatrix& worldToTexture) const
{
	return m_pShaderAPI->GetFlashlightState(worldToTexture);
}

bool CProxyShaderDynamicAPI::InEditorMode() const
{
	return m_pShaderAPI->InEditorMode();
}

MorphFormat_t CProxyShaderDynamicAPI::GetBoundMorphFormat()
{
	return m_pShaderAPI->GetBoundMorphFormat();
}

void CProxyShaderDynamicAPI::BindStandardTexture(Sampler_t sampler, StandardTextureId_t id)
{
	s_ShaderSpew.LogBindStandardTexture((int)sampler, id);
	m_pShaderAPI->BindStandardTexture(sampler, id);
}

ITexture* CProxyShaderDynamicAPI::GetRenderTargetEx(int nRenderTargetID)
{
	return m_pShaderAPI->GetRenderTargetEx(nRenderTargetID);
}

void CProxyShaderDynamicAPI::SetToneMappingScaleLinear(const Vector& scale)
{
	m_pShaderAPI->SetToneMappingScaleLinear(scale);
}

const Vector& CProxyShaderDynamicAPI::GetToneMappingScaleLinear(void) const
{
	return m_pShaderAPI->GetToneMappingScaleLinear();
}

float CProxyShaderDynamicAPI::GetLightMapScaleFactor(void) const
{
	return m_pShaderAPI->GetLightMapScaleFactor();
}

void CProxyShaderDynamicAPI::LoadBoneMatrix(int boneIndex, const float* m)
{
	m_pShaderAPI->LoadBoneMatrix(boneIndex, m);
}

void CProxyShaderDynamicAPI::PerspectiveOffCenterX(double fovx, double aspect, double zNear, double zFar, double bottom, double top, double left, double right) 
{
	m_pShaderAPI->PerspectiveOffCenterX(fovx, aspect, zNear, zFar, bottom, top, left, right);
}

void CProxyShaderDynamicAPI::SetFloatRenderingParameter(int parm_number, float value)
{
	m_pShaderAPI->SetFloatRenderingParameter(parm_number, value);
}

void CProxyShaderDynamicAPI::SetIntRenderingParameter(int parm_number, int value)
{
	m_pShaderAPI->SetIntRenderingParameter(parm_number, value);
}

void CProxyShaderDynamicAPI::SetVectorRenderingParameter(int parm_number, Vector const& value)
{
	m_pShaderAPI->SetVectorRenderingParameter(parm_number, value);
}

float CProxyShaderDynamicAPI::GetFloatRenderingParameter(int parm_number) const
{
	return m_pShaderAPI->GetFloatRenderingParameter(parm_number);
}

int CProxyShaderDynamicAPI::GetIntRenderingParameter(int parm_number) const
{
	return m_pShaderAPI->GetIntRenderingParameter(parm_number);
}

Vector CProxyShaderDynamicAPI::GetVectorRenderingParameter(int parm_number) const
{
	return m_pShaderAPI->GetVectorRenderingParameter(parm_number);
}

void CProxyShaderDynamicAPI::SetStencilEnable(bool onoff)
{
	m_pShaderAPI->SetStencilEnable(onoff);
}

void CProxyShaderDynamicAPI::SetStencilFailOperation(StencilOperation_t op)
{
	m_pShaderAPI->SetStencilFailOperation(op);
}

void CProxyShaderDynamicAPI::SetStencilZFailOperation(StencilOperation_t op)
{
	m_pShaderAPI->SetStencilZFailOperation(op);
}

void CProxyShaderDynamicAPI::SetStencilPassOperation(StencilOperation_t op)
{
	m_pShaderAPI->SetStencilPassOperation(op);
}

void CProxyShaderDynamicAPI::SetStencilCompareFunction(StencilComparisonFunction_t cmpfn)
{
	m_pShaderAPI->SetStencilCompareFunction(cmpfn);
}

void CProxyShaderDynamicAPI::SetStencilReferenceValue(int ref)
{
	m_pShaderAPI->SetStencilReferenceValue(ref);
}

void CProxyShaderDynamicAPI::SetStencilTestMask(uint32 msk)
{
	m_pShaderAPI->SetStencilTestMask(msk);
}

void CProxyShaderDynamicAPI::SetStencilWriteMask(uint32 msk)
{
	m_pShaderAPI->SetStencilWriteMask(msk);
}

void CProxyShaderDynamicAPI::ClearStencilBufferRectangle(int xmin, int ymin, int xmax, int ymax, int value)
{
	m_pShaderAPI->ClearStencilBufferRectangle(xmin, ymin, xmax, ymax, value);
}

void CProxyShaderDynamicAPI::GetDXLevelDefaults(uint& max_dxlevel, uint& recommended_dxlevel)
{
	m_pShaderAPI->GetDXLevelDefaults(max_dxlevel, recommended_dxlevel);
}
	
const FlashlightState_t& CProxyShaderDynamicAPI::GetFlashlightStateEx(VMatrix& worldToTexture, ITexture** pFlashlightDepthTexture) const
{
	return m_pShaderAPI->GetFlashlightStateEx(worldToTexture, pFlashlightDepthTexture);
}

float CProxyShaderDynamicAPI::GetAmbientLightCubeLuminance()
{
	return m_pShaderAPI->GetAmbientLightCubeLuminance();
}

void CProxyShaderDynamicAPI::GetDX9LightState(LightState_t* state) const
{
	m_pShaderAPI->GetDX9LightState(state);
}

int CProxyShaderDynamicAPI::GetPixelFogCombo()
{
	return m_pShaderAPI->GetPixelFogCombo();
}

void CProxyShaderDynamicAPI::BindStandardVertexTexture(VertexTextureSampler_t sampler, StandardTextureId_t id)
{
	s_ShaderSpew.LogVSBindStandardTexture((int)sampler, id);
	m_pShaderAPI->BindStandardVertexTexture(sampler, id);
}

bool CProxyShaderDynamicAPI::IsHWMorphingEnabled() const
{
	return m_pShaderAPI->IsHWMorphingEnabled();
}

void CProxyShaderDynamicAPI::GetStandardTextureDimensions(int* pWidth, int* pHeight, StandardTextureId_t id)
{
	m_pShaderAPI->GetStandardTextureDimensions(pWidth, pHeight, id);
}

void CProxyShaderDynamicAPI::SetBooleanVertexShaderConstant(int var, BOOL const* pVec, int numBools, bool bForce)
{
	s_ShaderSpew.LogVertexShaderRegisterB(var, pVec, numBools);
	m_pShaderAPI->SetBooleanVertexShaderConstant(var, pVec, numBools, bForce);	
}

void CProxyShaderDynamicAPI::SetIntegerVertexShaderConstant(int var, int const* pVec, int numIntVecs, bool bForce)
{
	s_ShaderSpew.LogVertexShaderRegisterN(var, pVec, numIntVecs);
	m_pShaderAPI->SetIntegerVertexShaderConstant(var, pVec, numIntVecs, bForce);	
}

void CProxyShaderDynamicAPI::SetBooleanPixelShaderConstant(int var, BOOL const* pVec, int numBools, bool bForce)
{
	s_ShaderSpew.LogPixelShaderRegisterB(var, pVec, numBools);
	m_pShaderAPI->SetBooleanPixelShaderConstant(var, pVec, numBools, bForce);
}

void CProxyShaderDynamicAPI::SetIntegerPixelShaderConstant(int var, int const* pVec, int numIntVecs, bool bForce)
{
	s_ShaderSpew.LogPixelShaderRegisterN(var, pVec, numIntVecs);
	m_pShaderAPI->SetIntegerPixelShaderConstant(var, pVec, numIntVecs, bForce);
}

bool CProxyShaderDynamicAPI::ShouldWriteDepthToDestAlpha(void) const
{
	return m_pShaderAPI->ShouldWriteDepthToDestAlpha();
}

void CProxyShaderDynamicAPI::PushDeformation(DeformationBase_t const* Deformation)
{
	m_pShaderAPI->PushDeformation(Deformation);
}

void CProxyShaderDynamicAPI::PopDeformation()
{
	m_pShaderAPI->PopDeformation(); 
}

int CProxyShaderDynamicAPI::GetNumActiveDeformations() const
{
	return m_pShaderAPI->GetNumActiveDeformations();
}

int CProxyShaderDynamicAPI::GetPackedDeformationInformation(int nMaskOfUnderstoodDeformations,
		float* pConstantValuesOut, int nBufferSize, int nMaximumDeformations, int* pNumDefsOut) const
{
	return m_pShaderAPI->GetPackedDeformationInformation(nMaskOfUnderstoodDeformations, pConstantValuesOut,
		nBufferSize, nMaximumDeformations, pNumDefsOut);	
}

void CProxyShaderDynamicAPI::MarkUnusedVertexFields(unsigned int nFlags, int nTexCoordCount, bool* pUnusedTexCoords)
{
	m_pShaderAPI->MarkUnusedVertexFields(nFlags, nTexCoordCount, pUnusedTexCoords);	
}

void CProxyShaderDynamicAPI::ExecuteCommandBuffer(uint8* pCmdBuffer)
{
	s_ShaderSpew.LogCommandBuffer(pCmdBuffer);
	m_pShaderAPI->ExecuteCommandBuffer(pCmdBuffer);	
}

void CProxyShaderDynamicAPI::SetStandardTextureHandle(StandardTextureId_t nId, ShaderAPITextureHandle_t nHandle)
{
	m_pShaderAPI->SetStandardTextureHandle(nId, nHandle);
}

void CProxyShaderDynamicAPI::GetCurrentColorCorrection(ShaderColorCorrectionInfo_t* pInfo)
{
	m_pShaderAPI->GetCurrentColorCorrection(pInfo);
}

void CProxyShaderDynamicAPI::SetPSNearAndFarZ(int pshReg)
{
	s_ShaderSpew.LogPixelShaderRegister(pshReg, 1, "NearZ and FarZ");
	m_pShaderAPI->SetPSNearAndFarZ(pshReg);	 
}

void CProxyShaderDynamicAPI::SetDepthFeatheringPixelShaderConstant(int iConstant, float fDepthBlendScale)
{
	s_ShaderSpew.LogPixelShaderRegister(iConstant, 1, "Depth Featering Constant");
	m_pShaderAPI->SetDepthFeatheringPixelShaderConstant(iConstant, fDepthBlendScale);
}

#ifdef TF2SDK
int CProxyShaderDynamicAPI::GetPixelFogCombo1(bool bSupportsRadial)
{
	return m_pShaderAPI->GetPixelFogCombo1(bSupportsRadial);
}
#endif