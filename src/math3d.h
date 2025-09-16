#pragma once

#include "types.h"
#include <math.h>

struct vec2 {
    union {
        f32 values[2];
        struct {
            f32 x, y;
        };
    };

    vec2() : x(0), y(0) {}
    vec2(f32 v) : x(v), y(v) {}
    vec2(f32 x, f32 y) : x(x), y(y) {}

    f32& operator[](i32 i) { return values[i]; }
    const f32& operator[](i32 i) const { return values[i]; }

    vec2 operator+(const vec2& v) const { return vec2(x + v.x, y + v.y); }
    vec2 operator-(const vec2& v) const { return vec2(x - v.x, y - v.y); }
    vec2 operator*(f32 s) const { return vec2(x * s, y * s); }
    vec2 operator/(f32 s) const { return vec2(x / s, y / s); }
    vec2 operator-() const { return vec2(-x, -y); }

    vec2& operator+=(const vec2& v) {
        x += v.x;
        y += v.y;
        return *this;
    }
    vec2& operator-=(const vec2& v) {
        x -= v.x;
        y -= v.y;
        return *this;
    }
    vec2& operator*=(f32 s) {
        x *= s;
        y *= s;
        return *this;
    }
    vec2& operator/=(f32 s) {
        x /= s;
        y /= s;
        return *this;
    }

    f32 length() const { return sqrtf(x * x + y * y); }
    f32 length_squared() const { return x * x + y * y; }
    vec2 normalized() const {
        f32 len = length();
        return len > 0 ? *this / len : vec2(0, 0);
    }
    void normalize() { *this = normalized(); }
    f32 dot(const vec2& v) const { return x * v.x + y * v.y; }
};

struct ivec2 {
    union {
        i32 values[2];
        struct {
            i32 x, y;
        };
    };

    ivec2() : x(0), y(0) {}
    ivec2(i32 v) : x(v), y(v) {}
    ivec2(i32 x, i32 y) : x(x), y(y) {}
    ivec2(const vec2& v) : x((i32)v.x), y((i32)v.y) {}

    i32& operator[](i32 i) { return (&x)[i]; }
    const i32& operator[](i32 i) const { return (&x)[i]; }

    ivec2 operator+(const ivec2& v) const { return ivec2(x + v.x, y + v.y); }
    ivec2 operator-(const ivec2& v) const { return ivec2(x - v.x, y - v.y); }
    ivec2 operator*(i32 s) const { return ivec2(x * s, y * s); }
    ivec2 operator/(i32 s) const { return ivec2(x / s, y / s); }
    ivec2 operator-() const { return ivec2(-x, -y); }

    ivec2& operator+=(const ivec2& v) {
        x += v.x;
        y += v.y;
        return *this;
    }
    ivec2& operator-=(const ivec2& v) {
        x -= v.x;
        y -= v.y;
        return *this;
    }
    ivec2& operator*=(i32 s) {
        x *= s;
        y *= s;
        return *this;
    }
    ivec2& operator/=(i32 s) {
        x /= s;
        y /= s;
        return *this;
    }

    bool operator==(const ivec2& v) const { return x == v.x && y == v.y; }
    bool operator!=(const ivec2& v) const { return !(*this == v); }

    f32 length() const { return sqrtf((f32)(x * x + y * y)); }
    i32 length_squared() const { return x * x + y * y; }
    i32 dot(const ivec2& v) const { return x * v.x + y * v.y; }
    
    vec2 to_vec2() const { return vec2((f32)x, (f32)y); }
};

struct vec3 {
    union {
        f32 values[3];
        struct {
            f32 x, y, z;
        };
        struct {
            f32 r, g, b;
        };
    };

    vec3() : x(0), y(0), z(0) {}
    vec3(f32 v) : x(v), y(v), z(v) {}
    vec3(f32 x, f32 y, f32 z) : x(x), y(y), z(z) {}
    vec3(const vec2& v, f32 z) : x(v.x), y(v.y), z(z) {}

