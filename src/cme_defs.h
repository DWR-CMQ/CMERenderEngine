#pragma once
namespace Cme
{
    enum class CameraControlType
    {
        FLY = 0,
        ORBIT,
    };

    enum class LightingModel
    {
        BLINN_PHONG = 0,
        COOK_TORRANCE_GGX,
    };

    enum class GBufferVis
    {
        DISABLED = 0,
        POSITIONS,
        AO,
        NORMALS,
        ROUGHNESS,
        ALBEDO,
        METALLIC,
        EMISSION,
    };

    enum class ToneMapping
    {
        NONE = 0,
        REINHARD,
        REINHARD_LUMINANCE,
        ACES_APPROX,
        AMD,
    };

    enum class SkyboxImage
    {
        ALEXS_APT = 0,
        FROZEN_WATERFALL,
        KLOPPENHEIM,
        MILKYWAY,
        MON_VALLEY,
        UENO_SHRINE,
        WINTER_FOREST,
        Six_Face
    };

    enum class Scale
    {
        LINEAR = 0,
        LOG,
    };
}