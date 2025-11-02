#include <R-Engine/Plugins/PostProcessingPlugin.hpp>

#include <R-Engine/Application.hpp>

#include <R-Engine/Core/Backend.hpp>
#include <R-Engine/Core/Filepath.hpp>
#include <R-Engine/Core/FrameTime.hpp>
#include <R-Engine/Core/Logger.hpp>

#include <R-Engine/Plugins/RenderPlugin.hpp>
#include <R-Engine/Plugins/Ui/Systems.hpp>
#include <R-Engine/Plugins/WindowPlugin.hpp>

namespace {

/**
 * globals
 */
// clang-format off

static struct {
        const std::string name;
        ::Shader shader;
        i32 resolutionLoc;
        i32 timeLoc;
        i32 contrastLoc;
} g_shaders[] = {
    {
        .name = "shaders/postprocessing/grayscale.frag",
        .shader = {},
        .resolutionLoc = -1,
        .timeLoc = -1,
        .contrastLoc = -1},
    {
        .name = "shaders/postprocessing/posterization.frag",
        .shader = {},
        .resolutionLoc = -1,
        .timeLoc = -1,
        .contrastLoc = -1},
    {
        .name = "shaders/postprocessing/dream_vision.frag",
        .shader = {},
        .resolutionLoc = -1,
        .timeLoc = -1,
        .contrastLoc = -1},
    {
        .name = "shaders/postprocessing/pixelizer.frag",
        .shader = {},
        .resolutionLoc = -1,
        .timeLoc = -1,
        .contrastLoc = -1},
    {
        .name = "shaders/postprocessing/cross_hatching.frag",
        .shader = {},
        .resolutionLoc = -1,
        .timeLoc = -1,
        .contrastLoc = -1},
    {
        .name = "shaders/postprocessing/cross_stitching.frag",
        .shader = {},
        .resolutionLoc = -1,
        .timeLoc = -1,
        .contrastLoc = -1},
    {
        .name = "shaders/postprocessing/predator.frag",
        .shader = {},
        .resolutionLoc = -1,
        .timeLoc = -1,
        .contrastLoc = -1},
    {
        .name = "shaders/postprocessing/scanlines.frag",
        .shader = {},
        .resolutionLoc = -1,
        .timeLoc = -1,
        .contrastLoc = -1},
    {
        .name = "shaders/postprocessing/fisheye.frag",
        .shader = {},
        .resolutionLoc = -1,
        .timeLoc = -1,
        .contrastLoc = -1},
    {
        .name = "shaders/postprocessing/sobel.frag",
        .shader = {},
        .resolutionLoc = -1,
        .timeLoc = -1,
        .contrastLoc = -1},
    {
        .name = "shaders/postprocessing/bloom.frag",
        .shader = {},
        .resolutionLoc = -1,
        .timeLoc = -1,
        .contrastLoc = -1
    },
    {
        .name = "shaders/postprocessing/blur.frag",
        .shader = {},
        .resolutionLoc = -1,
        .timeLoc = -1,
        .contrastLoc = -1
    },
    {
        .name = "shaders/postprocessing/contrast.frag",
        .shader = {},
        .resolutionLoc = -1,
        .timeLoc = -1,
        .contrastLoc = -1
    },
    {
        .name = "shaders/postprocessing/protanopia.frag",
        .shader = {},
        .resolutionLoc = -1,
        .timeLoc = -1,
        .contrastLoc = -1
    },
    {
        .name = "shaders/postprocessing/deuteranopia.frag",
        .shader = {},
        .resolutionLoc = -1,
        .timeLoc = -1,
        .contrastLoc = -1
    },
    {
        .name = "shaders/postprocessing/tritanopia.frag",
        .shader = {},
        .resolutionLoc = -1,
        .timeLoc = -1,
        .contrastLoc = -1
    },
};

static struct {
        bool initialized = false;
        ::RenderTexture2D target;
} g_render_texture;

/**
 * static helpers
 */

static inline void post_processing_begin_shader_mode(const ::Shader &shader) noexcept
{
    BeginShaderMode(shader);
}

static inline void post_processing_end_shader_mode(void) noexcept
{
    EndShaderMode();
}

static inline void post_processing_clear(const ::Color &rl_color) noexcept
{
    ClearBackground(rl_color);
}

static inline void post_processing_begin_texture_mode(void) noexcept
{
    BeginTextureMode(g_render_texture.target);
}

static inline void post_processing_end_texture_mode(void) noexcept
{
    EndTextureMode();
}

static inline void post_processing_draw_texture(void) noexcept
{
    const auto &texture = g_render_texture.target.texture;

    DrawTextureRec(texture, {0, 0, static_cast<f32>(texture.width), static_cast<f32>(-texture.height)}, {0, 0}, WHITE);
}

/**
 * systems
 */

static void post_processing_plugin_startup(const r::ecs::Res<r::WindowPluginConfig> config_ptr,
    const r::ecs::Res<r::PostProcessingPluginConfig> pp_config_ptr) noexcept
{
    const r::Vec2i w_size = static_cast<r::Vec2i>(config_ptr.ptr->size);

    g_render_texture.target = LoadRenderTexture(w_size.x, w_size.y);
    g_render_texture.initialized = true;

    const std::string &prefix = pp_config_ptr.ptr->engine_assets_prefix;

    for (auto &fx : g_shaders) {
        std::string final_path = r::path::get(prefix + fx.name);
        fx.shader = LoadShader(nullptr, final_path.c_str());
        fx.resolutionLoc = GetShaderLocation(fx.shader, "resolution");
        fx.timeLoc = GetShaderLocation(fx.shader, "time");
        fx.contrastLoc = GetShaderLocation(fx.shader, "contrast");
    }
}

static void post_processing_plugin_begin_capture(const r::ecs::Res<r::RenderPluginConfig> config) noexcept
{
    if (!g_render_texture.initialized) {
        return;
    }

    const ::Color rl_color = {
        .r = config.ptr->clear_color.r,
        .g = config.ptr->clear_color.g,
        .b = config.ptr->clear_color.b,
        .a = config.ptr->clear_color.a,
    };

    post_processing_begin_texture_mode();
    post_processing_clear(rl_color);
}

static void post_processing_plugin_resize_system(const r::ecs::Res<r::WindowPluginConfig> window_config) noexcept
{
    const r::Vec2i w_size = static_cast<r::Vec2i>(window_config.ptr->size);

    if (!g_render_texture.initialized) {
        return;
    }

    const int cur_w = g_render_texture.target.texture.width;
    const int cur_h = g_render_texture.target.texture.height;

    if (cur_w != w_size.x || cur_h != w_size.y) {
        UnloadRenderTexture(g_render_texture.target);
        g_render_texture.target = LoadRenderTexture(w_size.x, w_size.y);
        g_render_texture.initialized = true;
        const std::string msg = std::string("PostProcessing: resized render texture to ") + std::to_string(w_size.x) + " x " + std::to_string(w_size.y);
        r::Logger::debug(msg);
    }
}

static void post_processing_plugin_end_capture_and_draw(
    const r::ecs::Res<r::PostProcessingPluginConfig> config_ptr,
    const r::ecs::Res<r::WindowPluginConfig> window_config,
    const r::ecs::Res<r::core::FrameTime> frame_time
) noexcept
{
    if (!g_render_texture.initialized) {
        return;
    }

    post_processing_end_texture_mode();

    const auto &state = config_ptr.ptr->state;

    if (state == r::PostProcessingState::Disabled) {
        post_processing_draw_texture();
        return;
    }

    auto &active_shader_fx = g_shaders[static_cast<u32>(state)];

    if (active_shader_fx.resolutionLoc != -1) {
        const f32 resolution[2] = {static_cast<f32>(window_config.ptr->size.x), static_cast<f32>(window_config.ptr->size.y)};
        SetShaderValue(active_shader_fx.shader, active_shader_fx.resolutionLoc, resolution, SHADER_UNIFORM_VEC2);
    }

    if (active_shader_fx.timeLoc != -1) {
        const f32 time = frame_time.ptr->global_time;
        SetShaderValue(active_shader_fx.shader, active_shader_fx.timeLoc, &time, SHADER_UNIFORM_FLOAT);
    }

    if (active_shader_fx.contrastLoc != -1) {
        SetShaderValue(active_shader_fx.shader, active_shader_fx.contrastLoc, &config_ptr.ptr->contrast_level, SHADER_UNIFORM_FLOAT);
    }

    post_processing_begin_shader_mode(active_shader_fx.shader);
    post_processing_draw_texture();
    post_processing_end_shader_mode();
}

static void post_processing_plugin_shutdown(void) noexcept
{
    for (auto &fx : g_shaders) {
        UnloadShader(fx.shader);
    }
    if (g_render_texture.initialized) {
        UnloadRenderTexture(g_render_texture.target);
        g_render_texture.initialized = false;
    }
}

}// namespace

r::PostProcessingPlugin::PostProcessingPlugin(PostProcessingPluginConfig config) noexcept : _config(std::move(config))
{
    /* __ctor__ */
}

void r::PostProcessingPlugin::build(Application &app)
{
    app.insert_resource(_config);
    app.add_systems<post_processing_plugin_startup>(Schedule::STARTUP);
    app.add_systems<post_processing_plugin_resize_system, post_processing_plugin_begin_capture>(Schedule::BEFORE_RENDER_3D);
    app.add_systems<post_processing_plugin_end_capture_and_draw>(Schedule::RENDER_2D).after<r::ui::render_system>();
    app.add_systems<post_processing_plugin_shutdown>(Schedule::SHUTDOWN);
}
