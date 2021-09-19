#include "includes/models.h"

SimpleVertex::SimpleVertex(const glm::vec3 & position) {
    this->position = position;
}

ColorVertex::ColorVertex(const glm::vec3 & position) {
    this->position = position;
}

void ColorVertex::setColor(const glm::vec3 & color) {
    this->color = color;
}

glm::vec3 ColorVertex::getColor() {
    return this->color;
}

ModelVertex::ModelVertex(const glm::vec3 & position) {
    this->position = position;
}

VkVertexInputBindingDescription ModelVertex::getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(ModelVertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 5> ModelVertex::getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions{};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(ModelVertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(ModelVertex, normal);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(ModelVertex, uv);

    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(ModelVertex, tangent);

    attributeDescriptions[4].binding = 0;
    attributeDescriptions[4].location = 4;
    attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[4].offset = offsetof(ModelVertex, bitangent);

    return attributeDescriptions;
}

VkVertexInputBindingDescription SimpleVertex::getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(SimpleVertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 1> SimpleVertex::getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 1> attributeDescriptions{};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(SimpleVertex, position);

    return attributeDescriptions;
}

VkVertexInputBindingDescription ColorVertex::getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(ColorVertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 4> ColorVertex::getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(ColorVertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(ColorVertex, normal);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(ColorVertex, uv);

    attributeDescriptions[3].binding = 0;
    attributeDescriptions[3].location = 3;
    attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[3].offset = offsetof(ColorVertex, color);

    return attributeDescriptions;
}

void ColorVertex::setUV(const glm::vec2 & uv) {
    this->uv = uv;
}

void ColorVertex::setNormal(const glm::vec3 & normal) {
    this->normal = normal;
}

glm::vec3 ColorVertex::getPosition() {
    return this->position;
}

glm::vec3 ModelVertex::getPosition() {
    return this->position;
}

void ModelVertex::setUV(const glm::vec2 & uv) {
    this->uv = uv;
}

void ModelVertex::setNormal(const glm::vec3 & normal) {
    this->normal = normal;
}

void ModelVertex::setTangent(const glm::vec3 & tangent) {
    this->tangent = tangent;
}

void ModelVertex::setBitangent(const glm::vec3 & bitangent) {
    this->bitangent = bitangent;
}

glm::vec3 SimpleVertex::getPosition() {
    return this->position;
}

Mesh::Mesh(const std::vector<ModelVertex> & vertices) {
    this->vertices = vertices;
}

Mesh::Mesh(const std::vector<ModelVertex> & vertices, const std::vector<uint32_t> indices) : Mesh(vertices) {
    this->indices = indices;
}

Mesh::Mesh(const std::vector<ModelVertex> & vertices, const std::vector<uint32_t> indices, 
           const TextureInformation & texture, const MaterialInformation & material) : Mesh(vertices, indices) {
    this->texture = texture;
    this->material = material;
}

const std::vector<ModelVertex> & Mesh::getVertices() const {
    return this->vertices;
}

const std::vector<uint32_t> & Mesh::getIndices() const {
    return this->indices;
}

void Mesh::setColor(glm::vec4 color) {
    this->material.diffuseColor = color;
}

void Mesh::setOpacity(float opacity) {
    this->material.opacity = opacity;
}

std::string Mesh::getName() {
    return this->name;
}

void Mesh::setName(std::string name) {
    this->name = name;
}

TextureInformation Mesh::getTextureInformation() {
    return this->texture;
}

MaterialInformation Mesh::getMaterialInformation() {
    return this->material;
}

void Mesh::setTextureInformation(TextureInformation & textures) {
    this->texture = textures;
}

void Mesh::setMaterialInformation(MaterialInformation & materials) {
    this->material = materials;
}

int Texture::getId() {
    return this->id;
}

std::string Texture::getType() {
    return this->type;
}

bool Texture::isValid() {
    return this->valid;
}

VkFormat Texture::getImageFormat() {
    return this->imageFormat;
}

VkImageView & Texture::getTextureImageView() {
    return this->textureImageView;
}

void Texture::setId(const int & id) {
    this->id = id;
}

void Texture::setType(const std::string & type) {
    this->type = type;
}

void Texture::setPath(const std::filesystem::path & path) {
    this->path = path;
}

void Texture::setTextureImage(VkImage & image) {
    this->textureImage = image;
}

void Texture::setTextureImageMemory(VkDeviceMemory & imageMemory) {
    this->textureImageMemory = imageMemory;
}

void Texture::setTextureImageView(VkImageView & imageView) {
    this->textureImageView = imageView;
}

void Texture::load() {
    if (!this->loaded) {
        this->textureSurface = IMG_Load(this->path.string().c_str());
        if (this->textureSurface != nullptr) {
            if (!this->readImageFormat()) {
                logInfo("Unsupported Texture Format: " + this->path.string());
            } else if (this->getSize() != 0) {
                this->valid = true;
            }
        } else logInfo("Failed to load texture: " + this->path.string());
        this->loaded = true;
    }
}

void Texture::cleanUpTexture(const VkDevice & device) {
    if (device == nullptr) return;
    
    if (this->textureImage != nullptr) {
        vkDestroyImage(device, this->textureImage, nullptr);        
        this->textureImage = nullptr;
    }

    if (this->textureImageMemory != nullptr) {
        vkFreeMemory(device, this->textureImageMemory, nullptr);
        this->textureImageMemory = nullptr;
    }

    if (this->textureImageView != nullptr) {
        vkDestroyImageView(device, this->textureImageView, nullptr);
        this->textureImageView = nullptr;
    }
}

uint32_t Texture::getWidth() {
    return this->textureSurface == nullptr ? 0 : this->textureSurface->w; 
}

uint32_t Texture::getHeight() {
    return this->textureSurface == nullptr ? 0 : this->textureSurface->h;     
}

VkDeviceSize Texture::getSize() {
    int channels = this->textureSurface == nullptr ? 0 : textureSurface->format->BytesPerPixel;
    return this->getWidth() * this->getHeight() * channels;
}

void * Texture::getPixels() {
 return this->textureSurface == nullptr ? nullptr : this->textureSurface->pixels;
}

void Texture::freeSurface() {
    if (this->textureSurface != nullptr) {
        SDL_FreeSurface(this->textureSurface);
        this->textureSurface = nullptr;
    }    
}

Texture::Texture(bool empty,  VkExtent2D extent) {
    if (empty) {
        this->textureSurface = SDL_CreateRGBSurface(0,extent.width, extent.height, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

        this->imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
        
        this->loaded = true;
        this->valid = this->textureSurface != nullptr;
    }
}

Texture::Texture(SDL_Surface * surface) {
    if (surface != nullptr) {
        this->textureSurface = surface;
        this->imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
        
        this->loaded = true;
        this->valid = true;
    }
}

Texture::~Texture() {
    this->freeSurface();
}

bool Texture::readImageFormat() {
    if (this->textureSurface == nullptr) return false;

    const int nOfColors = this->textureSurface->format->BytesPerPixel;

    if (nOfColors == 4) {
        if (this->textureSurface->format->Rmask == 0x000000ff) this->imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
        else this->imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
    } else if (nOfColors == 3) {
        Uint32 aMask = 0x000000ff;
        if (this->textureSurface->format->Rmask == 0x000000ff) this->imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
        else {
            this->imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
            aMask = 0xff000000;
        }
                
        // convert to 32 bit
        SDL_Surface * tmpSurface = SDL_CreateRGBSurface(
            0, this->textureSurface->w, this->textureSurface->h, 32, 
            this->textureSurface->format->Rmask, this->textureSurface->format->Gmask, this->textureSurface->format->Bmask, aMask);

        // attempt twice with different pixel format
        if (tmpSurface == nullptr) {
            logError("Conversion Failed. Try something else for: " + this->path.string());

            tmpSurface = SDL_CreateRGBSurface(
                0, this->textureSurface->w, this->textureSurface->h, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);                

            if (tmpSurface == nullptr) {
                tmpSurface = SDL_CreateRGBSurface(
                    0, this->textureSurface->w, this->textureSurface->h, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);                
            }
        }
        
        // either conversion has worked or not
        if (tmpSurface != nullptr) {            
            SDL_SetSurfaceAlphaMod(tmpSurface, 0);
            if (SDL_BlitSurface(this->textureSurface, nullptr, tmpSurface , nullptr) == 0) {
                SDL_FreeSurface(this->textureSurface);
                this->textureSurface = tmpSurface;
            } else {
                logError("SDL_BlitSurface Failed (on conversion): " + std::string(SDL_GetError()));
                return false;
            }
        } else {
            logError("SDL_CreateRGBSurface Failed (on conversion): " + std::string(SDL_GetError()));
            return false;
        }
    } else return false;

    return true;
}

Model::Model(const std::vector< ModelVertex >& vertices, const std::vector< uint32_t > indices, std::string id) : Model(id)
{
    this->file = "";
    this->meshes = { Mesh(vertices, indices) };
    this->loaded = true;
}

Model::Model(const std::string id, const  std::filesystem::path file) : Model(id) {
    this->file = file;

    Assimp::Importer importer;

    #ifdef __ANDROID__
        const char * res = SDL_AndroidGetAssetContent(file.string().c_str());
        const aiScene *scene = importer.ReadFileFromMemory(res, strlen(res),
            aiProcess_Triangulate | aiProcess_GenBoundingBoxes | aiProcess_CalcTangentSpace | aiProcess_FlipUVs | aiProcess_GenSmoothNormals,
            nullptr);
        free((void *) res);
    #endif

    #ifndef __ANDROID__
        const aiScene *scene = importer.ReadFile(this->file.string().c_str(),    
            aiProcess_Triangulate | aiProcess_GenBoundingBoxes | aiProcess_CalcTangentSpace | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);
    #endif

    if (scene == nullptr) {
        logError(importer.GetErrorString());
        return;
    }

    if (scene->HasMeshes()) {
        this->processNode(scene->mRootNode, scene);
        
        this->loaded = true;
    } else logError("Model does not contain meshes");
}

void Model::processNode(const aiNode * node, const aiScene *scene) {
    for(unsigned int i=0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        this->meshes.push_back(this->processMesh(mesh, scene));
    }

    for(unsigned int i=0; i<node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

std::vector<Mesh> & Model::getMeshes() {
    return this->meshes;
}

bool Model::hasBeenLoaded() {
    return this->loaded;
};

std::filesystem::path Model::getFile() {
    return this->file;
}

std::string Model::getId() {
    return this->id;
}

Mesh Model::processMesh(const aiMesh *mesh, const aiScene *scene) {
     std::vector<ModelVertex> vertices;
     std::vector<unsigned int> indices;
     TextureInformation textures;
     MaterialInformation materials;

     const std::string name = mesh->mName.C_Str();
     
     if (scene->HasMaterials()) {
        const aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        std::unique_ptr<aiColor4D> ambient(new aiColor4D());
        std::unique_ptr<aiColor4D> diffuse(new aiColor4D());
        std::unique_ptr<aiColor4D> specular(new aiColor4D());
        
        float opacity = 1.0f;
        aiGetMaterialFloat(material, AI_MATKEY_OPACITY, &opacity);
        if (opacity > 0.0f) materials.opacity = opacity;
        
        const glm::vec3 nullVec3 = glm::vec3(0.0f);
        
        if (aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, ambient.get()) == aiReturn_SUCCESS) {
            glm::vec3 ambientVec3 = glm::vec3(ambient->r, ambient->g, ambient->b);
            if (ambientVec3 != nullVec3) materials.ambientColor = ambientVec3;
        };
        if (aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, diffuse.get()) == aiReturn_SUCCESS) {
            glm::vec3 diffuseVec3 = glm::vec3(diffuse->r, diffuse->g, diffuse->b);
            if (diffuseVec3 != nullVec3) materials.diffuseColor = diffuseVec3;
        };
        if (aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, specular.get()) == aiReturn_SUCCESS) {
            glm::vec3 specularVec3 = glm::vec3(specular->r, specular->g, specular->b);
            if (specularVec3 != nullVec3) materials.specularColor = specularVec3;
        };

        float shiny = 1.0f;
        aiGetMaterialFloat(material, AI_MATKEY_SHININESS, &shiny);
        if (shiny > 0.0f) materials.shininess = shiny;

        textures = this->addTextures(material);
     }
     
     if (mesh->mNumVertices > 0) vertices.reserve(mesh->mNumVertices);
     for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
         ModelVertex vertex(glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z));

         if (mesh->HasNormals())
             vertex.setNormal(glm::normalize(glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z)));

         if(mesh->HasTextureCoords(0)) {
             vertex.setUV(glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y));
         } else vertex.setUV(glm::vec2(0.0f, 0.0f));

         if (mesh->HasTangentsAndBitangents()) {
             if (mesh->mTangents->Length() == mesh->mNumVertices)
                 vertex.setTangent(glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z));

             if (mesh->mBitangents->Length() == mesh->mNumVertices)
                 vertex.setBitangent(glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z));
         }

         vertices.push_back(vertex);
     }

     for(unsigned int i = 0; i < mesh->mNumFaces; i++) {
         const aiFace face = mesh->mFaces[i];

         for(unsigned int j = 0; j < face.mNumIndices; j++) indices.push_back(face.mIndices[j]);
     }

     Mesh m = Mesh(vertices, indices, textures, materials);
     m.setName(name);
     
     return m;
}

