#include <R-Engine/UI/Fonts.hpp>

namespace r {

UiFonts::~UiFonts()
{
    if (IsWindowReady()) {
        for (auto &kv : cache) {
            UnloadFont(kv.second);
        }
    }
    cache.clear();
}

const ::Font *UiFonts::load(const std::string &path)
{
    auto it = cache.find(path);
    if (it != cache.end()) return &it->second;
    ::Font f = LoadFont(path.c_str());
    if (f.baseSize == 0) return nullptr;
    cache[path] = f;
    return &cache[path];
}

void UiFonts::unload(const std::string &path)
{
    auto it = cache.find(path);
    if (it == cache.end()) return;
    UnloadFont(it->second);
    cache.erase(it);
}

} /* namespace r */
