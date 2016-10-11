#include <opencv2/opencv.hpp>

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

void display_image(const string &name, const Mat &image)
{
	cerr << "Image " << name << " dims: " << image.size().width << "x" << image.size().height << endl;
	imshow(name, image);
	waitKey(0);
	destroyWindow(name);
}
