#include "Camera.hpp"

namespace gps {

    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);

        this->cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection));

    }

    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
    }

    void Camera::move(MOVE_DIRECTION direction, float speed) {
        switch (direction) {
            case MOVE_FORWARD:
                cameraPosition += speed * cameraFrontDirection;
                break;
            case MOVE_BACKWARD:
                cameraPosition -= speed * cameraFrontDirection;
                break;
            case MOVE_RIGHT:
                cameraPosition += speed * cameraRightDirection;
                break;
            case MOVE_LEFT:
                cameraPosition -= speed * cameraRightDirection;
                break;
            case MOVE_UP:
                cameraPosition.y += speed;
                break;
            case MOVE_DOWN:
                cameraPosition.y -= speed;
                break;
        }
        cameraTarget = cameraPosition + cameraFrontDirection;    }

    void Camera::rotate(float pitch, float yaw) {
        glm::vec3 front;
        front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
        front.y = sin(glm::radians(pitch));
        front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
        cameraFrontDirection = glm::normalize(front);

        glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, worldUp));
        cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));

        cameraTarget = cameraPosition + cameraFrontDirection; }

    void Camera::setPosition(const glm::vec3& position) {
        cameraPosition = position;

        // Update the target to maintain the front direction
        cameraTarget = cameraPosition + cameraFrontDirection;
    }

    // Set the camera target
    void Camera::setTarget(const glm::vec3& target) {
        cameraTarget = target;

        // Update the front direction to point towards the new target
        cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);

        // Recalculate the right and up directions
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
        cameraUpDirection = glm::normalize(glm::cross(cameraRightDirection, cameraFrontDirection));
    }
}