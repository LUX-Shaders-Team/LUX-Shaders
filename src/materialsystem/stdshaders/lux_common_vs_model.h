//
//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	07.12.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

#ifndef LUX_COMMON_VS_MODEL_H_
#define LUX_COMMON_VS_MODEL_H_

//==========================================================================//
//	Decompression Functions
//==========================================================================//

// Make sure this is defined in some way
#if !defined(COMPRESSION)
	#error "Compression not defined for a Model Vertex Shader. Define as 0 or 1"
#endif

	//==========================================================================//
	//	Bone Weight Decompression
	//==========================================================================//
	float4 DecompressBoneWeights(const float4 f4Weights)
	{
		float4 f4Result = f4Weights;
	
		#if	COMPRESSION
			// "Decompress from SHORT2 to float. In our case, [-1, +32767] -> [0, +1]
			// NOTE: we add 1 here so we can divide by 32768 - which is exact (divide by 32767 is not).
			//       This avoids cracking between meshes with different numbers of bone weights.
			//       We use SHORT2 instead of SHORT2N for a similar reason - the GPU's conversion
			//       from [-32768,+32767] to [-1,+1] is imprecise in the same way."
			f4Result += 1;
			f4Result /= 32768;
		#endif
	
		return f4Result;
	}

	//==========================================================================//
	// "Decompress just a normal from four-component compressed format (same as above)
	// We expect this data to come from an unsigned UBYTE4 stream in the range of 0..255
	// [ When compiled, this works out to approximately 17 asm instructions ]"
	//==========================================================================//
	void DecompressUByte4Normal(float4 f4CompressedInputNormal,
		out float3 f3OutputNormal)
	{
		float fOne = 1.0f;
	
		float2 ztSigns = (f4CompressedInputNormal.xy - 128.0f) < 0;			// sign bits for zs and binormal (1 or 0)  set-less-than (slt) asm instruction
		float2 xyAbs = abs(f4CompressedInputNormal.xy - 128.0f) - ztSigns;	// 0..127
		float2 xySigns = (xyAbs - 64.0f) < 0;								// sign bits for xs and ys (1 or 0)
		f3OutputNormal.xy = (abs(xyAbs - 64.0f) - xySigns) / 63.0f;			// abs({nX, nY})

		f3OutputNormal.z = 1.0f - f3OutputNormal.x - f3OutputNormal.y;		// Project onto x+y+z=1
		f3OutputNormal.xyz = normalize(f3OutputNormal.xyz);					// Normalize onto unit sphere

		f3OutputNormal.xy *= lerp(fOne.xx, -fOne.xx, xySigns);				// Restore x and y signs
		f3OutputNormal.z *= lerp(fOne.x, -fOne.x, ztSigns.x);				// Restore z sign
	}

	//==========================================================================//
	// "Decompress a normal and tangent from four-component compressed format
	// We expect this data to come from an unsigned UBYTE4 stream in the range of 0..255
	// The final Tangent.w contains the sign of the binormal"
	//==========================================================================//
	void DecompressUByte4NormalTangent(float4 f4CompressedInputNormal,
		out float3 f3OutputNormal,
		out float4 f4OutputTangent)
	{
		float fOne = 1.0f;
	
		float4 ztztSignBits = (f4CompressedInputNormal - 128.0f) < 0;			// sign bits for zs and binormal (1 or 0)  set-less-than (slt) asm instruction
		float4 xyxyAbs = abs(f4CompressedInputNormal - 128.0f) - ztztSignBits;	// 0..127
		float4 xyxySignBits = (xyxyAbs - 64.0f) < 0;							// sign bits for xs and ys (1 or 0)
		float4 normTan = (abs(xyxyAbs - 64.0f) - xyxySignBits) / 63.0f;			// abs({nX, nY, tX, tY})
		f3OutputNormal.xy = normTan.xy;											// abs({nX, nY, __, __})
		f4OutputTangent.xy = normTan.zw;										// abs({tX, tY, __, __})
	
		float4 xyxySigns = 1 - 2 * xyxySignBits;								// Convert sign bits to signs
		float4 ztztSigns = 1 - 2 * ztztSignBits;								// ( [1,0] -> [-1,+1] )
	
		f3OutputNormal.z = 1.0f - f3OutputNormal.x - f3OutputNormal.y;			// Project onto x+y+z=1
		f3OutputNormal.xyz = normalize(f3OutputNormal.xyz);						// Normalize onto unit sphere
		f3OutputNormal.xy *= xyxySigns.xy;										// Restore x and y signs
		f3OutputNormal.z *= ztztSigns.x;										// Restore z sign
	
		f4OutputTangent.z = 1.0f - f4OutputTangent.x - f4OutputTangent.y;		// Project onto x+y+z=1
		f4OutputTangent.xyz = normalize(f4OutputTangent.xyz);					// Normalize onto unit sphere
		f4OutputTangent.xy *= xyxySigns.zw;										// Restore x and y signs
		f4OutputTangent.z *= ztztSigns.z;										// Restore z sign
		f4OutputTangent.w = ztztSigns.w;										// Binormal sign
	}

	//==========================================================================//
	// Decompression Helper Function
	//==========================================================================//
	void DecompressVertexNormal(float4 f4InputNormal, out float3 f3OutputNormal)
	{
	#if COMPRESSION
		DecompressUByte4Normal(f4InputNormal, f3OutputNormal);
	#else
		f3OutputNormal = f4InputNormal.xyz;
	#endif
	}

	//==========================================================================//
	// Decompression Helper Function
	//==========================================================================//
	void DecompressVertexNormal(float4 f4InputNormal, float4 f4InputTangent, out float3 f4OutputNormal, out float4 f4OutputTangent)
	{
		#if COMPRESSION
			DecompressUByte4NormalTangent(f4InputNormal, f4OutputNormal, f4OutputTangent);
		#else
			f4OutputNormal = f4InputNormal.xyz;
			f4OutputTangent = f4InputTangent;
		#endif
	}

