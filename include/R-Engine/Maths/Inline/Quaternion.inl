#pragma once

namespace r {

inline constexpr Quaternion::Quaternion() noexcept : x(0.f), y(0.f), z(0.f), w(1.f)
{
}

inline constexpr Quaternion::Quaternion(f32 px, f32 py, f32 pz, f32 pw) noexcept : x(px), y(py), z(pz), w(pw)
{
}

inline Quaternion Quaternion::from_euler(const Vec3f &euler) noexcept
{
    const Quaternion qx(std::sin(euler.x * 0.5f), 0.f, 0.f, std::cos(euler.x * 0.5f));
    const Quaternion qy(0.f, std::sin(euler.y * 0.5f), 0.f, std::cos(euler.y * 0.5f));
    const Quaternion qz(0.f, 0.f, std::sin(euler.z * 0.5f), std::cos(euler.z * 0.5f));

    return qz * qy * qx;
}

inline Vec3f Quaternion::to_euler() const noexcept
{
    Vec3f euler;

    /* Roll (x-axis rotation) */
    const f32 sinr_cosp = 2.f * (w * x + y * z);
    const f32 cosr_cosp = 1.f - 2.f * (x * x + y * y);
    euler.x = std::atan2(sinr_cosp, cosr_cosp);

    /* Pitch (y-axis rotation) */
    const f32 sinp = 2.f * (w * y - z * x);
    if (std::abs(sinp) >= 1.f) {
        euler.y = std::copysign(R_PI / 2.0f, sinp); /* use 90 degrees if out of range */
    } else {
        euler.y = std::asin(sinp);
    }

    /* Yaw (z-axis rotation) */
    const f32 siny_cosp = 2.f * (w * z + x * y);
    const f32 cosy_cosp = 1.f - 2.f * (y * y + z * z);
    euler.z = std::atan2(siny_cosp, cosy_cosp);

    return euler;
}

inline constexpr Quaternion Quaternion::operator*(const Quaternion &rhs) const noexcept
{
    return Quaternion(w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y, w * rhs.y - x * rhs.z + y * rhs.w + z * rhs.x,
        w * rhs.z + x * rhs.y - y * rhs.x + z * rhs.w, w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z);
}

inline Vec3f Quaternion::rotate(const Vec3f &v) const noexcept
{
    const Vec3f q_vec = {x, y, z};
    const Vec3f uv = q_vec.cross(v);
    const Vec3f uuv = q_vec.cross(uv);
    return v + (uv * w + uuv) * 2.f;
}

inline void Quaternion::normalize() noexcept
{
    const f32 mag = std::sqrt(x * x + y * y + z * z + w * w);
    if (mag > 0.0f) {
        const f32 inv_mag = 1.0f / mag;
        x *= inv_mag;
        y *= inv_mag;
        z *= inv_mag;
        w *= inv_mag;
    }
}

}// namespace r
