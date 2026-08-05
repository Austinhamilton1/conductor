#pragma once

namespace conductor {
    class Application {
    private:
        bool running = false;

    protected:
        virtual void initialize() {};
        virtual void update() {};
        virtual void shutdown() {};

    public:
        void run();
        void stop();
        bool is_running() const;
    };
};