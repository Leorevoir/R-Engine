#pragma once

#include <R-Engine/Core/Error.hpp>
#include <R-Engine/ECS/Query.hpp>
#include <R-Engine/ECS/Scene.hpp>

namespace r {

namespace ecs {

struct Resolver {
    public:
        template<typename... Wrappers>
        using Q = Query<Wrappers...>;

        explicit Resolver(Scene *s);

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

        /**
        * @brief collect list for wrapper W (must be Mut<T> or Ref<T>)
        */
        template<typename W>
        void _collect_for(std::vector<std::vector<Entity>> &out);
};

}// namespace ecs

}// namespace r

#include "Inline/Resolver.inl"
