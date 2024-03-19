#ifndef QUARKGL_WINDOW_H_
#define QUARKGL_WINDOW_H_

// clang-format off
// Must precede glfw/glad, to include OpenGL functions.
#include "core.h"
// clang-format on

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include "camera.h"
#include "exceptions.h"
#include "screen.h"
#include "shader/shader.h"

#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include <tuple>
#include <vector>

namespace Cme 
{

    class WindowException : public QuarkException 
    {
        using QuarkException::QuarkException;
    };

    constexpr int DEFAULT_WIDTH = 800;
    constexpr int DEFAULT_HEIGHT = 600;
    constexpr char const* DEFAULT_TITLE = "quarkGL";
    const glm::vec4 DEFAULT_CLEAR_COLOR = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

    // Controls special convenience behavior when Esc is pressed.
    enum class EscBehavior 
    {
        NONE,
        TOGGLE_MOUSE_CAPTURE,
        CLOSE,
        // Uncaptures the mouse if it is captured, or else closes the app.
        UNCAPTURE_MOUSE_OR_CLOSE,
    };

    // Controls special convenience behavior when the LMB is pressed.
    enum class MouseButtonBehavior
    {
        NONE,
        CAPTURE_MOUSE,
    };

    class Window : public UniformSource 
    {
    public:
        Window(int width = DEFAULT_WIDTH, int height = DEFAULT_HEIGHT,
                const char* title = DEFAULT_TITLE, bool fullscreen = false,
                int samples = 0);
        virtual ~Window();
        GLFWwindow* getGlfwRef() const { return m_pWindow; }

        // TODO: Should this be called something different, and 'activate' be used for
        // setViewport?
        void activate();

        void setViewport()
        {
            ImageSize size = getSize();
            glViewport(0, 0, size.width, size.height);
        }

        void enableVsync()
        {
            activate();
            glfwSwapInterval(1);
        }
        void disableVsync()
        {
            activate();
            glfwSwapInterval(0);
        }

        // TODO: Extract all these as a "Context" object.
        void enableDepthTest()
        {
            glEnable(GL_DEPTH_TEST);
            m_bDepthTestEnabled = true;
        }

        void disableDepthTest()
        {
            glDisable(GL_DEPTH_TEST);
            m_bDepthTestEnabled = false;
        }

        // TODO: Consider extracting stencil logic out to a separate class.
        void enableStencilTest()
        {
            glEnable(GL_STENCIL_TEST);
            // Only replace the value in the stencil buffer if both the stencil and
            // depth test pass.
            glStencilOp(/*sfail=*/GL_KEEP, /*dpfail=*/GL_KEEP, /*dppass=*/GL_REPLACE);
            m_bStencilTestEnabled = true;
        }

        void disableStencilTest()
        {
            glDisable(GL_STENCIL_TEST);
            m_bStencilTestEnabled = false;
        }

        void enableStencilUpdates() { glStencilMask(0xFF); }
        void disableStencilUpdates() { glStencilMask(0x00); }

        void stencilAlwaysDraw() { setStencilFunc(GL_ALWAYS); }
        void stencilDrawWhenMatching() { setStencilFunc(GL_EQUAL); }
        void stencilDrawWhenNotMatching() { setStencilFunc(GL_NOTEQUAL); }
        void setStencilFunc(GLenum func) 
        {
            // Set the stencil test to use the given `func` when comparing for fragment
            // liveness.
            glStencilFunc(func, /*ref=*/1, /*mask=*/0xFF);
        }

        // TODO: Consider extracting blending logic.
        void enableAlphaBlending()
        {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBlendEquation(GL_FUNC_ADD);
        }
        void disableAlphaBlending() { glDisable(GL_BLEND); }

        void enableFaceCull() { glEnable(GL_CULL_FACE); }
        void disableFaceCull() { glDisable(GL_CULL_FACE); }

        void enableWireframe() { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }
        void disableWireframe() { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); }

