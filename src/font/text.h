#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../shader/shader.h"
#include "../camera.h"

#include <vector>
#include <array>

namespace Cme
{
    enum class Anchor : int
    {
        LeftTop = 11,
        LeftCenter = 12,
        LeftBottom = 13,
        CenterTop = 21,
        Center = 22,
        CenterBottom = 23,
        RightTop = 31,
        RightCenetr = 32,
        RightBottom = 33
    };

	class Text
	{
    public:
        static const std::size_t FONT_HEIGHT;

        struct Character
        {
            unsigned int size_x;
            unsigned int size_y;
            int bearing_x;
            int bearing_y;
            long int advance;
        };

        Text();
        ~Text();

        void Render(std::string const& text, Anchor anchor, std::shared_ptr<Cme::Camera> spCamera);

        void setFontHeight(std::size_t font_height) 
        {
            m_iFontHeight = font_height;
        }

        inline std::size_t const& fontHeight() const 
        { 
            return m_iFontHeight; 
        }

        void UpdateFont(int iX, int iY, int iScale, glm::vec3 color);

        std::size_t addFont(const std::string& filePath, std::size_t data_size);
        bool activateFont(std::size_t index);

    protected:
        std::vector<std::array<unsigned, 200>> textures;
        std::vector<Character> chars;
        GLuint m_VAO;
        GLuint m_VBO;
        Shader* m_pDrawShader = nullptr;

        std::size_t current_font;

    private:
        int m_iFontHeight;
        int m_iFontX = 0;
        int m_iFontY = 0;
        int m_iScale = 2;
        glm::vec3 m_vec3FontColor = glm::vec3(1.0f, 0.0f, 0.0f);
	};
}


