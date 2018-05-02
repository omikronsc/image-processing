#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <SDL2pp/SDL2pp.hh>
#include <SDL2/SDL2_gfxPrimitives.h>
#include "PointPlacer.h"
#include "triangulation/s_hull_pro.h"
#include "edge_detection/edge_detection.h"

static const int POINTS_NUM = 2000;
static const int POINT_SIZE = 2;

using namespace SDL2pp;
using namespace std;

void drawPoints(Renderer &renderer, const vector<oip::ColorPoint>& points);
vector<Triad> TriangularizePoints(vector<oip::ColorPoint> points);
void drawTriads(Renderer &renderer, const vector<oip::ColorPoint>& points,
		const vector<Triad>& triads);
Texture surfaceToTexture(Renderer &renderer, Surface& texture);
vector<Uint8> toGrayscale(Surface& surface);
Texture grayscaleToTexture(Renderer &renderer, const vector<Uint8>& grayscale, int width,
		int height);

int main(int argc, char **argv) {
	// TODO: Wpiąć ten projekt pod CMake'a i spróbować jeszcze raz czy zadziała SDL2pp - DONE
	// TODO: Podzielić kod na funkcje/moduły.
	// TODO: Zrobić wyciąganie krawędzi
	// TODO: Zrobić teselację na punktach krawędzi
	// TODO: Ewentualnie zrobić zmniejszanie liczby punktów
	// TODO: Zaimplementować wybór koloru trójkątów

	try {

		SDL sdl(SDL_INIT_VIDEO);
		Window window("Hello world!", 0, 0, 1920, 1080, SDL_WINDOW_FULLSCREEN);
		Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

		std::string imagePath = "/home/sebastian/data/genetmal/emily_bett_rickards.bmp";
		//	std::string imagePath = "/home/sebastian/data/genetmal/summer_glau_original.bmp";
		Surface surface(imagePath);

		cout << "Window size is: " << window.GetWidth() << "x" << window.GetHeight() << endl;
		cout << "Image size is: " << surface.GetWidth() << "x" << surface.GetHeight() << endl;
		cout << "Image PixelFormat is " << SDL_GetPixelFormatName(surface.GetFormat()) << endl;

		assert(window.GetWidth() == surface.GetWidth());
		assert(window.GetHeight() == surface.GetHeight());
		int width = surface.GetWidth();
		int height = surface.GetHeight();

		Texture bitmap = surfaceToTexture(renderer, surface);
		vector<Uint8> grayscale = toGrayscale(surface);
		Texture grayscaleTex = grayscaleToTexture(renderer, grayscale, width, height);

		vector<Uint8> canny = computeCanny(grayscale, width, height, 1, 40, 120);
		Texture cannyTex = grayscaleToTexture(renderer, canny, width, height);

		vector<Uint8> sobel = computeSobel(grayscale, width, height);
		Texture sobelTex = grayscaleToTexture(renderer, sobel, width, height);

		vector<Uint8> prewitt = computePrewitt(grayscale, width, height);
		Texture prewittTex = grayscaleToTexture(renderer, prewitt, width, height);

		vector<Uint8> roberts = computeRoberts(grayscale, width, height);
		Texture robertsTex = grayscaleToTexture(renderer, roberts, width, height);

		vector<Uint8> scharr = computeScharr(grayscale, width, height);
		Texture scharrTex = grayscaleToTexture(renderer, scharr, width, height);

		Texture *current = &bitmap;

		vector<oip::ColorPoint> points = oip::RandomPoints(width, height, POINTS_NUM, POINT_SIZE);
		vector<Triad> triads = TriangularizePoints(points);

		SDL_Event e;
		bool quit = false;
		while (!quit) {
			Uint64 begin = SDL_GetPerformanceCounter();
			while (SDL_PollEvent(&e)) {
				if (e.type == SDL_QUIT) {
					quit = true;
				}
				if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
					quit = true;
				}
				if (e.type == SDL_KEYDOWN) {
					switch (e.key.keysym.sym) {
					case SDLK_1:
						current = &bitmap;
						break;
					case SDLK_2:
						current = &grayscaleTex;
						break;
					case SDLK_3:
						current = &cannyTex;
						break;
					case SDLK_4:
						current = &sobelTex;
						break;
					case SDLK_5:
						current = &prewittTex;
						break;
					case SDLK_6:
						current = &robertsTex;
						break;
					case SDLK_7:
						current = &scharrTex;
						break;
					case SDLK_r:
						points.clear();
						points = oip::RandomPoints(width, height, POINTS_NUM, POINT_SIZE);
						triads.clear();
						triads = TriangularizePoints(points);
					}
				}
			}

			renderer.Clear();
			renderer.Copy(*current, NullOpt, NullOpt);

			drawTriads(renderer, points, triads);
			drawPoints(renderer, points);

			renderer.Present();
			Uint64 t = (SDL_GetPerformanceCounter() - begin) * 1000 / SDL_GetPerformanceFrequency();
			std::cout << t << "ms" << std::endl;
		}

	} catch (SDL2pp::Exception& e) {
		// Exception stores SDL_GetError() result and name of function which failed
		std::cerr << "Error in: " << e.GetSDLFunction() << std::endl;
		std::cerr << "  Reason: " << e.GetSDLError() << std::endl;
	} catch (std::exception& e) {
		// This also works (e.g. "SDL_Init failed: No available video device")
		std::cerr << e.what() << std::endl;
	}

	return 0;
}

