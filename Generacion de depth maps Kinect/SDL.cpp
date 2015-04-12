#include "SDL.h"
#include "main.h"

bool init(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Surface* screen = SDL_SetVideoMode(width, height, 32, SDL_HWSURFACE | SDL_GL_DOUBLEBUFFER | SDL_OPENGL);
    return screen;
}
void execute() {
    SDL_Event ev;
    bool running = true;
	int muestras = 0;
    while (running) {
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) running = false;
        }
		Sleep(250);
        drawKinectData();
        SDL_GL_SwapBuffers();
		if(muestras > 5){
			running = false;	
		}
		muestras++;
		
    }
}
