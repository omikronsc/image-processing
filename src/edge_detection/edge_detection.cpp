#include <queue>
#include "edge_detection.h"
#include "kernels.h"

using namespace std;

static inline int bound(int minval, int val, int maxval) {
	return max(minval, min(val, maxval));
}

template<class M>
vector<Uint8> convolution(const M& kernel, const vector<Uint8>& image, int width, int height) {
	int kw = kernel[0].size(), kh = kernel.size(), offsetx = kw / 2, offsety = kw / 2;
//	QImage out(image.size(), image.format());
	vector<Uint8> result;
	result.resize(width * height);
	float sum;

	Uint8 *line;
	const Uint8 *lookup_line;

	for (int y = 0; y < height; y++) {
//		line = out.scanLine(y);
		line = result.data() + y * width;
		for (int x = 0; x < width; x++) {
			sum = 0;

			for (int j = 0; j < kh; j++) {
				if (y + j < offsety || y + j >= height)
					continue;
//				lookup_line = image.constScanLine(y + j - offsety);
				lookup_line = image.data() + (y + j - offsety) * width;
				for (int i = 0; i < kw; i++) {
					if (x + i < offsetx || x + i >= width)
						continue;
					sum += kernel[j][i] * lookup_line[x + i - offsetx];
				}
			}

			line[x] = bound(0x00, static_cast<int>(sum), 0xFF);
		}
	}

	return result;
}

matrix<float, 5, 5> gaussian_kernel(float sigma) {
	matrix<float, 5, 5> gauss;
	float sum = 0, s = 2 * sigma * sigma;

	for (int x = -2; x <= 2; x++)
		for (int y = -2; y <= 2; y++)
			sum += (gauss[x + 2][y + 2] = exp(-(x * x + y * y) / s) / s / M_PI);

	for (auto& row : gauss)
		for (auto& x : row)
			x /= sum;

	return gauss;
}

void magnitude(vector<Uint8>& input, const vector<Uint8>& gx, const vector<Uint8>& gy, int width,
		int height) {
	Uint8 *line;
	const Uint8 *gx_line, *gy_line;

	for (int y = 0; y < height; y++) {
//		line = input.scanLine(y);
		line = input.data() + y * width;
//		gx_line = gx.constScanLine(y);
		gx_line = gx.data() + y * width;
//		gy_line = gy.constScanLine(y);
		gy_line = gy.data() + y * width;

		for (int x = 0; x < width; x++)
			line[x] = bound(0x00, static_cast<int>(hypot(gx_line[x], gy_line[x])), 0xFF);
	}
}

vector<Uint8> hysteresis(const vector<Uint8>& image, float tmin, float tmax, int width,
		int height) {
//	auto res = vector<Uint8>(image.size(), image.format());
//	res.fill(0x00);
	vector<Uint8> result;
	result.resize(width * height);
	fill(result.begin(), result.end(), 0x00);

	const Uint8 *original_line;
	Uint8 *result_line;
	int ni, nj;
	queue<pair<int, int>> edges;

	for (int y = 1; y < height - 1; y++) {
//		original_line = image.constScanLine(y);
		original_line = image.data() + y * width;
//		result_line = res.scanLine(y);
		result_line = result.data() + y * width;

		for (int x = 1; x < width - 1; x++) {
			if (original_line[x] >= tmax && result_line[x] == 0x00) {
				result_line[x] = 0xFF;
				edges.push(make_pair(x, y));

				while (!edges.empty()) {
					auto point = edges.front();
					edges.pop();

					for (int j = -1; j <= 1; j++) {
						nj = point.second + j;
						if (nj < 0 || nj >= height)
							continue;

//						original_line = image.constScanLine(nj);
						original_line = image.data() + nj * width;
//						result_line = res.scanLine(nj);
						result_line = result.data() + nj * width;

						for (int i = -1; i <= 1; i++) {
							if (!i && !j)
								continue;

							ni = point.first + i;
							if (ni < 0 || ni >= width)
								continue;

							if (original_line[ni] >= tmin && result_line[ni] == 0x00) {
								result_line[ni] = 0xFF;
								edges.push(make_pair(ni, nj));
							}
						}
					}
				}
			}
		}
	}

	return result;
}

vector<Uint8> computeCanny(const vector<Uint8>& input, int width, int height, float sigma,
		float tmin, float tmax) {
	vector<Uint8> result;
	result.resize(width * height);

////////////////////////////////////////////////
	vector<Uint8> res = convolution(gaussian_kernel(sigma), input, width, height); // Gaussian blur
	// Gradients
	vector<Uint8> gx = convolution(sobelx, res, width, height);
	vector<Uint8> gy = convolution(sobely, res, width, height);

	magnitude(res, gx, gy, width, height);

	// Non-maximum suppression
	Uint8 *line;
	const Uint8 *prev_line, *next_line, *gx_line, *gy_line;

	for (int y = 1; y < height - 1; y++) {
//		line = res.scanLine(y);
		line = res.data() + y * width;
//		prev_line = res.constScanLine(y - 1);
		prev_line = res.data() + (y - 1) * width;
//		next_line = res.constScanLine(y + 1);
		next_line = res.data() + (y + 1) * width;
//		gx_line = gx.constScanLine(y);
		gx_line = gx.data() + y * width;
//		gy_line = gy.constScanLine(y);
		gy_line = gy.data() + y * width;

		for (int x = 1; x < width - 1; x++) {
			double at = atan2(gy_line[x], gx_line[x]);
			const double dir = fmod(at + M_PI, M_PI) / M_PI * 8;

			if ((1 >= dir || dir > 7) && line[x - 1] < line[x] && line[x + 1] < line[x]
					|| (1 < dir || dir <= 3) && prev_line[x - 1] < line[x]
							&& next_line[x + 1] < line[x]
					|| (3 < dir || dir <= 5) && prev_line[x] < line[x] && next_line[x] < line[x]
					|| (5 < dir || dir <= 7) && prev_line[x + 1] < line[x]
							&& next_line[x - 1] < line[x])
				continue;

			line[x] = 0x00;
		}
	}

	// Hysteresis
	return hysteresis(res, tmin, tmax, width, height);

////////////////////////////////////////////////

	return result;
}

std::vector<Uint8> computeSobel(const std::vector<Uint8>& input, int width, int height) {
//    QImage res(input.size(), input.format());
	vector<Uint8> result;
	result.resize(width * height);
	magnitude(result, convolution(sobelx, input, width, height),
			convolution(sobely, input, width, height), width, height);
	return result;
}

std::vector<Uint8> computePrewitt(const std::vector<Uint8>& input, int width, int height) {
	vector<Uint8> result;
	result.resize(width * height);
	magnitude(result, convolution(prewittx, input, width, height),
			convolution(prewitty, input, width, height), width, height);
	return result;
}

std::vector<Uint8> computeRoberts(const std::vector<Uint8>& input, int width, int height) {
	vector<Uint8> result;
	result.resize(width * height);
	magnitude(result, convolution(robertsx, input, width, height),
			convolution(robertsy, input, width, height), width, height);
	return result;
}

std::vector<Uint8> computeScharr(const std::vector<Uint8>& input, int width, int height) {
	vector<Uint8> result;
	result.resize(width * height);
	magnitude(result, convolution(scharrx, input, width, height),
			convolution(scharry, input, width, height), width, height);
	return result;
}
