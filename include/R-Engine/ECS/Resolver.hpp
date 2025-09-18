#pragma once

#include <R-Engine/Core/Error.hpp>
#include <R-Engine/ECS/Command.hpp>
#include <R-Engine/ECS/Query.hpp>
#include <R-Engine/ECS/Scene.hpp>
#include <unordered_set>

namespace r {

namespace ecs {

struct Resolver {
    public:
        template<typename... Wrappers>
        using Q = Query<Wrappers...>;

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
         * @brief Collects entity list for a component if the wrapper is a requirement (Mut, Ref, With).
         */
        template<typename W>
        void _collect_required_for(std::vector<std::vector<Entity>> &out);

        /**
         * @brief Collects entities to be excluded into a hash set if the wrapper is a `Without`.
         */
        template<typename W>
        void _collect_excluded_for(std::unordered_set<Entity> &out);
};

}// namespace ecs

}// namespace r

#include "Inline/Resolver.inl"
