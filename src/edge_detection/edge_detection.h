/*
 * edge_detection.h
 *
 *  Created on: 2 maj 2018
 *      Author: sebastian
 */

#ifndef EDGE_DETECTION_H_
#define EDGE_DETECTION_H_

// Adapted after https://github.com/vaultah/edge-detection.git

#include <vector>
#include <SDL_stdinc.h>

std::vector<Uint8> computeCanny(const std::vector<Uint8>& input, int width, int height, float sigma,
		float tmin, float tmax);

std::vector<Uint8> computeSobel(const std::vector<Uint8>& input, int width, int height);
std::vector<Uint8> computePrewitt(const std::vector<Uint8>& input, int width, int height);
std::vector<Uint8> computeRoberts(const std::vector<Uint8>& input, int width, int height);
std::vector<Uint8> computeScharr(const std::vector<Uint8>& input, int width, int height);

#endif /* EDGE_DETECTION_H_ */
