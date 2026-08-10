#include <catch2/catch_test_macros.hpp>

#include "runtime/ApplicationContext.h"
#include "layers/Layer.h"
#include "layers/LayerStack.h"

class LayerState {
public:
    bool attached = false;
    bool detached = false;
    bool destroyed = false;
    int update_count = 0;
    double last_delta = 0.0;
};

class TestLayer : public conductor::Layer {
private:
    LayerState& state;

public:  
    explicit TestLayer(LayerState& state)
        : state(state) {}

    conductor::ApplicationContext* context = nullptr;

    void on_attach(conductor::ApplicationContext& ctx) override {
        state.attached = true;
        context = &ctx;
    }

    void on_detach() override {
        state.detached = true;
    }

    void on_update(double delta_time) override {
        state.update_count++;
        state.last_delta = delta_time;
    }
};

class OrderingLayer : public conductor::Layer {
public:
    OrderingLayer(int id, std::vector<int>& order)
        : id(id), order(order) {}

    void on_update(double) override {
        order.push_back(id);
    }

private:
    int id;
    std::vector<int>& order;
};


TEST_CASE("Layer is attached when pushed") {
    conductor::Clock clock;
    conductor::SimulationClock simulation_clock;
    conductor::EventBus events;
    conductor::Scheduler scheduler;

    conductor::ApplicationContext context{
        clock,
        simulation_clock,
        events,
        scheduler
    };

    conductor::LayerStack stack(context);

    LayerState state;

    auto layer = std::make_unique<TestLayer>(state);
    TestLayer* layer_ptr = layer.get();

    stack.push(std::move(layer));

    REQUIRE(stack.size() == 1);
    REQUIRE(state.attached);
    REQUIRE_FALSE(state.detached);
}


TEST_CASE("Layer is detached when popped") {
    conductor::Clock clock;
    conductor::SimulationClock simulation_clock;
    conductor::EventBus events;
    conductor::Scheduler scheduler;

    conductor::ApplicationContext context{
        clock,
        simulation_clock,
        events,
        scheduler
    };

    conductor::LayerStack stack(context);

    LayerState state;

    auto layer = std::make_unique<TestLayer>(state);
    TestLayer* layer_ptr = layer.get();

    stack.push(std::move(layer));

    REQUIRE(state.attached);
    REQUIRE_FALSE(state.detached);

    stack.pop();

    REQUIRE(stack.empty());
    REQUIRE(state.detached);
}


TEST_CASE("Layer receives ApplicationContext when attached") {
    conductor::Clock clock;
    conductor::SimulationClock simulation_clock;
    conductor::EventBus events;
    conductor::Scheduler scheduler;

    conductor::ApplicationContext context{
        clock,
        simulation_clock,
        events,
        scheduler
    };

    conductor::LayerStack stack(context);

    LayerState state;

    auto layer = std::make_unique<TestLayer>(state);
    TestLayer* layer_ptr = layer.get();

    stack.push(std::move(layer));

    REQUIRE(layer_ptr->context == &context);
    REQUIRE(&layer_ptr->context->clock == &clock);
    REQUIRE(&layer_ptr->context->simulation_clock == &simulation_clock);
    REQUIRE(&layer_ptr->context->events == &events);
    REQUIRE(&layer_ptr->context->scheduler == &scheduler);
}


TEST_CASE("Layer receives update calls") {
    conductor::Clock clock;
    conductor::SimulationClock simulation_clock;
    conductor::EventBus events;
    conductor::Scheduler scheduler;

    conductor::ApplicationContext context{
        clock,
        simulation_clock,
        events,
        scheduler
    };

    conductor::LayerStack stack(context);

    LayerState state;

    auto layer = std::make_unique<TestLayer>(state);
    TestLayer* layer_ptr = layer.get();

    stack.push(std::move(layer));

    constexpr double delta_time = 0.016;

    stack.update(delta_time);

    REQUIRE(state.update_count == 1);
    REQUIRE(state.last_delta == delta_time);

    stack.update(delta_time);

    REQUIRE(state.update_count == 2);
}


