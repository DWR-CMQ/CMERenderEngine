#ifndef QUARKGL_CAMERA_H_
#define QUARKGL_CAMERA_H_

// clang-format off
// Must precede glfw/glad, to include OpenGL functions.
#include "core.h"
// clang-format on

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include "light.h"
#include "screen.h"
#include "shader/shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

namespace Cme 
{
    enum class CameraDirection 
    {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT,
        UP,
        DOWN,
    };

    constexpr float DEFAULT_YAW = 270.0f;
    constexpr float DEFAULT_PITCH = 0.0f;
    constexpr float DEFAULT_FOV = 45.0f;
    constexpr float DEFAULT_NEAR = 0.1f;
    constexpr float DEFAULT_FAR = 100.0f;
    constexpr float DEFAULT_ASPECT_RATIO = 4.0f / 3.0f;

    constexpr float MIN_FOV = 1.0f;
    constexpr float MAX_FOV = 135.0f;

    class Camera : public UniformSource, public ViewSource 
    {
    public:
        // Constructs a new Camera. Angular values should be provided in degrees.
        Camera(glm::vec3 position = glm::vec3(0.0f),
                glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f),
                float yaw = DEFAULT_YAW, float pitch = DEFAULT_PITCH,
                float fov = DEFAULT_FOV, float aspectRatio = DEFAULT_ASPECT_RATIO,
                float near = DEFAULT_NEAR, float far = DEFAULT_FAR);
        virtual ~Camera() = default;

        void lookAt(glm::vec3 center);

        glm::vec3 getPosition() const { return m_vec3Position; }
        void setPosition(glm::vec3 position) { m_vec3Position = position; }
        float getYaw() const { return m_fYaw; }
        void setYaw(float yaw)
        {
            m_fYaw = yaw;
            updateCameraVectors();
        }
        float getPitch() const { return m_fPitch; }
        void setPitch(float pitch)
        {
            m_fPitch = pitch;
            updateCameraVectors();
        }
        float getFov() const { return m_fFov; }
        void setFov(float fov) { m_fFov = fov; }

        float getAspectRatio() const { return m_fAspectRatio; }
        void setAspectRatio(float aspectRatio) { m_fAspectRatio = aspectRatio; }
        void setAspectRatio(ImageSize size) 
        {
            m_fAspectRatio = size.width / static_cast<float>(size.height);
        }

        float getNearPlane() const { return m_fNear; }
        void setNearPlane(float near) { m_fNear = near; }
        float getFarPlane() const { return m_fFar; }
        void setFarPlane(float far) { m_fFar = far; }

        glm::mat4 getViewTransform() const override;
        glm::mat4 getProjectionTransform() const;

        void updateUniforms(Shader& shader) override;

        // Moves the camera in the given direction by the given amount.
        void move(CameraDirection direction, float velocity);
        // Rotates the camera by the given yaw and pitch offsets. If constrainPitch is
        // true, pitch is clamped when near the up and down directions.
        void rotate(float yawOffset, float pitchOffset, bool constrainPitch = true);
        // Changes the zoom level of the FoV by the given offset. Alternative to
        // setFov().
        void zoom(float fovOffset);

    private:
        void updateCameraVectors();

        glm::vec3 m_vec3Position;
        glm::vec3 m_vec3Front;
        glm::vec3 m_vec3Up;
        glm::vec3 m_vec3Right;
        glm::vec3 m_vec3WorldUp;

        // The **clockwise** angle that the camera is facing, measured from the
        // positive X axis.
        float m_fYaw;
        float m_fPitch;
        float m_fFov;

        float m_fAspectRatio;
        float m_fNear;
        float m_fFar;
    };

    struct MouseDelta 
    {
        float xoffset;
        float yoffset;
    };

    // Interface for camera controllers.
    class CameraControls 
    {
    public:
        virtual void resizeWindow(int width, int height) = 0;
        virtual void scroll(Camera& camera, double xoffset, double yoffset,
                            bool mouseCaptured) = 0;
        virtual void mouseMove(Camera& camera, double xpos, double ypos,
                                bool mouseCaptured) = 0;
        virtual void mouseButton(Camera& camera, int button, int action, int mods,
                                bool mouseCaptured) = 0;
        virtual void processInput(GLFWwindow* window, Camera& camera,
                                float deltaTime) = 0;

        // Base helpers; call these from subclasses.
        void handleDragStartEnd(int button, int action);
        MouseDelta calculateMouseDelta(double xpos, double ypos);

        float getSpeed() const { return m_fSpeed; }
        void setSpeed(float speed) { m_fSpeed = speed; }
        float getSensitivity() const { return m_fSensitivity; }
        void setSensitivity(float sensitivity) { m_fSensitivity = sensitivity; }

    protected:
        static constexpr float DEFAULT_SPEED = 5.0f;
        static constexpr float DEFAULT_SENSITIVITY = 0.1f;

        // Whether input updates should use the last mouse positions to calculate
        // delta, or to start from the current positions.
        bool m_bInitialized = false;
        // Whether a mouse is being click+dragged.
        bool m_bDragging = false;
        float m_fLastMouseX;
        float m_fLastMouseY;

        // General setting for camera movement speed.
        float m_fSpeed = DEFAULT_SPEED;
        // General setting for camera sensitivity.
        float m_fSensitivity = DEFAULT_SENSITIVITY;
    };

    // Camera controls that implement a fly mode, similar to DCC tools.
    class FlyCameraControls : public CameraControls
    {
    public:
        virtual ~FlyCameraControls() = default;
        void resizeWindow(int width, int height) override;
        void scroll(Camera& camera, double xoffset, double yoffset,
                    bool mouseCaptured) override;
        void mouseMove(Camera& camera, double xpos, double ypos,
                        bool mouseCaptured) override;
        void mouseButton(Camera& camera, int button, int action, int mods,
                        bool mouseCaptured) override;
        void processInput(GLFWwindow* window, Camera& camera,
                        float deltaTime) override;
    };

    // Camera controls that implement an orbit mode at a focal point.
    class OrbitCameraControls : public CameraControls
    {
    public:
        OrbitCameraControls(Camera& camera, glm::vec3 center = glm::vec3(0.0f));
        virtual ~OrbitCameraControls() = default;
        void resizeWindow(int width, int height) override;
        void scroll(Camera& camera, double xoffset, double yoffset,
                    bool mouseCaptured) override;
        void mouseMove(Camera& camera, double xpos, double ypos,
                        bool mouseCaptured) override;
        void mouseButton(Camera& camera, int button, int action, int mods,
                        bool mouseCaptured) override;
        void processInput(GLFWwindow* window, Camera& camera,
                        float deltaTime) override;

        glm::vec3 getCenter() const { return m_vec3Center; }
        void setCenter(glm::vec3 center) { m_vec3Center = center; }

        // Updates the camera position and orientation to match the current orbit
        // position.
        void updateCamera(Camera& camera);

    private:
        static constexpr float MIN_RADIUS = 0.1f;
        static constexpr float MAX_RADIUS = 100.0f;
        // The center point that we're orbiting around.
        glm::vec3 m_vec3Center;
        // The distance from the center.
        float m_fRadius;
        // The **clockwise** angle in degrees that the camera is rotated about the
        // center.
        float m_fAzimuth;
        // The angle in degrees that the camera is tilted up or down.
        float m_fAltitude;
    };

}  // namespace Cme

#endif