//==========================================================================//
//	Vertex Texture Morphing
//==========================================================================//
#if MORPHING

	//==========================================================================//
	// Simple Morph Delta
	//==========================================================================//
	float4 SampleMorphDelta(sampler2D vt, const float3 vMorphTargetTextureDim, const float4 vMorphSubrect, const float flVertexID, const float flField)
	{
		float flColumn = floor(flVertexID / vMorphSubrect.w);
	
		float4 t;
		t.x = vMorphSubrect.x + vMorphTargetTextureDim.z * flColumn + flField + 0.5f;
		t.y = vMorphSubrect.y + flVertexID - flColumn * vMorphSubrect.w + 0.5f;
		t.xy /= vMorphTargetTextureDim.xy;
		t.z = t.w = 0.f;
	
		return tex2Dlod(vt, t);
	}
	
	//==========================================================================//
	// 'Optimized Version' with 2 Morph Delta
	//==========================================================================//
	void SampleMorphDelta2(sampler2D vt, const float3 vMorphTargetTextureDim, const float4 vMorphSubrect, const float flVertexID, out float4 delta1, out float4 delta2)
	{
		float flColumn = floor(flVertexID / vMorphSubrect.w);
	
		float4 t;
		t.x = vMorphSubrect.x + vMorphTargetTextureDim.z * flColumn + 0.5f;
		t.y = vMorphSubrect.y + flVertexID - flColumn * vMorphSubrect.w + 0.5f;
		t.xy /= vMorphTargetTextureDim.xy;
		t.z = t.w = 0.f;
	
		delta1 = tex2Dlod(vt, t);
		t.x += 1.0f / vMorphTargetTextureDim.x;
		delta2 = tex2Dlod(vt, t);
	}

	//==========================================================================//
	// ApplyMorph for Vertex Texture Morphing, Position only
	//==========================================================================//
	void ApplyMorph(sampler2D morphSampler, const float3 vMorphTargetTextureDim, const float4 vMorphSubrect,
		const float flVertexID, const float3 vMorphTexCoord,
		inout float3 vPosition)
	{
	
	#if !DECAL
		// Flexes coming in from a separate stream
		float4 vPosDelta = SampleMorphDelta(morphSampler, vMorphTargetTextureDim, vMorphSubrect, flVertexID, 0);
		vPosition += vPosDelta.xyz;
	#else
		float4 t = float4(vMorphTexCoord.x, vMorphTexCoord.y, 0.0f, 0.0f);
		float3 vPosDelta = tex2Dlod(morphSampler, t).xyz;
		vPosition += vPosDelta.xyz * vMorphTexCoord.zzz;
	#endif
	}
	
	//==========================================================================//
	// ApplyMorph with Normal Morph Support
	//==========================================================================//
	void ApplyMorph(sampler2D morphSampler, const float3 vMorphTargetTextureDim, const float4 vMorphSubrect,
		const float flVertexID, const float3 vMorphTexCoord,
		inout float3 vPosition, inout float3 vNormal)
	{
	#if !DECAL
		float4 vPosDelta, vNormalDelta;
		SampleMorphDelta2(morphSampler, vMorphTargetTextureDim, vMorphSubrect, flVertexID, vPosDelta, vNormalDelta);
		vPosition += vPosDelta.xyz;
		vNormal += vNormalDelta.xyz;
	#else
		float4 t = float4(vMorphTexCoord.x, vMorphTexCoord.y, 0.0f, 0.0f);
		float4 vPosDelta = tex2Dlod(morphSampler, t);
		t.x += 1.0f / vMorphTargetTextureDim.x;
		float4 vNormalDelta = tex2Dlod(morphSampler, t);
		vPosition += vPosDelta.xyz * vMorphTexCoord.zzz;
		vNormal += vNormalDelta.xyz * vMorphTexCoord.zzz;
	#endif // DECAL
	}
	
	//==========================================================================//
	// ApplyMorph with Normal and Tangent
	//==========================================================================//
	void ApplyMorph(sampler2D morphSampler, const float3 vMorphTargetTextureDim, const float4 vMorphSubrect,
		const float flVertexID, const float3 vMorphTexCoord,
		inout float3 vPosition, inout float3 vNormal, inout float3 vTangent)
	{
	#if !DECAL
		float4 vPosDelta, vNormalDelta;
		SampleMorphDelta2(morphSampler, vMorphTargetTextureDim, vMorphSubrect, flVertexID, vPosDelta, vNormalDelta);
		vPosition += vPosDelta.xyz;
		vNormal += vNormalDelta.xyz;
		vTangent += vNormalDelta.xyz;
	#else
		float4 t = float4(vMorphTexCoord.x, vMorphTexCoord.y, 0.0f, 0.0f);
		float3 vPosDelta = tex2Dlod(morphSampler, t);
		t.x += 1.0f / vMorphTargetTextureDim.x;
		float3 vNormalDelta = tex2Dlod(morphSampler, t);
		vPosition += vPosDelta.xyz * vMorphTexCoord.zzz;
		vNormal += vNormalDelta.xyz * vMorphTexCoord.zzz;
		vTangent += vNormalDelta.xyz * vMorphTexCoord.zzz;
	#endif // DECAL
	}
	
	//==========================================================================//
	// ApplyMorph with Normal, Tangent and WrinkleWeights
	//==========================================================================//
	void ApplyMorph(sampler2D morphSampler, const float3 vMorphTargetTextureDim, const float4 vMorphSubrect,
		const float flVertexID, const float3 vMorphTexCoord,
		inout float3 vPosition, inout float3 vNormal, inout float3 vTangent, out float flWrinkle)
	{
	#if !DECAL
		float4 vPosDelta, vNormalDelta;
		SampleMorphDelta2(morphSampler, vMorphTargetTextureDim, vMorphSubrect, flVertexID, vPosDelta, vNormalDelta);
		vPosition += vPosDelta.xyz;
		vNormal += vNormalDelta.xyz;
		vTangent += vNormalDelta.xyz;
		flWrinkle = vPosDelta.w;
	#else
		float4 t = float4(vMorphTexCoord.x, vMorphTexCoord.y, 0.0f, 0.0f);
		float4 vPosDelta = tex2Dlod(morphSampler, t);
		t.x += 1.0f / vMorphTargetTextureDim.x;
		float3 vNormalDelta = tex2Dlod(morphSampler, t);
	
		vPosition += vPosDelta.xyz * vMorphTexCoord.zzz;
		vNormal += vNormalDelta.xyz * vMorphTexCoord.zzz;
		vTangent += vNormalDelta.xyz * vMorphTexCoord.zzz;
		flWrinkle = vPosDelta.w * vMorphTexCoord.zzz;
	#endif // DECAL
	}
	