    f32& operator[](usize i) { return values[i]; }
    const f32& operator[](usize i) const { return values[i]; }

    vec3 operator+(const vec3& v) const {
        return vec3(x + v.x, y + v.y, z + v.z);
    }
    vec3 operator-(const vec3& v) const {
        return vec3(x - v.x, y - v.y, z - v.z);
    }
    vec3 operator*(f32 s) const { return vec3(x * s, y * s, z * s); }
    vec3 operator/(f32 s) const { return vec3(x / s, y / s, z / s); }
    vec3 operator-() const { return vec3(-x, -y, -z); }

    vec3& operator+=(const vec3& v) {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }
    vec3& operator-=(const vec3& v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        return *this;
    }
    vec3& operator*=(f32 s) {
        x *= s;
        y *= s;
        z *= s;
        return *this;
    }
    vec3& operator/=(f32 s) {
        x /= s;
        y /= s;
        z /= s;
        return *this;
    }

    f32 length() const { return sqrtf(x * x + y * y + z * z); }
    f32 length_squared() const { return x * x + y * y + z * z; }
    vec3 normalized() const {
        f32 len = length();
        return len > 0 ? *this / len : vec3(0, 0, 0);
    }
    void normalize() { *this = normalized(); }
    f32 dot(const vec3& v) const { return x * v.x + y * v.y + z * v.z; }
    vec3 cross(const vec3& v) const {
        return vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
    }
};

struct vec4 {
    union {
        f32 values[4];
        struct {
            f32 x, y, z, w;
        };
        struct {
            f32 r, g, b, a;
        };
    };

    vec4() : x(0), y(0), z(0), w(0) {}
    vec4(f32 v) : x(v), y(v), z(v), w(v) {}
    vec4(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {}
    vec4(const vec3& v, f32 w) : x(v.x), y(v.y), z(v.z), w(w) {}

    f32& operator[](usize i) { return values[i]; }
    const f32& operator[](usize i) const { return values[i]; }

    vec4 operator+(const vec4& v) const {
        return vec4(x + v.x, y + v.y, z + v.z, w + v.w);
    }
    vec4 operator-(const vec4& v) const {
        return vec4(x - v.x, y - v.y, z - v.z, w - v.w);
    }
    vec4 operator*(f32 s) const { return vec4(x * s, y * s, z * s, w * s); }
    vec4 operator/(f32 s) const { return vec4(x / s, y / s, z / s, w / s); }
    vec4 operator-() const { return vec4(-x, -y, -z, -w); }

    vec4& operator+=(const vec4& v) {
        x += v.x;
        y += v.y;
        z += v.z;
        w += v.w;
        return *this;
    }
    vec4& operator-=(const vec4& v) {
        x -= v.x;
        y -= v.y;
        z -= v.z;
        w -= v.w;
        return *this;
    }
    vec4& operator*=(f32 s) {
        x *= s;
        y *= s;
        z *= s;
        w *= s;
        return *this;
    }
    vec4& operator/=(f32 s) {
        x /= s;
        y /= s;
        z /= s;
        w /= s;
        return *this;
    }

    f32 length() const { return sqrtf(x * x + y * y + z * z + w * w); }
    f32 length_squared() const { return x * x + y * y + z * z + w * w; }
    vec4 normalized() const {
        f32 len = length();
        return len > 0 ? *this / len : vec4(0, 0, 0, 0);
    }
    void normalize() { *this = normalized(); }
    f32 dot(const vec4& v) const {
        return x * v.x + y * v.y + z * v.z + w * v.w;
    }
};

struct mat4x4 {
    union {
        vec4 values[4];
        struct {
            float ax, bx, cx, dx;
            float ay, by, cy, dy;
            float az, bz, cz, dz;
            float aw, bw, cw, dw;
        };
    };

