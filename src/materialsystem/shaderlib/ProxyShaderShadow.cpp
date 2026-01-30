//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	17.09.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
//	Purpose of this File :	'Wrapper' for IShaderShadow
//
//==========================================================================//

#include "ProxyShaderShadow.h"

// NOTE: This must be the last include File in a .cpp File!
#include "tier0/memdbgon.h"

// Extern
CProxyShaderShadow s_ProxyShaderShadow;

void CProxyShaderShadow::SetDefaultState()
{
	// Make it known that we reset the State
	s_ShaderSpew.ResetShadowState();

	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->SetDefaultState();
}

void CProxyShaderShadow::EnableDepthTest(bool bEnable)
{
	s_ShaderSpew.LogDepthTests(bEnable);

	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->EnableDepthTest(bEnable);
}
void CProxyShaderShadow::DepthFunc(ShaderDepthFunc_t depthFunc)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->DepthFunc(depthFunc);
}

void CProxyShaderShadow::EnableDepthWrites(bool bEnable)
{
	s_ShaderSpew.LogDepthWrites(bEnable);

	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->EnableDepthWrites(bEnable);
}

void CProxyShaderShadow::EnablePolyOffset(PolygonOffsetMode_t nOffsetMode)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->EnablePolyOffset(nOffsetMode);
}

void CProxyShaderShadow::EnableStencil(bool bEnable)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;


	m_pShaderShadow->EnableStencil(bEnable);
}

void CProxyShaderShadow::StencilFunc(ShaderStencilFunc_t stencilFunc)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->StencilFunc(stencilFunc);
}

void CProxyShaderShadow::StencilPassOp(ShaderStencilOp_t stencilOp)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->StencilPassOp(stencilOp);
}

void CProxyShaderShadow::StencilFailOp(ShaderStencilOp_t stencilOp)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->StencilFailOp(stencilOp);
}

void CProxyShaderShadow::StencilDepthFailOp(ShaderStencilOp_t stencilOp)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->StencilDepthFailOp(stencilOp);
}

void CProxyShaderShadow::StencilReference(int nReference)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->StencilReference(nReference);
}

void CProxyShaderShadow::StencilMask(int nMask)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->StencilMask(nMask);
}

void CProxyShaderShadow::StencilWriteMask(int nMask)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->StencilWriteMask(nMask);
}

void CProxyShaderShadow::EnableColorWrites(bool bEnable)
{
	s_ShaderSpew.LogColorWrites(bEnable);

	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->EnableColorWrites(bEnable);
}

void CProxyShaderShadow::EnableAlphaWrites(bool bEnable)
{
	s_ShaderSpew.LogAlphaWrites(bEnable);

	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->EnableAlphaWrites(bEnable);
}

void CProxyShaderShadow::EnableBlending(bool bEnable)
{
	s_ShaderSpew.LogAlphaBlending(bEnable);

	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->EnableBlending(bEnable);
}

void CProxyShaderShadow::BlendFunc(ShaderBlendFactor_t srcFactor, ShaderBlendFactor_t dstFactor)
{
	s_ShaderSpew.LogAlphaBlending(srcFactor, dstFactor);

	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->BlendFunc(srcFactor, dstFactor);
}

void CProxyShaderShadow::EnableAlphaTest(bool bEnable)
{
	s_ShaderSpew.LogAlphaTest(bEnable);

	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->EnableAlphaTest(bEnable);
}

void CProxyShaderShadow::AlphaFunc(ShaderAlphaFunc_t alphaFunc, float alphaRef)
{
	s_ShaderSpew.LogAlphaFunc(alphaFunc, alphaRef);

	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->AlphaFunc(alphaFunc, alphaRef);
}

void CProxyShaderShadow::PolyMode(ShaderPolyModeFace_t face, ShaderPolyMode_t polyMode)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->PolyMode(face, polyMode);
}

void CProxyShaderShadow::EnableCulling(bool bEnable)
{
	s_ShaderSpew.LogCulling(bEnable);

	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->EnableCulling(bEnable);
}

void CProxyShaderShadow::EnableConstantColor(bool bEnable)
{

	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;


	m_pShaderShadow->EnableConstantColor(bEnable);
}

void CProxyShaderShadow::VertexShaderVertexFormat(unsigned int nFlags,
	int nTexCoordCount, int* pTexCoordDimensions, int nUserDataSize)
{
	s_ShaderSpew.LogVertexShaderVertexFormat(nFlags, nTexCoordCount, pTexCoordDimensions, nUserDataSize);

	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->VertexShaderVertexFormat(nFlags, nTexCoordCount, pTexCoordDimensions, nUserDataSize);
}

