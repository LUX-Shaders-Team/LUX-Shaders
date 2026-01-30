#include "cpp_floatx.h"

// float2_raw implementations
float2_raw float2_raw::operator-() const { return { -x, -y }; }

float2_raw::operator float* () { return &x; }
float2_raw::operator const float* () const { return &x; }
float2_raw::operator float2() const
{
    float2 result;
    result.x = x;
    result.y = y;
    return result;
}

/*
float2_raw& float2_raw::operator=(float scalar)
{
    x = scalar;
    y = scalar;
    return *this;
}

float2_raw& float2_raw::operator=(const float2_raw& other)
{
    x = other.x;
    y = other.y;
    return *this;
}

float2_raw& float2_raw::operator=(const float2& other)
{
    x = other.x;
    y = other.y;
    return *this;
}
*/

float2_raw& float2_raw::operator+=(float scalar)
{
    x += scalar;
    y += scalar;
    return *this;
}

float2_raw& float2_raw::operator+=(const float2_raw& other)
{
    x += other.x;
    y += other.y;
    return *this;
}

float2_raw& float2_raw::operator+=(const float2& other)
{
    x += other.x;
    y += other.y;
    return *this;
}

float2_raw& float2_raw::operator-=(float scalar)
{
    x -= scalar;
    y -= scalar;
    return *this;
}

float2_raw& float2_raw::operator-=(const float2_raw& other)
{
    x -= other.x;
    y -= other.y;
    return *this;
}

float2_raw& float2_raw::operator-=(const float2& other)
{
    x -= other.x;
    y -= other.y;
    return *this;
}

float2_raw& float2_raw::operator*=(float scalar)
{
    x *= scalar;
    y *= scalar;
    return *this;
}

float2_raw& float2_raw::operator*=(const float2_raw& other)
{
    x *= other.x;
    y *= other.y;
    return *this;
}

float2_raw& float2_raw::operator*=(const float2& other)
{
    x *= other.x;
    y *= other.y;
    return *this;
}

float2_raw& float2_raw::operator/=(float scalar)
{
    x /= scalar;
    y /= scalar;
    return *this;
}

float2_raw& float2_raw::operator/=(const float2_raw& other)
{
    x /= other.x;
    y /= other.y;
    return *this;
}

float2_raw& float2_raw::operator/=(const float2& other)
{
    x /= other.x;
    y /= other.y;
    return *this;
}

// float3_raw implementations
float3_raw float3_raw::operator-() const { return { -x, -y, -z }; }

float3_raw::operator float* () { return &x; }
float3_raw::operator const float* () const { return &x; }
float3_raw::operator float3() const
{
    float3 result;
    result.x = x;
    result.y = y;
    result.z = z;
    return result;
}

/*
float3_raw& float3_raw::operator=(float scalar)
{
    x = scalar;
    y = scalar;
    z = scalar;
    return *this;
}

float3_raw& float3_raw::operator=(const float3_raw& other)
{
    x = other.x;
    y = other.y;
    z = other.z;
    return *this;
}

float3_raw& float3_raw::operator=(const float3& other)
{
    x = other.x;
    y = other.y;
    z = other.z;
    return *this;
}
*/

float3_raw& float3_raw::operator+=(float scalar)
{
    x += scalar;
    y += scalar;
    z += scalar;
    return *this;
}