//==========================================================================//
//	Vertex Stream Morphing
//==========================================================================//
#else

	//==========================================================================//
	// ApplyMorph for Vertex Stream Morphing, Position only
	//==========================================================================//
	void ApplyMorph(float4 vPosFlex, inout float3 vPosition)
	{
		// Flexes coming in from a separate stream
		float3 vPosDelta = vPosFlex.xyz * cFlexScale.x;
		vPosition.xyz += vPosDelta;
	}

	//==========================================================================//
	// ApplyMorph with Normals
	//==========================================================================//
	void ApplyMorph(float4 vPosFlex, float4 vNormalFlex, inout float3 vPosition, inout float3 vNormal)
	{
		// Flexes coming in from a separate stream
		float3 vPosDelta = vPosFlex.xyz * cFlexScale.x;
		float3 vNormalDelta = vNormalFlex.xyz * cFlexScale.x;
		vPosition.xyz += vPosDelta;
		vNormal += vNormalDelta;
	}

	//==========================================================================//
	// ApplyMorph with Normal and Tangents
	//==========================================================================//
	void ApplyMorph(float4 vPosFlex, float4 vNormalFlex, inout float3 vPosition, inout float3 vNormal, inout float3 vTangent)
	{
		// Flexes coming in from a separate stream
		float3 vPosDelta = vPosFlex.xyz * cFlexScale.x;
		float3 vNormalDelta = vNormalFlex.xyz * cFlexScale.x;
		vPosition.xyz += vPosDelta;
		vNormal += vNormalDelta;
		vTangent.xyz += vNormalDelta;
	}

	//==========================================================================//
	// ApplyMorph with Normal, Tangent and WrinkleWeights
	//==========================================================================//
	void ApplyMorph(float4 vPosFlex, float4 vNormalFlex, inout float3 vPosition, inout float3 vNormal, inout float3 vTangent, out float flWrinkle)
	{
		// Flexes coming in from a separate stream
		float3 vPosDelta = vPosFlex.xyz * cFlexScale.x;
		float3 vNormalDelta = vNormalFlex.xyz * cFlexScale.x;
		flWrinkle = vPosFlex.w * cFlexScale.y;
		vPosition.xyz += vPosDelta;
		vNormal += vNormalDelta;
		vTangent.xyz += vNormalDelta;
	}
