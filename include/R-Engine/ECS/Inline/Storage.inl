#pragma once

#include "R-Engine/ECS/Storage.hpp"

/**
 * Storage Template Implementations
 */

template<typename T>
void r::ecs::Column<T>::push_back(std::any component)
{
    data.push_back(std::any_cast<T>(std::move(component)));
}

template<typename T>
void r::ecs::Column<T>::remove_swap_back(usize index)
{
    if (index < data.size() - 1) {
        std::swap(data[index], data.back());
    }
    data.pop_back();
}

template<typename T>
void *r::ecs::Column<T>::get_ptr(usize index)
{
    return &data[index];
}

template<typename T>
void r::ecs::Column<T>::move_to(usize index, IColumn &dest)
{
    auto &dest_col = static_cast<Column<T> &>(dest);
    dest_col.data.push_back(std::move(data[index]));
}

template<typename T>
std::unique_ptr<r::ecs::IColumn> r::ecs::Column<T>::clone_empty() const
{
    return std::make_unique<Column<T>>();
}
