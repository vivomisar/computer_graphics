#include <SDL2/SDL.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>

#define DEBUG 0

#define my_abs(x) ((x) >= 0? (x): -(x))
#define my_max(x, y) ((x) >= (y)? (x): (y))
#define my_round(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))

#define CH_MAX 255
#define WIDTH 640
#define HEIGHT 480
#define M 4
#define N 4

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

void move(float line[][M], float dx, float dy, float dz){
    float mat[M][M] = {{1, 0, 0,0},
                       {0, 1, 0, 0},
                       {0, 0, 1, 0},
                       {dx, dy, dz, 1}};
    m_mul(line, mat);
}

void rotate_x(float line[][M], float a, float center[]){
    const float cosinus = cos(a);
    const float sinus = sin(a);

    move(line, -center[0], -center[1], -center[2]);

    float mat[M][M] = {{1, 0, 0, 0},
                       {0, cosinus, -sinus, 0},
                       {0, sinus, cosinus, 0},
                       {0, 0, 0, 1}};
    m_mul(line, mat);

    move(line, center[0], center[1], center[2]);
    
}
void rotate_y(float line[][M], float a, float center[]){
    const float cosinus = cos(a);
    const float sinus = sin(a);

    move(line, -center[0], -center[1], -center[2]);

    float mat[M][M] = {{cosinus, 0, -sinus, 0},
                       {0, 1, 0, 0},
                       {sinus, 0, cosinus, 0},
                       {0, 0, 0, 1}};
    m_mul(line, mat);

    move(line, center[0], center[1], center[2]);
    
}
void rotate_z(float line[][M], float a, float center[]){
    const float cosinus = cos(a);
    const float sinus = sin(a);

    move(line, -center[0], -center[1], -center[2]);

    float mat[M][M] = {{cosinus, -sinus, 0, 0},
                       {sinus, cosinus, 0, 0},
                       {0, 0, 1, 0},
                       {0, 0, 0, 1}};
    m_mul(line, mat);

    move(line, center[0], center[1], center[2]);
    
}

void scale(float line[][M], float sx, float sy, float sz, float center[]){
    move(line, -center[0], -center[1], -center[2]);
    float mat[M][M] = {{sx, 0, 0, 0},
                       {0, sy, 0, 0},
                       {0, 0, sz, 0},
                       {0, 0, 0, 1}};
    m_mul(line, mat);
    move(line, center[0], center[1], center[2]);
}


void draw_perspective(float line[][M]){
    float perspective_line[N][M];
    float s = 0;
    float zk = 200;
    
    move(line, -WIDTH/2, -HEIGHT/2, 0);
    for(int i = 0; i < N; i++){
        float perspective_matrix[M][M] = {{zk/(zk + line[i][2]), 0, 0, 0},
                                          {0, zk/(zk + line[i][2]), 0, 0},
                                          {0, 0, 1, 0},
                                          {0, 0, 0, 1}};
        for(int row = 0; row < N; row++){
            s = 0;
            for (int col = 0; col < N; col++) {
                s += perspective_matrix[row][col] * line[i][col];
            }
            perspective_line[i][row] = s;
        }
    }
    move(line, WIDTH/2,HEIGHT/2, 0);
    move(perspective_line, WIDTH/2,HEIGHT/2, 0);

    for(int i = 0; i < N-1; ++i){
        SDL_RenderDrawLineF(renderer, perspective_line[i][0], perspective_line[i][1], perspective_line[(i+1)%(N-1)][0], perspective_line[(i+1)%(N-1)][1]);
        SDL_RenderDrawLineF(renderer, perspective_line[i][0], perspective_line[i][1], perspective_line[N-1][0], perspective_line[N-1][1]);
    }
}

void update(float line[][M]){
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, CH_MAX);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, CH_MAX, CH_MAX, CH_MAX, CH_MAX);
    draw_perspective(line);
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
        figure[i][2] = rand()%HEIGHT;

    for(int i = 0; i < N; ++i)
        figure[i][3] = 1;

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
            float avg_x = 0, avg_y = 0, avg_z = 0;
            for(int i = 0; i < N; ++i)
                avg_x += figure[i][0];
            for(int i = 0; i < N; ++i)
                avg_y += figure[i][1];
            for(int i = 0; i < N; ++i)
                avg_z += figure[i][2];
            float center[] = {avg_x/N,avg_y/N, avg_z/N};
            switch (e.key.keysym.sym) {
                case SDLK_w:
                    move(figure, 0, -5, 0);
                    break;
                case SDLK_a:
                    move(figure, -5, 0, 0);
                    break;
                case SDLK_s:
                    move(figure, 0, 5, 0);
                    break;
                case SDLK_d:
                    move(figure, 5, 0, 0);
                    break;
                case SDLK_z:
                    move(figure, 0, 0, -10);
                    break;
                case SDLK_x:
                    move(figure, 0, 0, 10);
                    break;
                case SDLK_i:
                    scale(figure, 1.05, 1.05, 1.05, center); 
                    break;
                case SDLK_k:
                    scale(figure, 0.95, 0.95, 0.95, center);
                    break;
                case SDLK_l:
                    rotate_z(figure, 0.087, center);
                    break;
                case SDLK_j:
                    rotate_z(figure, -0.087, center);
                    break;
                case SDLK_u:
                    rotate_y(figure, 0.087, center);
                    break;
                case SDLK_o:
                    rotate_y(figure, -0.087, center);
                    break;
                case SDLK_n:
                    rotate_x(figure, 0.087, center);
                    break;
                case SDLK_m:
                    rotate_x(figure, -0.087, center);
                    break;
            }
            if (e.key.keysym.sym == SDLK_q) 
                break;           
        }
        if (e.key.keysym.sym == SDLK_q) 
            break;
        if (e.window.event == SDL_WINDOWEVENT_CLOSE) 
            break;    
    }
    closegraph();
    return 0;
}
