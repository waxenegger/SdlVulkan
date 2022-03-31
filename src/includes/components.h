#ifndef SRC_INCLUDES_COMPONENTS_H_
#define SRC_INCLUDES_COMPONENTS_H_

#include "models.h"
#include "helper.h"

struct ComponentProperties final {
    public:
        MeshProperties meshProperties;
};


class Component;
class ComponentBehavior {
    protected:
        Component * component = nullptr;
        
    public:
        ComponentBehavior(const ComponentBehavior&) = delete;
        ComponentBehavior& operator=(const ComponentBehavior &) = delete;
        ComponentBehavior(ComponentBehavior &&) = delete;
        
        ComponentBehavior(Component * component);

        virtual void update(const float delta) = 0;
        virtual ~ComponentBehavior();
};

class RandomWalkBehavior : public ComponentBehavior {
    public:
        RandomWalkBehavior(const RandomWalkBehavior&) = delete;
        RandomWalkBehavior& operator=(const RandomWalkBehavior &) = delete;
        RandomWalkBehavior(RandomWalkBehavior &&) = delete;

        RandomWalkBehavior(Component * component);
        
        void update(const float delta);
        ~RandomWalkBehavior();
};


class Component final {
    private:
        Model * model = nullptr;
        glm::mat4 modelMatrix;
        uint32_t ssboIndex;

        std::vector<ComponentProperties> compProps;
        std::vector<std::unique_ptr<ComponentBehavior>> componentBehavior;
        
        std::string id = "";
        
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 rotation = glm::vec3(0.0f);
        float scaleFactor = 1.0f;
        
        bool dirty = true;
        bool visible = true;

        void updateModelMatrix();

    public:
        Component(const Component&) = delete;
        Component& operator=(const Component &) = delete;
        Component(Component &&) = delete;

        Component(std::string id);
        Component(std::string id, Model * model);
        MeshProperties & getModelProperties();
        ~Component();
        
        bool hasModel();
        Model * getModel();
        void setPosition(float x, float y, float z);
        void setPosition(glm::vec3 position);
        glm::vec3 getPosition();
        void setRotation(glm::vec3 rotation);
        void setColor(glm::vec3 color);
        void rotate(int xAxis = 0, int yAxis = 0, int zAxis = 0);
        void moveForward(const float delta);
        void scale(float factor);
        glm::mat4 getModelMatrix();
        glm::vec3 getRotation();
        bool isVisible();
        void setVisible(bool visible);
        std::string getId();
        void setSsboIndex(const uint32_t index);
        uint32_t getSsboIndex();
        VkDeviceSize getSsboSize();

        std::vector<ComponentProperties> & getProperties();
        void addComponentProperties(const ComponentProperties props);
        void addComponentBehavior(ComponentBehavior * behavior);
        void update(const float delta);
        
        bool isDirty();
        void markAsDirty();
        void markAsClean();
};

class Components final {
    private:
        static Components * instance;
        std::vector<std::unique_ptr<Component>> components;
        std::map<std::string,  std::vector<Component *>> componentsByModel;
        uint32_t ssboIndex = 0;
        
        Components();
    public:
        Components(const Components&) = delete;
        Components& operator=(const Components &) = delete;
        Components(Components &&) = delete;

        static Components * INSTANCE();
        Component * addComponent(Component * component);
        Component * addComponentFromModel(const std::string id, const std::string modelId);
        std::vector<Component *> getAllComponentsForModel(std::string model);
        std::vector<std::unique_ptr<Component>> & getComponents();
        Component * findComponent(const std::string id, const std::string modelId);
        
        void update(const float delta);
        
        bool checkCollision(const BoundingBox bbox1);
        std::vector<std::tuple<std::string, float>> checkRayIntersection(const glm::vec3 rayOrigin, const glm::vec3 rayDirection);
        
        ~Components();
};

#endif