    mat4x4() { identity(); }
    mat4x4(f32 diagonal) {
        for (i32 i = 0; i < 4; i++)
            values[i] = vec4(0);
        ax = ay = az = aw = diagonal;
    }

    vec4& operator[](usize i) { return values[i]; }
    const vec4& operator[](usize i) const { return values[i]; }

    f32& operator()(usize row, usize col) { return values[row][col]; }
    const f32& operator()(usize row, usize col) const { return values[row][col]; }

    mat4x4 operator*(const mat4x4& other) const {
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

    vec4 operator*(const vec4& v) const {
        return vec4(
            values[0].dot(v),
            values[1].dot(v),
            values[2].dot(v),
            values[3].dot(v)
        );
    }

    void identity() {
        for (i32 i = 0; i < 4; i++)
            values[i] = vec4(0);
        ax = by = cz = dw = 1.0f;
    }

    mat4x4 transposed() const {
        mat4x4 result;
        for (usize i = 0; i < 4; i++) {
            for (usize j = 0; j < 4; j++) {
                result(i, j) = (*this)(j, i);
            }
        }
        return result;
    }

    void transpose() { *this = transposed(); }

    static mat4x4 translate(const vec3& t) {
        mat4x4 result;
        result.dx = t.x;
        result.dy = t.y;
        result.dz = t.z;
        return result;
    }

    static mat4x4 scale(const vec3& s) {
        mat4x4 result;
        result.ax = s.x;
        result.by = s.y;
        result.cz = s.z;
        return result;
    }

    static mat4x4 rotate_x(f32 angle) {
        mat4x4 result;
        f32 c = cosf(angle);
        f32 s = sinf(angle);
        result.by = c;
        result.cy = -s;
        result.bz = s;
        result.cz = c;
        return result;
    }

    static mat4x4 rotate_y(f32 angle) {
        mat4x4 result;
        f32 c = cosf(angle);
        f32 s = sinf(angle);
        result.ax = c;
        result.cx = s;
        result.az = -s;
        result.cz = c;
        return result;
    }

    static mat4x4 rotate_z(f32 angle) {
        mat4x4 result;
        f32 c = cosf(angle);
        f32 s = sinf(angle);
        result.ax = c;
        result.bx = -s;
        result.ay = s;
        result.by = c;
        return result;
    }

    static mat4x4 perspective(f32 fov, f32 aspect, f32 near, f32 far) {
        mat4x4 result(0);
        f32 tan_half_fov = tanf(fov * 0.5f);
        result.ax = 1.0f / (aspect * tan_half_fov);
        result.by = 1.0f / tan_half_fov;
        result.cz = -(far + near) / (far - near);
        result.dz = -(2.0f * far * near) / (far - near);
        result.cw = -1.0f;
        return result;
    }

    static mat4x4 look_at(const vec3& eye, const vec3& center, const vec3& up) {
        vec3 f = (center - eye).normalized();
        vec3 s = f.cross(up).normalized();
        vec3 u = s.cross(f);

        mat4x4 result;
        result.ax = s.x; result.bx = s.y; result.cx = s.z; result.dx = -s.dot(eye);
        result.ay = u.x; result.by = u.y; result.cy = u.z; result.dy = -u.dot(eye);
        result.az = -f.x; result.bz = -f.y; result.cz = -f.z; result.dz = f.dot(eye);
        result.aw = 0; result.bw = 0; result.cw = 0; result.dw = 1;
        return result;
    }

    static mat4x4 orthographic_projection(float left, float right, float top, float bottom) {
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
};

inline vec2 operator*(f32 s, const vec2& v) { return v * s; }
inline vec3 operator*(f32 s, const vec3& v) { return v * s; }
inline vec4 operator*(f32 s, const vec4& v) { return v * s; }
inline ivec2 operator*(i32 s, const ivec2& v) { return v * s; }

inline f32 radians(f32 degrees) { return degrees * 0.017453292519943295f; }
inline f32 degrees(f32 radians) { return radians * 57.29577951308232f; }
