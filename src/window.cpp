#include "window.h"


namespace Cme
{
    bool Window::m_bGlfwErrorLoggingEnabled = true;
    bool Window::m_bGlErrorLoggingEnabled = true;

    Window::Window(int width, int height, const char* title, bool fullscreen, int samples)
    {
        if (!m_bInitialized)
        {
            glfwSetErrorCallback(reinterpret_cast<GLFWerrorfun>(&glfwErrorCallback));

            glfwInit();
            // TODO: Do we need this? Move into Cme::Window?
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

            m_bInitialized = true;
        }

        if (samples > 0)
        {
            glfwWindowHint(GLFW_SAMPLES, samples);
        }

        // nullptr indicates windowed.
        GLFWmonitor* monitor = nullptr;
        if (fullscreen) 
        {
            monitor = glfwGetPrimaryMonitor();
        }

        m_pWindow = glfwCreateWindow(width, height, title, monitor, /* share */ nullptr);

        if (m_pWindow == nullptr)
        {
            if (m_bInitialized)
            {
                glfwTerminate();
                m_bInitialized = false;
            }
            throw WindowException("ERROR::WINDOW::CREATE_FAILED");
        }

        activate();

        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) 
        {
            throw WindowException("ERROR::WINDOW::GLAD_INITIALIZATION_FAILED");
        }

        // 发生错误时是否禁止打印log
        if (m_bGlErrorLoggingEnabled)
        {
            m_bGlErrorLoggingEnabled = true;
            glEnable(GL_DEBUG_OUTPUT);
            glDebugMessageCallback(DebugCallback, 0);
        }
        else
        {
            m_bGlErrorLoggingEnabled = false;
            glDisable(GL_DEBUG_OUTPUT);
        }

        // Allow us to refer to the object while accessing C APIs.
        glfwSetWindowUserPointer(m_pWindow, this);

        // Enable multisampling if needed.
        if (samples > 0) 
        {
            glEnable(GL_MULTISAMPLE);
        }

