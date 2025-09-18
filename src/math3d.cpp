#include "math3d.h"
#include <math.h>

// vec2 implementations
vec2::vec2() : x(0), y(0) {
}

vec2::vec2(f32 v) : x(v), y(v) {
}

vec2::vec2(f32 x, f32 y) : x(x), y(y) {
}

vec2::vec2(const ivec2& v) : x((f32)v.x), y((f32)v.y) {
}

f32& vec2::operator[](i32 i) {
    return values[i];
}

const f32& vec2::operator[](i32 i) const {
    return values[i];
}

vec2 vec2::operator+(const vec2& v) const {
    return vec2(x + v.x, y + v.y);
}

vec2 vec2::operator-(const vec2& v) const {
    return vec2(x - v.x, y - v.y);
}

vec2 vec2::operator*(f32 s) const {
    return vec2(x * s, y * s);
}

vec2 vec2::operator*(vec2 s) const {
    return vec2(x * s.x, y * s.y);
}

vec2 vec2::operator/(f32 s) const {
    return vec2(x / s, y / s);
}

vec2 vec2::operator/(vec2 s) const {
    return vec2(x / s.x, y / s.y);
}

vec2 vec2::operator-() const {
    return vec2(-x, -y);
}

vec2& vec2::operator+=(const vec2& v) {
    x += v.x;
    y += v.y;
    return *this;
}

vec2& vec2::operator-=(const vec2& v) {
    x -= v.x;
    y -= v.y;
    return *this;
}

vec2& vec2::operator*=(f32 s) {
    x *= s;
    y *= s;
    return *this;
}

vec2& vec2::operator*=(vec2 s) {
    x *= s.x;
    y *= s.y;
    return *this;
}

vec2& vec2::operator/=(f32 s) {
    x /= s;
    y /= s;
    return *this;
}

vec2& vec2::operator/=(vec2 s) {
    x /= s.x;
    y /= s.y;
    return *this;
}

f32 vec2::length() const {
    return sqrtf(x * x + y * y);
}

f32 vec2::length_squared() const {
    return x * x + y * y;
}

vec2 vec2::normalized() const {
    f32 len = length();
    return len > 0 ? *this / len : vec2(0, 0);
}

void vec2::normalize() {
    *this = normalized();
}

f32 vec2::dot(const vec2& v) const {
    return x * v.x + y * v.y;
}

// ivec2 implementations
ivec2::ivec2() : x(0), y(0) {
}

ivec2::ivec2(i32 v) : x(v), y(v) {
}

ivec2::ivec2(i32 x, i32 y) : x(x), y(y) {
}

ivec2::ivec2(const vec2& v) : x((i32)v.x), y((i32)v.y) {
}

i32& ivec2::operator[](i32 i) {
    return (&x)[i];
}

const i32& ivec2::operator[](i32 i) const {
    return (&x)[i];
}

ivec2 ivec2::operator+(const ivec2& v) const {
    return ivec2(x + v.x, y + v.y);
}

ivec2 ivec2::operator-(const ivec2& v) const {
    return ivec2(x - v.x, y - v.y);
}

ivec2 ivec2::operator*(i32 s) const {
    return ivec2(x * s, y * s);
}

ivec2 ivec2::operator*(ivec2 s) const {
    return ivec2(x * s.x, y * s.y);
}

ivec2 ivec2::operator/(i32 s) const {
    return ivec2(x / s, y / s);
}

ivec2 ivec2::operator/(ivec2 s) const {
    return ivec2(x / s.x, y / s.y);
}

ivec2 ivec2::operator-() const {
    return ivec2(-x, -y);
}

ivec2& ivec2::operator+=(const ivec2& v) {
    x += v.x;
    y += v.y;
    return *this;
}

ivec2& ivec2::operator-=(const ivec2& v) {
    x -= v.x;
    y -= v.y;
    return *this;
}

ivec2& ivec2::operator*=(i32 s) {
    x *= s;
    y *= s;
    return *this;
}

ivec2& ivec2::operator*=(ivec2 s) {
    x *= s.x;
    y *= s.y;
    return *this;
}

ivec2& ivec2::operator/=(i32 s) {
    x /= s;
    y /= s;
    return *this;
}

ivec2& ivec2::operator/=(ivec2 s) {
    x /= s.x;
    y /= s.y;
    return *this;
}

bool ivec2::operator==(const ivec2& v) const {
    return x == v.x && y == v.y;
}

bool ivec2::operator!=(const ivec2& v) const {
    return !(*this == v);
}

f32 ivec2::length() const {
    return sqrtf((f32)(x * x + y * y));
}

i32 ivec2::length_squared() const {
    return x * x + y * y;
}

