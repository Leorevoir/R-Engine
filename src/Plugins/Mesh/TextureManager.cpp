#include <R-Engine/Core/Filepath.hpp>
#include <R-Engine/Core/Logger.hpp>
#include <R-Engine/Plugins/MeshPlugin.hpp>

/**
* public
*/

r::TextureManager::~TextureManager()
{
    for (auto &[path, texture] : _textures) {
        UnloadTexture(texture);
        Logger::debug("Unloaded texture: " + path);
    }
}

const ::Texture2D *r::TextureManager::load(const std::string &path)
{
    const auto it = _textures.find(path);

    if (it == _textures.end()) {
        return _load(path);
    }

    return &it->second;
}

void r::TextureManager::unload(const std::string &path)
{
    const auto it = _textures.find(path);

    if (it == _textures.end()) {
        Logger::warn("Texture not found in manager: " + path);
        return;
    }
    UnloadTexture(it->second);
    _textures.erase(it);
    Logger::debug("Unloaded texture: " + path);
}

/**
* private
*/

const ::Texture2D *r::TextureManager::_load(const std::string &path)
{
    if (!path::exists(path)) {
        Logger::error("Texture file does not exist: " + path);
        return nullptr;
    }

    const auto texture = LoadTexture(path.c_str());

    if (texture.id == 0) {
        Logger::error("Failed to load texture: " + path);
        return nullptr;
    }

    _textures[path] = texture;
    Logger::debug("Loaded texture: " + path);
    return &_textures[path];
}
