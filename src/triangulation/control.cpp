#include <iostream>
//#include <hash_set>
#include <set>
#include <vector>
#include <fstream>
#include <stdlib.h>
#include <math.h>

#include <memory.h>
//#include <malloc.h>
#include <ctype.h>
#include <string.h>
#include <string>
#include <strings.h>
#include <sys/types.h>

#include "s_hull_pro.h"

/* copyright 2016 Dr David Sinclair
 david@s-hull.org
 
 program to compute Delaunay triangulation of a set of points.

 this code is released under GPL3,
 a copy ofthe license can be found at
 http://www.gnu.org/licenses/gpl-3.0.html

 you can purchase a un-restricted licnese from
 http://www.s-hull.org
 for the price of one beer!

 revised 12/feb/2016
 
 */

#include <sys/time.h>

using namespace std;

int main3(int argc, char *argv[]) {

	if (argc == 1) {
		cerr << "s_hull_pro delaunay triangulation demo" << endl;
		cerr << "usage: />   shullpro <points_file> <triangles_file> " << endl;

		float goat = (2147483648.0 - 1) / 100.0;

		std::vector<Shx> pts, pts2;
		Shx pt;
		srandom(1);

		//      for(int g=0; g<20; g++){
		pts.clear();
		for (int v = 0; v < 1000; v++) {
			//for(int v=0; v<20000; v++){
			//for(int v=0; v<1000000; v++){
			pt.id = v;
			pt.r = (int) (((float) rand() / RAND_MAX + 1) * 5000); // pts.txt
			pt.c = (int) (((float) rand() / RAND_MAX + 1) * 5000);

			//	pt.r = ((float) random())/goat - 50;
			//pt.c = ((float) random())/goat - 50;

			pts.push_back(pt);
		}

		std::vector<Triad> triads;

		std::vector<int> outx;
		int nx = de_duplicateX(pts, outx, pts2);
		pts.clear();

		write_Shx(pts2, "pts.mat");
		cerr << pts2.size() << " randomly generated points written to pts.mat" << endl;

		struct timeval tv1, tv2;
		gettimeofday(&tv1, NULL);

		int ts = s_hull_pro(pts2, triads);
		pts2.clear();

		gettimeofday(&tv2, NULL);
		float tx = (tv2.tv_sec + tv2.tv_usec / 1000000.0) - (tv1.tv_sec + tv1.tv_usec / 1000000.0);

		cerr << tx << " seconds for triangulation" << endl;

		write_Triads(triads, "triangles.mat");
		cerr << "triangles written to triangles.mat" << endl;
		//}

		exit(0);
	} else if (argc > 1) {

		cerr << "reading points from " << argv[1] << endl;
		std::vector<Shx> pts4, pts3;

		int nump = read_Shx(pts4, argv[1]);
		cerr << nump << " points read" << endl;

		// check for duplicates.
		std::vector<int> dupes;
		//int numd = de_duplicateX( pts4, dupes, pts3); // note:  de_duplicateX changes point order and ids.
		int numd = de_duplicate(pts4, dupes);
		cerr << "duplicates filtered  " << numd << endl;

		//     write_Shx(pts3, "pts_deduped.mat");

		struct timeval tv1, tv2;
		gettimeofday(&tv1, NULL);

		std::vector<Triad> triads;
		//int ts = s_hull_pro( pts3, triads);
		int ts = s_hull_pro(pts4, triads);

		gettimeofday(&tv2, NULL);
		float tx = (tv2.tv_sec + tv2.tv_usec / 1000000.0) - (tv1.tv_sec + tv1.tv_usec / 1000000.0);

		cerr << tx << " seconds for triangulation" << endl;

		if (argc == 2) {
			write_Triads(triads, "triangles.mat");
			cerr << "triangles written to triangles.mat" << endl;
		} else {
			write_Triads(triads, argv[2]);
			cerr << "triangles written to " << argv[2] << endl;
		}
	}

	exit(0);
}

#include <sys/time.h>
//static inline 
double getpropertime() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec + tv.tv_usec / 1000000.0;
}