i32 ivec2::dot(const ivec2& v) const {
    return x * v.x + y * v.y;
}

// vec3 implementations
vec3::vec3() : x(0), y(0), z(0) {
}

vec3::vec3(f32 v) : x(v), y(v), z(v) {
}

vec3::vec3(f32 x, f32 y, f32 z) : x(x), y(y), z(z) {
}

vec3::vec3(const vec2& v, f32 z) : x(v.x), y(v.y), z(z) {
}

f32& vec3::operator[](usize i) {
    return values[i];
}

const f32& vec3::operator[](usize i) const {
    return values[i];
}

vec3 vec3::operator+(const vec3& v) const {
    return vec3(x + v.x, y + v.y, z + v.z);
}

vec3 vec3::operator-(const vec3& v) const {
    return vec3(x - v.x, y - v.y, z - v.z);
}

vec3 vec3::operator*(f32 s) const {
    return vec3(x * s, y * s, z * s);
}

vec3 vec3::operator/(f32 s) const {
    return vec3(x / s, y / s, z / s);
}

vec3 vec3::operator-() const {
    return vec3(-x, -y, -z);
}

vec3& vec3::operator+=(const vec3& v) {
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
}

vec3& vec3::operator-=(const vec3& v) {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
}

vec3& vec3::operator*=(f32 s) {
    x *= s;
    y *= s;
    z *= s;
    return *this;
}

vec3& vec3::operator/=(f32 s) {
    x /= s;
    y /= s;
    z /= s;
    return *this;
}

f32 vec3::length() const {
    return sqrtf(x * x + y * y + z * z);
}

f32 vec3::length_squared() const {
    return x * x + y * y + z * z;
}

vec3 vec3::normalized() const {
    f32 len = length();
    return len > 0 ? *this / len : vec3(0, 0, 0);
}

void vec3::normalize() {
    *this = normalized();
}

f32 vec3::dot(const vec3& v) const {
    return x * v.x + y * v.y + z * v.z;
}

vec3 vec3::cross(const vec3& v) const {
    return vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
}

// vec4 implementations
vec4::vec4() : x(0), y(0), z(0), w(0) {
}

vec4::vec4(f32 v) : x(v), y(v), z(v), w(v) {
}

