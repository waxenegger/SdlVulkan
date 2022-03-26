#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 camera;
    vec4 sun;
} modelUniforms;

void main() {
    mat4 viewMatrix = modelUniforms.view;
    viewMatrix[3] = vec4(0.0f, 0.0f, 0.0f, 1.0f);

    gl_Position = modelUniforms.proj * viewMatrix * vec4(inPosition.xyz, 1.0);
}

