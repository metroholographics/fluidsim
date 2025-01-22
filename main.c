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
	WALL=0,
	WATER
} cell_type;

typedef struct Cell {
	cell_type type;
	SDL_FRect cell;
	double fill_level; 	/* between 0(empty) - 1(full) */
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
			cells[i][j].type = WATER;
			cells[i][j].fill_level = 0.0f;
		}
	}
}

void reset_cells(cell cells[][COLS])
{
	for (int i = 0; i < ROWS; i++) {
		for (int j =0; j < COLS; j++) {
			cells[i][j].type = WATER;
			cells[i][j].fill_level = 0.0f;
		}
	}
}

void update_sim_state(cell c[][COLS])
{
	cell next_state[ROWS][COLS];

	for (int i = 0; i < ROWS; i++) {
		for (int j = 0; j < COLS; j++) {
			next_state[i][j] = c[i][j];
		}
	}

	/*Rule1: If current cell has water and below cell is water type and has less water than current cell,
			water falls down
	  Rule2: If current cell has water and below cell has more or equal water, or is a wall or the end, 
	  		disperse 1/3rd of the current fill Left and Right (if not Wall, Full, or edge columns
	  Rule3: If current cell has more liquid than max_amount, it pushes that amount upwards*/

	double max_fill_level = 1.3f;
	double compression = 0.02;

	for (int y = 0; y < ROWS; y++) {
		for (int x = 0; x < COLS; x++) {
			cell focus_cell = c[y][x];
			double focus_fill = focus_cell.fill_level;

			if (focus_cell.type == WALL) continue;

			bool rule_1 = false;
			if (y < ROWS-1 && focus_cell.fill_level > 0.0f) {
				rule_1 = c[y+1][x].type != WALL;
			}

			bool rule_3 = false;
			if (y > 0 ) {
				rule_3 = true;
			}

			if (rule_1) {
				cell below_cell = c[y+1][x];
				if (below_cell.fill_level < max_fill_level) {
					double diff_needed = max_fill_level - below_cell.fill_level;

					double transfer = focus_fill - diff_needed;
					transfer = (transfer < 0.0f) ? focus_fill : diff_needed;

					next_state[y][x].fill_level -= transfer;
					next_state[y+1][x].fill_level += transfer;
					focus_fill -= transfer;
				} 
			}

			if (focus_fill <= 0.0f) continue;

			/*rule 2*/
			double split_amount = focus_fill / 3.0f;
			double fill_copy = focus_fill;
			if (x > 0) {
				if (c[y][x-1].type != WALL && c[y][x-1].fill_level < focus_fill) {
			 		next_state[y][x].fill_level -= split_amount;
			 		next_state[y][x-1].fill_level += split_amount;
			 		fill_copy -= split_amount;
			 	}
			}

			 if (x < COLS - 1) {
			 	if (c[y][x+1].type != WALL && c[y][x+1].fill_level < focus_fill) {
			 		next_state[y][x].fill_level -= split_amount;
			 		next_state[y][x+1].fill_level += split_amount;
			 		fill_copy -= split_amount;
			 	}
			}

			// focus_fill = fill_copy;

			// if (rule_3) {
			// 	cell above_cell = c[y-1][x];
			// 	double above_target = 1.0f - focus_fill;
			// 	if (above_cell.fill_level < above_target) {
			// 		double diff = above_target - above_cell.fill_level;
			// 		next_state[y][x].fill_level -= diff;
			// 		next_state[y-1][x].fill_level += diff;
			// 		focus_fill -= diff;
			// 	}
			// }
		}
	}

	for (int i = 0; i < ROWS; i++) {
		for (int j = 0; j < COLS; j++) {
			c[i][j] = next_state[i][j];
		}
	}
}

SDL_FRect get_water_tile(cell c) {
	/*Return properties for a water tile based on position and fill level of passed cell*/
	double fill = c.fill_level;

	if (fill > 1.0f) {
		fill = 1.0f;
	}

	double empty = 1.0f - fill;

	SDL_FRect r = (SDL_FRect) {
		.x = c.cell.x,
		.y = c.cell.y + (empty * c.cell.h),
		.w = c.cell.w,
		.h = fill * c.cell.h 
	};



	return r;
}

void draw_cells(SDL_Renderer* renderer, cell grid[][COLS])
{
	for (int i = 0; i < ROWS; i++) {
		for (int j = 0; j < COLS; j++) {
			cell c = grid[i][j];
			switch (c.type) {
				case WALL:
					SDL_SetRenderDrawColor(renderer, COLOR_WHITE);
					SDL_RenderFillRect(renderer, &c.cell);
					break;
				case WATER: {
					SDL_SetRenderDrawColor(renderer, COLOR_GRAY);
					SDL_RenderRect(renderer, &c.cell);
					SDL_FRect water_tile = get_water_tile(c);
					SDL_SetRenderDrawColor(renderer, COLOR_BLUE);
					SDL_RenderFillRect(renderer, &water_tile);
				}
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
	float default_fill = 1.0f;

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

							if (selected_cell == WATER) {
								cells[c_y][c_x].fill_level += default_fill;
								if (cells[c_y][c_x].fill_level > 1.0f) {
									cells[c_y][c_x].fill_level = 1.0f;
								}
							} else {
								cells[c_y][c_x].fill_level = 0.0f;
							}
						} else if (e.motion.state == SDL_BUTTON_RMASK) {
							cells[c_y][c_x].type = WATER;
							cells[c_y][c_x].fill_level = 0.0f;
						}
					}
					break;
				case SDL_EVENT_MOUSE_BUTTON_DOWN:
					if (mouse_in_bounds(e.button.x, e.button.y)) {
						int c_x = e.button.x / CELL_SIZE;
						int c_y = e.button.y / CELL_SIZE;
						if (e.button.down && e.button.button == SDL_BUTTON_LEFT) {
							cells[c_y][c_x].type = selected_cell;

							if (selected_cell == WATER) {
								cells[c_y][c_x].fill_level += default_fill;
								if (cells[c_y][c_x].fill_level > 1.0f) {
									cells[c_y][c_x].fill_level = 1.0f;
								}
							} else {
								cells[c_y][c_x].fill_level = 0.0f;
							}
						} else if (e.button.down && e.button.button == SDL_BUTTON_RIGHT) {
							cells[c_y][c_x].type = WATER;
							cells[c_y][c_x].fill_level = 0.0f;
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

		update_sim_state(cells);

		draw_cells(renderer, cells);

		SDL_RenderPresent(renderer);
		SDL_Delay(17);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}