#endif

//==========================================================================//
//	Skinning Functions
//==========================================================================//
	void SkinPosition(const bool bSkinning, const float4 f4ModelPos,
		const float4 f4BoneWeights, float4 f4BoneIndices,
		out float3 f3WorldPos)
	{
		int4 nBoneIndices = D3DCOLORtoUBYTE4(f4BoneIndices);
	
		if (!bSkinning)
		{
			f3WorldPos = mul4x3(f4ModelPos, cModel[0]);
		}
		else // skinning - always three bones
		{
			float4x3 mxBoneMatrix1 = cModel[nBoneIndices[0]];
			float4x3 mxBoneMatrix2 = cModel[nBoneIndices[1]];
			float4x3 mxBoneMatrix3 = cModel[nBoneIndices[2]];
	
			float3 f3Weights = DecompressBoneWeights(f4BoneWeights).xyz;
			f3Weights[2] = 1 - (f3Weights[0] + f3Weights[1]);
	
			float4x3 mxBlendMatrix = mxBoneMatrix1 * f3Weights[0] + mxBoneMatrix2 * f3Weights[1] + mxBoneMatrix3 * f3Weights[2];
			f3WorldPos = mul4x3(f4ModelPos, mxBlendMatrix);
		}
	}

//==========================================================================//
//	Used on models to adjust the actual positions for rendering.
//==========================================================================//
void SkinPositionAndNormal(const bool bSkinning, const float4 f4ModelPos, const float3 f3ModelNormal,
	const float4 f4BoneWeights, float4 f4BoneIndices,
	out float3 f3WorldPos, out float3 f3WorldNormal)
{
	// Note : The GPU just uses floats for ints
	int4 boneIndices = D3DCOLORtoUBYTE4(f4BoneIndices);

	if (!bSkinning)
	{
		f3WorldPos = mul4x3(f4ModelPos, cModel[0]);
		f3WorldNormal = mul3x3(f3ModelNormal, (const float3x3)cModel[0]);
	}
	else // skinning - always three bones
	{
		float4x3 mat1 = cModel[boneIndices[0]];
		float4x3 mat2 = cModel[boneIndices[1]];
		float4x3 mat3 = cModel[boneIndices[2]];

		float3 weights = DecompressBoneWeights(f4BoneWeights).xyz;
		weights[2] = 1 - (weights[0] + weights[1]);

		float4x3 blendMatrix = mat1 * weights[0] + mat2 * weights[1] + mat3 * weights[2];
		f3WorldPos = mul4x3(f4ModelPos, blendMatrix);
		f3WorldNormal = mul3x3(f3ModelNormal, (float3x3)blendMatrix);
	}
}

