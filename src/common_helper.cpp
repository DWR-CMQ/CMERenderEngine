#include "common_helper.h"

namespace Cme
{
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

    int CommonHelper::calculateNumMips(int width, int height)
    {
        return 1 + static_cast<int>(std::floor(std::log2(std::max(width, height))));
    }

    ImageSize CommonHelper::calculateNextMip(const ImageSize& mipSize)
    {
        ImageSize temp;
        temp.width = std::max(mipSize.width / 2, 1);
        temp.height = std::max(mipSize.height / 2, 1);
        return temp;
    }

    ImageSize CommonHelper::calculateMipLevel(int mip0Width, int mip0Height, int level)
    {
        ImageSize size = { mip0Width, mip0Height };
        if (level == 0)
        {
            return size;
        }
        for (int mip = 0; mip < level; ++mip)
        {
            size = calculateNextMip(size);
        }
        return size;
    }

    void CommonHelper::PrintMat4(const glm::mat4& mat)
    {
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                std::cout << mat[i][j] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
}