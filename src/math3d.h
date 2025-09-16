#pragma once

#include "types.h"

struct vec2 {
    union {
        f32 values[2];
        struct {
            f32 x, y;
        };
    };

    vec2();
    vec2(f32 v);
    vec2(f32 x, f32 y);

    f32& operator[](i32 i);
    const f32& operator[](i32 i) const;

    vec2 operator+(const vec2& v) const;
    vec2 operator-(const vec2& v) const;
    vec2 operator*(f32 s) const;
    vec2 operator/(f32 s) const;
    vec2 operator-() const;

    vec2& operator+=(const vec2& v);
    vec2& operator-=(const vec2& v);
    vec2& operator*=(f32 s);
    vec2& operator/=(f32 s);

    f32 length() const;
    f32 length_squared() const;
    vec2 normalized() const;
    void normalize();
    f32 dot(const vec2& v) const;
};

struct ivec2 {
    union {
        i32 values[2];
        struct {
            i32 x, y;
        };
    };

    ivec2();
    ivec2(i32 v);
    ivec2(i32 x, i32 y);
    ivec2(const vec2& v);

    i32& operator[](i32 i);
    const i32& operator[](i32 i) const;

    ivec2 operator+(const ivec2& v) const;
    ivec2 operator-(const ivec2& v) const;
    ivec2 operator*(i32 s) const;
    ivec2 operator/(i32 s) const;
    ivec2 operator-() const;

    ivec2& operator+=(const ivec2& v);
    ivec2& operator-=(const ivec2& v);
    ivec2& operator*=(i32 s);
    ivec2& operator/=(i32 s);

    bool operator==(const ivec2& v) const;
    bool operator!=(const ivec2& v) const;

    f32 length() const;
    i32 length_squared() const;
    i32 dot(const ivec2& v) const;
    vec2 to_vec2() const;
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

    vec3();
    vec3(f32 v);
    vec3(f32 x, f32 y, f32 z);
    vec3(const vec2& v, f32 z);

    f32& operator[](usize i);
    const f32& operator[](usize i) const;

    vec3 operator+(const vec3& v) const;
    vec3 operator-(const vec3& v) const;
    vec3 operator*(f32 s) const;
    vec3 operator/(f32 s) const;
    vec3 operator-() const;

    vec3& operator+=(const vec3& v);
    vec3& operator-=(const vec3& v);
    vec3& operator*=(f32 s);
    vec3& operator/=(f32 s);

    f32 length() const;
    f32 length_squared() const;
    vec3 normalized() const;
    void normalize();
    f32 dot(const vec3& v) const;
    vec3 cross(const vec3& v) const;
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

    vec4();
    vec4(f32 v);
    vec4(f32 x, f32 y, f32 z, f32 w);
    vec4(const vec3& v, f32 w);

    f32& operator[](usize i);
    const f32& operator[](usize i) const;

    vec4 operator+(const vec4& v) const;
    vec4 operator-(const vec4& v) const;
    vec4 operator*(f32 s) const;
    vec4 operator/(f32 s) const;
    vec4 operator-() const;

    vec4& operator+=(const vec4& v);
    vec4& operator-=(const vec4& v);
    vec4& operator*=(f32 s);
    vec4& operator/=(f32 s);

    f32 length() const;
    f32 length_squared() const;
    vec4 normalized() const;
    void normalize();
    f32 dot(const vec4& v) const;
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

    mat4x4();
    mat4x4(f32 diagonal);

    vec4& operator[](usize i);
    const vec4& operator[](usize i) const;

    f32& operator()(usize row, usize col);
    const f32& operator()(usize row, usize col) const;

    mat4x4 operator*(const mat4x4& other) const;
    vec4 operator*(const vec4& v) const;

    void identity();
    mat4x4 transposed() const;
    void transpose();

    static mat4x4 translate(const vec3& t);
    static mat4x4 scale(const vec3& s);
    static mat4x4 rotate_x(f32 angle);
    static mat4x4 rotate_y(f32 angle);
    static mat4x4 rotate_z(f32 angle);
    static mat4x4 perspective(f32 fov, f32 aspect, f32 near, f32 far);
    static mat4x4 look_at(const vec3& eye, const vec3& center, const vec3& up);
    static mat4x4 orthographic_projection(float left, float right, float top, float bottom);
};

// Global operator overloads
vec2 operator*(f32 s, const vec2& v);
vec3 operator*(f32 s, const vec3& v);
vec4 operator*(f32 s, const vec4& v);
ivec2 operator*(i32 s, const ivec2& v);

// Utility functions
f32 radians(f32 degrees);
f32 degrees(f32 radians);
