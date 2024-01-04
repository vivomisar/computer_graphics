#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DEBUG 0

#define my_abs(x) ((x) >= 0 ? (x) : -(x))
#define my_max(x, y) ((x) >= (y) ? (x) : (y))
#define my_round(x) ((x) >= 0 ? (long)((x) + 0.5) : (long)((x)-0.5))
#define COLOR(c)                                                              \
    ((c)&0xFF000000) >> 24, ((c)&0x00FF0000) >> 16, ((c)&0x0000FF00) >> 8,    \
        ((c)&0x000000FF)

#define CH_MAX 255
#define WIDTH 480
#define HEIGHT 360
#define M 4
#define N 4

SDL_Window *window;
SDL_Renderer *renderer;

void
print_mat (float line[][M])
{
    for (int i = 0; i < N; ++i)
	{
	    for (int j = 0; j < M; ++j)
		printf ("(%d, %d): %f ", i, j, line[i][j]);
	    putchar (10);
	}
}

void
m_mul (float line[][M], float mod[][M])
{
    int i, j, n;
    float s;
    float res[N][M];
    for (n = 0; n < N; ++n)
	{
	    for (i = 0; i < M; ++i)
		{
		    s = 0;
		    for (j = 0; j < M; ++j)
			{
			    s += line[n][j] * mod[j][i];
#if DEBUG
			    printf ("%f\n", s);
#endif
			}
		    res[n][i] = s;
#if DEBUG
		    print_mat (line);
#endif
		}
	}
    for (i = 0; i < N; ++i)
	{
	    for (j = 0; j < M; ++j)
		{
		    line[i][j] = res[i][j];
		}
	}
#if DEBUG
    printf ("end mul###############################\n");
#endif
}