        // A few options are enabled by default.
        enableDepthTest();
        enableSeamlessCubemap();
        enableResizeUpdates();
        enableKeyInput();
        enableScrollInput();
        enableMouseMoveInput();
        enableMouseButtonInput();
    }

    Window::~Window()
    {
        if (m_pWindow != nullptr)
        {
            glfwDestroyWindow(m_pWindow);
        }

        glfwTerminate();
        m_bInitialized = false;
    }

    void Window::activate() { glfwMakeContextCurrent(m_pWindow); }

    ImageSize Window::getSize() const 
    {
        ImageSize size;
        glfwGetWindowSize(m_pWindow, &size.width, &size.height);
        return size;
    }

    void Window::setSize(int width, int height) 
    {
        glfwSetWindowSize(m_pWindow, width, height);
    }

    void Window::enableResizeUpdates()
    {
        if (m_bResizeUpdatesEnabled)
        {
            return;
        }
        auto callback = [](GLFWwindow* window, int width, int height) 
        {
            auto self = static_cast<Window*>(glfwGetWindowUserPointer(window));
            self->framebufferSizeCallback(window, width, height);
        };
        glfwSetFramebufferSizeCallback(m_pWindow, callback);
        m_bResizeUpdatesEnabled = true;
    }

    void Window::disableResizeUpdates() 
    {
        if (!m_bResizeUpdatesEnabled)
        {
            return;
        }
        glfwSetFramebufferSizeCallback(m_pWindow, nullptr);
        m_bResizeUpdatesEnabled = false;
    }

    float Window::getAspectRatio() const 
    {
        ImageSize size = getSize();
        return size.width / static_cast<float>(size.height);
    }

    void Window::enableKeyInput() 
    {
        if (m_bKeyInputEnabled) return;
        auto callback = [](GLFWwindow* window, int key, int scancode, int action, int mods) 
        {
            auto self = static_cast<Window*>(glfwGetWindowUserPointer(window));
            self->keyCallback(key, scancode, action, mods);
        };
        glfwSetKeyCallback(m_pWindow, callback);
        m_bKeyInputEnabled = true;
    }

    void Window::disableKeyInput()
    {
        if (!m_bKeyInputEnabled) return;
        glfwSetKeyCallback(m_pWindow, nullptr);
        m_bKeyInputEnabled = false;
    }

    void Window::enableScrollInput() 
    {
        if (m_bScrollInputEnabled) return;
        auto callback = [](GLFWwindow* window, double xoffset, double yoffset) 
        {
            auto self = static_cast<Window*>(glfwGetWindowUserPointer(window));
            self->scrollCallback(xoffset, yoffset);
        };
        glfwSetScrollCallback(m_pWindow, callback);
        m_bScrollInputEnabled = true;
    }

    void Window::disableScrollInput()
    {
        if (!m_bScrollInputEnabled) return;
        glfwSetScrollCallback(m_pWindow, nullptr);
        m_bScrollInputEnabled = false;
    }

    void Window::enableMouseMoveInput() 
    {
        if (m_bMouseMoveInputEnabled) return;
        auto callback = [](GLFWwindow* window, double xpos, double ypos) 
        {
            auto self = static_cast<Window*>(glfwGetWindowUserPointer(window));
            self->mouseMoveCallback(xpos, ypos);
        };
        glfwSetCursorPosCallback(m_pWindow, callback);
        m_bMouseMoveInputEnabled = true;
    }

    void Window::disableMouseMoveInput() 
    {
        if (!m_bMouseMoveInputEnabled) return;
        glfwSetCursorPosCallback(m_pWindow, nullptr);
        m_bMouseMoveInputEnabled = false;
    }

    void Window::enableMouseButtonInput()
    {
        if (m_bMouseButtonInputEnabled) return;
        auto callback = [](GLFWwindow* window, int button, int action, int mods) 
        {
            auto self = static_cast<Window*>(glfwGetWindowUserPointer(window));
            self->mouseButtonCallback(button, action, mods);
        };
        glfwSetMouseButtonCallback(m_pWindow, callback);
        m_bMouseButtonInputEnabled = true;
    }

    void Window::disableMouseButtonInput() 
    {
        if (!m_bMouseButtonInputEnabled) return;
        glfwSetMouseButtonCallback(m_pWindow, nullptr);
        m_bMouseButtonInputEnabled = false;
    }

    void Window::framebufferSizeCallback(GLFWwindow* window, int width, int height) 
    {
        glViewport(0, 0, width, height);

        if (m_spBoundCamera)
        {
            m_spBoundCamera->setAspectRatio(width / static_cast<float>(height));
        }
        if (m_spBoundCameraControls)
        {
            m_spBoundCameraControls->resizeWindow(width, height);
        }
    }

    void Window::makeFullscreen() 
    {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        auto size = getSize();
        glfwSetWindowMonitor(m_pWindow, monitor, /* unused xpos */ 0,
                            /* unused ypos */ 0, size.width, size.height,
                            /* refreshRate */ GLFW_DONT_CARE);
    }

    void Window::makeWindowed()
    {
        auto size = getSize();
        glfwSetWindowMonitor(m_pWindow, /* monitor */ nullptr, /* xpos */ 0,
                            /* ypos */ 0, size.width, size.height,
                            /* refreshRate */ GLFW_DONT_CARE);
    }

    void Window::processInput(float deltaTime) 
    {
        if (m_spBoundCameraControls)
        {
            m_spBoundCameraControls->processInput(m_pWindow, *m_spBoundCamera, deltaTime);
        }
    }

    void Window::keyCallback(int key, int scancode, int action, int mods) 
    {
        if (m_bkeyInputPaused) return;

        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) 
        {
            if (e_eEscBehavior == EscBehavior::TOGGLE_MOUSE_CAPTURE)
            {
                auto inputMode = glfwGetInputMode(m_pWindow, GLFW_CURSOR);
                if (inputMode == GLFW_CURSOR_NORMAL)
                {
                    enableMouseCapture();
                } 
                else 
                {
                    disableMouseCapture();
                }
            } 
            else if (e_eEscBehavior == EscBehavior::CLOSE)
            {
                glfwSetWindowShouldClose(m_pWindow, true);
            } 
            else if (e_eEscBehavior == EscBehavior::UNCAPTURE_MOUSE_OR_CLOSE)
            {
                auto inputMode = glfwGetInputMode(m_pWindow, GLFW_CURSOR);
                if (inputMode == GLFW_CURSOR_DISABLED)
                {
                    disableMouseCapture();
                } 
                else 
                {
                    // Close since mouse is not captured.
                    glfwSetWindowShouldClose(m_pWindow, true);
                }
            }
        }

        // Run handlers.
        if (action == GLFW_PRESS)
        {
            for (auto pair : m_vecKeyPressHandlers)
            {
                int glfwKey;
                std::function<void(int)> handler;
                std::tie(glfwKey, handler) = pair;

                if (key == glfwKey)
                {
                    handler(mods);
                }
            }
        }
    }

    void Window::scrollCallback(double xoffset, double yoffset)
    {
        if (m_bMouseInputPaused) return;

        if (m_spBoundCameraControls)
        {
            m_spBoundCameraControls->scroll(*m_spBoundCamera, xoffset, yoffset, m_bMouseCaptured);
        }
    }

    void Window::mouseMoveCallback(double xpos, double ypos)
    {
        if (m_bMouseInputPaused) return;
        if (m_spBoundCameraControls)
        {
            m_spBoundCameraControls->mouseMove(*m_spBoundCamera, xpos, ypos, m_bMouseCaptured);
        }
    }

    void Window::mouseButtonCallback(int button, int action, int mods)
    {
        if (m_bMouseInputPaused) return;

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) 
        {
            if (m_eMouseButtonBehavior == MouseButtonBehavior::CAPTURE_MOUSE)
            {
                enableMouseCapture();
            }
        }

        // Run handlers.
        if (action == GLFW_PRESS)
        {
            for (auto pair : m_vecMouseButtonHandlers)
            {
                int glfwMouseButton;
                std::function<void(int)> handler;
                std::tie(glfwMouseButton, handler) = pair;

                if (button == glfwMouseButton) 
                {
                    handler(mods);
                }
            }
        }

        if (m_spBoundCameraControls)
        {
            m_spBoundCameraControls->mouseButton(*m_spBoundCamera, button, action, mods, m_bMouseCaptured);
        }
    }

    void Window::addKeyPressHandler(int glfwKey, std::function<void(int)> handler)
    {
        m_vecKeyPressHandlers.push_back(std::make_tuple(glfwKey, handler));
    }
    void Window::addMouseButtonHandler(int glfwMouseButton, std::function<void(int)> handler) 
    {
        m_vecMouseButtonHandlers.push_back(std::make_tuple(glfwMouseButton, handler));
    }

    void Window::enableMouseCapture() 
    {
        glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        m_bMouseCaptured = true;
    }

    void Window::disableMouseCapture()
    {
        glfwSetInputMode(m_pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        m_bMouseCaptured = false;
    }

    void Window::bindCamera(std::shared_ptr<Camera> camera)
    {
        m_spBoundCamera = camera;
        m_spBoundCamera->setAspectRatio(getSize());
    }

    void Window::bindCameraControls(std::shared_ptr<CameraControls> cameraControls) 
    {
        if (!m_spBoundCamera)
        {
            throw WindowException(
                "ERROR::WINDOW::BIND_CAMERA_CONTROLS_FAILED\n"
                "Camera must be bound before camera controls.");
        }
        m_spBoundCameraControls = cameraControls;
        ImageSize size = getSize();
        m_spBoundCameraControls->resizeWindow(size.width, size.height);
    }

    const float* Window::getFrameDeltas() const { return &m_fFrameDeltas[0]; }

    int Window::getNumFrameDeltas() const { return NUM_FRAME_DELTAS; }

    int Window::getFrameDeltasOffset() const
    {
        return m_uiFrameCount % NUM_FRAME_DELTAS;
    }

    float Window::getAvgFPS() const 
    {
        int denominator = std::min<int>(m_uiFrameCount, NUM_FRAME_DELTAS);
        float avgFrameDelta = m_fFrameDeltaSum / denominator;
        return 1.0f / avgFrameDelta;
    }

    void Window::updateFrameStats(float deltaTime) 
    {
        unsigned int offset = getFrameDeltasOffset();
        float oldDeltaTime = m_fFrameDeltas[offset];
        m_fFrameDeltaSum -= oldDeltaTime;
        m_fFrameDeltaSum += deltaTime;
        m_fFrameDeltas[offset] = deltaTime;
    }

    void Window::loop(std::function<void(float)> callback)
    {
        // TODO: Add exception handling here.
        while (!glfwWindowShouldClose(m_pWindow))
        {
            float currentTime = glfwGetTime();
            m_fDeltaTime = currentTime - m_fLastTime;
            m_fLastTime = currentTime;

            updateFrameStats(m_fDeltaTime);

            // enableAlphaBlending();
            // Clear the appropriate buffers.
            glClearColor(m_vecClearColor.r, m_vecClearColor.g, m_vecClearColor.b, m_vecClearColor.a);
            auto clearBits = GL_COLOR_BUFFER_BIT;
            if (m_bDepthTestEnabled)
            {
                clearBits |= GL_DEPTH_BUFFER_BIT;
            }
            if (m_bStencilTestEnabled)
            {
                clearBits |= GL_STENCIL_BUFFER_BIT;
            }
            glClear(clearBits);

            // Process necessary input.
            processInput(m_fDeltaTime);

            // Call the loop function.
            callback(m_fDeltaTime);

            qrkCheckForGlError();

            glfwSwapBuffers(m_pWindow);
            glfwPollEvents();

            ++m_uiFrameCount;
        }
    }

    void Window::glfwErrorCallback(int error, const char* description)
    {
        if (m_bGlfwErrorLoggingEnabled)
        {
            fprintf(stderr, "GLFW ERROR: %s [error code %d]\n", description, error);
        }
    }

    void GLAPIENTRY Window::DebugCallback(GLenum source, GLenum type, GLuint id,
                                            GLenum severity, GLsizei length,
                                            const GLchar* message, const void* userParam)
    {
        if (m_bGlErrorLoggingEnabled && type == GL_DEBUG_TYPE_ERROR)
        {
            fprintf(stderr, "GL ERROR: %s [error type = 0x%x, severity = 0x%x]\n",
                message, type, severity);
        }
    }

} 
