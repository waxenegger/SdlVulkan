#models
/home/harald/Downloads/1.2.189.0/x86_64/bin/glslc --target-env=vulkan1.0 /opt/projects/SdlVulkan/shaders/models.vert -o /tmp/models-vert.spv
/home/harald/Downloads/1.2.189.0/x86_64/bin/spirv-opt /tmp/models-vert.spv -o /opt/projects/SdlVulkan/app/src/main/assets/shaders/models-vert.spv
/home/harald/Downloads/1.2.189.0/x86_64/bin/glslc --target-env=vulkan1.0 /opt/projects/SdlVulkan/shaders/models.frag -o /tmp/models-frag.spv
/home/harald/Downloads/1.2.189.0/x86_64/bin/spirv-opt /tmp/models-frag.spv -o /opt/projects/SdlVulkan/app/src/main/assets/shaders/models-frag.spv
#skybox
/home/harald/Downloads/1.2.189.0/x86_64/bin/glslc --target-env=vulkan1.0 /opt/projects/SdlVulkan/shaders/skybox.vert -o /tmp/skybox-vert.spv
/home/harald/Downloads/1.2.189.0/x86_64/bin/spirv-opt /tmp/skybox-vert.spv -o /opt/projects/SdlVulkan/app/src/main/assets/shaders/skybox-vert.spv
/home/harald/Downloads/1.2.189.0/x86_64/bin/glslc --target-env=vulkan1.0 /opt/projects/SdlVulkan/shaders/skybox.frag -o /tmp/skybox-frag.spv
/home/harald/Downloads/1.2.189.0/x86_64/bin/spirv-opt /tmp/skybox-frag.spv -o /opt/projects/SdlVulkan/app/src/main/assets/shaders/skybox-frag.spv
