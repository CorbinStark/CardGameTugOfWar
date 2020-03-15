
//Graphics Library
#include "engine/bahamut.h"

//States
#include "game.h"

//Defines
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define WINDOW_NAME "Someone Come Up With A Good Name"

//
//  PROTOTYPES
//

void initialize();
void setup_environment();

//
//  MAIN
//

int main(int argc, char** argv) {
    initialize();

    QuadBatch* renderer = &create_quad_batch();

    StateGroup states = create_state_group();
    add_state(&states, new Game(), "game");

    //MAIN GAME LOOP
    while(window_open()) {
        begin_drawing();
        setup_environment();

        bind_quad_batch(renderer);

        upload_mat4(renderer->shader, "projection", orthographic_projection(0, 0, get_window_width(), get_window_height(), -1, 1));
        run_current_state(&states, renderer);

        unbind_quad_batch(renderer);

        end_drawing();
    }
}

//
//  FUNCTIONS
//

#include <stdlib.h>
#include <time.h>

void initialize() {
    srand(time(NULL));
    //INITIALIZE WINDOW
    printf("\n/////////////////////////////////\n       BAHAMUT ENGINE\n/////////////////////////////////\n\n");
    init_window(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_NAME, false, true, true);
    //TODO: Fix fps cap
    set_fps_cap(60);
    //set_vsync(true);
    set_clear_color(0, 0, 0, 0);
    set_mouse_state(MOUSE_NORMAL);

    glDepthFunc(GL_LESS);
    glEnable(GL_MULTISAMPLE);
    setup_environment();
}

void setup_environment() {
    //SET GLOBAL OPENGL STATES
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    set_viewport(0, 0, get_window_width(), get_window_height());
}
