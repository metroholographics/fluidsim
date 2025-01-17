#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define WIDTH 900
#define HEIGHT 600
#define CELL_SIZE 20
#define COLS (WIDTH / CELL_SIZE)
#define ROWS (HEIGHT / CELL_SIZE)
#define COLOR_GRAY 50, 50, 50, 255
#define COLOR_WHITE 220, 220, 220, 255
#define COLOR_BLACK 9, 0, 0, 255
#define COLOR_BLUE 19, 86, 105, 255

typedef enum cell_types {
	EMPTY=0,
	WALL,
	WATER
} cell_type;

typedef struct Cell {
	cell_type type;
	SDL_FRect cell;
} cell;

void init_cell_grid(cell cells[][COLS])
{
	for (int i = 0; i < ROWS; i++) {
		for (int j = 0; j < COLS; j++) {
			cells[i][j].cell = (SDL_FRect) {
				.x = (float) j * CELL_SIZE,
				.y = (float) i * CELL_SIZE,
				.w = CELL_SIZE,
				.h = CELL_SIZE
			};
			cells[i][j].type = EMPTY;
		}
	}
}

void reset_cells(cell cells[][COLS])
{
	for (int i = 0; i < ROWS; i++) {
		for (int j =0; j < COLS; j++) {
			cells[i][j].type = EMPTY;
		}
	}
}

void draw_cells(SDL_Renderer* renderer, cell grid[][COLS])
{
	for (int i = 0; i < ROWS; i++) {
		for (int j = 0; j < COLS; j++) {
			cell c = grid[i][j];
			switch (c.type) {
				case EMPTY: 
					SDL_SetRenderDrawColor(renderer, COLOR_GRAY);
					SDL_RenderRect(renderer, &c.cell);
					break;
				case WALL:
					SDL_SetRenderDrawColor(renderer, COLOR_WHITE);
					SDL_RenderFillRect(renderer, &c.cell);
					break;
				case WATER:
					SDL_SetRenderDrawColor(renderer, COLOR_BLUE);
					SDL_RenderFillRect(renderer, &c.cell);
					break;
				default:
					SDL_SetRenderDrawColor(renderer, 230,10,10,255);
					SDL_RenderFillRect(renderer, &c.cell);
					break;
			}
		}
	}
}

bool mouse_in_bounds(float x, float y)
{
	return x > 0 && x < WIDTH && y > 0 && y < HEIGHT;
}

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	printf("~~fluidsim~~\nSPACE: Switch Wall/Water cell\nL-Mouse: Draw\nR-Mouse: Delete\n'r': Reset\n");

	SDL_Window* window;
	SDL_Renderer* renderer;
	cell cells[ROWS][COLS];
	cell_type selected_cell = WALL;

	init_cell_grid(cells);

	SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");

	if (!SDL_Init(SDL_INIT_VIDEO)) {
		printf("SDL couldn't initialise: %s\n", SDL_GetError());
		return 1;
	}
	if (!SDL_CreateWindowAndRenderer("fluidsim", WIDTH, HEIGHT, 0, &window, &renderer)) {
		printf("SDL couldn't initialise window and/or renderer: %s\n", SDL_GetError());
		return 1;
	}

	bool running = true;
	SDL_Event e;

	while (running) {
		SDL_SetRenderDrawColor(renderer, COLOR_BLACK);
		SDL_RenderClear(renderer);

		while (SDL_PollEvent(&e)) {
			switch (e.type) {
				case SDL_EVENT_QUIT:
					running = false;
					break;
				case SDL_EVENT_MOUSE_MOTION:
					if (mouse_in_bounds(e.motion.x, e.motion.y)) {
						int c_x = e.motion.x / CELL_SIZE;
						int c_y = e.motion.y / CELL_SIZE;
						if (e.motion.state == SDL_BUTTON_LMASK) {
							cells[c_y][c_x].type = selected_cell;
						} else if (e.motion.state == SDL_BUTTON_RMASK) {
							cells[c_y][c_x].type = EMPTY;
						}
					}
					break;
				case SDL_EVENT_MOUSE_BUTTON_DOWN:
					if (mouse_in_bounds(e.button.x, e.button.y)) {
						int c_x = e.button.x / CELL_SIZE;
						int c_y = e.button.y / CELL_SIZE;
						if (e.button.down && e.button.button == SDL_BUTTON_LEFT) {
							cells[c_y][c_x].type = selected_cell;
						} else if (e.button.down && e.button.button == SDL_BUTTON_RIGHT) {
							cells[c_y][c_x].type = EMPTY;
						}
					}
					break;
				case SDL_EVENT_KEY_DOWN:
					if (e.key.key == SDLK_SPACE) {
						selected_cell = (selected_cell == WALL) ? WATER : WALL;
					}
					if (e.key.key == SDLK_R) {
						reset_cells(cells);
					}
					break;
				default:
					break;
			}
		}

		draw_cells(renderer, cells);

		SDL_RenderPresent(renderer);
		SDL_Delay(17);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}




