#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(triangles) in;
layout (line_strip, max_vertices = 6) out;

struct MeshProperties {
    int ambientTexture;
    int diffuseTexture;
    int specularTexture;
    int normalTexture;
    vec3 ambientColor;
    float emissiveFactor;
    vec3 diffuseColor;
    float opacity;
    vec3 specularColor;
    float shininess;
};

layout(binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec4 camera;
    vec4 sun;
} modelUniforms;
layout(location = 1) in vec3[] fragPosition0;
layout(location = 2) in vec2[] fragTexCoord0;
layout(location = 3) in vec3[] fragNormals0;
layout(location = 4) in vec4[] eye0;
layout(location = 5) in vec4[] light0;
layout(location = 6) flat in MeshProperties[] meshProperties0;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormals;
layout(location = 3) out vec4 eye;
layout(location = 4) out vec4 light;
layout(location = 5) flat out MeshProperties meshProperties;

void main() { 
    int i;

    for (i = 0; i < gl_in.length(); i++) {
        fragPosition = fragPosition0[i];
        fragTexCoord = fragTexCoord0[i];
        fragNormals = fragNormals0[i];
        eye = eye0[i];
        light = light0[i];
        meshProperties = meshProperties0[i];
        
        gl_Position = gl_in[i].gl_Position;
        gl_Position = gl_in[i].gl_Position + vec4(fragNormals, 0);
        EmitVertex();
        EndPrimitive();
    }
}
