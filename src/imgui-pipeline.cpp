#include "includes/graphics.h"

ImGuiPipeline::ImGuiPipeline(const Renderer * renderer) : GraphicsPipeline(renderer) { }

bool ImGuiPipeline::createGraphicsPipeline(const VkPushConstantRange & pushConstantRange) {
    if (this->renderer == nullptr || !this->renderer->isReady()) return false;
    
    this->pushConstantRange = pushConstantRange;

    if (!this->createDescriptorPool()) {
        logError("Failed to create ImGui Pipeline Descriptor Pool");
        return false;
    }

    return true;
}

bool ImGuiPipeline::updateGraphicsPipeline() {
    return (this->renderer == nullptr || !this->renderer->isReady());
}

bool ImGuiPipeline::createDescriptorPool() {
    if (this->renderer == nullptr || !this->renderer->isReady()) return false;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui::StyleColorsDark();

    const uint8_t LIMIT = 10;
    std::array<VkDescriptorPoolSize, 11> pool_sizes =
    {
        {
            {VK_DESCRIPTOR_TYPE_SAMPLER, LIMIT },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, LIMIT },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, LIMIT },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, LIMIT },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, LIMIT },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, LIMIT },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LIMIT },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, LIMIT },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, LIMIT },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, LIMIT },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, LIMIT }
        }
    };
    
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = LIMIT * 11;
    pool_info.poolSizeCount = LIMIT;
    pool_info.pPoolSizes = pool_sizes.data();
        
    VkResult ret = vkCreateDescriptorPool(this->renderer->getLogicalDevice(), &pool_info, nullptr, &this->descriptorPool);
    if (ret != VK_SUCCESS) {
       logError("Failed to Create Descriptor Pool for ImGui!");
       return false;
    }
    
    // Setup Platform/Renderer backends
    if (!ImGui_ImplSDL2_InitForVulkan(this->renderer->getGraphicsContext()->getSdlWindow())) {
       logError("Failed to init ImGui Sdl!");
       return false;
    }

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = this->renderer->getGraphicsContext()->getVulkanInstance();
    init_info.PhysicalDevice = this->renderer->getPhysicalDevice();
    init_info.Device = this->renderer->getLogicalDevice();
    init_info.QueueFamily = this->renderer->getGraphicsQueueIndex();
    init_info.Queue = this->renderer->getGraphicsQueue();
    init_info.DescriptorPool = this->descriptorPool;
    init_info.MinImageCount = this->renderer->getImageCount();
    init_info.ImageCount = this->renderer->getImageCount();

    if (!ImGui_ImplVulkan_Init(&init_info, this->renderer->getRenderPass())) {
       logError("Failed to init ImGui for Vulkan!");
       return false;        
    }
    
    VkCommandBuffer buffer = Helper::allocateAndBeginCommandBuffer(this->renderer->getLogicalDevice(), this->renderer->getCommandPool());
    ImGui_ImplVulkan_CreateFontsTexture(buffer);
    Helper::endCommandBufferWithSubmit(this->renderer->getLogicalDevice(), this->renderer->getCommandPool(), this->renderer->getGraphicsQueue(), buffer);
    ImGui::GetIO().Fonts->Build();
    
    ImGui_ImplVulkan_DestroyFontUploadObjects();
    
    return true;
}

bool ImGuiPipeline::createDescriptorSetLayout() {
    return (this->renderer == nullptr || !this->renderer->isReady());
}

bool ImGuiPipeline::createDescriptorSets() {
    return (this->renderer == nullptr || !this->renderer->isReady());
}

void ImGuiPipeline::draw(const VkCommandBuffer & commandBuffer, const uint16_t commandBufferIndex) {
    if (this->renderer == nullptr || !this->renderer->isReady() || !this->isEnabled()) return;
    
    const VkExtent2D extent = this->renderer->getSwapChainExtent();
    
    ImGui::GetIO().DisplaySize = ImVec2(extent.width, extent.height);

    //ImGui_ImplVulkan_NewFrame();
    //ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);

    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowSize(ImVec2(150, 500), ImGuiCond_Always);
    
    ImGui::Begin(std::string("FPS:\t" + std::to_string(this->renderer->getFrameRate())).c_str() , nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
    
    
    
    
    auto hits = Helper::getCameraCrossHairIntersection();
    if (!hits.empty()) {
        ImGui::PushID(0);
        ImGui::BeginListBox("", ImVec2(150, 50));
        for (auto h : hits) {
            ImGui::Selectable(std::string(std::get<0>(h) + "<" + std::to_string(std::get<1>(h))).c_str());
        }
        ImGui::EndListBox();
        ImGui::PopID();
    }

    
    

    ImGui::End();

    ImGui::PopStyleVar();
    
    ImGui::Render();

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

bool ImGuiPipeline::canRender() const {
    return true;
}

void ImGuiPipeline::update() { }

ImGuiPipeline::~ImGuiPipeline() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    
    ImGui::DestroyContext();
}

