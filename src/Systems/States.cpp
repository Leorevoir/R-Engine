#include <R-Engine/Systems/States.hpp>

/**
* public
*/

bool r::sys::States::Transition::operator==(const Transition &other) const
{
    return from == other.from && to == other.to;
}

usize r::sys::States::TransitionHasher::operator()(const Transition &t) const
{
    const usize h1 = std::hash<usize>{}(t.from);
    const usize h2 = std::hash<usize>{}(t.to);

    return h1 ^ (h2 << 1);
}
