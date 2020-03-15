
#ifndef GAME_H
#define GAME_H

#include "render.h"
#include "state.h"

struct Game : public State {
    //====== METHODS ======//
    Game();
    ~Game();
    void enter( std::string parameters );
    void run( StateGroup* statelist, QuadBatch* renderer );
    void leave();


    //====== VARIABLES ======//
    StateGroup substates;
    Texture test;
};


#endif
