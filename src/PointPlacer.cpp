/*
 * PointPlacer.cpp
 *
 *  Created on: 2 maj 2018
 *      Author: sebastian
 */

#include <stdlib.h>
#include "PointPlacer.h"

namespace oip {

PointPlacer::PointPlacer() {
	// TODO Auto-generated constructor stub

}

PointPlacer::~PointPlacer() {
	// TODO Auto-generated destructor stub
}

std::vector<ColorPoint> RandomPoints(int w, int h, int pointsNum, int margin) {
	std::vector<ColorPoint> points;
	ColorPoint p;
	for (int i = 0; i < pointsNum; i++) {
		p.x = rand() % (w - 2 * margin) + margin;
		p.y = rand() % (h - 2 * margin) + margin;
		p.r = rand() % 256;
		p.g = rand() % 256;
		p.b = rand() % 256;
		points.push_back(p);
	}
	return points;
}

} /* namespace oip */
