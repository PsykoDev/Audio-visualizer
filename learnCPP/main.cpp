#include <iostream>
#include <iostream>
#include "SDL2/SDL.h"
#include "SDL2/SDL_rect.h"
#include "SDL2/SDL_mixer.h"
#include "SDL2/SDL_render.h"
#include <vector>
#include <cstdlib>
#include <ctime>
#include "FFT/fftw3.h"

int running = 1;
int SCREEN_HEIGHT = 800;
int SCREEN_WIDTH = 800;

int dir = 1;
int alpha = 128; // Initialisation de alpha avec une valeur fixe

// Taille du tableau de données audio
#define AUDIO_SIZE 4096

struct Audio {
  double* data; // Données audio
  double volume; // Volume des données audio
  int channels; // Nombre de canaux des données audio
    int sampleRate; // Taux d'échantillonnage des données audio
      int samples; // Nombre d'échantillons des données audio
    };

    void callback(void* userdata, Uint8* stream, int len) {
      // Récupération des données audio
      int samples = len / 4; // 4 octets par échantillon (2 octets par canal pour du stéréo)
      double* data = (double*)userdata;
      for (int i = 0; i < samples; i++) {
        stream[i * 4] = data[i] * 128.0 + 128.0; // Canal gauche
        stream[i * 4 + 1] = data[i] * 128.0 + 128.0; // Canal droit
      }
    }

    SDL_Color getRandomColor() {
      srand(static_cast<unsigned int>(time(NULL)));
      return {static_cast<Uint8>(rand() % 256), static_cast<Uint8>(rand() % 256), static_cast<Uint8>(rand() % 256), static_cast<Uint8>(alpha)};
    }

    void DrawRectWithWidth(SDL_Renderer* renderer, SDL_Rect* dest, int width, Uint8 rR, Uint8 gR, Uint8 bR, Uint8 aR, Uint8 rB, Uint8 gB, Uint8 bB, Uint8 aB, double delay, SDL_Color backgroundColor) {
      static int dirplus = 1; // Initialisation de dirplus avec une valeur fixe
      SDL_Rect outter = (*dest);
      outter.x -= width / 2;
      outter.y -= width / 2;
      outter.w += width;
      outter.h += width;

      SDL_Rect inner = (*dest);
      inner.x += width / 2;
      inner.y += width / 2;
      inner.w -= width;
      inner.h -= width;

    SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);
    SDL_RenderFillRect(renderer, &outter);
    SDL_SetRenderDrawColor(renderer, rB, gB, bB, aB);
    SDL_RenderFillRect(renderer, &inner);
     
     //if (dest->h <= SCREEN_HEIGHT) {
     //    dirplus = 1;
     //} else if (dest->h >= -SCREEN_HEIGHT) {
     //    dirplus = -1;
     //}
     //dest->h += sin(delay+i)+1*dirplus;
     dest->h = delay+1 *dirplus;
}

int main(int argc, const char * argv[]) {
    
    // init fftw
    fftw_complex* in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * AUDIO_SIZE);
    fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * AUDIO_SIZE);
    fftw_plan plan = fftw_plan_dft_1d(AUDIO_SIZE, in, out, FFTW_FORWARD, FFTW_MEASURE);
    
    // init sdl
    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Meow", 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // init sdl sound
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, AUDIO_SIZE);
    Mix_Music *musique= Mix_LoadMUS("/Users/psyko/Desktop/Doja_Cat_Woman.mp3");
    Mix_PlayMusic(musique, -1);
    
    // Récupération des données audio
    Audio audio;
    audio.data = (double*)malloc(AUDIO_SIZE * sizeof(double));
    audio.channels = 2;
    audio.sampleRate = 44100;
    audio.samples = AUDIO_SIZE;
    audio.volume = 0.0;
    //Mix_SetPostMix(callback, audio.data);
    
    // build les vecteur pour les barres
    std::vector<SDL_Rect> myRect = { };
    for(int i = 0; i<int(SCREEN_WIDTH * 2); i++)
    {
        SDL_Rect rect = {16, SCREEN_HEIGHT/3, 5, 100};
        rect.x = 11*i;
        myRect.push_back(rect);
    }
    float truc = 0;

    while(running) {
        
        Mix_SetPostMix(callback, audio.data);
        
        // setup alpha des barres
        float i = 0;
        alpha += 1 * dir;
        if (alpha == 255) {
          dir = -1;
        } else if (alpha == 50) {
          dir = 1;
        }
        
        // Calcul de la FFT
        for (int i = 0; i < AUDIO_SIZE; i++) {
          in[i][0] = audio.data[i];
          in[i][1] = 0.0;
        }
        fftw_execute(plan);

        // Récupération du volume moyen des données audio
        double sum = 0.0;
        for (int i = 0; i < AUDIO_SIZE; i++) {
          sum += sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
        }
        audio.volume = sum / AUDIO_SIZE;
        std::cout << "Sum " << sum << std::endl;
        std::cout << "AUDIO_SIZE " << AUDIO_SIZE << std::endl;
        std::cout << "Vol " << audio.volume << std::endl;
        
        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        //int barWidth = SCREEN_WIDTH / 10;
        //for (int i = 0; i < 10; i++) {
        //  SDL_Rect dest;
        //  dest.x = i * barWidth;
        //  dest.y = SCREEN_HEIGHT / 2;
        //  dest.w = barWidth;
        //  dest.h = -audio.volume * SCREEN_HEIGHT / 2;
        //    DrawRectWithWidth(renderer, &dest, barWidth / 5, 0, 0, 0, 255, 255, 255, 255, 255, audio.volume, getRandomColor());
        //}

        // print les barres
        for(auto& rect :myRect)
        {
            double y = SCREEN_HEIGHT / 1000 + sin(i + truc)*cos(i + truc) *  SCREEN_HEIGHT / 2 - (audio.volume * SCREEN_HEIGHT / 128);
            DrawRectWithWidth(renderer, &rect, rect.w, 255,0,255,255,0,0,0,255, -y , getRandomColor()); // 1st cube color // 2nd back color
            i++;
        }
        truc +=0.1;
        
        SDL_RenderPresent(renderer);
        
        SDL_Event event;
      while(SDL_PollEvent(&event)) {
          
        if(event.type == SDL_QUIT) {
          running = 0;
        }
      }
        
        SDL_Delay( 1/60 );
    }
    
    Mix_HaltMusic();
    Mix_FreeMusic(musique);
    Mix_CloseAudio();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
