#ifndef SRC_INCLUDES_COMPONENTS_H_
#define SRC_INCLUDES_COMPONENTS_H_

#include "models.h"

class Component final {
    private:
        Model * model = nullptr;
        uint32_t ssboIndex;
        
        std::string id = "";
        
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 rotation = glm::vec3(0.0f);
        float scaleFactor = 1.0f;
        
        bool visible = true;
    public:
        Component(std::string id);
        Component(std::string id, Model * model);
        MeshProperties & getModelProperties();
        bool hasModel();
        Model * getModel();
        void setPosition(float x, float y, float z);
        void setPosition(glm::vec3 position);
        glm::vec3 getPosition();
        void setRotation(glm::vec3 rotation);
        void rotate(int xAxis = 0, int yAxis = 0, int zAxis = 0);
        void move(float xAxis, float yAxis, float zAxis);
        void scale(float factor);
        glm::mat4 getModelMatrix(bool includeRotation = true);
        glm::vec3 getRotation();
        bool isVisible();
        void setVisible(bool visible);
        std::string getId();
        void setSsboIndex(const uint32_t index);
        uint32_t getSsboIndex();
        VkDeviceSize getSsboSize();
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
        ~Components();
};

#endif

