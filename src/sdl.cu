/*
 ============================================================================
 Name        : sdl.cu
 Author      : omikronsc
 Version     :
 Copyright   : Your copyright notice
 Description : CUDA compute reciprocals
 ============================================================================
 */

#include <iostream>
#include <numeric>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include "triangulation/s_hull_pro.h"

using namespace std;

static void CheckCudaErrorAux(const char *, unsigned, const char *, cudaError_t);
#define CUDA_CHECK_RETURN(value) CheckCudaErrorAux(__FILE__,__LINE__, #value, value)

/**
 * CUDA kernel that computes reciprocal values for a given vector
 */
__global__ void reciprocalKernel(float *data, unsigned vectorSize) {
	unsigned idx = blockIdx.x * blockDim.x + threadIdx.x;
	if (idx < vectorSize)
		data[idx] = 1.0 / data[idx];
}

/**
 * Host function that copies the data and launches the work on GPU
 */
float *gpuReciprocal(float *data, unsigned size) {
	float *rc = new float[size];
	float *gpuData;

	CUDA_CHECK_RETURN(cudaMalloc((void ** )&gpuData, sizeof(float) * size));
	CUDA_CHECK_RETURN(cudaMemcpy(gpuData, data, sizeof(float) * size, cudaMemcpyHostToDevice));

	static const int BLOCK_SIZE = 256;
	const int blockCount = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
	reciprocalKernel<<<blockCount, BLOCK_SIZE>>>(gpuData, size);

	CUDA_CHECK_RETURN(cudaMemcpy(rc, gpuData, sizeof(float) * size, cudaMemcpyDeviceToHost));
	CUDA_CHECK_RETURN(cudaFree(gpuData));
	return rc;
}

float *cpuReciprocal(float *data, unsigned size) {
	float *rc = new float[size];
	for (unsigned cnt = 0; cnt < size; ++cnt)
		rc[cnt] = 1.0 / data[cnt];
	return rc;
}

void initialize(float *data, unsigned size) {
	for (unsigned i = 0; i < size; ++i)
		data[i] = .5 * (i + 1);
}