void Model::setMaterialInformation(MaterialInformation & materials) {
    for (Mesh & m : this->meshes) {
        m.setMaterialInformation(materials);
    }
}

void Model::correctTexturePath(char * path) {
    int index = 0;

    while(path[index] == '\0') index++;

    if(index != 0) {
        int i = 0;
        while(path[i + index] != '\0') {
            path[i] = path[i + index];
            i++;
        }
        path[i] = '\0';
    }
}

TextureInformation Model::addTextures(const aiMaterial * mat) {
    TextureInformation textureInfo;
    
    if (mat->GetTextureCount(aiTextureType_AMBIENT) > 0) {
        aiString str;
        mat->GetTexture(aiTextureType_AMBIENT, 0, &str);
        
        if (str.length > 0) this->correctTexturePath(str.data);
        textureInfo.ambientTextureLocation = (this->file.parent_path() / std::filesystem::path(str.C_Str())).string();
    }

    if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        aiString str;
        mat->GetTexture(aiTextureType_DIFFUSE, 0, &str);

        if (str.length > 0) this->correctTexturePath(str.data);
        textureInfo.diffuseTextureLocation = (this->file.parent_path() / std::filesystem::path(str.C_Str())).string();
    }

    if (mat->GetTextureCount(aiTextureType_SPECULAR) > 0) {
        aiString str;
        mat->GetTexture(aiTextureType_SPECULAR, 0, &str);

        if (str.length > 0) this->correctTexturePath(str.data);
        textureInfo.specularTextureLocation = (this->file.parent_path().string() / std::filesystem::path(str.C_Str())).string();
    }
    
    if (mat->GetTextureCount(aiTextureType_NORMALS) > 0) {
        aiString str;
        mat->GetTexture(aiTextureType_NORMALS, 0, &str);

        if (str.length > 0) this->correctTexturePath(str.data);
        textureInfo.normalTextureLocation = (this->file.parent_path().string() / std::filesystem::path(str.C_Str())).string();
    }

    return textureInfo;
}

