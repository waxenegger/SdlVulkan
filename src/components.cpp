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
    this->updateComponentProperties();
}

void Component::updateComponentProperties() {
    if (!this->hasModel()) return;

    for (auto & p : this->compProps) {
        p.modelProperties.matrix = this->getModelMatrix();
    }
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
    this->updateComponentProperties();
}

void Component::move(float xAxis, float yAxis, float zAxis) {
    this->position.x += xAxis;
    this->position.y += yAxis;
    this->position.z += zAxis;
    this->updateComponentProperties();
}

void Component::setRotation(glm::vec3 rotation) {
    this->rotation = rotation;
}

void Component::scale(float factor) {
    if (factor <= 0) return;
    this->scaleFactor = factor;
    this->updateComponentProperties();
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
    return sizeof(struct ComponentProperties) * this->compProps.size();
}

void Component::addComponentProperties(const ComponentProperties props) {
    this->compProps.push_back(props);
}

std::vector<ComponentProperties> & Component::getProperties() {
    return this->compProps;
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
        for (auto & m : meshes) {
            ComponentProperties props = {};
            TextureInformation textureInfo = m.getTextureInformation();
            MaterialInformation materialInfo = m.getMaterialInformation();
            props.meshProperties = { 
                textureInfo.ambientTexture,
                textureInfo.diffuseTexture,
                textureInfo.specularTexture,
                textureInfo.normalTexture,
                materialInfo.ambientColor,
                materialInfo.emissiveFactor,
                materialInfo.diffuseColor,
                materialInfo.opacity,
                materialInfo.specularColor,
                materialInfo.shininess
            };
            props.modelProperties.matrix = component->getModelMatrix();
            component->addComponentProperties(props);
        }

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
    this->componentsByModel.clear();
    this->components.clear();
}

std::map<std::string,  std::vector<Component *>> & Components::getComponentsPerModel() {
    return this->componentsByModel;
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
