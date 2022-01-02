#include "includes/components.h"

Component::Component(std::string id) {
    this->id = id;
}

Component::Component(std::string id, Model * model) : Component(id) {
    this->model = model;
}

bool Component::hasModel() {
    return this->model != nullptr;
}

Model * Component::getModel() {
    return this->model;
}

glm::mat4 Component::getModelMatrix(bool includeRotation) {
    glm::mat4 transformation = glm::mat4(1.0f);

    transformation = glm::translate(transformation, this->position);

    if (includeRotation) {
        if (this->rotation.x != 0.0f) transformation = glm::rotate(transformation, this->rotation.x, glm::vec3(1, 0, 0));
        if (this->rotation.y != 0.0f) transformation = glm::rotate(transformation, this->rotation.y, glm::vec3(0, 1, 0));
        if (this->rotation.z != 0.0f) transformation = glm::rotate(transformation, this->rotation.z, glm::vec3(0, 0, 1));
    }

    return glm::scale(transformation, glm::vec3(this->scaleFactor));   
}

void Component::setPosition(float x, float y, float z) {
    this->setPosition(glm::vec3(x,y,z));
}

void Component::setPosition(glm::vec3 position) {
    this->position = position;
}

glm::vec3 Component::getPosition() {
    return this->position;
}   

void Component::rotate(int xAxis, int yAxis, int zAxis) {
    glm::vec3 rot;
    rot.x = glm::radians(static_cast<float>(xAxis));
    rot.y = glm::radians(static_cast<float>(yAxis));
    rot.z = glm::radians(static_cast<float>(zAxis));
    this->rotation += rot;
}

void Component::move(float xAxis, float yAxis, float zAxis) {
    this->position.x += xAxis;
    this->position.y += yAxis;
    this->position.z += zAxis;
}

void Component::setRotation(glm::vec3 rotation) {
    this->rotation = rotation;
}

void Component::scale(float factor) {
    if (factor <= 0) return;
    this->scaleFactor = factor;
}

std::string Component::getId() {
    return this->id;
}

void Component::setSsboIndex(const uint32_t index) {
    this->ssboIndex = index;
}

uint32_t Component::getSsboIndex() {
    return this->ssboIndex;
}

VkDeviceSize Component::getSsboSize() {
    return sizeof(struct MeshProperties);
}

Component * Components::addComponentFromModel(const std::string id, const std::string modelId) {
    Model * model = Models::INSTANCE()->findModel(modelId);
    if (model == nullptr) return nullptr;
    
    return this->addComponent(new Component(id, model));
}


Component * Components::addComponent(Component * component) {
    if (component == nullptr) return nullptr;

    if (component->hasModel()) {
        auto & meshes = component->getModel()->getMeshes();
        if (!meshes.empty()) {
            component->setSsboIndex(this->ssboIndex);
            this->ssboIndex += meshes.size();
        }
    }
    
    this->components.push_back(std::unique_ptr<Component>(component));
    
    const std::string modelId = component->hasModel() ? component->getModel()->getId() : "";
    
    std::map<std::string, std::vector<Component*>>::iterator it = this->componentsByModel.find(modelId);
    if (it != this->componentsByModel.end()) {
        it->second.push_back(component);
    } else {
        std::vector<Component *> allComponentsPerModel;
        allComponentsPerModel.push_back(component);
        this->componentsByModel[modelId] = std::move(allComponentsPerModel);
    }
    
    return component;
}

Component * Components::findComponent(const std::string id, const std::string modelId) {
    std::map<std::string, std::vector<Component*>>::iterator it = this->componentsByModel.find(modelId);
    if (it != this->componentsByModel.end()) {
        for(auto & c : it->second) {
            if (c->getId() == id) return c;
        }
    }
    
    return nullptr;
}

std::vector<std::unique_ptr<Component>> & Components::getComponents() {
    return this->components;
}

Components::~Components() {
    this->components.clear();
}

std::vector<Component *> Components::getAllComponentsForModel(std::string model) {
    std::vector<Component *> allMeshProperties;
    
    std::map<std::string, std::vector<Component*>>::iterator it = this->componentsByModel.find(model);
    if (it == this->componentsByModel.end()) return allMeshProperties;
    
    std::vector<Component*> & comps = it->second;
    for (Component * comp : comps) {
        if (comp->hasModel()) allMeshProperties.push_back(comp);
    }
    
    return allMeshProperties;
}

bool Component::isVisible() {
    return this->visible;
};

void Component::setVisible(bool visible) {
    this->visible = visible;
};

glm::vec3 Component::getRotation() {
    return this->rotation;
}

Components * Components::INSTANCE() {
    if (Components::instance == nullptr) {
        Components::instance = new Components();
    }
    return Components::instance;
}

Components::Components() { }


Components * Components::instance = nullptr;
