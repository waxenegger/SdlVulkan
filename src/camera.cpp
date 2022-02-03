#include "includes/world.h"

const float PI_HALF = glm::pi<float>() / 2;

void Camera::updateViewMatrix() {
    glm::mat4 rotM = glm::mat4(1.0f);
    glm::mat4 transM;

    rotM = glm::rotate(rotM, this->rotation.x * (this->flipY ? -1.0f : 1.0f), glm::vec3(1.0f, 0.0f, 0.0f));    
    rotM = glm::rotate(rotM, this->rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    rotM = glm::rotate(rotM, this->rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

    glm::vec3 translation = this->position;

    if (this->flipY) {
        translation.y *= -1.0f;
    }
    transM = glm::translate(glm::mat4(1.0f), translation);

    if (type == CameraType::firstperson) {
        this->view = rotM * transM;
    } else {
        this->view = transM * rotM;
    }
};

glm::vec3 Camera::getPosition() {
    return this->position;
}

bool Camera::moving()
{
    return this->keys.left || this->keys.right || this->keys.up || this->keys.down;
}

void Camera::move(KeyPress key, bool isPressed, float delta) {    
    switch(key) {
        case LEFT:
            this->keys.left = isPressed;
            break;
        case RIGHT:
            this->keys.right = isPressed;
            break;
        case UP:
            this->keys.up = isPressed;
            break;
        case DOWN:
            this->keys.down = isPressed;
            break;
        default:
            break;
    }
    
    if (isPressed) this->update(delta);
}

void Camera::setAspectRatio(float aspect) {
    this->aspect = aspect;
    this->setPerspective();
};

void Camera::setFovY(float degrees)
{
    this->fovy = degrees;
    this->setPerspective();
};

float Camera::getFovY()
{
    return this->fovy;
};

void Camera::setPerspective()
{
    this->perspective = glm::perspective(glm::radians(this->fovy), this->aspect, 0.1f, 1000.0f);
    if (this->flipY) {
        this->perspective[1][1] *= -1.0f;
    }
    this->updateViewMatrix();
};

void Camera::setPosition(glm::vec3 position) {
    this->position = position;
    this->updateViewMatrix();
}

void Camera::setRotation(glm::vec3 rotation) {
    this->rotation = rotation;
    this->updateViewMatrix();
}

void Camera::rotate(glm::vec3 delta) {
    this->rotation += delta;
    
    if (this->rotation.x < -PI_HALF) {
        this->rotation.x = -PI_HALF;
    } else if (this->rotation.x > PI_HALF) {
        this->rotation.x = PI_HALF;
    }
    
    this->updateViewMatrix();
}

void Camera::setTranslation(glm::vec3 translation) {
    this->position = translation;
    this->updateViewMatrix();
};

void Camera::translate(glm::vec3 delta) {
    this->position += delta;
    this->updateViewMatrix();
}

glm::vec3 Camera::getCameraFront() {
    glm::vec3 camFront;
    
    camFront.x = -cos(this->rotation.x) * sin(this->rotation.y);
    camFront.y = sin(this->rotation.x);
    camFront.z = cos(this->rotation.x) * cos(rotation.y);
    camFront = glm::normalize(camFront);
    
    return camFront;
}

void Camera::update(const float delta) {
    if (type == CameraType::firstperson) {
        if (moving()) {
            glm::vec3 camFront = this->getCameraFront();

            if (this->keys.up) {
                this->position += camFront * delta;
            }
            if (this->keys.down) {
                this->position -= camFront * delta;
            }
            if (this->keys.left) {
                this->position -= glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * delta;
            }
            if (this->keys.right) {
                this->position += glm::normalize(glm::cross(camFront, glm::vec3(0.0f, 1.0f, 0.0f))) * delta;
            }
            
            //this->position.y = terrainHeight + CameraHeight;

            this->updateViewMatrix();
        }
    }
};

void Camera::updateDirection(const float deltaX, const float  deltaY, float delta) {    
    glm::vec3 rot(0.0f);
    rot.y = deltaX * delta;
    rot.x = -deltaY * delta;
        
    this->rotate(rot);
}

void Camera::setType(CameraType type) {
    this->type = type;
}

glm::mat4 Camera::getViewMatrix() {
    return this->view;
};

glm::mat4 Camera::getProjectionMatrix() {
    return this->perspective;
};

Camera::Camera(glm::vec3 position)
{
    setPosition(position);
}

Camera * Camera::INSTANCE(glm::vec3 position) {
    if (Camera::instance == nullptr) Camera::instance = new Camera(position);
    return Camera::instance;
}

Camera * Camera::INSTANCE() {
    if (Camera::instance == nullptr) Camera::instance = new Camera(glm::vec3(0.0f));
    return Camera::instance;
}

void Camera::destroy() {
    if (Camera::instance != nullptr) {
        delete Camera::instance;
        Camera::instance = nullptr;
    }
}

Camera * Camera::instance = nullptr;

