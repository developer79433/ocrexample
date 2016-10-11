#include <opencv2/opencv.hpp>

#include "util.h"

using namespace std;
using namespace cv;

template <typename T> void dump_array_n(const string &name, const T *pts, unsigned int size)
{
	cerr << name << endl;
	const T *pt;
	for (pt = &pts[0]; pt < &pts[size]; pt++) {
		cerr << *pt << ", ";
	}
	cerr << endl;
}

#define dump_array(name, arr) do { dump_array_n(name, &(arr)[0], ELEMENTSOF(arr)); } while (0)

template <typename T> void dump_point_array_n(const string &name, const Point_<T> *pts, unsigned int size)
{
	cerr << name << endl;
	const Point_<T> *pt;
	for (pt = &pts[0]; pt < &pts[size]; pt++) {
		cerr << *pt << ", ";
	}
	cerr << endl;
}

#define dump_point_array(name, arr) do { dump_point_array_n(name, &(arr)[0], ELEMENTSOF(arr)); } while (0)

template <typename T> void dump_point_vector(const string &name, vector<Point_<T> > &vec)
{
	cerr << name << endl;
	for_each(vec.begin(), vec.end(), [](const Point_<T> &pt) {
		cerr << pt << ", ";
	});
	cerr << endl;
}

static const char *depth_names[] = {
	"CV_8U",
	"CV_8S",
	"CV_16U",
	"CV_16S",
	"CV_32S",
	"CV_32F",
	"CV_64F"
};

/**
 * Docs say:
 * CV_MAKETYPE(depth, n) == (depth&7) + ((n-1)<<3)
 * This means that the constant type is formed from the depth, taking the lowest 3 bits, and the number of channels minus 1, taking the next log2(CV_CN_MAX) bits.
 */
#define TYPE_TO_DEPTH_INDEX(type) ((type) & 7)
#define DEPTH_IDX_TO_DEPTH_NAME(idx) (((idx) >= 0 && static_cast<unsigned>(idx) < ELEMENTSOF(depth_names)) ? depth_names[(idx)] : "unknown")
#define TYPE_TO_DEPTH_NAME(type) DEPTH_IDX_TO_DEPTH_NAME(TYPE_TO_DEPTH_INDEX(type))
#define TYPE_TO_CHANNELS(type) (((type) >> 3) + 1)

void display_image(const string &name, const Mat &image)
{
	cerr
	<< "Image " << name
	<< " dims: " << image.size().width << "x" << image.size().height
	<< " depth: " << TYPE_TO_DEPTH_NAME(image.type()) << "C" << image.channels()
	<< endl;
	imshow(name, image);
	waitKey(0);
	destroyWindow(name);
}
