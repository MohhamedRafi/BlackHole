#include "engine.h"

#include <chrono>

static double now_seconds() {
    using clock = std::chrono::steady_clock;

    static const auto t0 = clock::now(); 
    return std::chrono::duration<double> (clock::now() - t0).count();
}

int main() {
    Engine E; 
    E.on_enter(E.state); 
    E.go(EngineState::InitGL); 
    E.go(EngineState::Loading); 
    E.go(EngineState::Running); 


    E.time_prev = now_seconds();

    while (E.running && !glfwWindowShouldClose(E.window)) {
        // platform layer should feed E with events (afterwards process)

        glfwPollEvents();
        E.process_events();

        /**
         * Frame Timing maybe be variable, hence we need to check how many (real) time we have 
         * left to process. We don't want the simulation to update with variable frame timing, 
         * hence we update with fixed delta T.
         */
    
        E.time_now = now_seconds(); 
        double frame = E.time_now - E.time_prev; // grab how many unprocessed time is left
        if (frame > 0.25) frame = 0.25; // clamp for hitches 

        E.time_prev = E.time_now;
        E.accumulator  += frame; 

        while (E.accumulator  >= Engine::DT) { // while we have accumulated time left
            E.update_fixed(Engine::DT); // updated simulation by one tick
            E.accumulator  -= Engine::DT; // consume that much real time
        }

        E.update_variable(frame); // this is for camera smoothing or lerps.
        E.render();

        // swap buffers, OS events 
    }

    E.go(EngineState::ShuttingDown);
    return 0;
}