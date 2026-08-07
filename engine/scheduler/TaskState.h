#pragma once

namespace conductor {
    enum class TaskState {
        Waiting,
        Ready,
        Running,
        Failed
    };
};