//==========================================================================//
//	Used on models with tangents, otherwise same as the above
//==========================================================================//
void SkinPositionAndNormalAndTangents(const bool bSkinning, const float4 f4ModelPos, const float3 f3ModelNormal, const float4 TangentS,
	const float4 boneWeights, float4 f4BoneIndices,
	out float3 f3WorldPos, out float3 f3WorldNormal, out float3 f3Tangent)
{
	// Note : The GPU just uses floats for ints
	int4 boneIndices = D3DCOLORtoUBYTE4(f4BoneIndices);

	if (!bSkinning)
	{
		f3WorldPos = mul4x3(f4ModelPos, cModel[0]);
		f3WorldNormal = mul3x3(f3ModelNormal, (const float3x3)cModel[0]);
		f3Tangent = mul3x3(TangentS.xyz, (const float3x3)cModel[0] );
	}
	else // skinning - 'always three bones'
	{
		float4x3 mat1 = cModel[boneIndices[0]];
		float4x3 mat2 = cModel[boneIndices[1]];
		float4x3 mat3 = cModel[boneIndices[2]];

		float3 weights = DecompressBoneWeights(boneWeights).xyz;
		weights[2] = 1 - (weights[0] + weights[1]);

		float4x3 blendMatrix = mat1 * weights[0] + mat2 * weights[1] + mat3 * weights[2];
		f3WorldPos = mul4x3(f4ModelPos, blendMatrix);
		f3WorldNormal = mul3x3(f3ModelNormal, (const float3x3)blendMatrix);
		f3Tangent = mul3x3(TangentS.xyz, (const float3x3)blendMatrix ); // Why const?
	}

	// NOTE: This is now done in AlignVertexInputs
	// That is because sometimes we don't actually need the Binormal, only its sign value

	// Create the Binormal by taking the Cross-Product of Normal and Tangent
	// We do it now because we already rotated those two above and that way they only had to compress the Tangent
	// TangentS.w here is the Binormals sign, which determines what direction on its axis it should be pointing
//	f3Binormal = cross(f3WorldNormal, f3Tangent) * TangentS.w;
}

//==========================================================================//
// Overloaded function to help with the case of tangents and tangent + binormal sign
//==========================================================================//

// This one handles : WorldPos
void AlignVertexInputs(const bool bSkinning, const float4 f4ModelPos,
	const float4 f4BoneWeights, float4 f4BoneIndices,
	out float3 f3oWorldPos)
{
	// Prepare variables
	float3 f3WorldPos;

	// Apply Skinning/Rotation
	SkinPosition(bSkinning, f4ModelPos, f4BoneWeights, f4BoneIndices, f3WorldPos);

	// Apply outputs
	f3oWorldPos = f3WorldPos;
}

