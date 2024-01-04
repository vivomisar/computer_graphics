#include <SDL2/SDL.h>
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
#define N 2

SDL_Window* window;
SDL_Renderer* renderer;


void print_mat(float line[][M]){
    for(int i = 0; i < N; ++i){
        for(int j = 0; j < M; ++j)
            printf("%f ", line[i][j]);
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

void bresenham_line(float line[][M]){
    float dx = line[1][0]-line[0][0] >= 0? 1: -1;
    float dy = line[1][1]-line[0][1] >= 0? 1: -1;

    float lenX = my_abs(line[1][0]-line[0][0]);
    float lenY = my_abs(line[1][1]-line[0][1]);

    int len = my_max(lenX, lenY);
    if(len == 0){
        SDL_RenderDrawPointF(renderer, line[0][0], line[0][1]);
        return;
    }
    float x = line[0][0];
    float y = line[0][1];
    len++;
    if (lenY <= lenX) {
        while (len--) {
            SDL_RenderDrawPointF(renderer, x, y);
            x += dx;
            y += dy * lenY / lenX;
        }
    }
    else {
        while (len--) {
            SDL_RenderDrawPointF(renderer, x, y);
            y += dy;
            x += dx * lenX / lenY;
        }
    }

}

void update(float line[][M]){
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, CH_MAX);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, CH_MAX, CH_MAX, CH_MAX, CH_MAX);
    SDL_RenderDrawLineF(renderer, line[0][0], line[0][1], line[1][0], line[1][1]);
    SDL_SetRenderDrawColor(renderer, 0, CH_MAX, 0, CH_MAX);

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


    float line1[N][M];
    float line2[N][M];
    float (*line)[M] = line1;
    char ch_line;

    srand(time(NULL));
    for(int i = 0; i < N; ++i)
        line1[i][0] = rand()%WIDTH;
    
    for(int i = 0; i < N; ++i)
        line1[i][1] = rand()%HEIGHT;
    
    for(int i = 0; i < N; ++i)
        line2[i][0] = rand()%WIDTH;
    
    for(int i = 0; i < N; ++i)
        line2[i][1] = rand()%HEIGHT;

    for(int i = 0; i < N; ++i)
        line1[i][2] = line2[i][2] = 1;

    if(initgraph())
        return -1;
#if DEBUG
print_mat(line1);
print_mat(line2);
#endif
    while(1){
        SDL_Event e;
        while(SDL_PollEvent(&e)){
            update(line1);
            bresenham_line(line2);
            SDL_RenderPresent(renderer);
            if (e.type != SDL_KEYDOWN)
                continue;

            float center[] = {0, 0};
            if(ch_line == 1){
                center[0] = line2[1][0];
                center[1] = line2[1][1];
            }
            else {
                center[0] = (line[1][0]+line[0][0])/2;
                center[1] = (line[1][1]+line[0][1])/2;
            }
            switch (e.key.keysym.sym) {
                case SDLK_w:
                    move(line, 0, -5);
                    break;
                case SDLK_a:
                    move(line, -5, 0);
                    break;
                case SDLK_s:
                    move(line, 0, 5);
                    break;
                case SDLK_d:
                    move(line, 5, 0);
                    break;
                case SDLK_i:
                    scale(line, 1.05, 1.05, center);
                    break;
                case SDLK_k:
                    scale(line, 0.95, 0.95,center);
                    break;
                case SDLK_l:
                    rotate(line, 0.087, center);
                    break;
                case SDLK_j:
                    rotate(line, -0.087, center);
                    break;
                case SDLK_SPACE:
                    ch_line = ~ch_line & 0x01;
                    if(ch_line)
                        line = line2;
                    else
                        line = line1;

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
