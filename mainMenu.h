#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "render.h"
#include "state.h"

struct MainMenu : public State {
    //====== METHODS ======//
    MainMenu();
    ~MainMenu();
    void enter( std::string parameters );
    void run( StateGroup* states );
    void leave();


    //====== VARIABLES ======//
    QuadBatch batch;
};


#endif
