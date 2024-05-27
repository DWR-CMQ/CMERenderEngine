#include "text.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "../common_helper.h"

//const std::size_t Text::FONT_HEIGHT = 40;

namespace Cme
{
    Text::Text()
    {
        m_VAO = 0;
        m_VBO = 0;
        current_font = 0;
        m_iFontHeight = 40;

        if (m_pDrawShader == nullptr)
        {
            m_pDrawShader = new Shader(Cme::ShaderPath("assets//shaders//text_shader.vert"), Cme::ShaderPath("assets//shaders//text_shader.frag"));
        }

        m_pDrawShader->output("color");

        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);

        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
    }

    std::size_t Text::addFont(const std::string& filePath, std::size_t data_size)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


        FT_Face face;
        FT_Library ft;
        auto ftError = FT_Init_FreeType(&ft);
        if (ftError)
        {
            return false;
        }

        //FT_New_Memory_Face(ft, reinterpret_cast<const FT_Byte*>(data), data_size, 0, &face);
        ftError = FT_New_Face(ft, filePath.c_str(), 0, &face);
        if (ftError)
        {
            return false;
        }

        textures.push_back(std::array<unsigned int, 200>());
        glGenTextures(200, textures.back().data());

        FT_Set_Pixel_Sizes(face, 0, m_iFontHeight);
        for (auto i = 0; i < 200; ++i)
        {
            auto e =
                FT_Load_Char(face, i + 32, FT_LOAD_RENDER);
            if (e != 0) std::cout << "Err: " << e << std::endl;
            glBindTexture(GL_TEXTURE_2D, textures.back()[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, 0x2801, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                face->glyph->bitmap.width, face->glyph->bitmap.rows,
                0, GL_RED, GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer);

            chars.push_back({ face->glyph->bitmap.width, face->glyph->bitmap.rows,
                             face->glyph->bitmap_left, face->glyph->bitmap_top,
                             face->glyph->advance.x >> 6 });
        }

        FT_Done_Face(face);
        FT_Done_FreeType(ft);

        return textures.size() - 1;
    }

    bool Text::activateFont(std::size_t index)
    {
        if (index < textures.size())
        {
            current_font = index;
            return true;
        }
        return false;
    }

    Text::~Text()
    {
        for (auto& t : textures)
        {
            glDeleteTextures(128, t.data());
        }
            
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
    }

    void Text::Render(std::string const& text, Anchor anchor, std::shared_ptr<Cme::Camera> spCamera)
    {
        if (textures.empty())
        {
            return;
        }
        auto iX = m_iFontX;
        auto iY = m_iFontY;

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);

        int w, h;
        glfwGetFramebufferSize(glfwGetCurrentContext(), &w, &h);
        // 屏幕坐标系需要转到视口坐标系下才能实现3D效果
        glm::mat4 mat4Screen2NDC = glm::mat4(1.0f);
        mat4Screen2NDC[0][0] = 2.0f / w;
        mat4Screen2NDC[0][1] = 0.0f;
        mat4Screen2NDC[0][2] = 0.0f;
        mat4Screen2NDC[0][3] = -1.0f;

        mat4Screen2NDC[1][0] = 0.0f;
        mat4Screen2NDC[1][1] = -2.0f / h;
        mat4Screen2NDC[1][2] = 0.0f;
        mat4Screen2NDC[1][3] = 1.0f;

        mat4Screen2NDC[2][0] = 0.0f;
        mat4Screen2NDC[2][1] = 0.0f;
        mat4Screen2NDC[2][2] = 0.0f;
        mat4Screen2NDC[2][3] = 1.0f;

        mat4Screen2NDC[3][0] = 0.0f;
        mat4Screen2NDC[3][1] = 0.0f;
        mat4Screen2NDC[3][2] = 0.0f;
        mat4Screen2NDC[3][3] = 1.0f;
        // CommonHelper::PrintMat4(mat4Screen2NDC);

        float xoff, yoff = h - iY;
        auto width = 0.0f, height = 0.0f;
        for (const auto& c : text)
        {
            auto i = c - 32;
            if (i < 0 || i > 94)
                continue;
            width += chars[i].advance * m_iScale;
            if (chars[i].size_y > height)
            {
                height = chars[i].size_y;
            }
        }

        switch (static_cast<int>(anchor) / 10)
        {
        case 2:
            xoff = -width / 2.0f;
            break;
        case 3:
            xoff = -width;
            break;
        default:
            xoff = 0;
        }

        switch (static_cast<int>(anchor) - static_cast<int>(anchor) / 10 * 10)
        {
        case 1:
            yoff -= height * m_iScale;
            break;
        case 2:
            yoff -= height * m_iScale / 2.0f;
            break;
        default:
            break;
        }

        m_pDrawShader->activate();

        m_pDrawShader->setVec4("text_color", glm::vec4(m_vec3FontColor, 1.0f));
        // 贴于屏幕
        //m_pDrawShader->setMat4("proj", glm::ortho(0.0f, static_cast<float>(w), 0.0f, static_cast<float>(h)));
        // 立体效果
        m_pDrawShader->setMat4("proj", spCamera->getProjectionTransform());
        m_pDrawShader->setMat4("view", spCamera->getViewTransform());
        m_pDrawShader->setMat4("dc2ndc", glm::transpose(mat4Screen2NDC));
       
        auto model = glm::mat4(1.0f);
        // 翻转成正
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        // 绕Z轴旋转
        model = glm::rotate(model, (float)glfwGetTime() * glm::radians(30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        m_pDrawShader->setMat4("model", model);

        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(m_VAO);
        for (const auto& c : text)
        {
            auto i = c - 32;
            if (i < 0 || i > 94)
            {
                continue;
            }
            auto xpos = xoff + iX + chars[i].bearing_x * m_iScale;
            auto ypos = yoff - (chars[i].size_y - chars[i].bearing_y) * m_iScale;
            auto w = chars[i].size_x * m_iScale;
            auto h = chars[i].size_y * m_iScale;
            float vertices[] =
            {
                    xpos,   ypos + h, 0.0f, 0.0f,
                    xpos,   ypos,   0.0f, 1.0f,
                    xpos + w, ypos,   1.0f, 1.0f,
                    xpos,   ypos + h, 0.0f, 0.0f,
                    xpos + w, ypos,   1.0f, 1.0f,
                    xpos + w, ypos + h, 1.0f, 0.0f
            };

            glBindTexture(GL_TEXTURE_2D, textures[current_font][i]);
            glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

            glDrawArrays(GL_TRIANGLES, 0, 6);

            iX += chars[i].advance * m_iScale;
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        m_pDrawShader->deactivate();

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
    }

    void Text::UpdateFont(int iX, int iY, int iScale, glm::vec3 color)
    {
        m_iFontX = iX;
        m_iFontY = iY;
        m_vec3FontColor = color;
        m_iScale = iScale;
    }
}