// This one handles : WorldPos, WorldNormal
void AlignVertexInputs(const bool bSkinning, const float4 f4ModelPos, const float3 f3ModelNormal,
	const float4 f4BoneWeights, float4 f4BoneIndices,
	out float3 f3oWorldPos, out float3 f3oWorldNormal)
{
	// Prepare variables
	float3 f3WorldPos, f3WorldNormal;

	// Apply Skinning/Rotation
	SkinPositionAndNormal(bSkinning, f4ModelPos, f3ModelNormal, f4BoneWeights, f4BoneIndices, f3WorldPos, f3WorldNormal);

	// Apply outputs
	f3oWorldPos = f3WorldPos;

	// Flexing is not done to all models and handled by a runtime constant
	// Means we have to always normalize T, B and N respectively!
	f3oWorldNormal = normalize(f3WorldNormal);
}

// This one handles : WorldPos, WorldNormal, Tangent + Binormal
void AlignVertexInputs(const bool bSkinning, const float4 f4ModelPos, const float3 f3ModelNormal, const float4 f4TangentS,
	const float4 f4BoneWeights, float4 f4BoneIndices,
	out float3 f3oWorldPos, out float3 f3oWorldNormal, out float4 f4oTangent_Binormal)
{
	// Prepare variables
	float3 f3WorldPos, f3WorldNormal, f3Tangent;

	// Apply Skinning/Rotation
	SkinPositionAndNormalAndTangents(bSkinning, f4ModelPos, f3ModelNormal, f4TangentS, f4BoneWeights, f4BoneIndices, f3WorldPos, f3WorldNormal, f3Tangent);

	// Apply outputs
	f3oWorldPos = f3WorldPos;

	// Flexing is not done to all models and handled by a runtime constant
	// Means we have to always normalize T, B and N respectively!
	f3oWorldNormal = normalize(f3WorldNormal);
	f4oTangent_Binormal = float4(normalize(f3Tangent), f4TangentS.w); // Pass on the Binormal Sign
}

// This one handles : WorldPos, WorldNormal, Tangent, Binormal
void AlignVertexInputs(const bool bSkinning, const float4 f4ModelPos, const float3 f3ModelNormal, const float4 f4TangentS,
	const float4 f4BoneWeights, float4 f4BoneIndices,
	out float3 f3oWorldPos, out float3 f3oWorldNormal, out float3 f3oTangent, out float3 f3oBinormal)
{
	// Prepare variables
	float3 f3WorldPos, f3WorldNormal, f3Tangent;

	// Apply Skinning/Rotation
	SkinPositionAndNormalAndTangents(bSkinning, f4ModelPos, f3ModelNormal, f4TangentS, f4BoneWeights, f4BoneIndices, f3WorldPos, f3WorldNormal, f3Tangent);

	// Apply outputs
	f3oWorldPos = f3WorldPos;

	// Flexing is not done to all models and handled by a runtime constant
	// Means we have to always normalize T, B and N respectively!
	f3oWorldNormal = normalize(f3WorldNormal);
	f3oTangent = normalize(f3Tangent);
	f3oBinormal = cross(f3oWorldNormal, f3oTangent) * f4TangentS.www; // Construct Binormal and apply sign
}

//==========================================================================//
//	Function to compute the Attenuation for a given Light.
//	Used for Bumped Lighting *and* Vertex Lighting!
//==========================================================================//
float ComputeLightAttenuation(const float3 f3WorldPos, int nLight)
{
	// Can't use Static-Control-Flow, compiler will complain to us that we are using them incorrectly...
	// if(g_bLightEnabled[nLight])

	// Unnormalized Light Direction
	float3 f3LightDir = cLightInfo[nLight].pos.xyz - f3WorldPos;

	// Distance Squared
	float f1LightDistSquared = dot(f3LightDir, f3LightDir);

	// Get 1/ Linear Distance
	float f1LightDist = rsqrt(f1LightDistSquared);

	// Manually normalize the LightDir
	f3LightDir *= f1LightDist;

	// dst is an intrinsic Distance Function
	float3 f3Dist = (float3)dst(f1LightDistSquared, f1LightDist);

	float f1AttenuationFactor = 1.0f / dot(cLightInfo[nLight].atten.xyz, f3Dist);

	// Spot Attenuation
	float f1CosTheta = dot(cLightInfo[nLight].dir.xyz, -f3LightDir);
	float f1SpotLightAttenuation = (f1CosTheta - cLightInfo[nLight].spotParams.z) * cLightInfo[nLight].spotParams.w;

	// Avoid too small Values
	f1SpotLightAttenuation = max(0.0001f, f1SpotLightAttenuation);

	// Falloff?
	f1SpotLightAttenuation = pow(f1SpotLightAttenuation, cLightInfo[nLight].spotParams.x);

	// Normalize
	f1SpotLightAttenuation = saturate(f1SpotLightAttenuation);

	// Select between point and spot
	f1AttenuationFactor = lerp(f1AttenuationFactor, f1AttenuationFactor * f1SpotLightAttenuation, cLightInfo[nLight].dir.w);

	// Select between above and directional (no attenuation)
	float f1Result = lerp(f1AttenuationFactor, 1.0f, cLightInfo[nLight].color.w);

	return f1Result;
}

