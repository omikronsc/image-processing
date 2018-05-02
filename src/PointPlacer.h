/*
 * PointPlacer.h
 *
 *  Created on: 2 maj 2018
 *      Author: sebastian
 */

#ifndef POINTPLACER_H_
#define POINTPLACER_H_

#include <vector>

namespace oip {

struct ColorPoint {
	int x, y, r, g, b;
};

std::vector<ColorPoint> RandomPoints(int w, int h, int pointsNum, int margin);

class PointPlacer {
public:
	PointPlacer();
	virtual ~PointPlacer();
};

} /* namespace oip */

#endif /* POINTPLACER_H_ */
