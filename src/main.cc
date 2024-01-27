#include <SDL2/SDL.h>

#include <csignal>
#include <cstdint>
#include <iostream>
#include <vector>

enum ret : uint8_t {
  SUCCESS,

  // Initialisation errors
  SDL_INITIALISATION_FAILED,
  SDL_DISPLAY_MODE_FAILED,
  WIN_INIT_FAILED,
  WINSURFACE_INIT_FAILED
};

SDL_Window* window;
std::vector<SDL_Surface*> surfaces;
std::vector<SDL_Texture*> textures;

void dealloc(SDL_Window*&, std::vector<SDL_Surface*>&,
             std::vector<SDL_Texture*>&, SDL_Renderer*&& = nullptr);

// void sigint_handler(int signal) {
//   dealloc(window, surfaces, textures);
//   std::cout << "Interrupt " << signal;
//   return;
// }

void dealloc(SDL_Window*& window, std::vector<SDL_Surface*>& surfaces,
             std::vector<SDL_Texture*>&& textures = {},
             SDL_Renderer*&& renderer) {
  for (SDL_Surface* surface : surfaces) {
    SDL_FreeSurface(surface);
    surface = nullptr;
  }

  for (SDL_Texture* texture : textures) {
    SDL_DestroyTexture(texture);
    texture = nullptr;
  }

  SDL_DestroyWindow(window);
  window = nullptr;

  SDL_DestroyRenderer(renderer);
  renderer = nullptr;

  SDL_Quit();
}

int main(int argc, char** argv) {
  (void)argc, (void)argv;

  // signal(SIGINT, sigint_handler);
  // signal(SIGSEGV, sigint_handler);

  if (SDL_Init(SDL_INIT_EVERYTHING)) {
    return ret::SDL_INITIALISATION_FAILED;
  }

  SDL_DisplayMode dDM;
  if (SDL_GetDesktopDisplayMode(0, &dDM)) {
    return ret::SDL_DISPLAY_MODE_FAILED;
  }

  window =
      SDL_CreateWindow("tetris", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                       dDM.w, dDM.h, SDL_WINDOW_BORDERLESS);
  if (!window) {
    std::cout << SDL_GetError();
    return ret::WIN_INIT_FAILED;
  }

  SDL_Surface* windowSurface = nullptr;
  windowSurface = SDL_GetWindowSurface(window);
  if (!windowSurface) {
    std::cout << SDL_GetError();
    return ret::WINSURFACE_INIT_FAILED;
  }

  (void)windowSurface;

  uint8_t running = 1;
  SDL_Event event;
  while (running) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:
          running = 0;
          break;

        default:
          break;
      }
    }
    SDL_FillRect(windowSurface, NULL,
                 SDL_MapRGB(windowSurface->format, 255, 255, 255));
    SDL_UpdateWindowSurface(window);
  }

  dealloc(window, surfaces);

  return ret::SUCCESS;
}