Model::Model(std::string id) {
    this->id = id;
}

Model::~Model() {}

Models * Models::INSTANCE() {
    if (Models::instance == nullptr) {
        Models::instance = new Models();
    }
    return Models::instance;
}

void Models::addModel(const std::string id, const  std::filesystem::path file)
{
    std::unique_ptr<Model> model = std::make_unique<Model>(id, file);
    if (!model->hasBeenLoaded()) return;

    auto & meshes = model->getMeshes();
    for (Mesh & m : meshes) {
        this->processTextures(m);
        TextureInformation info =  m.getTextureInformation();
    }
    
    this->models.push_back(std::move(model));
}

void Models::addTextModel(std::string id, std::string font, std::string text, uint16_t size) {
    TTF_Font * f = TTF_OpenFont(font.c_str(), size);

    if (f != nullptr) {
        TTF_SetFontStyle(f, TTF_STYLE_NORMAL);
        const SDL_Color bg = { 255, 255, 255 };
        const SDL_Color fg = { 0, 0, 0 };

        SDL_Surface * tmp = TTF_RenderUTF8_Shaded(f, text.c_str(), fg, bg);
        
        bool succeeded = false;
        if (tmp != nullptr) {

            SDL_PixelFormat *format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA8888);
            std::unique_ptr<Texture> tex = std::make_unique<Texture>(SDL_ConvertSurface(tmp, format, 0));

            SDL_FreeFormat(format);
            SDL_FreeSurface(tmp);
            
            if (tex != nullptr && tex->isValid()) {
                VkExtent2D extent = {
                  tex->getWidth(), tex->getHeight()  
                };
                std::unique_ptr<Model> m(Models::createPlaneModel(id, extent));
                if (m != nullptr && m->hasBeenLoaded()) {
                    
                    tex->setId(this->textures.size());
                    tex->setPath(m->getId());

                    TextureInformation texInfo;
                    texInfo.diffuseTexture = tex->getId();
                    texInfo.diffuseTextureLocation = m->getId();
                    m->getMeshes()[0].setTextureInformation(texInfo);

                    this->textures[m->getId()] = std::move(tex);    
                    this->models.push_back(std::move(m));
                    
                    succeeded = true;
                }
            }
        }

        TTF_CloseFont(f);
        
        if (succeeded) return;
    }
    
    logError("Failed to add Text Model");
}

