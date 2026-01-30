//===================== File of the LUX Shader Project =====================//
//
//	Description :	float2, float3, float4 Structs and Functions
//
//	Initial D.	:	17.08.2023 DMY
//	Last Change :	 30.01.2026 DMY
//
//	Purpose of this File :	C++ only allows us to use various things like, Vector, Vector4D etc
//							This will let us use proper HLSL Swizzel techniques
//
//==========================================================================//

/*
    MSVC Is going to throw a warning here about using a nonstandard extension
    ( nameless struct/union )
*/
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4201)
#endif

#ifndef CPP_FLOATX_H
#define CPP_FLOATX_H

#ifdef _WIN32
#pragma once
#endif

// need this for memcpy
#include <cstring>

// Forward declarations
class float2;
class float3;

class float2_raw
{
public:
    union
    {
        struct { float x, y; };
        struct { float r, g; };
    };

    // Required for C++20
    // Doesn't work on GCC
#ifndef LINUX
#if _MSVC_LANG >= 202002L
    constexpr float2_raw(float _x, float _y) : x(_x), y(_y) {}
#endif
#endif
    float2_raw() = default;

    float2_raw operator-() const;

    operator float* ();
    operator const float* () const;
	operator float2() const;

    // Assignment operators
    // This is fine for gcc as is
    float2_raw& operator=(float scalar)
    {
        x = scalar;
        y = scalar;
        return *this;
    }

    // This isn't.
    /*
    float2_raw& operator=(const float2_raw& other)
    {
        x = other.x;
        y = other.y;
    }

    float2_raw& operator=(const float2& other);
    */

    // Of course this is totally fine according to gcc
    float2_raw(const float2_raw&) = default;
    float2_raw& operator=(const float2_raw&) = default;

     // Compound assignment operators
    float2_raw& operator+=(float scalar);
    float2_raw& operator+=(const float2_raw& other);
    float2_raw& operator+=(const float2& other);
    float2_raw& operator-=(float scalar);
    float2_raw& operator-=(const float2_raw& other);
    float2_raw& operator-=(const float2& other);
    float2_raw& operator*=(float scalar);
    float2_raw& operator*=(const float2_raw& other);
    float2_raw& operator*=(const float2& other);
    float2_raw& operator/=(float scalar);
    float2_raw& operator/=(const float2_raw& other);
    float2_raw& operator/=(const float2& other);
};

class float3_raw
{
public:
    union
    {
        struct { float x, y, z; };
        struct { float2_raw xy; float zpad; };
        struct { float xpad; float2_raw yz; };
        struct { float r, g, b; };
        struct { float2_raw rg; float bpad; };
        struct { float rpad; float2_raw gb; };
    };

    // Required for C++20
    // Doesn't work on GCC
#ifndef LINUX
#if _MSVC_LANG >= 202002L
    constexpr float3_raw(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
#endif
#endif
    float3_raw() = default;

    float3_raw operator-() const;

    operator float* ();
    operator const float* () const;
    operator float3() const;

    // Assignment operators
    // This is fine for gcc as is
    float3_raw& operator=(float scalar)
    {
        x = scalar;
        y = scalar;
        z = scalar;
        return *this;
    }

    // This isn't.
    /*
    float3_raw& operator=(const float3_raw& other);
    float3_raw& operator=(const float3& other);
    */

    // Of course this is totally fine according to gcc
    float3_raw(const float3_raw&) = default;
    float3_raw& operator=(const float3_raw&) = default;

    // Compound assignment operators
    float3_raw& operator+=(float scalar);
    float3_raw& operator+=(const float3_raw& other);
    float3_raw& operator+=(const float3& other);
    float3_raw& operator-=(float scalar);
    float3_raw& operator-=(const float3_raw& other);
    float3_raw& operator-=(const float3& other);
    float3_raw& operator*=(float scalar);
    float3_raw& operator*=(const float3_raw& other);
    float3_raw& operator*=(const float3& other);
    float3_raw& operator/=(float scalar);
    float3_raw& operator/=(const float3_raw& other);
    float3_raw& operator/=(const float3& other);
};

class float2
{
public:
    union
    {
        struct { float x, y; };
        struct { float r, g; };
    };

