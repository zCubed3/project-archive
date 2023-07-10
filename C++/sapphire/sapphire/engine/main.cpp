#include <engine/engine.h>

#include <typing/class_registry.h>

int main(int argc, char **argv) {
    Engine engine;
    engine.initialize();

    while (engine.engine_loop()) {

    }

    engine.shutdown();

    return 0;
}