Model * Models::createPlaneModel(std::string id, VkExtent2D extent) {
    const float zDirNormal = 1.0f;
    const float w = extent.width / extent.height;
    const float h = 1;

    std::vector<ModelVertex> vertices;
    
    ModelVertex one(glm::vec3(-w/2, -h/2, 0.0f));
    one.setUV(glm::vec2(-1.0f, 1.0f));
    one.setNormal(glm::vec3(-1, -1, zDirNormal));
    vertices.push_back(one);

    ModelVertex two(glm::vec3(-w/2, h/2, 0.0f));
    two.setUV(glm::vec2(-1.0f, 0.0f));
    two.setNormal(glm::vec3(-1, 1, zDirNormal));
    vertices.push_back(two);

    ModelVertex three(glm::vec3(w/2, h/2, 0.0f));
    three.setUV(glm::vec2(0.0f, 0.0f));
    three.setNormal(glm::vec3(1, 1, zDirNormal));
    vertices.push_back(three);

    ModelVertex four(glm::vec3(w/2, -h/2, 0.0f));
    four.setUV(glm::vec2(0.0f, 1.0f));
    four.setNormal(glm::vec3(1, -1, zDirNormal));
    vertices.push_back(four);

    ModelVertex backOne(glm::vec3(-w/2, -h/2, 0.0f));
    backOne.setNormal(glm::vec3(-1, -1, -zDirNormal));
    vertices.push_back(backOne);

    ModelVertex backTwo(glm::vec3(-w/2, h/2, 0.0f));
    backTwo.setNormal(glm::vec3(-1, 1, -zDirNormal));
    vertices.push_back(backTwo);

    ModelVertex backThree(glm::vec3(w/2, h/2, 0.0f));
    backThree.setNormal(glm::vec3(1, 1, -zDirNormal));
    vertices.push_back(backThree);

    ModelVertex backFour(glm::vec3(w/2, -h/2, 0.0f));
    backFour.setNormal(glm::vec3(1, -1, -zDirNormal));
    vertices.push_back(backFour);

    
    std::vector<uint32_t> indices;
    
    indices.push_back(3);
    indices.push_back(1);
    indices.push_back(0);

    indices.push_back(2);
    indices.push_back(1);
    indices.push_back(3);

    indices.push_back(4);
    indices.push_back(5);
    indices.push_back(7);

    indices.push_back(7);
    indices.push_back(5);
    indices.push_back(6);
    
    return new Model(vertices, indices, id);
}

