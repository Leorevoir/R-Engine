#pragma once

#include <R-Engine/Systems/ScheduleGraph.hpp>

namespace r {

namespace sys {

struct States {

        struct Transition {
                usize from;
                usize to;

                bool operator==(const Transition &other) const;
        };

        struct TransitionHasher {
                usize operator()(const Transition &t) const;
        };

        std::unordered_map<usize, ScheduleGraph> on_enter;
        std::unordered_map<usize, ScheduleGraph> on_exit;
        std::unordered_map<Transition, ScheduleGraph, TransitionHasher> on_transition;
};

}// namespace sys

}// namespace r