    float2() = default;
    float2(float _x, float _y);
    float2(float v);

    float2 operator-() const;

    operator float* ();
    operator const float* () const;

    operator float2_raw() const
    {
        float2_raw result;
        result.x = x;
        result.y = y;
        return result;
    }

    // Assignment operators
    float2& operator=(float scalar);
    float2& operator=(const float2_raw& other);
    float2& operator=(const float2& other);

    // Compound assignment operators
    float2& operator+=(float scalar);
    float2& operator+=(const float2_raw& other);
    float2& operator+=(const float2& other);
    float2& operator-=(float scalar);
    float2& operator-=(const float2_raw& other);
    float2& operator-=(const float2& other);
    float2& operator*=(float scalar);
    float2& operator*=(const float2_raw& other);
    float2& operator*=(const float2& other);
    float2& operator/=(float scalar);
    float2& operator/=(const float2_raw& other);
    float2& operator/=(const float2& other);
};

// Binary operators for float2
float2 operator+(const float2& lhs, float rhs);
float2 operator+(float lhs, const float2& rhs);
float2 operator+(const float2& lhs, const float2& rhs);
float2 operator-(const float2& lhs, float rhs);
float2 operator-(float lhs, const float2& rhs);
float2 operator-(const float2& lhs, const float2& rhs);
float2 operator*(const float2& lhs, float rhs);
float2 operator*(float lhs, const float2& rhs);
float2 operator*(const float2& lhs, const float2& rhs);
float2 operator/(const float2& lhs, float rhs);
float2 operator/(float lhs, const float2& rhs);
float2 operator/(const float2& lhs, const float2& rhs);

// Binary operators for float2_raw
float2_raw operator+(const float2_raw& lhs, float rhs);
float2_raw operator+(float lhs, const float2_raw& rhs);
float2_raw operator+(const float2_raw& lhs, const float2_raw& rhs);
float2_raw operator+(const float2_raw& lhs, const float2& rhs);
float2_raw operator+(const float2& lhs, const float2_raw& rhs);
float2_raw operator-(const float2_raw& lhs, float rhs);
float2_raw operator-(float lhs, const float2_raw& rhs);
float2_raw operator-(const float2_raw& lhs, const float2_raw& rhs);
float2_raw operator-(const float2_raw& lhs, const float2& rhs);
float2_raw operator-(const float2& lhs, const float2_raw& rhs);
float2_raw operator*(const float2_raw& lhs, float rhs);
float2_raw operator*(float lhs, const float2_raw& rhs);
float2_raw operator*(const float2_raw& lhs, const float2_raw& rhs);
float2_raw operator*(const float2_raw& lhs, const float2& rhs);
float2_raw operator*(const float2& lhs, const float2_raw& rhs);
float2_raw operator/(const float2_raw& lhs, float rhs);
float2_raw operator/(float lhs, const float2_raw& rhs);
float2_raw operator/(const float2_raw& lhs, const float2_raw& rhs);
float2_raw operator/(const float2_raw& lhs, const float2& rhs);
float2_raw operator/(const float2& lhs, const float2_raw& rhs);

class float3
{
public:
    union
    {
        struct { float x, y, z; };
        struct { float2_raw xy; float zpad; };
        struct { float xpad; float2_raw yz; };
        struct { float r, g, b; };
        struct { float2_raw rg; float bpad; };
        struct { float rpad; float2_raw gb; };
    };

    float3() = default;
    float3(float _x, float _y, float _z);
    float3(float v);

    float3 operator-() const;

    operator float* ();
    operator const float* () const;
    operator float3_raw() const
    {
        float3_raw result;
        result.x = x;
        result.y = y;
        result.z = z;
        return result;
    }
  
