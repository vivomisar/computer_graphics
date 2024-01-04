#include <SDL2/SDL.h>
#include <SDL2/SDL_endian.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

#define DEBUG 1

#define my_abs(x) ((x) >= 0? (x): -(x))
#define my_max(x, y) ((x) >= (y)? (x): (y))


#define CH_MAX 255
#define WIDTH 640
#define HEIGHT 480
#define M 3
#define N 3

SDL_Window* window;
SDL_Renderer* renderer;

void print_mat(float line[][M]){
    for(int i = 0; i < N; ++i){
        for(int j = 0; j < M; ++j)
            printf("(%d, %d): %f ", i, j, line[i][j]);
        putchar(10);
    }
}


void m_mul(float line[][M], float mod[][M]){
    int i, j, n;
    float s;
    float res[N][M];
    for(n = 0; n < N; ++n){
        for(i = 0; i < M; ++i){
            s = 0;
            for(j = 0; j < M; ++j){
                s += line[n][j] * mod[j][i];
#if DEBUG
printf("%f\n", s);
#endif
            }
            res[n][i] = s;
#if DEBUG
print_mat(line);
#endif
        }
    }
    for(i = 0; i < N; ++i){
        for(j = 0; j < M; ++j){
            line[i][j] = res[i][j];
        }
    }
#if DEBUG
printf("end mul###############################\n");
#endif
}

void move(float line[][M], float dx, float dy){
    float mat[M][M] = {{1, 0, 0},
                       {0, 1, 0},
                       {dx, dy, 1}};
    m_mul(line, mat);
}

void rotate(float line[][M], float a, float center[]){
    const float cosinus = cos(a);
    const float sinus = sin(a);

    move(line, -center[0], -center[1]);

    float mat[M][M] = {{cosinus, sinus, 0},
                       {-sinus, cosinus, 0},
                       {0, 0, 1}};
    m_mul(line, mat);

    move(line, center[0], center[1]);
    
}

void scale(float line[][M], float sx, float sy, float center[]){
    move(line, -center[0], -center[1]);
    float mat[M][M] = {{sx, 0, 0},
                       {0, sy, 0},
                       {0, 0, 1}};
    m_mul(line, mat);
    move(line, center[0], center[1]);
}


void update(float line[][M]){
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, CH_MAX);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, CH_MAX, CH_MAX, CH_MAX, CH_MAX);
    for(int i = 0; i < N; ++i){
        SDL_RenderDrawLineF(renderer, line[i][0], line[i][1], line[(i+1)%N][0], line[(i+1)%N][1]);
    }
    SDL_RenderPresent(renderer);

}

void closegraph(){
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int initgraph(){
    if(SDL_Init(SDL_INIT_VIDEO) != 0){
        printf("SDL_Init error: %s", SDL_GetError());
        return 1;
    }
    window = SDL_CreateWindow("Hello", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);

    if(window == NULL){
        printf("SDL_CreateWindow error: %s", SDL_GetError());
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if(renderer == NULL){
        printf("SDL_CreateRenderer error: %s", SDL_GetError());
        return 1;
    }

    return 0;

}

int main(void){


    float figure[N][M];

    srand(time(NULL));
    for(int i = 0; i < N; ++i)
        figure[i][0] = rand()%WIDTH;
    
    for(int i = 0; i < N; ++i)
        figure[i][1] = rand()%HEIGHT;
    
    for(int i = 0; i < N; ++i)
        figure[i][2] = 1;

    if(initgraph())
        return -1;
#if DEBUG
print_mat(figure);
#endif
    while(1){
        SDL_Event e;
        while(SDL_PollEvent(&e)){
            SDL_Delay(10);
            update(figure);
            if (e.type != SDL_KEYDOWN)
                continue;
            float avg_x = 0, avg_y = 0;
            for(int i = 0; i < N; ++i)
                avg_x += figure[i][0];
            for(int i = 0; i < N; ++i)
                avg_y += figure[i][1];
            float center[] = {avg_x/N,avg_y/N};
            switch (e.key.keysym.sym) {
                case SDLK_w:
                    move(figure, 0, -5);
                    break;
                case SDLK_a:
                    move(figure, -5, 0);
                    break;
                case SDLK_s:
                    move(figure, 0, 5);
                    break;
                case SDLK_d:
                    move(figure, 5, 0);
                    break;
                case SDLK_i:
                    scale(figure, 1.05, 1.05, center);
                    break;
                case SDLK_k:
                    scale(figure, 0.95, 0.95,center);
                    break;
                case SDLK_l:
                    rotate(figure, 0.087, center);
                    break;
                case SDLK_j:
                    rotate(figure, -0.087, center);
                    break;
            }
            if (e.key.keysym.sym == SDLK_q) 
                break;           
        }
        if (e.key.keysym.sym == SDLK_q) 
            break;  
    }
    closegraph();
    return 0;
}
