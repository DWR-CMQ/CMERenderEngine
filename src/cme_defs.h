#pragma once
#include <glm/glm.hpp>
#include <glm/fwd.hpp>
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

    // Options for the model render UI. The defaults here are used at startup.
    struct ModelRenderOptions
    {
        // Model.
        glm::quat modelRotation = glm::identity<glm::quat>();
        float modelScale = 1.0f;

        // Rendering.
        LightingModel lightingModel = LightingModel::COOK_TORRANCE_GGX;

        glm::vec3 directionalDiffuse = glm::vec3(0.5f);
        glm::vec3 directionalSpecular = glm::vec3(0.5f);
        float directionalIntensity = 10.0f;
        glm::vec3 directionalDirection = glm::normalize(glm::vec3(-0.2f, -1.0f, -0.3f));

        bool shadowMapping = false;
        float shadowCameraCuboidExtents = 2.0f;
        float shadowCameraNear = 0.1f;
        float shadowCameraFar = 15.0f;
        float shadowCameraDistance = 5.0f;
        float shadowBiasMin = 0.0001;
        float shadowBiasMax = 0.001;

        SkyboxImage skyboxImage = SkyboxImage::WINTER_FOREST;

        bool useIBL = true;
        glm::vec3 ambientColor = glm::vec3(0.1f);
        bool ssao = false;
        float ssaoRadius = 0.5f;
        float ssaoBias = 0.025f;
        float shininess = 32.0f;
        float emissionIntensity = 5.0f;
        glm::vec3 emissionAttenuation = glm::vec3(0, 0, 1.0f);

        bool bloom = true;
        float bloomMix = 0.004;
        ToneMapping toneMapping = ToneMapping::ACES_APPROX;
        bool gammaCorrect = true;
        float gamma = 2.2f;

        bool fxaa = true;

        // Camera.
        CameraControlType cameraControlType = CameraControlType::ORBIT;
        float speed = 0;
        float sensitivity = 0;
        float fov = 0;
        float near = 0;
        float far = 0;
        bool captureMouse = false;

        // Debug.
        GBufferVis gBufferVis = GBufferVis::DISABLED;
        bool wireframe = false;
        bool drawNormals = false;

        // Performance.
        const float* frameDeltas = nullptr;
        int numFrameDeltas = 0;
        int frameDeltasOffset = 0;
        float avgFPS = 0;
        bool enableVsync = true;
    };
}