float3_raw& float3_raw::operator+=(const float3_raw& other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

float3_raw& float3_raw::operator+=(const float3& other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

float3_raw& float3_raw::operator-=(float scalar)
{
    x -= scalar;
    y -= scalar;
    z -= scalar;
    return *this;
}

float3_raw& float3_raw::operator-=(const float3_raw& other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}

float3_raw& float3_raw::operator-=(const float3& other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}

float3_raw& float3_raw::operator*=(float scalar)
{
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}

float3_raw& float3_raw::operator*=(const float3_raw& other)
{
    x *= other.x;
    y *= other.y;
    z *= other.z;
    return *this;
}

float3_raw& float3_raw::operator*=(const float3& other)
{
    x *= other.x;
    y *= other.y;
    z *= other.z;
    return *this;
}

float3_raw& float3_raw::operator/=(float scalar)
{
    x /= scalar;
    y /= scalar;
    z /= scalar;
    return *this;
}

float3_raw& float3_raw::operator/=(const float3_raw& other)
{
    x /= other.x;
    y /= other.y;
    z /= other.z;
    return *this;
}

float3_raw& float3_raw::operator/=(const float3& other)
{
    x /= other.x;
    y /= other.y;
    z /= other.z;
    return *this;
}

// float2 implementations
float2::float2(float _x, float _y) : x(_x), y(_y)
{}

float2::float2(float v) : x(v), y(v)
{}

float2 float2::operator-() const { return { -x, -y }; }

float2::operator float* ()
{ return &x; }
float2::operator const float* () const { return &x; }

float2& float2::operator=(float scalar)
{
    x = scalar;
    y = scalar;
    return *this;
}

float2& float2::operator=(const float2_raw& other)
{
    x = other.x;
    y = other.y;
    return *this;
}

float2& float2::operator=(const float2& other)
{
    x = other.x;
    y = other.y;
    return *this;
}

float2& float2::operator+=(float scalar)
{
    x += scalar;
    y += scalar;
    return *this;
}

float2& float2::operator+=(const float2_raw& other)
{
    x += other.x;
    y += other.y;
    return *this;
}

float2& float2::operator+=(const float2& other)
{
    x += other.x;
    y += other.y;
    return *this;
}

float2& float2::operator-=(float scalar)
{
    x -= scalar;
    y -= scalar;
    return *this;
}

float2& float2::operator-=(const float2_raw& other)
{
    x -= other.x;
    y -= other.y;
    return *this;
}

float2& float2::operator-=(const float2& other)
{
    x -= other.x;
    y -= other.y;
    return *this;
}

float2& float2::operator*=(float scalar)
{
    x *= scalar;
    y *= scalar;
    return *this;
}

float2& float2::operator*=(const float2_raw& other)
{
    x *= other.x;
    y *= other.y;
    return *this;
}

float2& float2::operator*=(const float2& other)
{
    x *= other.x;
    y *= other.y;
    return *this;
}

float2& float2::operator/=(float scalar)
{
    x /= scalar;
    y /= scalar;
    return *this;
}

float2& float2::operator/=(const float2_raw& other)
{
    x /= other.x;
    y /= other.y;
    return *this;
}

float2& float2::operator/=(const float2& other)
{
    x /= other.x;
    y /= other.y;
    return *this;
}

// float3 implementations
float3::float3(float _x, float _y, float _z) : x(_x), y(_y), z(_z)
{}

float3::float3(float v) : x(v), y(v), z(v)
{}

float3 float3::operator-() const { return { -x, -y, -z }; }

float3::operator float* () { return &x; }
float3::operator const float* () const { return &x; }

/*
float3::operator float3_raw() const {
    float3_raw result;
    result.x = x;
    result.y = y;
    result.z = z;
    return result;
}
*/

float3& float3::operator=(float scalar)
{
    x = scalar;
    y = scalar;
    z = scalar;
    return *this;
}

float3& float3::operator=(const float3_raw& other)
{
    x = other.x;
    y = other.y;
    z = other.z;
    return *this;
}

float3& float3::operator=(const float3& other)
{
    x = other.x;
    y = other.y;
    z = other.z;
    return *this;
}

float3& float3::operator+=(float scalar)
{
    x += scalar;
    y += scalar;
    z += scalar;
    return *this;
}

float3& float3::operator+=(const float3_raw& other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

float3& float3::operator+=(const float3& other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

float3& float3::operator-=(float scalar)
{
    x -= scalar;
    y -= scalar;
    z -= scalar;
    return *this;
}

float3& float3::operator-=(const float3_raw& other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}

float3& float3::operator-=(const float3& other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}

float3& float3::operator*=(float scalar)
{
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}

float3& float3::operator*=(const float3_raw& other)
{
    x *= other.x;
    y *= other.y;
    z *= other.z;
    return *this;
}

float3& float3::operator*=(const float3& other)
{
    x *= other.x;
    y *= other.y;
    z *= other.z;
    return *this;
}

float3& float3::operator/=(float scalar)
{
    x /= scalar;
    y /= scalar;
    z /= scalar;
    return *this;
}

float3& float3::operator/=(const float3_raw& other)
{
    x /= other.x;
    y /= other.y;
    z /= other.z;
    return *this;
}

float3& float3::operator/=(const float3& other)
{
    x /= other.x;
    y /= other.y;
    z /= other.z;
    return *this;
}

// float4 implementations
float4::float4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w)
{}
float4::float4(float v) : x(v), y(v), z(v), w(v)
{}

float4 float4::operator-() const { return { -x, -y, -z, -w }; }

float4::operator float* () { return &x; }
float4::operator const float* () const { return &x; }

float4& float4::operator=(float scalar)
{
    x = scalar;
    y = scalar;
    z = scalar;
    w = scalar;
    return *this;
}

float4& float4::operator=(const float4& other)
{
    x = other.x;
    y = other.y;
    z = other.z;
    w = other.w;
    return *this;
}

float4& float4::operator+=(float scalar)
{
    x += scalar;
    y += scalar;
    z += scalar;
    w += scalar;
    return *this;
}

float4& float4::operator+=(const float4& other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;
    return *this;
}

float4& float4::operator-=(float scalar)
{
    x -= scalar;
    y -= scalar;
    z -= scalar;
    w -= scalar;
    return *this;
}

float4& float4::operator-=(const float4& other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;
    return *this;
}

float4& float4::operator*=(float scalar)
{
    x *= scalar;
    y *= scalar;
    z *= scalar;
    w *= scalar;
    return *this;
}

float4& float4::operator*=(const float4& other)
{
    x *= other.x;
    y *= other.y;
    z *= other.z;
    w *= other.w;
    return *this;
}

float4& float4::operator/=(float scalar)
{
    x /= scalar;
    y /= scalar;
    z /= scalar;
    w /= scalar;
    return *this;
}

float4& float4::operator/=(const float4& other)
{
    x /= other.x;
    y /= other.y;
    z /= other.z;
    w /= other.w;
    return *this;
}

// Binary operators for float2
float2 operator+(const float2& lhs, float rhs)
{
    return float2{ lhs.x + rhs, lhs.y + rhs };
}

float2 operator+(float lhs, const float2& rhs)
{
    return float2{ lhs + rhs.x, lhs + rhs.y };
}

float2 operator+(const float2& lhs, const float2& rhs)
{
    return float2{ lhs.x + rhs.x, lhs.y + rhs.y };
}

float2 operator-(const float2& lhs, float rhs)
{
    return float2{ lhs.x - rhs, lhs.y - rhs };
}

float2 operator-(float lhs, const float2& rhs)
{
    return float2{ lhs - rhs.x, lhs - rhs.y };
}

float2 operator-(const float2& lhs, const float2& rhs)
{
    return float2{ lhs.x - rhs.x, lhs.y - rhs.y };
}

float2 operator*(const float2& lhs, float rhs)
{
    return float2{ lhs.x * rhs, lhs.y * rhs };
}

float2 operator*(float lhs, const float2& rhs)
{
    return float2{ lhs * rhs.x, lhs * rhs.y };
}

float2 operator*(const float2& lhs, const float2& rhs)
{
    return float2{ lhs.x * rhs.x, lhs.y * rhs.y };
}

float2 operator/(const float2& lhs, float rhs)
{
    return float2{ lhs.x / rhs, lhs.y / rhs };
}

float2 operator/(float lhs, const float2& rhs)
{
    return float2{ lhs / rhs.x, lhs / rhs.y };
}

float2 operator/(const float2& lhs, const float2& rhs)
{
    return float2{ lhs.x / rhs.x, lhs.y / rhs.y };
}

// Binary operators for float2_raw
float2_raw operator+(const float2_raw& lhs, float rhs)
{
    return float2_raw{ lhs.x + rhs, lhs.y + rhs };
}

float2_raw operator+(float lhs, const float2_raw& rhs)
{
    return float2_raw{ lhs + rhs.x, lhs + rhs.y };
}

float2_raw operator+(const float2_raw& lhs, const float2_raw& rhs)
{
    return float2_raw{ lhs.x + rhs.x, lhs.y + rhs.y };
}

float2_raw operator+(const float2_raw& lhs, const float2& rhs)
{
    return float2_raw{ lhs.x + rhs.x, lhs.y + rhs.y };
}

float2_raw operator+(const float2& lhs, const float2_raw& rhs)
{
    return float2_raw{ lhs.x + rhs.x, lhs.y + rhs.y };
}

float2_raw operator-(const float2_raw& lhs, float rhs)
{
    return float2_raw{ lhs.x - rhs, lhs.y - rhs };
}

float2_raw operator-(float lhs, const float2_raw& rhs)
{
    return float2_raw{ lhs - rhs.x, lhs - rhs.y };
}

float2_raw operator-(const float2_raw& lhs, const float2_raw& rhs)
{
    return float2_raw{ lhs.x - rhs.x, lhs.y - rhs.y };
}

float2_raw operator-(const float2_raw& lhs, const float2& rhs)
{
    return float2_raw{ lhs.x - rhs.x, lhs.y - rhs.y };
}

float2_raw operator-(const float2& lhs, const float2_raw& rhs)
{
    return float2_raw{ lhs.x - rhs.x, lhs.y - rhs.y };
}

float2_raw operator*(const float2_raw& lhs, float rhs)
{
    return float2_raw{ lhs.x * rhs, lhs.y * rhs };
}

float2_raw operator*(float lhs, const float2_raw& rhs)
{
    return float2_raw{ lhs * rhs.x, lhs * rhs.y };
}

float2_raw operator*(const float2_raw& lhs, const float2_raw& rhs)
{
    return float2_raw{ lhs.x * rhs.x, lhs.y * rhs.y };
}

float2_raw operator*(const float2_raw& lhs, const float2& rhs)
{
    return float2_raw{ lhs.x * rhs.x, lhs.y * rhs.y };
}

float2_raw operator*(const float2& lhs, const float2_raw& rhs)
{
    return float2_raw{ lhs.x * rhs.x, lhs.y * rhs.y };
}

float2_raw operator/(const float2_raw& lhs, float rhs)
{
    return float2_raw{ lhs.x / rhs, lhs.y / rhs };
}

float2_raw operator/(float lhs, const float2_raw& rhs)
{
    return float2_raw{ lhs / rhs.x, lhs / rhs.y };
}

float2_raw operator/(const float2_raw& lhs, const float2_raw& rhs)
{
    return float2_raw{ lhs.x / rhs.x, lhs.y / rhs.y };
}

float2_raw operator/(const float2_raw& lhs, const float2& rhs)
{
    return float2_raw{ lhs.x / rhs.x, lhs.y / rhs.y };
}

float2_raw operator/(const float2& lhs, const float2_raw& rhs)
{
    return float2_raw{ lhs.x / rhs.x, lhs.y / rhs.y };
}

// Binary operators for float3
float3 operator+(const float3& lhs, float rhs)
{
    return float3{ lhs.x + rhs, lhs.y + rhs, lhs.z + rhs };
}

float3 operator+(float lhs, const float3& rhs)
{
    return float3{ lhs + rhs.x, lhs + rhs.y, lhs + rhs.z };
}

float3 operator+(const float3& lhs, const float3& rhs)
{
    return float3{ lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
}

float3 operator-(const float3& lhs, float rhs)
{
    return float3{ lhs.x - rhs, lhs.y - rhs, lhs.z - rhs };
}

float3 operator-(float lhs, const float3& rhs)
{
    return float3{ lhs - rhs.x, lhs - rhs.y, lhs - rhs.z };
}

float3 operator-(const float3& lhs, const float3& rhs)
{
    return float3{ lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
}

float3 operator*(const float3& lhs, float rhs)
{
    return float3{ lhs.x * rhs, lhs.y * rhs, lhs.z * rhs };
}

float3 operator*(float lhs, const float3& rhs)
{
    return float3{ lhs * rhs.x, lhs * rhs.y, lhs * rhs.z };
}

float3 operator*(const float3& lhs, const float3& rhs)
{
    return float3{ lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z };
}

float3 operator/(const float3& lhs, float rhs)
{
    return float3{ lhs.x / rhs, lhs.y / rhs, lhs.z / rhs };
}

float3 operator/(float lhs, const float3& rhs)
{
    return float3{ lhs / rhs.x, lhs / rhs.y, lhs / rhs.z };
}

float3 operator/(const float3& lhs, const float3& rhs)
{
    return float3{ lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z };
}

// Binary operators for float3_raw
float3_raw operator+(const float3_raw& lhs, float rhs)
{
    return float3_raw{ lhs.x + rhs, lhs.y + rhs, lhs.z + rhs };
}

float3_raw operator+(float lhs, const float3_raw& rhs)
{
    return float3_raw{ lhs + rhs.x, lhs + rhs.y, lhs + rhs.z };
}

float3_raw operator+(const float3_raw& lhs, const float3_raw& rhs)
{
    return float3_raw{ lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
}

float3_raw operator+(const float3_raw& lhs, const float3& rhs)
{
    return float3_raw{ lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
}

float3_raw operator+(const float3& lhs, const float3_raw& rhs)
{
    return float3_raw{ lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
}

float3_raw operator-(const float3_raw& lhs, float rhs)
{
    return float3_raw{ lhs.x - rhs, lhs.y - rhs, lhs.z - rhs };
}

float3_raw operator-(float lhs, float3_raw& rhs)
{
    return float3_raw{ lhs - rhs.x, lhs - rhs.y, lhs - rhs.z };
}

float3_raw operator-(const float3_raw& lhs, const float3_raw& rhs)
{
    return float3_raw{ lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
}

float3_raw operator-(const float3_raw& lhs, const float3& rhs)
{
    return float3_raw{ lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
}

float3_raw operator-(const float3& lhs, const float3_raw& rhs)
{
    return float3_raw{ lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
}

float3_raw operator*(const float3_raw& lhs, float rhs)
{
    return float3_raw{ lhs.x * rhs, lhs.y * rhs, lhs.z * rhs };
}

float3_raw operator*(float lhs, const float3_raw& rhs)
{
    return float3_raw{ lhs * rhs.x, lhs * rhs.y, lhs * rhs.z };
}

float3_raw operator*(const float3_raw& lhs, const float3_raw& rhs)
{
    return float3_raw{ lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z };
}

float3_raw operator*(const float3_raw& lhs, const float3& rhs)
{
    return float3_raw{ lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z };
}

float3_raw operator*(const float3& lhs, const float3_raw& rhs)
{
    return float3_raw{ lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z };
}

float3_raw operator/(const float3_raw& lhs, float rhs)
{
    return float3_raw{ lhs.x / rhs, lhs.y / rhs, lhs.z / rhs };
}

float3_raw operator/(float lhs, const float3_raw& rhs)
{
    return float3_raw{ lhs / rhs.x, lhs / rhs.y, lhs / rhs.z };
}

float3_raw operator/(const float3_raw& lhs, const float3_raw& rhs)
{
    return float3_raw{ lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z };
}

float3_raw operator/(const float3_raw& lhs, const float3& rhs)
{
    return float3_raw{ lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z };
}

float3_raw operator/(const float3& lhs, const float3_raw& rhs)
{
    return float3_raw{ lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z };
}

// Binary operators for float4
float4 operator+(const float4& lhs, float rhs)
{
    return float4{ lhs.x + rhs, lhs.y + rhs, lhs.z + rhs, lhs.w + rhs };
}

float4 operator+(float lhs, const float4& rhs)
{
    return float4{ lhs + rhs.x, lhs + rhs.y, lhs + rhs.z, lhs + rhs.w };
}

float4 operator+(const float4& lhs, const float4& rhs)
{
    return float4{ lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w };
}

float4 operator-(const float4& lhs, float rhs)
{
    return float4{ lhs.x - rhs, lhs.y - rhs, lhs.z - rhs, lhs.w - rhs };
}

float4 operator-(float lhs, const float4& rhs)
{
    return float4{ lhs - rhs.x, lhs - rhs.y, lhs - rhs.z, lhs - rhs.w };
}

float4 operator-(const float4& lhs, const float4& rhs)
{
    return float4{ lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w };
}

float4 operator*(const float4& lhs, float rhs)
{
    return float4{ lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs };
}

float4 operator*(float lhs, const float4& rhs)
{
    return float4{ lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w };
}

float4 operator*(const float4& lhs, const float4& rhs)
{
    return float4{ lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z, lhs.w * rhs.w };
}

float4 operator/(const float4& lhs, float rhs)
{
    return float4{ lhs.x / rhs, lhs.y / rhs, lhs.z / rhs, lhs.w / rhs };
}

float4 operator/(float lhs, const float4& rhs)
{
    return float4{ lhs / rhs.x, lhs / rhs.y, lhs / rhs.z, lhs / rhs.w };
}

float4 operator/(const float4& lhs, const float4& rhs)
{
    return float4{ lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z, lhs.w / rhs.w };
}