void drawPoints(Renderer &renderer, const vector<oip::ColorPoint>& points) {
	for (int i = 0; i < POINTS_NUM; ++i) {
		renderer.SetDrawColor(points[i].r, points[i].g, points[i].b, 0xFF);
		renderer.FillRect(points[i].x - POINT_SIZE, points[i].y - POINT_SIZE,
				points[i].x + POINT_SIZE, points[i].y + POINT_SIZE);
	}
}

vector<Triad> TriangularizePoints(vector<oip::ColorPoint> points) {
	std::vector<Shx> pts, hull;
	Shx pt;
	for (int v = 0; v < POINTS_NUM; v++) {
		pt.id = v;
		pt.r = points[v].x;
		pt.c = points[v].y;

		pts.push_back(pt);
	}
	sort(pts.begin(), pts.end(), pointSortPredicate);
	std::vector<Shx>::iterator newEnd = unique(pts.begin(), pts.end(), pointComparisonPredicate);
	pts.resize(newEnd - pts.begin());
	std::vector<Triad> triads;
	s_hull_pro(pts, triads);
	return triads;
}

void drawTriads(Renderer &renderer, const vector<oip::ColorPoint>& points,
		const vector<Triad>& triads) {
	for (auto triad : triads) {
		int x1 = points[triad.a].x;
		int y1 = points[triad.a].y;
		int x2 = points[triad.b].x;
		int y2 = points[triad.b].y;
		int x3 = points[triad.c].x;
		int y3 = points[triad.c].y;

		trigonRGBA(renderer.Get(), x1, y1, x2, y2, x3, y3, 0, 0, 0, 0xFF);
//		filledTrigonRGBA(ren, x1, y1, x2, y2, x3, y3, r, g, b, 0xFF);
	}
}

Texture surfaceToTexture(Renderer &renderer, Surface& surface) {
	Texture result(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, surface.GetWidth(),
			surface.GetHeight());
	result.Update(NullOpt, surface);
	return result;
}

vector<Uint8> toGrayscale(Surface& surface) {
	vector<Uint8> result;
	result.resize(surface.GetWidth() * surface.GetHeight());
	{
		Surface::LockHandle surfaceLock = surface.Lock();
		Uint8 *surfaceStart = static_cast<Uint8*>(surfaceLock.GetPixels());
		int inBpp = surfaceLock.GetFormat().BytesPerPixel;
		for (int h = 0; h < surface.GetHeight(); ++h) {
			for (int w = 0; w < surface.GetWidth(); ++w) {
				Uint8 *pixel = surfaceStart + h * surfaceLock.GetPitch() + w * inBpp;
				Uint8 r = *pixel;
				Uint8 g = *(pixel + 1);
				Uint8 b = *(pixel + 2);

				result[h * surface.GetWidth() + w] = 0.212671f * r + 0.715160f * g + 0.072169f * b;
			}
		}
	}
	return result;
}

Texture grayscaleToTexture(Renderer &renderer, const vector<Uint8>& grayscale, int width,
		int height) {
	Texture result(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
	{
		Texture::LockHandle resultLock = result.Lock();
		Uint8 *resultStart = static_cast<Uint8*>(resultLock.GetPixels());
		for (int h = 0; h < height; ++h) {
			for (int w = 0; w < width; ++w) {
				Uint8 gray = grayscale[h * width + w];

				Uint8 *outPixel = resultStart + h * resultLock.GetPitch() + w * 4;
				*outPixel = gray;
				*(outPixel + 1) = gray;
				*(outPixel + 2) = gray;
				*(outPixel + 3) = 0xFF;
			}
		}
	}
	return result;
}