int main(void) {
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
		return 1;
	}
	SDL_Window *win = SDL_CreateWindow("Hello World!", 0, 0, 1920, 1080, SDL_WINDOW_FULLSCREEN);
	if (win == nullptr) {
		std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}
	SDL_Renderer *ren = SDL_CreateRenderer(win, -1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (ren == nullptr) {
		SDL_DestroyWindow(win);
		std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}
	std::string imagePath = "/home/sebastian/data/genetmal/emily_bett_rickards.bmp";
//	std::string imagePath = "/home/sebastian/data/genetmal/summer_glau_original.bmp";
	SDL_Surface *bmp = SDL_LoadBMP(imagePath.c_str());
	if (bmp == nullptr) {
		SDL_DestroyRenderer(ren);
		SDL_DestroyWindow(win);
		std::cout << "SDL_LoadBMP Error: " << SDL_GetError() << std::endl;
		SDL_Quit();
		return 1;
	}
//	SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, bmp);
//	SDL_FreeSurface(bmp);
//	if (tex == nullptr) {
//		SDL_DestroyRenderer(ren);
//		SDL_DestroyWindow(win);
//		std::cout << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << std::endl;
//		SDL_Quit();
//		return 1;
//	}

	int screenWidth, screenHeight;
	SDL_GetWindowSize(win, &screenWidth, &screenHeight);
	cout << "Window size is: " << screenWidth << "x" << screenHeight << endl;
	int imageWidth = bmp->w;
	int imageHeight = bmp->h;
	cout << "Image size is: " << imageWidth << "x" << imageHeight << endl;

	int width = min(screenWidth, imageWidth);
	int height = min(screenHeight, imageHeight);

	cout << "pixelFormat is " << SDL_GetPixelFormatName(bmp->format->format) << endl;

	Uint8 *pixels = new Uint8[width * height * 4];

	SDL_Texture* texture = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING, width, height);

	struct Point {
		int x, y, r, g, b;
	};

	vector<Point> points;
	const int POINTS_NUM = 1000;
	const int MARGIN = 2;
	Point p;
	for (int i = 0; i < POINTS_NUM; i++) {
		p.x = rand() % (width - 2 * MARGIN) + MARGIN;
		p.y = rand() % (height - 2 * MARGIN) + MARGIN;
		p.r = rand() % 256;
		p.g = rand() % 256;
		p.b = rand() % 256;
		points.push_back(p);
	}

	std::vector<Shx> pts, hull;
	Shx pt;
	for (int v = 0; v < POINTS_NUM; v++) {
		pt.id = v;
		pt.r = points[v].x;
		pt.c = points[v].y;

		pts.push_back(pt);
	}
	sort(pts.begin(), pts.end(), pointSortPredicate);
	vector<Shx>::iterator newEnd = unique(pts.begin(), pts.end(), pointComparisonPredicate);
	pts.resize(newEnd - pts.begin());
	vector<Triad> triads;
	s_hull_pro(pts, triads);

	SDL_LockSurface(bmp);

	SDL_Event e;
	bool quit = false;
	while (!quit) {
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) {
				quit = true;
			}
			if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
				quit = true;
			}
		}

		Uint64 begin = SDL_GetPerformanceCounter();

		SDL_RenderClear(ren);
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {
				Uint32 * targetPixel;
				int pixelPosition = y * bmp->pitch + x * bmp->format->BytesPerPixel;
				targetPixel = (Uint32*) ((Uint8 *) bmp->pixels + pixelPosition);
				Uint8 r, g, b;
				SDL_GetRGB(*targetPixel, bmp->format, &r, &g, &b);

				const unsigned int offset = (y * imageWidth + x) * 4;
				pixels[offset] = b;
				pixels[offset + 1] = g;
				pixels[offset + 2] = r;
				pixels[offset + 3] = 0xFF;

//				SDL_SetRenderDrawColor(ren, r, g, b, a);
//				SDL_RenderDrawPoint(ren, x, y);
			}
		}

		SDL_UpdateTexture(texture, NULL, pixels, width * 4);
		SDL_RenderCopy(ren, texture, NULL, NULL);

		for (auto triad : triads) {
			int x1 = points[triad.a].x;
			int y1 = points[triad.a].y;
			int x2 = points[triad.b].x;
			int y2 = points[triad.b].y;
			int x3 = points[triad.c].x;
			int y3 = points[triad.c].y;

			trigonRGBA(ren, x1, y1, x2, y2, x3, y3, 0, 0, 0, 0xFF);
		}

		for (int i = 0; i < POINTS_NUM; i++) {
			Point *p = &points[i];
			boxRGBA(ren, p->x - MARGIN, p->y - MARGIN, p->x + MARGIN, p->y + MARGIN, p->r, p->g,
					p->b, 0xFF);
		}

		//Update the screen
		SDL_RenderPresent(ren);

		Uint64 t = (SDL_GetPerformanceCounter() - begin) * 1000 / SDL_GetPerformanceFrequency();
		cout << t << "ms" << endl;
	}

	SDL_UnlockSurface(bmp);

	delete pixels;

	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	SDL_Quit();
}

int main2(void) {
	static const int WORK_SIZE = 65530;
	float *data = new float[WORK_SIZE];

	initialize(data, WORK_SIZE);

	float *recCpu = cpuReciprocal(data, WORK_SIZE);
	float *recGpu = gpuReciprocal(data, WORK_SIZE);
	float cpuSum = std::accumulate(recCpu, recCpu + WORK_SIZE, 0.0);
	float gpuSum = std::accumulate(recGpu, recGpu + WORK_SIZE, 0.0);

	/* Verify the results */
	std::cout << "gpuSum = " << gpuSum << " cpuSum = " << cpuSum << std::endl;

	/* Free memory */
	delete[] data;
	delete[] recCpu;
	delete[] recGpu;

	return 0;
}

/**
 * Check the return value of the CUDA runtime API call and exit
 * the application if the call has failed.
 */
static void CheckCudaErrorAux(const char *file, unsigned line, const char *statement,
		cudaError_t err) {
	if (err == cudaSuccess)
		return;
	std::cerr << statement << " returned " << cudaGetErrorString(err) << "(" << err << ") at "
			<< file << ":" << line << std::endl;
	exit(1);
}