VkImageView Models::findTextureImageViewById(int id) {
    for (auto & t : this->textures) {
        if (t.second->getId() == id) return t.second->getTextureImageView();
    }
    
    return nullptr;
}

BufferSummary Models::getModelsBufferSizes(bool printInfo) {
    BufferSummary bufferSizes;
    
    for (auto & model : this->models) {
        auto meshes = model->getMeshes();
        for (Mesh & mesh : meshes) {
            VkDeviceSize vertexSize = mesh.getVertices().size();
            VkDeviceSize indexSize = mesh.getIndices().size();

            bufferSizes.vertexBufferSize += vertexSize * sizeof(class ModelVertex);
            bufferSizes.indexBufferSize += indexSize * sizeof(uint32_t);
            bufferSizes.ssboBufferSize += sizeof(struct MeshProperties);
        }
    }
    
    if (printInfo) {
        logInfo("Models Vertex Buffer Size: " + std::to_string(bufferSizes.vertexBufferSize / MEGA_BYTE) + " MB");
        logInfo("Models Index Buffer Size: " + std::to_string(bufferSizes.indexBufferSize / MEGA_BYTE) + " MB");
        logInfo("Models SSBO Buffer Size: " + std::to_string(bufferSizes.ssboBufferSize / MEGA_BYTE) + " MB");
    }
    
    return bufferSizes;
}


