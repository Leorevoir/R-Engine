#pragma once

#include <R-Engine/Core/Error.hpp>
#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/ECS/Query.hpp>
#include <R-Engine/ECS/Scene.hpp>
#include <typeindex>
#include <vector>

namespace r {

namespace ecs {

struct R_ENGINE_API Resolver {
    public:
        template<typename... Wrappers>
        using Q = Query<Wrappers...>;

        /**
         * @brief Construct a new Resolver object
         * @param s A pointer to the scene.
         * @param cmd A pointer to the command buffer.
         */
        explicit Resolver(Scene *s, CommandBuffer *cmd);

        /**
         * @brief Res<T>
         */
        template<typename T>
        Res<T> resolve(std::type_identity<Res<T>>);

        /**
        * @brief Commands
        */
        Commands resolve(std::type_identity<Commands>);

        /**
        * @brief Query<Wrappers...>
        */
        template<typename... Wrappers>
        Q<Wrappers...> resolve(std::type_identity<Query<Wrappers...>>);

        /**
         * @brief fallback for unsupported types
         */
        template<typename T>
        T resolve(std::type_identity<T>);

    private:
        Scene *_scene;
        CommandBuffer *_cmd_buffer;

        /**
         * @brief Helper to collect required and excluded component types from a query wrapper.
         * @tparam W The wrapper type (e.g., Mut<T>, Without<T>).
         * @param required Vector to store required type indices.
         * @param excluded Vector to store excluded type indices.
         */
        template<typename W>
        void _collect_component_types(std::vector<std::type_index> &required, std::vector<std::type_index> &excluded);
};

}// namespace ecs

}// namespace r

#include "Inline/Resolver.inl"