TEST_CASE("Layers are updated in insertion order") {
    conductor::Clock clock;
    conductor::SimulationClock simulation_clock;
    conductor::EventBus events;
    conductor::Scheduler scheduler;

    conductor::ApplicationContext context{
        clock,
        simulation_clock,
        events,
        scheduler
    };

    conductor::LayerStack stack(context);

    std::vector<int> order;

    stack.push(
        std::make_unique<OrderingLayer>(1, order)
    );

    stack.push(
        std::make_unique<OrderingLayer>(2, order)
    );

    stack.push(
        std::make_unique<OrderingLayer>(3, order)
    );

    stack.update(0.016);

    REQUIRE(order.size() == 3);
    REQUIRE(order[0] == 1);
    REQUIRE(order[1] == 2);
    REQUIRE(order[2] == 3);
}


TEST_CASE("Popping a layer removes the most recently added layer") {
    conductor::Clock clock;
    conductor::SimulationClock simulation_clock;
    conductor::EventBus events;
    conductor::Scheduler scheduler;

    conductor::ApplicationContext context{
        clock,
        simulation_clock,
        events,
        scheduler
    };

    conductor::LayerStack stack(context);

    LayerState first_state;
    LayerState second_state;

    auto first = std::make_unique<TestLayer>(first_state);
    auto second = std::make_unique<TestLayer>(second_state);

    TestLayer* first_ptr = first.get();
    TestLayer* second_ptr = second.get();

    stack.push(std::move(first));
    stack.push(std::move(second));

    REQUIRE(stack.size() == 2);

    stack.pop();

    REQUIRE(stack.size() == 1);
    REQUIRE(second_state.detached);
    REQUIRE_FALSE(first_state.detached);
}


TEST_CASE("Clearing the layer stack detaches every layer") {
    conductor::Clock clock;
    conductor::SimulationClock simulation_clock;
    conductor::EventBus events;
    conductor::Scheduler scheduler;

    conductor::ApplicationContext context{
        clock,
        simulation_clock,
        events,
        scheduler
    };

    conductor::LayerStack stack(context);

    LayerState first_state;
    LayerState second_state;
    LayerState third_state;

    auto first = std::make_unique<TestLayer>(first_state);
    auto second = std::make_unique<TestLayer>(second_state);
    auto third = std::make_unique<TestLayer>(third_state);

    TestLayer* first_ptr = first.get();
    TestLayer* second_ptr = second.get();
    TestLayer* third_ptr = third.get();

    stack.push(std::move(first));
    stack.push(std::move(second));
    stack.push(std::move(third));

    REQUIRE(stack.size() == 3);

    stack.clear();

    REQUIRE(stack.empty());
    REQUIRE(first_state.detached);
    REQUIRE(second_state.detached);
    REQUIRE(third_state.detached);
}


TEST_CASE("Popping an empty layer stack does nothing") {
    conductor::Clock clock;
    conductor::SimulationClock simulation_clock;
    conductor::EventBus events;
    conductor::Scheduler scheduler;

    conductor::ApplicationContext context{
        clock,
        simulation_clock,
        events,
        scheduler
    };

    conductor::LayerStack stack(context);

    REQUIRE_NOTHROW(stack.pop());
    REQUIRE(stack.empty());
}


TEST_CASE("Pushing a null layer does nothing") {
    conductor::Clock clock;
    conductor::SimulationClock simulation_clock;
    conductor::EventBus events;
    conductor::Scheduler scheduler;

    conductor::ApplicationContext context{
        clock,
        simulation_clock,
        events,
        scheduler
    };

    conductor::LayerStack stack(context);

    std::unique_ptr<conductor::Layer> layer;

    stack.push(std::move(layer));

    REQUIRE(stack.empty());
}

class LifetimeLayer : public conductor::Layer {
public:
    explicit LifetimeLayer(LayerState& state)
        : state(state) {}

    ~LifetimeLayer() override {
        state.destroyed = true;
    }

    void on_attach(conductor::ApplicationContext&) override {
        state.attached = true;
    }

    void on_detach() override {
        state.detached = true;
    }

private:
    LayerState& state;
};


TEST_CASE("Destroying LayerStack detaches layers before destroying them") {
    conductor::Clock clock;
    conductor::SimulationClock simulation_clock;
    conductor::EventBus events;
    conductor::Scheduler scheduler;

    conductor::ApplicationContext context{
        clock,
        simulation_clock,
        events,
        scheduler
    };

    LayerState state;

    {
        conductor::LayerStack stack(context);

        stack.push(
            std::make_unique<LifetimeLayer>(state)
        );

        REQUIRE(state.attached);
        REQUIRE_FALSE(state.detached);
        REQUIRE_FALSE(state.destroyed);
    }

    REQUIRE(state.detached);
    REQUIRE(state.destroyed);
}
