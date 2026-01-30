//===================== File of the LUX Shader Project =====================//
//
//	Initial D.	:	15.12.2025 DMY
//	Last Change :	 30.01.2026 DMY
//
//==========================================================================//

#ifndef LUX_COMMON_PRAGMAS_H_
#define LUX_COMMON_PRAGMAS_H_

// ShiroDkxtro2: Avoid disabling Warnings, you should solve them instead.

// WARNING X3207: Implicit truncation of vector type
// Warning X3206 happens when a Variable is set to another Variable with a larger Size
// For Example, 'float3 = f4Variable' will cause this Warning to appear. The correct way of doing this would be 'float3 = f4Variable.xyz'
// #pragma warning ( disable : 3206 )

// WARNING X3571: pow(f,e) will not work with negative f
// Bogus Warning in most cases. As it describes, negative f Values will not work.
// However this happens almost always when the Compiler can't determine the Range of the Value.
// Intrinsic Functions like max, saturate, or clamp can be used to force it to interpret the Value as >= 0.0f
// #pragma warning ( disable : 3571 )

// WARNING X4121: gradient-based operations must be moved out of flow control to prevent divergence. Performance may improve by using a non-gradient operation
// Usually leads to for-loops and while-loops being unrolled. ( Which drastically impacts Performance and can lead you to hit Instruction-Count Limits )
// Happens when Texture sampling is used in control flow that may be divergent across the Rendering Quad.
// This is an Issue when Parts of a Loop are skipped ( break, continue, return ), divergent Loop Size across the Quad or Data-dependent Branching.
// If a Loop is valid ( outside of Derivative Usage ), this Issue can be solved by using tex?Dlod instead.
// #pragma warning ( disable : 4121 )

// Previous Pragmas
// #pragma warning ( disable : 3557 ) // warning X3557: Loop only executes for N iteration(s), forcing loop to unroll
// #pragma warning ( disable : 3595 ) // warning X3595: Microcode Compiler possible performance issue: pixel shader input semantic ___ is unused
// #pragma warning ( disable : 3596 ) // warning X3596: Microcode Compiler possible performance issue: pixel shader input semantic ___ is unused
// #pragma warning ( disable : 4702 ) // warning X4702: complement opportunity missed because input result WAS clamped from 0 to 1

#endif // End of LUX_COMMON_PRAGMAS_H_