void
move (float line[][M], float dx, float dy, float dz)
{
    float mat[M][M] = {
	{ 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { dx, dy, dz, 1 }
    };
    m_mul (line, mat);
}

void
rotate_x (float line[][M], float a, float center[])
{
    const float cosinus = cos (a);
    const float sinus = sin (a);

    move (line, -center[0], -center[1], -center[2]);

    float mat[M][M] = { { 1, 0, 0, 0 },
	                { 0, cosinus, -sinus, 0 },
	                { 0, sinus, cosinus, 0 },
	                { 0, 0, 0, 1 } };
    m_mul (line, mat);

    move (line, center[0], center[1], center[2]);
}
void
rotate_y (float line[][M], float a, float center[])
{
    const float cosinus = cos (a);
    const float sinus = sin (a);

    move (line, -center[0], -center[1], -center[2]);

    float mat[M][M] = { { cosinus, 0, -sinus, 0 },
	                { 0, 1, 0, 0 },
	                { sinus, 0, cosinus, 0 },
	                { 0, 0, 0, 1 } };
    m_mul (line, mat);

    move (line, center[0], center[1], center[2]);
}
void
rotate_z (float line[][M], float a, float center[])
{
    const float cosinus = cos (a);
    const float sinus = sin (a);

    move (line, -center[0], -center[1], -center[2]);

    float mat[M][M] = { { cosinus, -sinus, 0, 0 },
	                { sinus, cosinus, 0, 0 },
	                { 0, 0, 1, 0 },
	                { 0, 0, 0, 1 } };
    m_mul (line, mat);

    move (line, center[0], center[1], center[2]);
}

void
scale (float line[][M], float sx, float sy, float sz, float center[])
{
    move (line, -center[0], -center[1], -center[2]);
    float mat[M][M] = {
	{ sx, 0, 0, 0 }, { 0, sy, 0, 0 }, { 0, 0, sz, 0 }, { 0, 0, 0, 1 }
    };
    m_mul (line, mat);
    move (line, center[0], center[1], center[2]);
}

float *
equation_plane (float x1, float y1, float z1, float x2, float y2, float z2,
                float x3, float y3, float z3)
{
    float a1 = x2 - x1;
    float b1 = y2 - y1;
    float c1 = z2 - z1;
    float a2 = x3 - x1;
    float b2 = y3 - y1;
    float c2 = z3 - z1;
    float *ret = (float *)malloc (sizeof (float) * 4);
    ret[0] = b1 * c2 - b2 * c1;
    ret[1] = a2 * c1 - a1 * c2;
    ret[2] = a1 * b2 - b1 * a2;
    ret[3] = (-ret[0] * x1 - ret[1] * y1 - ret[2] * z1);
    return ret;
}

int
min (float line[][M], int col)
{
    float res = line[0][col];
    for (int i = 1; i < M; i++)
	{
	    res = res > line[i][col] ? line[i][col] : res;
	}
    return res;
}

int
max (float line[][M], int col)
{
    float res = line[0][col];
    for (int i = 1; i < M; i++)
	{
	    res = res < line[i][col] ? line[i][col] : res;
	}
    return res;
}

void
fill (float figure[N][M])
{
    float z_buffer[WIDTH][HEIGHT];
    memset (z_buffer, INFINITY, sizeof (z_buffer));
    int color_buffer[WIDTH][HEIGHT];
    memset (color_buffer, 0x00000000, sizeof (color_buffer));
    const int colors[4] = { 0xFF0000FF, 0x00FF00FF, 0x0000FFFF, 0xFF00FFFF };
    for (int p = 0; p < N; p++)
	{
	    float *plane = equation_plane (
	        figure[p][0], figure[p][1], figure[p][2],
	        figure[(p + 1) % N][0], figure[(p + 1) % N][1],
	        figure[(p + 1) % N][2], figure[(p + 2) % N][0],
	        figure[(p + 2) % N][1], figure[(p + 2) % N][2]);
	    int minx, miny, maxx, maxy;
	    minx = min (figure, 0);
	    miny = min (figure, 1);
	    maxx = max (figure, 0);
	    maxy = max (figure, 1);

	    minx = minx > WIDTH ? WIDTH : minx;
	    miny = miny > HEIGHT ? HEIGHT : miny;
	    maxx = maxx > WIDTH ? WIDTH : maxx;
	    maxy = maxy > HEIGHT ? HEIGHT : maxy;

	    minx = minx < 0 ? 0 : minx;
	    miny = miny < 0 ? 0 : miny;
	    maxx = maxx < 0 ? 0 : maxx;
	    maxy = maxy < 0 ? 0 : maxy;
	    for (int y = miny + 1; y < maxy; y++)
		{
		    for (int x = minx + 1; x < maxx; x++)
			{
			    float a
			        = (figure[p][0] - x)
			              * (figure[(p + 1) % N][1] - figure[p][1])
			          - (figure[(p + 1) % N][0] - figure[p][0])
			                * (figure[p][1] - y);
			    float b = (figure[(p + 1) % N][0] - x)
			                  * (figure[(p + 2) % N][1]
			                     - figure[(p + 1) % N][1])
			              - (figure[(p + 2) % N][0]
			                 - figure[(p + 1) % N][0])
			                    * (figure[(p + 1) % N][1] - y);
			    float c
			        = (figure[(p + 2) % N][0] - x)
			              * (figure[p][1] - figure[(p + 2) % N][1])
			          - (figure[p][0] - figure[(p + 2) % N][0])
			                * (figure[(p + 2) % N][1] - y);
			    if ((a >= 0 && b >= 0 && c >= 0)
			        || (a <= 0 && b <= 0 && c <= 0))
				{
				    float z = (plane[0] * x + plane[1] * y
				               + plane[3])
				              / plane[2];
				    if (z_buffer[x][y] >= z)
					{
					    z_buffer[x][y] = z;
					    color_buffer[x][y] = colors[p];
					}
				}
			}
		}
	    free (plane);
	}
    for (int y = 0; y < HEIGHT; y++)
	{
	    for (int x = 0; x < WIDTH; x++)
		{
		    SDL_SetRenderDrawColor (renderer,
		                            COLOR (color_buffer[x][y]));
		    SDL_RenderDrawPoint (renderer, x, y);
		}
	}
}

void
draw_perspective (float line[][M])
{
    float perspective_line[N][M];
    float s = 0;
    float zk = 2000;

    for (int i = 0; i < N; i++)
	{
	    move (line, -WIDTH / 2, -HEIGHT / 2, 0);
	    float perspective_matrix[M][M]
	        = { { zk / (zk + line[i][2]), 0, 0, 0 },
		    { 0, zk / (zk + line[i][2]), 0, 0 },
		    { 0, 0, 1, 0 },
		    { 0, 0, 0, 1 } };
	    for (int row = 0; row < N; row++)
		{
		    s = 0;
		    for (int col = 0; col < N; col++)
			{
			    s += perspective_matrix[row][col] * line[i][col];
			}
		    perspective_line[i][row] = s;
		}
	    move (line, WIDTH / 2, HEIGHT / 2, 0);
	}
    move (perspective_line, WIDTH / 2, HEIGHT / 2, 0);
    fill (perspective_line);
    /* for(int i = 0; i < N-1; ++i){
        SDL_SetRenderDrawColor(renderer, CH_MAX, CH_MAX, CH_MAX, CH_MAX);
        SDL_RenderDrawLineF(renderer, perspective_line[i][0],
    perspective_line[i][1], perspective_line[(i+1)%(N-1)][0],
    perspective_line[(i+1)%(N-1)][1]); SDL_RenderDrawLineF(renderer,
    perspective_line[i][0], perspective_line[i][1], perspective_line[N-1][0],
    perspective_line[N-1][1]);
    } */
}

void
update (float line[][M])
{
    draw_perspective (line);
    SDL_RenderPresent (renderer);
}

void
closegraph ()
{
    SDL_DestroyRenderer (renderer);
    SDL_DestroyWindow (window);
    SDL_Quit ();
}

int
initgraph ()
{
    if (SDL_Init (SDL_INIT_VIDEO) != 0)
	{
	    printf ("SDL_Init error: %s", SDL_GetError ());
	    return 1;
	}
    window = SDL_CreateWindow ("Hello", SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);

    if (window == NULL)
	{
	    printf ("SDL_CreateWindow error: %s", SDL_GetError ());
	    return 1;
	}

    renderer = SDL_CreateRenderer (window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == NULL)
	{
	    printf ("SDL_CreateRenderer error: %s", SDL_GetError ());
	    return 1;
	}

    return 0;
}

int
main (void)
{

    float figure[N][M];

    srand (time (NULL));
    for (int i = 0; i < N; ++i)
	figure[i][0] = rand () % WIDTH;

    for (int i = 0; i < N; ++i)
	figure[i][1] = rand () % HEIGHT;

    for (int i = 0; i < N; ++i)
	figure[i][2] = rand () % HEIGHT;

    for (int i = 0; i < N; ++i)
	figure[i][3] = 1;

    if (initgraph ())
	return -1;
#if DEBUG
    print_mat (figure);
#endif
    while (1)
	{
	    SDL_Event e;
	    while (SDL_PollEvent (&e))
		{
		    SDL_Delay (10);
		    update (figure);
		    if (e.type != SDL_KEYDOWN)
			continue;
		    float avg_x = 0, avg_y = 0, avg_z = 0;
		    for (int i = 0; i < N; ++i)
			avg_x += figure[i][0];
		    for (int i = 0; i < N; ++i)
			avg_y += figure[i][1];
		    for (int i = 0; i < N; ++i)
			avg_z += figure[i][2];
		    float center[] = { avg_x / N, avg_y / N, avg_z / N };
		    switch (e.key.keysym.sym)
			{
			case SDLK_w:
			    move (figure, 0, -5, 0);
			    break;
			case SDLK_a:
			    move (figure, -5, 0, 0);
			    break;
			case SDLK_s:
			    move (figure, 0, 5, 0);
			    break;
			case SDLK_d:
			    move (figure, 5, 0, 0);
			    break;
			case SDLK_z:
			    move (figure, 0, 0, -10);
			    break;
			case SDLK_x:
			    move (figure, 0, 0, 10);
			    break;
			case SDLK_i:
			    scale (figure, 1.05, 1.05, 1.05, center);
			    break;
			case SDLK_k:
			    scale (figure, 0.95, 0.95, 0.95, center);
			    break;
			case SDLK_l:
			    rotate_z (figure, 0.087, center);
			    break;
			case SDLK_j:
			    rotate_z (figure, -0.087, center);
			    break;
			case SDLK_u:
			    rotate_y (figure, 0.087, center);
			    break;
			case SDLK_o:
			    rotate_y (figure, -0.087, center);
			    break;
			case SDLK_n:
			    rotate_x (figure, 0.087, center);
			    break;
			case SDLK_m:
			    rotate_x (figure, -0.087, center);
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
    closegraph ();
    return 0;
}
