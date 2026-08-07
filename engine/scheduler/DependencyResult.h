#pragma once

namespace conductor {
    enum class DependencyResult {
        Success,
        InvalidChild,
        InvalidParent,
        SelfDependency,
        DuplicateDependency,
        CycleDetected
    };
};