void CProxyShaderShadow::SetVertexShader(const char* pFileName, int nStaticVshIndex)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	const char* pShaderVCS = pFileName;
	pShaderVCS = g_ShaderReload.GetResolvedFileName(pFileName);
	m_pShaderShadow->SetVertexShader(pShaderVCS, nStaticVshIndex);
}

void CProxyShaderShadow::SetPixelShader(const char* pFileName, int nStaticPshIndex)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	const char* pShaderVCS = pFileName;
	pShaderVCS = g_ShaderReload.GetResolvedFileName(pFileName);
	m_pShaderShadow->SetPixelShader(pShaderVCS, nStaticPshIndex);
}

void CProxyShaderShadow::EnableLighting(bool bEnable)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->EnableLighting(bEnable);
}

void CProxyShaderShadow::EnableSpecular(bool bEnable)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->EnableSpecular(bEnable);
}

void CProxyShaderShadow::EnableSRGBWrite(bool bEnable)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->EnableSRGBWrite(bEnable);
}

void CProxyShaderShadow::EnableSRGBRead(Sampler_t sampler, bool bEnable)
{
	s_ShaderSpew.LogSamplerGammaRead((int)sampler, bEnable);
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->EnableSRGBRead(sampler, bEnable);
}

void CProxyShaderShadow::EnableVertexBlend(bool bEnable)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->EnableVertexBlend(bEnable);
}

void CProxyShaderShadow::OverbrightValue(TextureStage_t stage, float value)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->OverbrightValue(stage, value);
}

void CProxyShaderShadow::EnableTexture(Sampler_t sampler, bool bEnable)
{
	s_ShaderSpew.LogSamplerEnabled((int)sampler, bEnable);

	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->EnableTexture(sampler, bEnable);
}

void CProxyShaderShadow::EnableTexGen(TextureStage_t stage, bool bEnable)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->EnableTexGen(stage, bEnable);
}

void CProxyShaderShadow::TexGen(TextureStage_t stage, ShaderTexGenParam_t param)
{

	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->TexGen(stage, param);
}

void CProxyShaderShadow::EnableCustomPixelPipe(bool bEnable)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->EnableCustomPixelPipe(bEnable);
}

void CProxyShaderShadow::CustomTextureStages(int stageCount)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->CustomTextureStages(stageCount);
}

void CProxyShaderShadow::CustomTextureOperation(TextureStage_t stage, ShaderTexChannel_t channel,
	ShaderTexOp_t op, ShaderTexArg_t arg1, ShaderTexArg_t arg2)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->CustomTextureOperation(stage, channel, op, arg1, arg2);
}

void CProxyShaderShadow::DrawFlags(unsigned int drawFlags)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->DrawFlags(drawFlags);
}

void CProxyShaderShadow::EnableAlphaPipe(bool bEnable)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->EnableAlphaPipe(bEnable);
}

void CProxyShaderShadow::EnableConstantAlpha(bool bEnable)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->EnableConstantAlpha(bEnable);
}

void CProxyShaderShadow::EnableVertexAlpha(bool bEnable)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->EnableVertexAlpha(bEnable);
}

void CProxyShaderShadow::EnableTextureAlpha(TextureStage_t stage, bool bEnable)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->EnableTextureAlpha(stage, bEnable);
}

void CProxyShaderShadow::EnableBlendingSeparateAlpha(bool bEnable)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->EnableBlendingSeparateAlpha(bEnable);
}

void CProxyShaderShadow::BlendFuncSeparateAlpha(ShaderBlendFactor_t srcFactor, ShaderBlendFactor_t dstFactor)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->BlendFuncSeparateAlpha(srcFactor, dstFactor);
}

void CProxyShaderShadow::FogMode(ShaderFogMode_t fogMode)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->FogMode(fogMode);
}

void CProxyShaderShadow::SetDiffuseMaterialSource(ShaderMaterialSource_t materialSource)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->SetDiffuseMaterialSource(materialSource);
}

void CProxyShaderShadow::SetMorphFormat(MorphFormat_t flags)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->SetMorphFormat(flags);
}

void CProxyShaderShadow::DisableFogGammaCorrection(bool bDisable)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->DisableFogGammaCorrection(bDisable);
}

void CProxyShaderShadow::EnableAlphaToCoverage(bool bEnable)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->EnableAlphaToCoverage(bEnable);
}

// Indicates that the Sampler is used for a Shadow Depth Texture
void CProxyShaderShadow::SetShadowDepthFiltering(Sampler_t stage)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->SetShadowDepthFiltering(stage);
}

void CProxyShaderShadow::BlendOp(ShaderBlendOp_t blendOp)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->BlendOp(blendOp);
}
void CProxyShaderShadow::BlendOpSeparateAlpha(ShaderBlendOp_t blendOp)
{
	if (!m_bRealShaderShadow && m_bFakeShadowState)
		return;

	m_pShaderShadow->BlendOpSeparateAlpha(blendOp);
}