    // Assignment operators
    float3& operator=(float scalar);
    float3& operator=(const float3_raw& other);
    float3& operator=(const float3& other);

    // Compound assignment operators
    float3& operator+=(float scalar);
    float3& operator+=(const float3_raw& other);
    float3& operator+=(const float3& other);
    float3& operator-=(float scalar);
    float3& operator-=(const float3_raw& other);
    float3& operator-=(const float3& other);
    float3& operator*=(float scalar);
    float3& operator*=(const float3_raw& other);
    float3& operator*=(const float3& other);
    float3& operator/=(float scalar);
    float3& operator/=(const float3_raw& other);
    float3& operator/=(const float3& other);
};

// Binary operators for float3
float3 operator+(const float3& lhs, float rhs);
float3 operator+(float lhs, const float3& rhs);
float3 operator+(const float3& lhs, const float3& rhs);
float3 operator-(const float3& lhs, float rhs);
float3 operator-(float lhs, const float3& rhs);
float3 operator-(const float3& lhs, const float3& rhs);
float3 operator*(const float3& lhs, float rhs);
float3 operator*(float lhs, const float3& rhs);
float3 operator*(const float3& lhs, const float3& rhs);
float3 operator/(const float3& lhs, float rhs);
float3 operator/(float lhs, const float3& rhs);
float3 operator/(const float3& lhs, const float3& rhs);

// Binary operators for float3_raw
float3_raw operator+(const float3_raw& lhs, float rhs);
float3_raw operator+(float lhs, const float3_raw& rhs);
float3_raw operator+(const float3_raw& lhs, const float3_raw& rhs);
float3_raw operator+(const float3_raw& lhs, const float3& rhs);
float3_raw operator+(const float3& lhs, const float3_raw& rhs);
float3_raw operator-(const float3_raw& lhs, float rhs);
float3_raw operator-(float lhs, const float3_raw& rhs);
float3_raw operator-(const float3_raw& lhs, const float3_raw& rhs);
float3_raw operator-(const float3_raw& lhs, const float3& rhs);
float3_raw operator-(const float3& lhs, const float3_raw& rhs);
float3_raw operator*(const float3_raw& lhs, float rhs);
float3_raw operator*(float lhs, const float3_raw& rhs);
float3_raw operator*(const float3_raw& lhs, const float3_raw& rhs);
float3_raw operator*(const float3_raw& lhs, const float3& rhs);
float3_raw operator*(const float3& lhs, const float3_raw& rhs);
float3_raw operator/(const float3_raw& lhs, float rhs);
float3_raw operator/(float lhs, const float3_raw& rhs);
float3_raw operator/(const float3_raw& lhs, const float3_raw& rhs);
float3_raw operator/(const float3_raw& lhs, const float3& rhs);
float3_raw operator/(const float3& lhs, const float3_raw& rhs);

class float4
{
public:
    union
    {
        struct { float x, y, z, w; };
        struct { float2_raw xy; float2_raw zw; };
        struct { float xpad; float2_raw yz; float wpad; };
        struct { float3_raw xyz; float wpad2; };
        struct { float xpad2; float3_raw yzw; };
        struct { float r, g, b, a; };
        struct { float2_raw rg; float2_raw ba; };
        struct { float rpad; float2_raw gb; float apad; };
        struct { float3_raw rgb; float apad2; };
        struct { float rpad2; float3_raw gba; };
    };

    float4() = default;
    float4(float _x, float _y, float _z, float _w);
    float4(float v);

    float4 operator-() const;

    operator float* ();
    operator const float* () const;

    // Assignment operators
    float4& operator=(float scalar);
    float4& operator=(const float4& other);

