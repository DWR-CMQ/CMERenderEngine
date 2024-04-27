#include "camera.h"

namespace Cme
{
    namespace 
    {
        constexpr float POLAR_CAP = 90.0f - 0.1f;
    }

    Camera::Camera(glm::vec3 position, glm::vec3 worldUp, float yaw, float pitch,
                   float fov, float aspectRatio, float near, float far)
        : m_vec3Position(position),
        m_vec3WorldUp(worldUp),
        m_fYaw(yaw),
        m_fPitch(pitch),
        m_fFov(fov),
        m_fAspectRatio(aspectRatio),
        m_fNear(near),
        m_fFar(far)
    {
        updateCameraVectors();
    }

    void Camera::updateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(m_fYaw)) * cos(glm::radians(m_fPitch));
        front.y = sin(glm::radians(m_fPitch));
        front.z = sin(glm::radians(m_fYaw)) * cos(glm::radians(m_fPitch));

        m_vec3Front = glm::normalize(front);
        m_vec3Right = glm::normalize(glm::cross(m_vec3Front, m_vec3WorldUp));
        m_vec3Up = glm::normalize(glm::cross(m_vec3Right, m_vec3Front));
    }

    void Camera::lookAt(glm::vec3 center) 
    {
        glm::vec3 dir = glm::normalize(center - m_vec3Position);
        m_fPitch = glm::degrees(asin(dir.y));
        m_fYaw = glm::mod<float>(glm::degrees(atan2(dir.x, dir.z)) * -1.0f, 360.0f) +
                90.0f;
        updateCameraVectors();
    }

    glm::mat4 Camera::getViewTransform() const 
    {
        glm::vec3 center = m_vec3Position + m_vec3Front;
        return glm::lookAt(m_vec3Position, center, m_vec3Up);
    }

    glm::mat4 Camera::getProjectionTransform() const 
    {
        return glm::perspective(glm::radians(getFov()), m_fAspectRatio, m_fNear, m_fFar);
    }

    void Camera::move(CameraDirection direction, float velocity)
    {
        switch (direction) 
        {
        case CameraDirection::FORWARD:
            m_vec3Position += m_vec3Front * velocity;
            break;
        case CameraDirection::BACKWARD:
            m_vec3Position -= m_vec3Front * velocity;
            break;
        case CameraDirection::LEFT:
            m_vec3Position -= m_vec3Right * velocity;
            break;
        case CameraDirection::RIGHT:
            m_vec3Position += m_vec3Right * velocity;
            break;
        case CameraDirection::UP:
            m_vec3Position += m_vec3Up * velocity;
            break;
        case CameraDirection::DOWN:
            m_vec3Position -= m_vec3Up * velocity;
            break;
        }
    }

    void Camera::rotate(float yawOffset, float pitchOffset, bool constrainPitch)
    {
        // Constrain yaw to be 0-360 to avoid floating point error.
        m_fYaw = glm::mod(m_fYaw + yawOffset, 360.0f);
        m_fPitch += pitchOffset;

        if (constrainPitch) 
        {
            m_fPitch = glm::clamp(m_fPitch, -POLAR_CAP, POLAR_CAP);
        }
        updateCameraVectors();
    }

    void Camera::zoom(float offset)
    {
        m_fFov = glm::clamp(m_fFov - offset, MIN_FOV, MAX_FOV);
    }

    void CameraControls::handleDragStartEnd(int button, int action) 
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            m_bDragging = true;
            m_bInitialized = false;
        } 
        else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        {
            m_bDragging = false;
        }
    }

    MouseDelta CameraControls::calculateMouseDelta(double xpos, double ypos)
    {
        if (!m_bInitialized)
        {
            m_fLastMouseX = xpos;
            m_fLastMouseY = ypos;
            m_bInitialized = true;
        }
        float xoffset = xpos - m_fLastMouseX;
        // Reversed since y-coordinates range from bottom to top.
        float yoffset = m_fLastMouseY - ypos;
        m_fLastMouseX = xpos;
        m_fLastMouseY = ypos;
        return MouseDelta{xoffset, yoffset};
    }

    void FlyCameraControls::resizeWindow(int width, int height) {}

    void FlyCameraControls::scroll(Camera& camera, double xoffset, double yoffset, bool mouseCaptured)
    {
        // Always respond to scroll.
        camera.zoom(yoffset * m_fSensitivity);
    }

    void FlyCameraControls::mouseMove(Camera& camera, double xpos, double ypos, bool mouseCaptured)
    {
        // Only move when dragging, or when the mouse is captured.
        if (!(m_bDragging || mouseCaptured))
        {
            return;
        }

        MouseDelta delta = calculateMouseDelta(xpos, ypos);
        camera.rotate(delta.xoffset * m_fSensitivity, delta.yoffset * m_fSensitivity);
    }

    void FlyCameraControls::mouseButton(Camera& camera, int button, int action, int mods, bool mouseCaptured)
    {
        handleDragStartEnd(button, action);
    }

    void FlyCameraControls::processInput(GLFWwindow* window, Camera& camera, float deltaTime)
    {
        float velocity = m_fSpeed * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            camera.move(Cme::CameraDirection::FORWARD, velocity);
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) 
        {
            camera.move(Cme::CameraDirection::LEFT, velocity);
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            camera.move(Cme::CameraDirection::BACKWARD, velocity);
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            camera.move(Cme::CameraDirection::RIGHT, velocity);
        }
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) 
        {
            camera.move(Cme::CameraDirection::UP, velocity);
        }
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        {
            camera.move(Cme::CameraDirection::DOWN, velocity);
        }
    }

    OrbitCameraControls::OrbitCameraControls(Camera& camera, glm::vec3 center) : m_vec3Center(center)
    {
        glm::vec3 dir = camera.getPosition() - center;
        m_fRadius = glm::clamp(glm::length(dir), MIN_RADIUS, MAX_RADIUS);
        glm::vec3 normdir = glm::normalize(dir);
        // TODO: Fix this, atan2 args are backwards.
        m_fAzimuth = glm::mod<float>(glm::degrees(atan2(normdir.x, normdir.z)) * -1.0f, 360.0f) + 90.0f;
        m_fAltitude = glm::clamp<float>(glm::degrees(asin(normdir.y)), -POLAR_CAP, POLAR_CAP);

        updateCamera(camera);
    }

    void OrbitCameraControls::resizeWindow(int width, int height) {}

    void OrbitCameraControls::scroll(Camera& camera, double xoffset, double yoffset, bool mouseCaptured)
    {
        m_fRadius = glm::clamp(m_fRadius - static_cast<float>(yoffset * m_fSensitivity), MIN_RADIUS, MAX_RADIUS);
        updateCamera(camera);
    }

    void OrbitCameraControls::mouseMove(Camera& camera, double xpos, double ypos, bool mouseCaptured)
    {
        // Only move when dragging, or when the mouse is captured.
        if (!(m_bDragging || mouseCaptured))
        {
            return;
        }

        MouseDelta delta = calculateMouseDelta(xpos, ypos);

        // Constrain azimuth to be 0-360 to avoid floating point error.
        m_fAzimuth = glm::mod(m_fAzimuth + delta.xoffset * m_fSensitivity, 360.0f);
        m_fAltitude = glm::clamp(m_fAltitude - delta.yoffset * m_fSensitivity, -POLAR_CAP,  POLAR_CAP);

        updateCamera(camera);
    }

    void OrbitCameraControls::mouseButton(Camera& camera, int button, int action,
                                          int mods, bool mouseCaptured) 
    {
        handleDragStartEnd(button, action);
    }

    void OrbitCameraControls::processInput(GLFWwindow* window, Camera& camera,
                                           float deltaTime) {}

    void OrbitCameraControls::updateCamera(Camera& camera) 
    {
        // Compute camera position.
        glm::vec3 cameraPosition;
        cameraPosition.x = m_vec3Center.x + m_fRadius * cos(glm::radians(m_fAltitude)) *
                                            cos(glm::radians(m_fAzimuth));
        cameraPosition.y = m_vec3Center.y + m_fRadius * sin(glm::radians(m_fAltitude));
        cameraPosition.z = m_vec3Center.z + m_fRadius * cos(glm::radians(m_fAltitude)) *
                                            sin(glm::radians(m_fAzimuth));

        camera.setPosition(cameraPosition);
        camera.lookAt(m_vec3Center);
    }

}  // namespace Cme