vec4::vec4(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {
}

vec4::vec4(const vec3& v, f32 w) : x(v.x), y(v.y), z(v.z), w(w) {
}

f32& vec4::operator[](usize i) {
    return values[i];
}

const f32& vec4::operator[](usize i) const {
    return values[i];
}

vec4 vec4::operator+(const vec4& v) const {
    return vec4(x + v.x, y + v.y, z + v.z, w + v.w);
}

vec4 vec4::operator-(const vec4& v) const {
    return vec4(x - v.x, y - v.y, z - v.z, w - v.w);
}

vec4 vec4::operator*(f32 s) const {
    return vec4(x * s, y * s, z * s, w * s);
}

vec4 vec4::operator/(f32 s) const {
    return vec4(x / s, y / s, z / s, w / s);
}

vec4 vec4::operator-() const {
    return vec4(-x, -y, -z, -w);
}

vec4& vec4::operator+=(const vec4& v) {
    x += v.x;
    y += v.y;
    z += v.z;
    w += v.w;
    return *this;
}

vec4& vec4::operator-=(const vec4& v) {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    w -= v.w;
    return *this;
}

vec4& vec4::operator*=(f32 s) {
    x *= s;
    y *= s;
    z *= s;
    w *= s;
    return *this;
}

vec4& vec4::operator/=(f32 s) {
    x /= s;
    y /= s;
    z /= s;
    w /= s;
    return *this;
}

f32 vec4::length() const {
    return sqrtf(x * x + y * y + z * z + w * w);
}

f32 vec4::length_squared() const {
    return x * x + y * y + z * z + w * w;
}

vec4 vec4::normalized() const {
    f32 len = length();
    return len > 0 ? *this / len : vec4(0, 0, 0, 0);
}

void vec4::normalize() {
    *this = normalized();
}

f32 vec4::dot(const vec4& v) const {
    return x * v.x + y * v.y + z * v.z + w * v.w;
}

// mat4x4 implementations
mat4x4::mat4x4() {
    identity();
}

mat4x4::mat4x4(f32 diagonal) {
    for (i32 i = 0; i < 4; i++)
        values[i] = vec4(0);
    ax = by = cz = dw = diagonal;
}

vec4& mat4x4::operator[](usize i) {
    return values[i];
}

const vec4& mat4x4::operator[](usize i) const {
    return values[i];
}

f32& mat4x4::operator()(usize row, usize col) {
    return values[row][col];
}

const f32& mat4x4::operator()(usize row, usize col) const {
    return values[row][col];
}

mat4x4 mat4x4::operator*(const mat4x4& other) const {
    mat4x4 result;
    for (usize i = 0; i < 4; i++) {
        for (usize j = 0; j < 4; j++) {
            result(i, j) = 0;
            for (usize k = 0; k < 4; k++) {
                result(i, j) += (*this)(i, k) * other(k, j);
            }
        }
    }
    return result;
}

vec4 mat4x4::operator*(const vec4& v) const {
    return vec4(
        values[0].dot(v),
        values[1].dot(v),
        values[2].dot(v),
        values[3].dot(v)
    );
}

void mat4x4::identity() {
    for (i32 i = 0; i < 4; i++)
        values[i] = vec4(0);
    ax = by = cz = dw = 1.0f;
}

mat4x4 mat4x4::transposed() const {
    mat4x4 result;
    for (usize i = 0; i < 4; i++) {
        for (usize j = 0; j < 4; j++) {
            result(i, j) = (*this)(j, i);
        }
    }
    return result;
}

void mat4x4::transpose() {
    *this = transposed();
}

mat4x4 mat4x4::translate(const vec3& t) {
    mat4x4 result;
    result.dx = t.x;
    result.dy = t.y;
    result.dz = t.z;
    return result;
}

mat4x4 mat4x4::scale(const vec3& s) {
    mat4x4 result;
    result.ax = s.x;
    result.by = s.y;
    result.cz = s.z;
    return result;
}

mat4x4 mat4x4::rotate_x(f32 angle) {
    mat4x4 result;
    f32 c = cosf(angle);
    f32 s = sinf(angle);
    result.by = c;
    result.cy = -s;
    result.bz = s;
    result.cz = c;
    return result;
}

mat4x4 mat4x4::rotate_y(f32 angle) {
    mat4x4 result;
    f32 c = cosf(angle);
    f32 s = sinf(angle);
    result.ax = c;
    result.cx = s;
    result.az = -s;
    result.cz = c;
    return result;
}

mat4x4 mat4x4::rotate_z(f32 angle) {
    mat4x4 result;
    f32 c = cosf(angle);
    f32 s = sinf(angle);
    result.ax = c;
    result.bx = -s;
    result.ay = s;
    result.by = c;
    return result;
}

mat4x4 mat4x4::perspective(f32 fov, f32 aspect, f32 z_near, f32 z_far) {
    mat4x4 result(0);
    f32 tan_half_fov = tanf(fov * 0.5f);
    result.ax = 1.0f / (aspect * tan_half_fov);
    result.by = 1.0f / tan_half_fov;
    result.cz = -(z_far + z_near) / (z_far - z_near);
    result.dz = -(2.0f * z_far * z_near) / (z_far - z_near);
    result.cw = -1.0f;
    return result;
}

mat4x4 mat4x4::look_at(const vec3& eye, const vec3& center, const vec3& up) {
    vec3 f = (center - eye).normalized();
    vec3 s = f.cross(up).normalized();
    vec3 u = s.cross(f);

    mat4x4 result;
    result.ax = s.x;
    result.bx = s.y;
    result.cx = s.z;
    result.dx = -s.dot(eye);
    result.ay = u.x;
    result.by = u.y;
    result.cy = u.z;
    result.dy = -u.dot(eye);
    result.az = -f.x;
    result.bz = -f.y;
    result.cz = -f.z;
    result.dz = f.dot(eye);
    result.aw = 0;
    result.bw = 0;
    result.cw = 0;
    result.dw = 1;
    return result;
}

mat4x4 mat4x4::orthographic_projection(
    float left,
    float right,
    float top,
    float bottom
) {
    mat4x4 result(0);
    result.aw = -(right + left) / (right - left);
    result.bw = (top + bottom) / (top - bottom);
    result.cw = 0.0f; // Near Plane
    result[0][0] = 2.0f / (right - left);
    result[1][1] = 2.0f / (top - bottom);
    result[2][2] = 1.0f / (1.0f - 0.0f); // Far and Near
    result[3][3] = 1.0f;

    return result;
}

// Global operator overloads
vec2 operator*(f32 s, const vec2& v) {
    return v * s;
}

vec3 operator*(f32 s, const vec3& v) {
    return v * s;
}

vec4 operator*(f32 s, const vec4& v) {
    return v * s;
}

ivec2 operator*(i32 s, const ivec2& v) {
    return v * s;
}

// Utility functions
f32 radians(f32 degrees) {
    return degrees * 0.017453292519943295f;
}

f32 degrees(f32 radians) {
    return radians * 57.29577951308232f;
}
