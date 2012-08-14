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

void build_heap(Uint32 *pixels, int n)
{
	int i;
	for (i = n/2 - 1; i >= 0; --i)
	{
		heapify(pixels, n, i);
	}
}

// if show_sort is NULL, the image won't be drawn as it is being sorted.
void sort_pixels(Uint32 *pixels, int n, SDL_Surface *show_sort)
{
	SDL_LockSurface(show_sort);
	build_heap(pixels, n);
	SDL_UnlockSurface(show_sort);

	while (n > 1)
	{
		SDL_LockSurface(show_sort);
		swap(pixels, pixels+n-1);
		--n;
		heapify(pixels, n, 0);
		SDL_UnlockSurface(show_sort);

		if (show_sort && (n&1))
		{
			SDL_Flip(show_sort);
			SDL_BlitSurface(show_sort,NULL,screen,NULL);
			SDL_Flip(screen);
		}
	}
}

int main(int argc, char *argv[])
{
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

	sort_pixels(img->pixels, img->w * img->h, img);

	SDL_Flip(img);

	SDL_BlitSurface(img, NULL, screen, NULL);
	SDL_Flip(screen);

	SDL_Delay(3000);

	if (argc >= 3)
		SDL_SaveBMP(img, argv[2]);

	IMG_Quit();

	SDL_FreeSurface(img);
	SDL_Quit();
}
