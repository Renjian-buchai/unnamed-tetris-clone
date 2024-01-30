#include <SDL2/SDL.h>

#include <array>
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

SDL_Rect initRect(int h = 0, int w = 0, int x = 0, int y = 0) {
  SDL_Rect toRet;
  toRet.h = h;
  toRet.w = w;
  toRet.x = x;
  toRet.y = y;
  return toRet;
}

SDL_Window* window;
std::vector<SDL_Surface*> surfaces;
std::vector<SDL_Texture*> textures;

void dealloc(SDL_Window*&, std::vector<SDL_Surface*>&,
             std::vector<SDL_Texture*>&, SDL_Renderer*&& = nullptr);

void dealloc(SDL_Window*& window, std::vector<SDL_Surface*>& surfaces,
             std::vector<SDL_Texture*>&& textures = {},
             SDL_Renderer*&& renderer = nullptr) {
  for (SDL_Surface* surface : surfaces) {
    SDL_FreeSurface(surface);
    surface = nullptr;
  }

  for (SDL_Texture* texture : textures) {
    SDL_DestroyTexture(texture);
    texture = nullptr;
  }

  if (window) {
    SDL_DestroyWindow(window);
    window = nullptr;
  }

  if (renderer) {
    SDL_DestroyRenderer(renderer);
    renderer = nullptr;
  }

  SDL_Quit();
}

template <size_t _X, size_t _Y>
using matrix = std::array<std::array<uint8_t, _Y>, _X>;

using pieceMatrix = matrix<4, 4>;

struct updateTickInput {
 public:
  matrix<10, 20> playfield{0};
  uint8_t* position;
  uint8_t& running;
};

int main(int argc, char** argv) {
  (void)argc, (void)argv;

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

  int block = dDM.h / 22;

  SDL_Rect playArea =
      initRect(20 * block, 10 * block, dDM.w / 5 + block, 0 + block);

  SDL_Rect close = initRect(block, 2 * block, dDM.w - 2 * block, 0);

  uint32_t closeRGB = 0;

  SDL_Point mousePos;
  mousePos.x = 0;
  mousePos.y = 0;

  uint8_t running = 1;
  SDL_Event event;

  matrix<10, 20> playfield{0};

  SDL_Rect activeObject =
      initRect(block, block, playArea.x + 4 * block, playArea.y);
  playfield[8][0] = 1;
  uint8_t point[2] = {4, 0};

  uint64_t current = SDL_GetTicks64(), next;
  int16_t timeDelay = 1000;

  while (running) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:
          running = 0;
          break;

        case SDL_MOUSEBUTTONDOWN:
          SDL_GetMouseState(&mousePos.x, &mousePos.y);
          if (SDL_PointInRect(&mousePos, &close)) {
            running = 0;
          }
          break;

        case SDL_MOUSEMOTION:
          SDL_GetMouseState(&mousePos.x, &mousePos.y);
          if (SDL_PointInRect(&mousePos, &close)) {
            closeRGB = SDL_MapRGB(windowSurface->format, 255, 15, 15);
          } else {
            closeRGB = SDL_MapRGB(windowSurface->format, 150, 15, 15);
          }
          break;

        case SDL_KEYDOWN:
          switch (event.key.keysym.sym) {
            case SDLK_RIGHT:
              if (point[0] < 9) {
                activeObject.x += block;
                playfield[point[0]][point[1]] = 0;
                playfield[++point[0]][point[1]] = 1;
              }
              break;

            case SDLK_LEFT:
              if (point[0] > 0) {
                activeObject.x -= block;
                playfield[point[0]][point[1]] = 0;
                playfield[--point[0]][point[1]] = 1;
              }
              break;

            default:
              break;
          }

        default:
          break;
      }
    }

    // Timekeeping
    next = SDL_GetTicks();
    timeDelay -= next - current;
    if (timeDelay <= 0 && point[1] < 19) {
      activeObject.y += block;
      current = next;
      timeDelay = 500;
      playfield[point[0]][point[1]] = 0;
      playfield[point[0]][++point[1]] = 1;
    }

    if (point[1] == 19) {
        }

    SDL_FillRect(windowSurface, NULL,
                 SDL_MapRGB(windowSurface->format, 124, 124, 124));
    SDL_FillRect(windowSurface, &playArea,
                 SDL_MapRGB(windowSurface->format, 0, 0, 0));
    SDL_FillRect(windowSurface, &close, closeRGB);
    SDL_FillRect(windowSurface, &activeObject,
                 SDL_MapRGB(windowSurface->format, 226, 229, 73));
    SDL_UpdateWindowSurface(window);
  }

  dealloc(window, surfaces);

  return ret::SUCCESS;
}
