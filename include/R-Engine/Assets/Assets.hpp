#pragma once

#include <R-Engine/Assets/Handle.hpp>

#include <memory>
#include <unordered_map>

namespace r {

template<typename T>
class Assets
{
    public:
        using HandleT = Handle<T>;
        using HandleId = typename HandleT::IdType;

        Assets() = default;
        ~Assets() = default;

        /**
        * @brief add asset ptr to storage
        * @details thread safe & return a stable handle id
        */
        HandleT add(std::unique_ptr<T> asset) noexcept;

        /**
        * @brief add asset by value to storage
        * @details thread safe & return a stable handle id
        */
        HandleT add(T asset) noexcept;

        /**
        * @brief get asset ptr by handle
        * @details get the non-owning ptr | nullptr
        */
        T *get(const HandleT &handle) noexcept;

        /**
        * @brief remove the asset from storage by handle
        * @details erase asset if exists
        */
        void remove(const HandleT &handle) noexcept;

        /**
        * @info debug
        */
        template<typename Func>
        void for_each(Func &&func) noexcept;

    private:
        std::unordered_map<u32, std::unique_ptr<T>> _map;
        u32 _last_id = 0;
        std::mutex _mutex;
};

#include "Inline/Assets.inl"

}// namespace r
