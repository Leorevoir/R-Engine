#include <R-Engine/Components/Material3d.hpp>

/**
* public
*/

r::Material3d::Material3d(const ShaderHandle shader) noexcept : _shader_id(shader)
{
    /* __ctor__ */
}

void r::Material3d::set_shader(ShaderHandle shader) noexcept
{
    this->_shader_id = shader;
}

void r::Material3d::set_uniform(const std::string &name, const std::any &value) noexcept
{
    this->_uniforms[name] = value;
}

void r::Material3d::set_uniform_loc(const std::string &name, const ShaderLocation location) noexcept
{
    this->_uniform_locations[name] = location;
}

r::ShaderHandle r::Material3d::get_shader() const noexcept
{
    return this->_shader_id;
}

const std::unordered_map<std::string, std::any> &r::Material3d::get_uniforms() const noexcept
{
    return this->_uniforms;
}

r::ShaderLocation r::Material3d::get_uniform_loc(const std::string &name) const noexcept
{
    const auto it = this->_uniform_locations.find(name);

    if (it == this->_uniform_locations.end()) {
        return ShaderInvalidLocation;
    }
    return it->second;
}