    // Compound assignment operators
    float4& operator+=(float scalar);
    float4& operator+=(const float4& other);
    float4& operator-=(float scalar);
    float4& operator-=(const float4& other);
    float4& operator*=(float scalar);
    float4& operator*=(const float4& other);
    float4& operator/=(float scalar);
    float4& operator/=(const float4& other);
};

// Binary operators for float4
float4 operator+(const float4& lhs, float rhs);
float4 operator+(float lhs, const float4& rhs);
float4 operator+(const float4& lhs, const float4& rhs);
float4 operator-(const float4& lhs, float rhs);
float4 operator-(float lhs, const float4& rhs);
float4 operator-(const float4& lhs, const float4& rhs);
float4 operator*(const float4& lhs, float rhs);
float4 operator*(float lhs, const float4& rhs);
float4 operator*(const float4& lhs, const float4& rhs);
float4 operator/(const float4& lhs, float rhs);
float4 operator/(float lhs, const float4& rhs);
float4 operator/(const float4& lhs, const float4& rhs);

//==========================================================================//
// Mathematical functions also used in shaders...
//==========================================================================//

// Helper Template. This will allows us to easily throw float2 or float3
// for mixed types
template<typename T>
inline auto to_vec(const T& v) -> decltype(float(v))
{
    return v;
}

inline float2 to_vec(const float2_raw& v) { return float2(v); }
inline float3 to_vec(const float3_raw& v) { return float3(v); }

// Template Lerp
template<typename A, typename B>
auto lerp(const A& a, const B& b, float t) -> decltype(to_vec(a) + t * (to_vec(b) - to_vec(a)))
{
    auto va = to_vec(a);
    auto vb = to_vec(b);
    return va + t * (vb - va);
}

// Template Clamp
template<typename A, typename B, typename C>
auto clamp(const A& x, const B& minVal, const C& maxVal) -> decltype(to_vec(x))
{
    auto vx = to_vec(x);
    auto vmin = to_vec(minVal);
    auto vmax = to_vec(maxVal);

    return vx < vmin ? vmin : (vx > vmax ? vmax : vx);
}

// Saturate
template<typename A>
auto saturate(const A& x) -> decltype(clamp(x, 0.0f, 1.0f))
{
    return clamp(x, 0.0f, 1.0f);
}

// abs has some existing functions..
// so we call it fxabs
inline float fxabs(float x) { return x < 0.0f ? -x : x; }

inline float2 abs(const float2& x)
{
    return float2(fxabs(x.x), fxabs(x.y));
}

inline float3 abs(const float3& x)
{
    return float3(fxabs(x.x), fxabs(x.y), fxabs(x.z));
}

inline float4 abs(const float4& x)
{
    return float4(fxabs(x.x), fxabs(x.y), fxabs(x.z), fxabs(x.w));
}

template<typename T>
inline auto abs(const T& x) -> decltype(abs(to_vec(x)))
{
    return abs(to_vec(x));
}

// Dot Products
inline float dot(const float2& a, const float2& b)
{
    return a.x * b.x + a.y * b.y;
}

inline float dot(const float3& a, const float3& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// float4 tot, althought not needed for Template
inline float dot(const float4& a, const float4 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

template<typename A, typename B>
inline float dot(const A& a, const B& b)
{
    return dot(to_vec(a), to_vec(b));
}

// sign
//====================//
// matlib has some function called sign apparently
// also fx prefix here
inline float fxsign(float a)
{
	return a < 0 ? -1.0f : (a > 0 ? 1.0f : 0.0f);
}

inline float2 sign(const float2& v)
{
    return float2(fxsign(v.x), fxsign(v.y));
}

inline float3 sign(const float3& v)
{
    return float3(fxsign(v.x), fxsign(v.y), fxsign(v.z));
}

inline float4 sign(const float4& v)
{
    return float4(fxsign(v.x), fxsign(v.y), fxsign(v.z), fxsign(v.w));
}

// Smoothstep, no overloads
inline float smoothstep(float edge0, float edge1, float x)
{
	// Scale, Bias, Saturate x
	x = saturate((x - edge0) / (edge1 - edge0));

	// Evaluate polynomial
	return x * x * (3 - 2 * x);
}

#endif // CPP_FLOATX_H

#ifdef _MSC_VER
#pragma warning(pop)
#endif