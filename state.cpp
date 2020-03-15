#include "state.h"

StateGroup create_state_group() {
    StateGroup group;
    group.current = 0;
    return group;
}

void add_state(StateGroup* set, State* state, std::string identifier, std::string parameters) {
    state->identifier = identifier;
    state->paused = false;
    if(set->states.size() == 0) {
        state->enter( parameters );
    }
    set->states.push_back(state);
}

void remove_state(StateGroup* set, std::string identifier) {
    for(int i = 0; i < set->states.size(); ++i) {
        if(set->states[i]->identifier == identifier) {
            //SWAP
            State* swap = set->states[i];
            set->states[i] = set->states[set->states.size()-1];
            set->states[set->states.size()-1] = swap;

            //... AND POP!
            delete set->states[set->states.size()-1];
            set->states.pop_back();
        }   
    }
}

void set_state(StateGroup* set, std::string identifier, std::string parameters, bool pauseCurrent) {
    for(int i = 0; i < set->states.size(); ++i) {
        if(set->states[i]->identifier == identifier) {
            //dont set the state if its already set
            if(i == set->current) return;
            //pause current state (don't de-allocate it by leaving it)
            //then enter the next state
            if(pauseCurrent) {
                set->states[set->current]->paused = true;
                set->current = i;
                set->states[set->current]->enter( parameters );
                return;
            }

            //leave current state and enter the new one
            set->states[set->current]->leave();
            set->current = i;

            //if new state is paused, it's memory is already allocated, so 
            //skip entering it and just switch.
            if(!set->states[set->current]->paused)
                set->states[set->current]->enter( parameters );
            else
                set->states[set->current]->paused = false;
        }
    }
}

void set_state(StateGroup* set, std::string identifier, std::string parameters) {
    set_state(set, identifier, parameters, false);
}

void set_state(StateGroup* set, std::string identifier, bool pauseCurrent) {
    set_state(set, identifier, "", pauseCurrent);
}

void set_state(StateGroup* set, std::string identifier) {
    set_state(set, identifier, "", false);
}

void run_current_state(StateGroup* set, QuadBatch* renderer) {
    set->states[set->current]->run( set, renderer );
}

void dispose_state_group(StateGroup& group) {
    for(State* s : group.states)
        delete s;
    group.states.clear();
    group.current = 0;
}
