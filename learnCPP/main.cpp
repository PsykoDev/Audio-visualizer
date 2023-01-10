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
const int SCREEN_HEIGHT = 800;
const int SCREEN_WIDTH = 800 * 2;

bool redraw = false;

// Taille du tableau de donn�es audio
const int BUCKET_NUMBER = 30;
const int BUCKET_WIDTH = SCREEN_WIDTH / BUCKET_NUMBER;
const int SAMPLE_SIZE = BUCKET_NUMBER * 2;

struct Audio {
    double* data;    // Donn�es audio
    double volume;   // Volume des donn�es audio
    int channels;    // Nombre de canaux des donn�es audio
    int sampleRate;  // Taux d'�chantillonnage des donn�es audio
    int samples;     // Nombre d'�chantillons des donn�es audio
};

void callback(void* userdata, Uint8* stream, int len) {
    // variable globale qui permet d'attendre
    // plutot que de redraw deux frames avec les memes
    // donnees a chaque fois
    redraw = true;

    // userdata est mon tableau "in"
    fftw_complex* out = (fftw_complex*)userdata;

    // le stream vient du mixer, et je l'ai config
    // pour qu'il me sorte des float, donc je converti
    // le pointeur
    float* in = (float*)stream;

    // je vais de 0 a len/4 parce que len
    // est la longueur de stream en bytes
    // et un float c'est 4 bytes
    for (int i = 0; i < (len / 4); i++) {
        out[i][0] = in[i];
        out[i][1] = 0;
    }
}

SDL_Color getRandomColor() {
    srand(static_cast<unsigned int>(time(NULL)));
    return { static_cast<Uint8>(rand() % 256), static_cast<Uint8>(rand() % 256), static_cast<Uint8>(rand() % 256), static_cast<Uint8>(256) };
}

void DrawRectWithWidth(SDL_Renderer* renderer, SDL_Color color, int i, double scale) {
    // taille du rectangle = scale (entre 0 et 1) * taille max de la barre
    int height = scale * (SCREEN_HEIGHT / 2);

    SDL_Rect to_draw;
    // chaque barre [i] est d�cal�e de i * largeur sur x
    to_draw.x = i * BUCKET_WIDTH;
    // on draw le "bas" de la barre au milieu donc
    // le y doit etre a milieu - taille
    to_draw.y = SCREEN_HEIGHT - height;

    // toutes les barres ont la meme largeur
    // mais leur hauteur varie
    to_draw.w = BUCKET_WIDTH;
    to_draw.h = height;

    // on setup la couleur
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    // on draw la barre
    SDL_RenderFillRect(renderer, &to_draw);
}

int main(int argc, const char* argv[]) {

    // init fftw
    fftw_complex* in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * SAMPLE_SIZE);
    fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * SAMPLE_SIZE);
    fftw_plan plan = fftw_plan_dft_1d(SAMPLE_SIZE, in, out, FFTW_FORWARD, FFTW_MEASURE);

    // init sdl
    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Meow", 100, 100, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // init sdl sound
    Mix_OpenAudio(44100, AUDIO_F32SYS, 1, SAMPLE_SIZE);
    Mix_Music* musique = Mix_LoadMUS("/Users/psyko/Desktop/Doja_Cat_Woman.mp3");

    if (!musique) {
        std::cout << "could not load music \n";
        return 0;
    }

    // R�cup�ration des donn�es audio
    Audio audio;
    audio.data = (double*)malloc(SAMPLE_SIZE * sizeof(double));
    audio.channels = 1;
    audio.sampleRate = 44100;
    audio.samples = SAMPLE_SIZE;
    audio.volume = 0.0;
    Mix_SetPostMix(callback, in);

    Mix_PlayMusic(musique, -1);

    while (running) {
        // busy waiting for sdl mixer data to load
        //
        while (!redraw) {
        }
        redraw = false;

        fftw_execute(plan);

        // R�cup�ration du volume moyen des donn�es audio
        double sum = 0.0;
        for (int i = 0; i < SAMPLE_SIZE / 2; i++) {
            double length = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
            std::cout << "length(" << out[i][0] << " + " << out[i][1] << "i) == " << length << "\n";
            sum += length;
        }
        audio.volume = sum * 2 / SAMPLE_SIZE;
        std::cout << "Sum " << sum << std::endl;
        std::cout << "SAMPLE_SIZE " << SAMPLE_SIZE << std::endl;
        std::cout << "Vol " << audio.volume << std::endl;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        for (int i = 0; i < BUCKET_NUMBER; ++i) {
            double scale = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
            std::cout << "scale= " << scale << "\n";
            DrawRectWithWidth(renderer, getRandomColor(), i, scale);
        }

        SDL_RenderPresent(renderer);

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }
        SDL_Delay(1 / 60);
    }

    Mix_HaltMusic();
    Mix_FreeMusic(musique);
    Mix_CloseAudio();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
