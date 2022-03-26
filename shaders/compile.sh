#models
/opt/projects/SdlVulkan/thirdParty/VulkanSDK/1.2.189.0/x86_64/bin/glslc --target-env=vulkan1.0 /opt/projects/SdlVulkan/shaders/models.vert -o /tmp/models-vert.spv
/opt/projects/SdlVulkan/thirdParty/VulkanSDK/1.2.189.0/x86_64/bin/spirv-opt /tmp/models-vert.spv -o /opt/projects/SdlVulkan/assets/shaders/models-vert.spv
/opt/projects/SdlVulkan/thirdParty/VulkanSDK/1.2.189.0/x86_64/bin/glslc --target-env=vulkan1.0 /opt/projects/SdlVulkan/shaders/models.frag -o /tmp/models-frag.spv
/opt/projects/SdlVulkan/thirdParty/VulkanSDK/1.2.189.0/x86_64/bin/spirv-opt /tmp/models-frag.spv -o /opt/projects/SdlVulkan/assets/shaders/models-frag.spv
#models-memory
/opt/projects/SdlVulkan/thirdParty/VulkanSDK/1.2.189.0/x86_64/bin/glslc --target-env=vulkan1.0 /opt/projects/SdlVulkan/shaders/models-memory.vert -o /tmp/models-memory-vert.spv
/opt/projects/SdlVulkan/thirdParty/VulkanSDK/1.2.189.0/x86_64/bin/spirv-opt /tmp/models-memory-vert.spv -o /opt/projects/SdlVulkan/assets/shaders/models-memory-vert.spv
/opt/projects/SdlVulkan/thirdParty/VulkanSDK/1.2.189.0/x86_64/bin/glslc --target-env=vulkan1.0 /opt/projects/SdlVulkan/shaders/models-memory.frag -o /tmp/models-memory-frag.spv
/opt/projects/SdlVulkan/thirdParty/VulkanSDK/1.2.189.0/x86_64/bin/spirv-opt /tmp/models-memory-frag.spv -o /opt/projects/SdlVulkan/assets/shaders/models-memory-frag.spv
#skybox
/opt/projects/SdlVulkan/thirdParty/VulkanSDK/1.2.189.0/x86_64/bin/glslc --target-env=vulkan1.0 /opt/projects/SdlVulkan/shaders/skybox.vert -o /tmp/skybox-vert.spv
/opt/projects/SdlVulkan/thirdParty/VulkanSDK/1.2.189.0/x86_64/bin/spirv-opt /tmp/skybox-vert.spv -o /opt/projects/SdlVulkan/assets/shaders/skybox-vert.spv
/opt/projects/SdlVulkan/thirdParty/VulkanSDK/1.2.189.0/x86_64/bin/glslc --target-env=vulkan1.0 /opt/projects/SdlVulkan/shaders/skybox.frag -o /tmp/skybox-frag.spv
/opt/projects/SdlVulkan/thirdParty/VulkanSDK/1.2.189.0/x86_64/bin/spirv-opt /tmp/skybox-frag.spv -o /opt/projects/SdlVulkan/assets/shaders/skybox-frag.spv
#screen midpoint
/opt/projects/SdlVulkan/thirdParty/VulkanSDK/1.2.189.0/x86_64/bin/glslc --target-env=vulkan1.0 /opt/projects/SdlVulkan/shaders/midpoint.vert -o /tmp/midpoint-vert.spv
/opt/projects/SdlVulkan/thirdParty/VulkanSDK/1.2.189.0/x86_64/bin/spirv-opt /tmp/midpoint-vert.spv -o /opt/projects/SdlVulkan/assets/shaders/midpoint-vert.spv
/opt/projects/SdlVulkan/thirdParty/VulkanSDK/1.2.189.0/x86_64/bin/glslc --target-env=vulkan1.0 /opt/projects/SdlVulkan/shaders/midpoint.frag -o /tmp/midpoint-frag.spv
/opt/projects/SdlVulkan/thirdParty/VulkanSDK/1.2.189.0/x86_64/bin/spirv-opt /tmp/midpoint-frag.spv -o /opt/projects/SdlVulkan/assets/shaders/midpoint-frag.spv
#normals
/opt/projects/SdlVulkan/thirdParty/VulkanSDK/1.2.189.0/x86_64/bin/glslc --target-env=vulkan1.0 /opt/projects/SdlVulkan/shaders/normals.geom -o /tmp/normals-geom.spv
/opt/projects/SdlVulkan/thirdParty/VulkanSDK/1.2.189.0/x86_64/bin/spirv-opt /tmp/normals-geom.spv -o /opt/projects/SdlVulkan/assets/shaders/normals-geom.spv
