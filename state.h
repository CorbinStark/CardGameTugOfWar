#ifndef STATE_H
#define STATE_H

#include <string>
#include <vector>
#include "engine/bahamut.h"

struct StateGroup;
//Forgive me procedural gods, for I have sinned and resorted to polymorphism
struct State {
    State() { identifier = ""; }
    virtual void enter( std::string parameters ) = 0;
    virtual void run( StateGroup* states, QuadBatch* renderer ) = 0;
    virtual void leave() = 0;
    virtual ~State() {}
    std::string identifier;
    bool paused;
};

struct StateGroup {
    std::vector<State*> states;
    int current;
};

void add_state(StateGroup* set, State* state, std::string identifier, std::string parameters = "");
void remove_state(StateGroup* set, std::string identifier);
void set_state(StateGroup* set, std::string identifier, std::string parameters);
void set_state(StateGroup* set, std::string identifier, std::string parameters, bool pauseCurrent);
void set_state(StateGroup* set, std::string identifier, bool pauseCurrent);
void set_state(StateGroup* set, std::string identifier);
void run_current_state(StateGroup* set, QuadBatch* renderer);
StateGroup create_state_group();
void dispose_state_group(StateGroup& group);

#endif
