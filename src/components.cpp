#include "includes/components.h"

Component::Component(std::string id) {
    this->id = id;
}

Component::Component(std::string id, Model * model) : Component(id) {
    this->model = model;
    this->updateModelMatrix();
}

bool Component::hasModel() {
    return this->model != nullptr;
}

Model * Component::getModel() {
    return this->model;
}

glm::mat4 Component::getModelMatrix() {
    return this->modelMatrix;
}

float Component::getScalingFactor() {
    return this->scaleFactor;
}

void Component::updateModelMatrix() {
    glm::mat4 transformation = glm::mat4(1.0f);

    transformation = glm::translate(transformation, this->position);
    
    if (this->rotation.x != 0.0f) transformation = glm::rotate(transformation, this->rotation.x, glm::vec3(1, 0, 0));
    if (this->rotation.y != 0.0f) transformation = glm::rotate(transformation, this->rotation.y, glm::vec3(0, 1, 0));
    if (this->rotation.z != 0.0f) transformation = glm::rotate(transformation, this->rotation.z, glm::vec3(0, 0, 1));

    this->modelMatrix = glm::scale(transformation, glm::vec3(this->scaleFactor));
}

void Component::setPosition(float x, float y, float z) {
    this->setPosition(glm::vec3(x,y,z));
}

void Component::setPosition(glm::vec3 position) {
    this->position = position;
    this->updateModelMatrix();
}

void Component::setColor(glm::vec3 color) {
    if (!this->hasModel()) return;

    auto & meshes = this->getModel()->getMeshes();
    uint32_t i=0;
    for (auto & m :  meshes) {
        if (!m.isBoundingBoxMesh()) {
            this->compProps[i].meshProperties.diffuseColor = color;
            i++;
        }
    }
    
    this->markAsDirty();
}

void Component::update(const float delta) {
    for (auto & behavior : this->componentBehavior) {
        behavior->update(delta);
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
    this->updateModelMatrix();
}

void Component::moveForward(const float delta) {
    glm::vec3 frontOfComponent(
        cos(this->rotation.x) * sin(this->rotation.y),
        sin(this->rotation.x),
        cos(this->rotation.x) * cos(this->rotation.y));
    frontOfComponent = glm::normalize(frontOfComponent);
    
    glm::vec3 deltaPosition = this->position;
    deltaPosition = frontOfComponent * delta;
    
    this->setPosition(this->position.x + deltaPosition.x, this->position.y + deltaPosition.y, this->position.z + deltaPosition.z);
}

void Component::setRotation(glm::vec3 rotation) {
    this->rotation = rotation;
}

void Component::scale(float factor) {
    if (factor <= 0) return;
    this->scaleFactor = factor;
    this->updateModelMatrix();
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

bool Component::isDirty() {
    return this->dirty;
}

void Component::markAsDirty() {
    this->dirty = true;
}

void Component::markAsClean() {
    this->dirty = false;
}

Component::~Component() {
    this->componentBehavior.clear();
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

Component * Components::findComponent(const std::string id, const std::string modelId = "") {
    if (modelId.empty()) {
        for (auto & c : this->components) {
            if (c->getId() == id) return c.get();
        }
        
        return nullptr;
    }
    
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

void Component::addComponentBehavior(ComponentBehavior * behavior) {
    this->componentBehavior.push_back(std::unique_ptr<ComponentBehavior>(behavior));
}

void Components::update(const float delta) {
    for (auto & comp : this->components) {
        comp->update(delta);
    }
}

bool Components::checkCollision(const BoundingBox bbox) {
    // very very naive and bad, TODO: improve
    auto & allComps = Components::INSTANCE()->getComponents();
        
    for (auto & c : allComps) { 
        const BoundingBox compModelBbox = 
            Helper::getBBoxAfterModelMatrixMultiply(c->getModel()->getBoundingBox(), c->getModelMatrix());

        if (Helper::checkBBoxIntersection(bbox, compModelBbox)) {                
            for (auto & m : c->getModel()->getMeshes()) {
                const BoundingBox compMeshBbox = 
                    Helper::getBBoxAfterModelMatrixMultiply(m.getBoundingBox(), c->getModelMatrix());
                if (Helper::checkBBoxIntersection(bbox, compMeshBbox)) return true;
            }
        }
    }; 
    
    return false;
}

std::vector<std::tuple<std::string, float>> Components::checkRayIntersection(const glm::vec3 rayOrigin, const glm::vec3 rayDirection) {
    // very very naive and bad, TODO: improve
    auto & allComps = Components::INSTANCE()->getComponents();
        
    std::vector<std::tuple<std::string, float>> hits;
    
    for (auto & c : allComps) { 
        const BoundingBox compModelBbox = Helper::getScaledBBox(c->getModel()->getBoundingBox(), c->getScalingFactor());
            
        float distance = Helper::checkRayIntersection(compModelBbox, rayOrigin, rayDirection, c->getModelMatrix());
        if (distance > 0) {
            for (auto & m : c->getModel()->getMeshes()) {
                if (m.isBoundingBoxMesh()) continue;
                
                const BoundingBox compMeshBbox = Helper::getScaledBBox(m.getBoundingBox(), c->getScalingFactor());
    
                distance = Helper::checkRayIntersection(compMeshBbox, rayOrigin, rayDirection, c->getModelMatrix());
                if (distance > 0) {
                    hits.push_back(std::make_tuple(c->getId(), distance));
                    break;
                }
            }
        }
    }; 
    
    return hits;
}

Components * Components::INSTANCE() {
    if (Components::instance == nullptr) {
        Components::instance = new Components();
    }
    return Components::instance;
}

Components::Components() { }


Components * Components::instance = nullptr;

ComponentBehavior::ComponentBehavior(Component* component) : component(component) { }
ComponentBehavior::~ComponentBehavior() {}

RandomWalkBehavior::RandomWalkBehavior(Component* component) : ComponentBehavior(component) {
    
}

void RandomWalkBehavior::update(const float delta) {
    if (this->component == nullptr) return;

    const float randFloat = Helper::getRandomFloatBetween0and1();
    if (randFloat < 0.01f) {
        this->component->rotate(0, 10,0);
    } else if (randFloat > 0.99f) {
        this->component->rotate(0, -10,0);
    }
    
    this->component->moveForward((randFloat / 50) * delta);
}

RandomWalkBehavior::~RandomWalkBehavior() {}
