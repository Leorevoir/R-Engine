#pragma once

#include <R-Engine/Assets/Handle.hpp>

#include <vector>

namespace r {

/**
* @brief AssetEvent is used to notify the system about changes to assets
*/
template<typename T>
struct AssetEvent {
    public:
        enum class Type { Created, Modified, Removed };
        Type type;
        Handle<T> handle;
};

/**
* @brief AssetEvents is used to store events for a specific asset type
*/
template<typename T>
struct AssetEvents {
    public:
        /**
        * @brief push a new event to the event list
        * @param type the type of event
        * @param h the handle of the asset
        */
        void push(typename AssetEvent<T>::Type type, Handle<T> h) noexcept;

        /**
        * @brief read the event list
        * @return a const reference to the event list
        * @note the event list is not cleared after reading
        */
        const std::vector<AssetEvent<T>> &read() const noexcept;

        /**
        * @brief clear the event list
        */
        void clear() noexcept;

    private:
        std::vector<AssetEvent<T>> events;
};

#include "Inline/AssetEvents.inl"

}// namespace r