        void enableSeamlessCubemap() { glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); }
        void disableSeamlessCubemap() { glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS); }

        void cullFrontFaces() { glCullFace(GL_FRONT); }
        void cullBackFaces() { glCullFace(GL_BACK); }

        void updateUniforms(Shader& shader);

        ImageSize getSize() const;
        void setSize(int width, int height);
        void enableResizeUpdates();
        void disableResizeUpdates();
        float getAspectRatio() const;

        unsigned int getFrameCount() const { return m_uiFrameCount; }
        const float* getFrameDeltas() const;
        int getNumFrameDeltas() const;
        int getFrameDeltasOffset() const;
        float getAvgFPS() const;
        glm::vec4 getClearColor() const { return m_vecClearColor; }
        void setClearColor(glm::vec4 color) { m_vecClearColor = color; }

        void makeFullscreen();
        void makeWindowed();

        EscBehavior getEscBehavior() const { return e_eEscBehavior; }
        void setEscBehavior(EscBehavior behavior) { e_eEscBehavior = behavior; }

        MouseButtonBehavior getMouseButtonBehavior() const 
        {
            return m_eMouseButtonBehavior;
        }
        void setMouseButtonBehavior(MouseButtonBehavior behavior)
        {
            m_eMouseButtonBehavior = behavior;
        }

        void enableKeyInput();
        void disableKeyInput();
        void enableScrollInput();
        void disableScrollInput();
        void enableMouseMoveInput();
        void disableMouseMoveInput();
        void enableMouseButtonInput();
        void disableMouseButtonInput();

        void setKeyInputPaused(bool isPaused) { m_bkeyInputPaused = isPaused; }
        void setMouseInputPaused(bool isPaused) { m_bMouseInputPaused = isPaused; }

        void enableMouseCapture();
        void disableMouseCapture();

        void bindCamera(std::shared_ptr<Camera> camera);
        void bindCameraControls(std::shared_ptr<CameraControls> cameraControls);

        // TODO: Add a way to remove handlers.
        void addKeyPressHandler(int glfwKey, std::function<void(int)> handler);
        void addMouseButtonHandler(int glfwMouseButton,
                                    std::function<void(int)> handler);

        void loop(std::function<void(float)> callback);

        // TODO: Allow setting window icon.

    private:
        void processInput(float deltaTime);
        void keyCallback(int key, int scancode, int action, int mods);
        void scrollCallback(double xoffset, double yoffset);
        void mouseMoveCallback(double xpos, double ypos);
        void mouseButtonCallback(int button, int action, int mods);
        void framebufferSizeCallback(GLFWwindow* window, int width, int height);

        void updateFrameStats(float deltaTime);

        GLFWwindow* m_pWindow;
        bool m_bDepthTestEnabled = false;
        bool m_bStencilTestEnabled = false;

        float m_fLastTime = 0.0f;
        float m_fDeltaTime = 0.0f;
        unsigned int m_uiFrameCount = 0;
        static constexpr int NUM_FRAME_DELTAS = 120;
        float m_fFrameDeltas[NUM_FRAME_DELTAS] = {0.0f};
        float m_fFrameDeltaSum = 0.0f;
        glm::vec4 m_vecClearColor = DEFAULT_CLEAR_COLOR;

        EscBehavior e_eEscBehavior = EscBehavior::NONE;
        MouseButtonBehavior m_eMouseButtonBehavior = MouseButtonBehavior::NONE;
        bool m_bResizeUpdatesEnabled = false;
        bool m_bKeyInputEnabled = false;
        bool m_bScrollInputEnabled = false;
        bool m_bMouseMoveInputEnabled = false;
        bool m_bMouseButtonInputEnabled = false;
        bool m_bkeyInputPaused = false;
        bool m_bMouseInputPaused = false;
        bool m_bMouseCaptured = false;
        std::vector<std::tuple<int, std::function<void(int)>>> m_vecKeyPressHandlers;
        std::vector<std::tuple<int, std::function<void(int)>>> m_vecMouseButtonHandlers;

        std::shared_ptr<Camera> m_spBoundCamera = nullptr;
        std::shared_ptr<CameraControls> m_spBoundCameraControls = nullptr;
    };
}  // namespace Cme

#endif
