#pragma once
#include "cubemap.h"
#include "debug.h"

#include "ibl/brdf_map.h"
#include "ibl/irradiance_map.h"
#include "ibl/prefilter_map.h"

#include "cme_defs.h"
#include "shape/skybox.h"

#include <stdlib.h>
#include <filesystem>
#include <regex>
#include <string>
#include <iostream>

#include "../imguizmo_quat/imGuIZMOquat.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"


namespace Cme
{
    class CommonHelper
    {
    public:

        static inline float lerp(float a, float b, float weight)
        {
            return a + weight * (b - a);
        }

        static inline std::string ltrim(std::string s)
        {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                [](int ch) { return !std::isspace(ch); }));
            return s;
        }

        static inline std::string rtrim(std::string s)
        {
            s.erase(std::find_if(s.rbegin(), s.rend(),
                [](int ch) { return !std::isspace(ch); })
                .base(),
                s.end());
            return s;
        }

        static inline std::string trim(std::string s) { return ltrim(rtrim(s)); }

        static inline bool string_has_suffix(const std::string& str,
            const std::string& suffix)
        {
            return str.size() >= suffix.size() &&
                str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
        }

        static inline std::string resolvePath(std::string const& path)
        {
            std::filesystem::path p = path;
            return std::filesystem::absolute(p).string();
        }

        template <class BidirIt, class Traits, class CharT, class UnaryFunction>
        static std::basic_string<CharT> regexReplace(BidirIt first, BidirIt last,
            const std::basic_regex<CharT, Traits>& re,
            UnaryFunction f)
        {
            std::basic_string<CharT> s;

            typename std::match_results<BidirIt>::difference_type positionOfLastMatch = 0;
            auto endOfLastMatch = first;

            auto callback = [&](const std::match_results<BidirIt>& match)
            {
                auto positionOfThisMatch = match.position(0);
                auto diff = positionOfThisMatch - positionOfLastMatch;

                auto startOfThisMatch = endOfLastMatch;
                std::advance(startOfThisMatch, diff);

                s.append(endOfLastMatch, startOfThisMatch);
                s.append(f(match));

                auto lengthOfMatch = match.length(0);

                positionOfLastMatch = positionOfThisMatch + lengthOfMatch;

                endOfLastMatch = startOfThisMatch;
                std::advance(endOfLastMatch, lengthOfMatch);
            };

            std::sregex_iterator begin(first, last, re), end;
            std::for_each(begin, end, callback);

            s.append(endOfLastMatch, last);

            return s;
        }

        template <class Traits, class CharT, class UnaryFunction>
        static std::string regexReplace(const std::string& s,
            const std::basic_regex<CharT, Traits>& re,
            UnaryFunction f)
        {
            return regexReplace(s.cbegin(), s.cend(), re, f);
        }

        static void imguiHelpMarker(const char* desc);
        // Helper for a float slider value.
        static bool imguiFloatSlider(const char* desc, float* value, float min, float max, const char* fmt = nullptr,  Scale scale = Scale::LINEAR);

        // Helper for an image control.
        static void imguiImage(const Cme::Texture& texture, glm::vec2 size);

        // Returns the number of mips for an image of a given width/height.
        static int calculateNumMips(int width, int height);

        // Returns the next mip size given an initial size.
        static ImageSize calculateNextMip(const ImageSize& mipSize);

        // Returns the calculated size for a mip level.
        static ImageSize calculateMipLevel(int mip0Width, int mip0Height, int level);

        static void PrintMat4(const glm::mat4& mat);

    };
}  