void Models::processTextures(Mesh & mesh) {
    TextureInformation textureInfo = mesh.getTextureInformation();
    
    std::map<std::string, std::unique_ptr<Texture>>::iterator val;
    
    if (!textureInfo.ambientTextureLocation.empty()) {
        val = this->textures.find(textureInfo.ambientTextureLocation);
        if (val == this->textures.end()) {
            std::unique_ptr<Texture> texture = std::make_unique<Texture>();
            texture->setPath(textureInfo.ambientTextureLocation);
            texture->load();
            if (texture->isValid()) {
                textureInfo.ambientTexture = static_cast<int>(this->textures.empty() ? 0 : this->textures.size());
                texture->setId(textureInfo.ambientTexture);
                this->textures[textureInfo.ambientTextureLocation] = std::move(texture);
            }
        }
    }

    if (!textureInfo.diffuseTextureLocation.empty()) {
        val = this->textures.find(textureInfo.diffuseTextureLocation);
        if (val == this->textures.end()) {
            std::unique_ptr<Texture> texture = std::make_unique<Texture>();
            texture->setPath(textureInfo.diffuseTextureLocation);
            texture->load();
            if (texture->isValid()) {
                textureInfo.diffuseTexture = static_cast<int>(this->textures.empty() ? 0 : this->textures.size());
                texture->setId(textureInfo.diffuseTexture);
                this->textures[textureInfo.diffuseTextureLocation] = std::move(texture);
            }
        }
    }

    if (!textureInfo.specularTextureLocation.empty()) {
        val = this->textures.find(textureInfo.specularTextureLocation);
        if (val == this->textures.end()) {
            std::unique_ptr<Texture> texture = std::make_unique<Texture>();
            texture->setPath(textureInfo.specularTextureLocation);
            texture->load();
            if (texture->isValid()) {
                textureInfo.specularTexture = static_cast<int>(this->textures.empty() ? 0 : this->textures.size());
                texture->setId(textureInfo.specularTexture);
                this->textures[textureInfo.specularTextureLocation] = std::move(texture);
            }
        }
    }

    if (!textureInfo.normalTextureLocation.empty()) {
        val = this->textures.find(textureInfo.normalTextureLocation);
        if (val == this->textures.end()) {
            std::unique_ptr<Texture> texture = std::make_unique<Texture>();
            texture->setPath(textureInfo.normalTextureLocation);
            texture->load();
            if (texture->isValid()) {
                textureInfo.normalTexture = static_cast<int>(this->textures.empty() ? 0 : this->textures.size());
                texture->setId(textureInfo.normalTexture);
                this->textures[textureInfo.normalTextureLocation] = std::move(texture);
            }
        }
    }
    
    mesh.setTextureInformation(textureInfo);
}

std::vector<std::unique_ptr<Model>> & Models::getModels() {
    return this->models;
}

Model * Models::findModel(std::string id) {
    for (auto & m : this->models) {
        if (m->getId().compare(id) == 0) return m.get();
    }
    return nullptr;
}

void Models::clear() {
    this->models.clear();
    this->textures.clear();
}

std::map< std::string, std::unique_ptr< Texture >>& Models::getTextures()
{
    return this->textures;
}

std::vector<std::string>  Models::getModelIds() {
    std::vector<std::string> modelIds;
    for (auto & m : this->models) {
        modelIds.push_back(m->getId());
    }
    return modelIds;
}

void Models::setMaterialInformation(MaterialInformation & material) {
    for (auto & model : this->models) {
        model->setMaterialInformation(material);
    }
}


void Models::cleanUpTextures(const VkDevice & device) {
    logInfo("Destroying Model Textures...");

    for (auto & texture : this->textures) {
        texture.second->cleanUpTexture(device);
    }
    
    logInfo("Destroyed Model Textures");
}

Models::~Models() {
    this->clear();
}

Models * Models::instance = nullptr;
const std::string Models::AMBIENT_TEXTURE = "ambient";
const std::string Models::DIFFUSE_TEXTURE = "diffuse";
const std::string Models::SPECULAR_TEXTURE = "specular";
const std::string Models::TEXTURE_NORMALS = "normals";
