#include "game.h"

Game::Game() {
    substates = create_state_group();
    //pass a reference to this state to them so that they can always access the game data 
    //(the current deck must be shared among all substates, for example. You build it in deck-builder and use it in level)
    //add_state(&substates, new Level(this), "level");
    //add_state(&substates, new Overworld(this), "overworld");
    //add_state(&substates, new DeckBuilder(this), "deck-builder");
}

void Game::enter( std::string parameters ) {
    //GL_LINEAR means that it will scale the image using bilinear filtering. Not super important what that is right now. 
    //GL_NEAREST = pixel-y look, GL_LINEAR = smoother look
    test = load_texture("data/art/test.png", GL_LINEAR);
}

void Game::run( StateGroup* statelist, QuadBatch* renderer ) {
    //run_current_state(&substates, renderer);

    //draw test texture at x = 50 and y = 50
    draw_texture(renderer, test, 50, 50);
}

void Game::leave() {
    dispose_texture(test); //never forget to dispose in leave()!
}

Game::~Game() {

}
