#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <SDL2pp/SDL2pp.hh>
#include "PointPlacer.h"
#include "triangulation/s_hull_pro.h"
#include <SDL2/SDL2_gfxPrimitives.h>

static const int POINTS_NUM = 2000;
static const int POINT_SIZE = 2;

using namespace SDL2pp;
using namespace std;

void drawPoints(Renderer &renderer, const vector<oip::ColorPoint>& points);
vector<Triad> TriangularizePoints(vector<oip::ColorPoint> points);
void drawTriads(Renderer &renderer, const vector<oip::ColorPoint>& points,
		const vector<Triad>& triads);

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
		Texture bitmap(renderer, imagePath);

		cout << "Window size is: " << window.GetWidth() << "x" << window.GetHeight() << endl;
		cout << "Image size is: " << bitmap.GetWidth() << "x" << bitmap.GetHeight() << endl;
		cout << "Image PixelFormat is " << SDL_GetPixelFormatName(bitmap.GetFormat()) << endl;

		assert(window.GetWidth() == bitmap.GetWidth());
		assert(window.GetHeight() == bitmap.GetHeight());
		int width = bitmap.GetWidth();
		int height = bitmap.GetHeight();

//		Uint8 *pixels = new Uint8[width * height * 4];

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
			}

			renderer.Clear();
			renderer.Copy(bitmap, NullOpt, NullOpt);

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
