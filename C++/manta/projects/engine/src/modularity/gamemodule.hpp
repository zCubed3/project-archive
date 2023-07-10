#ifndef MANTA_GAMEMODULE_HPP
#define MANTA_GAMEMODULE_HPP

namespace Manta {
    class EngineContext;

    class GameModule {
    public:
        virtual void Initialize(EngineContext* context) = 0;
        virtual void Update(EngineContext* engine) = 0;
        virtual void Draw(EngineContext* engine) = 0;
        virtual void DrawGUI(EngineContext* engine) = 0;
    };
}

#endif
