#include "common_helper.h"

namespace Cme
{
    void CommonHelper::LoadSkyboxImage(
        SkyboxImage skyboxImage, Cme::SkyboxMesh& skybox,
        Cme::EquirectCubemap& equirectCubemapConverter,
        Cme::IrradianceMap& irradianceCalculator,
        Cme::PrefilterMap& prefilteredEnvMapCalculator)
    {
        std::string hdrPath;
        switch (skyboxImage)
        {
        case SkyboxImage::ALEXS_APT:
            hdrPath = "assets//models//ibl/AlexsApt.hdr";
            break;
        case SkyboxImage::FROZEN_WATERFALL:
            hdrPath = "assets//models//ibl//FrozenWaterfall.hdr";
            break;
        case SkyboxImage::KLOPPENHEIM:
            hdrPath = "assets//models//ibl//Kloppenheim.hdr";
            break;
        case SkyboxImage::MILKYWAY:
            hdrPath = "assets//models//ibl//Milkyway.hdr";
            break;
        case SkyboxImage::MON_VALLEY:
            hdrPath = "assets//models//ibl//MonValley.hdr";
            break;
        case SkyboxImage::UENO_SHRINE:
            hdrPath = "assets//models//ibl//UenoShrine.hdr";
            break;
        case SkyboxImage::WINTER_FOREST:
            hdrPath = "assets//models//ibl//WinterForest.hdr";
            break;
        }

        Cme::Texture hdr = Cme::Texture::LoadHDR(hdrPath.c_str());

        // Process HDR cubemap
        {
            Cme::DebugGroup debugGroup("HDR equirect to cubemap");
            equirectCubemapConverter.multipassDraw(hdr);
        }
        auto cubemap = equirectCubemapConverter.getCubemap();
        {
            Cme::DebugGroup debugGroup("Irradiance calculation");
            irradianceCalculator.multipassDraw(cubemap);
        }
        {
            Cme::DebugGroup debugGroup("Prefiltered env map calculation");
            prefilteredEnvMapCalculator.multipassDraw(cubemap);
        }

        skybox.setTexture(cubemap);

        // Don't need this anymore.
        hdr.free();
    }

    // Helper to display a little (?) mark which shows a tooltip when hovered.
    void CommonHelper::imguiHelpMarker(const char* desc)
    {
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

    // Helper for a float slider value.
    bool CommonHelper::imguiFloatSlider(const char* desc, float* value, float min,
        float max, const char* fmt, Scale scale)
    {
        ImGuiSliderFlags flags = ImGuiSliderFlags_None;
        if (scale == Scale::LOG)
        {
            flags = ImGuiSliderFlags_Logarithmic;
        }
        return ImGui::SliderScalar(desc, ImGuiDataType_Float, value, &min, &max, fmt, flags);
    }

    // Helper for an image control.
    void CommonHelper::imguiImage(const Cme::Texture& texture, glm::vec2 size)
    {
        ImTextureID texID = reinterpret_cast<void*>(texture.getId());
        // Flip the image.
        ImGui::Image(texID, ImVec2(size.x, size.y), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
        //ImGui::Image(texID, size, /*uv0=*/glm::vec2(0.0f, 1.0f),
        //ImGui::Image(texID, ImGui::GetContentRegionAvail());
    }
}