//==========================================================================//
// Per-Vertex Ambient Cube
//==========================================================================//
float3 ComputeAmbientCube(const float3 f3WorldNormal)
{
	float3 nSquared = f3WorldNormal * f3WorldNormal;
	int3 isNegative = (f3WorldNormal < 0.0);
	float3 linearColor;
	linearColor =	nSquared.xxx * cAmbientCubeX[isNegative.x] +
					nSquared.yyy * cAmbientCubeY[isNegative.y] +
					nSquared.zzz * cAmbientCubeZ[isNegative.z];
	return linearColor;
}

//==========================================================================//
//	Helper-Function
//==========================================================================//
float ComputeCosineTermInternal(const float3 f3WorldPos, const float3 f3WorldNormal, int nLight, const bool bHalfLambert)
{
	// Calculate light direction assuming this is a point or spot
	float3 f3LightDir = normalize(cLightInfo[nLight].pos.xyz - f3WorldPos);

	// Select the above direction or the one in the structure, based upon light type
	f3LightDir = lerp(f3LightDir, -cLightInfo[nLight].dir.xyz, cLightInfo[nLight].color.w);

	// compute N dot L
	float NDotL = dot(f3WorldNormal, f3LightDir);

	if (!bHalfLambert)
	{
		NDotL = max(0.0f, NDotL);
	}
	else	// Half-Lambert
	{
		NDotL = NDotL * 0.5f + 0.5f;
		NDotL = NDotL * NDotL;
	}
	return NDotL;
}

//==========================================================================//
//	Helper-Function
//==========================================================================//
float3 ComputeLightInternal(const float3 worldPos, const float3 worldNormal, int lightNum, bool bHalfLambert) // this used to just be a bool despite HalfLambert being a Static Combo.
{
	return (cLightInfo[lightNum].color.rgb *
		ComputeCosineTermInternal(worldPos, worldNormal, lightNum, bHalfLambert) *
		ComputeLightAttenuation(worldPos, lightNum));
}

//==========================================================================//
//	Computes Lighting for Static, Dynamic, Physics props with no BumpMapping.
//==========================================================================//
float3 ComputeVertexLighting(const float3 f3WorldPos, const float3 f3WorldNormal,
	const float3 f3StaticLightingColor, const bool bStaticLight,
	const bool bDynamicLight, const bool bHalfLambert)
{
	float3 f3LinearColor = float3(0.0f, 0.0f, 0.0f);
	if (bStaticLight) // -StaticPropLighting
	{
		f3LinearColor += pow(max(f3StaticLightingColor * g_f1Overbright, 0.0f), 2.2f); // Gamma to Linear
	}

	if (bDynamicLight) // Dynamic lighting via toggleable-Lights or on dynamic objects
	{
		// Integer LightCount can only be used for for()-Loops!
		for (int i = 0; i < g_nLightCount; i++)
		{
			f3LinearColor += ComputeLightInternal(f3WorldPos, f3WorldNormal, i, bHalfLambert);
		}

		// Always when dynamic
		f3LinearColor += ComputeAmbientCube(f3WorldNormal); // ambient light is already remapped
	}
	return f3LinearColor;
}

#endif //#ifndef LUX_COMMON_VS_FXC_H_
