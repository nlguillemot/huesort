#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <math.h>

SDL_Surface *screen;

float hue_from_rgb(int r, int g, int b)
{
	// return atan2(sqrt(3.0) * (g - b), 2.0f * r - g - b);
	return atan2(2.0f * r - g - b, sqrt(3.0) * (g - b));
}

// assumes format of pixel is the same as the screen
float hue_from_u32(Uint32 pixel)
{
	Uint8 r, g, b;
	SDL_GetRGB(pixel, screen->format, &r, &g, &b);
	return hue_from_rgb(r, g, b);
}

void swap(Uint32 *x, Uint32 *y)
{
	Uint32 tmp = *x;
	*x = *y;
	*y = tmp;
}

void heapify(Uint32 *pixels, int n, int i)
{
	int toswap = i;
	int lchild = 2*i+1;
	int rchild = 2*i+2;
	float smallest = hue_from_u32(pixels[i]);

	if (lchild < n)
	{
		float val = hue_from_u32(pixels[lchild]);
		if (val < smallest)
		{
			toswap = lchild;
			smallest = val;
		}
	}
	if (rchild < n)
	{
		float val = hue_from_u32(pixels[rchild]);
		if (val < smallest)
		{
			toswap = rchild;
			smallest = val;
		}
	}

	if (toswap != i)
	{
		swap(pixels+i, pixels+toswap);
		// printf("swapping %d with %d\n", i, toswap);
		heapify(pixels, n, toswap);
	}
}

void maybe_draw_picture(SDL_Surface *picture, int n, int i)
{
	if (picture && (i % (n/5000 + 1) == 0))
	{
		SDL_Flip(picture);
		SDL_BlitSurface(picture,NULL,screen,NULL);
		SDL_Flip(screen);
	}
}

void build_heap(Uint32 *pixels, int n, SDL_Surface *show_build)
{
	int i;
	for (i = n/2 - 1; i >= 0; --i)
	{
		maybe_draw_picture(show_build, n, i);
		SDL_LockSurface(show_build);
		heapify(pixels, n, i);
		SDL_UnlockSurface(show_build);
	}
}

void check_for_death()
{
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		if (e.type == SDL_QUIT)
			exit(0);
	}
}
// if show_sort is NULL, the image won't be drawn as it is being sorted.
void sort_pixels(Uint32 *pixels, int n, SDL_Surface *show_sort)
{
	build_heap(pixels, n, show_sort);

	int initial_n = n;

	while (n > 1)
	{
		check_for_death();

		maybe_draw_picture(show_sort, initial_n, n);

		SDL_LockSurface(show_sort);
		swap(pixels, pixels+n-1);
		--n;
		heapify(pixels, n, 0);
		SDL_UnlockSurface(show_sort);
	}
}

int main(int argc, char *argv[])
{
	start:;

	if (argc < 2)
	{
		printf("Usage: %s <image file> <output image>\n", argv[0]);
		return 1;
	}

	if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
	{
		printf("Failed to init SDL\n");
		return -1;
	}
	int sdlimgflags = IMG_INIT_JPG|IMG_INIT_PNG|IMG_INIT_TIF;
	if (IMG_Init(sdlimgflags) != sdlimgflags)
	{
		printf("Warning: failed to init support for some image formats\n");
		printf("IMG_Init: %s\n", IMG_GetError());
	}

	SDL_Surface *rawimg = IMG_Load(argv[1]);
	if (!rawimg)
	{
		printf("Could not load image: %s\n", argv[1]);
		return -1;
	}
	
	screen = SDL_SetVideoMode(rawimg->w, rawimg->h, 32, SDL_SWSURFACE);
	if (!screen)
	{
		printf("Failed to initialize screen\n");
		return -1;
	}

	SDL_Surface *img = SDL_ConvertSurface(rawimg, screen->format, screen->flags);
	if (!img)
	{
		printf("Failed to convert input image to 32bit color\n");
		return -1;
	}
	SDL_FreeSurface(rawimg);

	SDL_BlitSurface(img,NULL,screen,NULL);
	SDL_Flip(screen);
	SDL_Delay(500);

	sort_pixels(img->pixels, img->w * img->h, img);

	SDL_Flip(img);

	SDL_BlitSurface(img, NULL, screen, NULL);
	SDL_Flip(screen);

	if (argc >= 3)
		SDL_SaveBMP(img, argv[2]);

	SDL_Event e;
	int paused = 1;
	while (paused)
	{
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_QUIT)
				paused = 0;
			if (e.type == SDL_KEYDOWN)
			{
				if (e.key.keysym.sym == SDLK_SPACE)
				{
					SDL_FreeSurface(img);
					goto start; // LOLOLOLOLOLOLOLOLO
				}
			}
		}
	}

	IMG_Quit();

	SDL_FreeSurface(img);
	SDL_Quit();
}
