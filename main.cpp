#include <iostream>
#include <cmath>

#include <opencv2/opencv.hpp>

#include "distance.h"
#include "ocr.h"
#include "debug.h"
#include "util.h"

using namespace std;
using namespace cv;
using namespace ocr;

#define EDGE_DETECTION_SIZE 500
#define CONTOUR_COUNT 5
#if 0
#define DISPLAY_INTERMEDIATE_IMAGES
#endif

template <typename T> void order_points(const array<Point_<T>, 4> &pts, array<Point_<T>, 4> &ordered)
{
    // initialzie a list of coordinates that will be ordered
    // such that the first entry in the list is the top-left,
    // the second entry is the top-right, the third is the
    // bottom-right, and the fourth is the bottom-left

	// the top-left point will have the smallest sum, whereas
    // the bottom-right point will have the largest sum
    // NOTE: Therefore the "top-left point" is neither necessarily the top-most point,
    // nor necessarily the left-most point.
    array<T, 4> sums = { pts[0].x + pts[0].y, pts[1].x + pts[1].y, pts[2].x + pts[2].y, pts[3].x + pts[3].y };
    // dump_array("sum:", sums);
    ordered[0] = pts[min_element(sums.begin(), sums.end()) - sums.begin()];
    ordered[2] = pts[max_element(sums.begin(), sums.end()) - sums.begin()];

    // now, compute the difference between the points, the
    // top-right point will have the smallest difference,
    // whereas the bottom-left will have the largest difference
    array<T, 4> differences = { pts[0].y - pts[0].x, pts[1].y - pts[1].x, pts[2].y - pts[2].x, pts[3].y - pts[3].x };
    // dump_array("differences:", differences);

    ordered[1] = pts[min_element(differences.begin(), differences.end()) - differences.begin()];
    ordered[3] = pts[max_element(differences.begin(), differences.end()) - differences.begin()];
    // dump_point_array("order_points ordered", ordered);
  }

template <typename T> void four_point_transform(const Mat &image, const array<Point_<T>, 4> &pts, Mat &warped)
{
	// dump_point_array("four_point_transform", pts);
    // obtain a consistent order of the points and unpack them
    // individually
	array<Point_<T>, 4> rect;
    order_points(pts, rect);
    // dump_point_array("four_point_transform rect", rect);
    const Point_<T> &tl = rect[0], &tr = rect[1], &br = rect[2], &bl = rect[3];

    // compute the width of the new image, which will be the
    // maximum distance between bottom-right and bottom-left
    // x-coordiates or the top-right and top-left x-coordinates
    T maxWidth = max(pythagorean_distance(br, bl), pythagorean_distance(tr, tl));

    // compute the height of the new image, which will be the
    // maximum distance between the top-right and bottom-right
    // y-coordinates or the top-left and bottom-left y-coordinates
    T maxHeight = max(pythagorean_distance(tr, br), pythagorean_distance(tl, bl));

    // now that we have the dimensions of the new image, construct
    // the set of destination points to obtain a "birds eye view",
    // (i.e. top-down view) of the image, again specifying points
    // in the top-left, top-right, bottom-right, and bottom-left
    // order
    array<Point_<T>, 4> dst = {{{0, 0}, {maxWidth - 1, 0}, {maxWidth - 1, maxHeight - 1}, {0, maxHeight - 1}}};
    // dump_array("dst", dst);

    // compute the perspective transform matrix and then apply it
    Mat M = getPerspectiveTransform(&rect[0], &dst[0]);
    // cerr << "Matrix: " << M << endl;
    // display_image("Perspective input", image);
    warpPerspective(image, warped, M, Size(maxWidth, maxHeight));
    // display_image("Perspective output", warped);
}

static void process(Mat &image)
{
#ifdef DISPLAY_INTERMEDIATE_IMAGES
	display_image("Original", image);
#endif /* DISPLAY_INTERMEDIATE_IMAGES */
	Mat smaller;
	float ratio;
	if (image.size().width > EDGE_DETECTION_SIZE || image.size().height > EDGE_DETECTION_SIZE) {
		// The image is bigger than we need in order to detect edges.
		// Isotropically scale it down to speed up edge detection.
		ratio = (float) max(image.size().width, image.size().height) / EDGE_DETECTION_SIZE;
		resize(image, smaller, Size(image.size().width / ratio, image.size().height / ratio));
	} else {
		ratio = 1;
		smaller = image;
	}
#ifdef DISPLAY_INTERMEDIATE_IMAGES
	display_image("Work", smaller);
#endif /* DISPLAY_INTERMEDIATE_IMAGES */
	// Convert inplace to greyscale for contour detection
	cvtColor(smaller, smaller, COLOR_BGR2GRAY);
	// Remove high frequency noise
	GaussianBlur(smaller, smaller, Size(3, 3), 0);
	// Find edges using Canny
	Mat edges;
	Canny(smaller, edges, 75, 200);
#ifdef DISPLAY_INTERMEDIATE_IMAGES
	display_image("Edges", edges);
#endif /* DISPLAY_INTERMEDIATE_IMAGES */
	// Find contours in edge image
	vector<vector<Point> > contours;
	findContours(edges, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	// Sort the contours in decreasing area
	sort(contours.begin(), contours.end(), [](const vector<Point>& c1, const vector<Point>& c2){
		return contourArea(c1, false) > contourArea(c2, false);
	});
	// Find the first roughly quadrilateral contour in the few largest-area contours
	vector<Point> *quadrilateral = NULL;
	vector<vector<Point> >::iterator border_iter = find_if(contours.begin(), min(contours.end(), contours.begin() + CONTOUR_COUNT), [&quadrilateral](vector<Point> &contour) {
		double len = arcLength(contour, true);
		vector<Point> simplified;
		approxPolyDP(contour, simplified, 0.02 * len, true);
		if (simplified.size() == 4) {
			quadrilateral = new vector<Point>(simplified);
			return true;
		}
		return false;
	});
	int border_idx;
	if (border_iter == contours.begin() + CONTOUR_COUNT) {
		border_idx = -1; // All
	} else {
		border_idx = border_iter - contours.begin();
	}
	// cerr << "border_idx: " << border_idx << endl;
	assert((quadrilateral == NULL) == (border_idx == -1));
	drawContours(smaller, contours, border_idx, Scalar(255, 0, 0), CV_FILLED);
#ifdef DISPLAY_INTERMEDIATE_IMAGES
	display_image("Contours", smaller);
#endif /* DISPLAY_INTERMEDIATE_IMAGES */

	// Warp image
	if (quadrilateral) {
		// dump_point_vector("Quadrilateral before scaling", *quadrilateral);
		for_each(quadrilateral->begin(), quadrilateral->end(), [ratio](Point &p) {
			p.x *= ratio;
			p.y *= ratio;
		});
		// dump_point_vector("Quadrilateral after scaling", *quadrilateral);
		vector<vector<Point> > quad_contours;
		quad_contours.push_back(*quadrilateral);
		Mat warped(image.size(), CV_8UC3);
		array<Point2f, 4> border_arr;
		// FIXME: Ugly conversion from vector to array
		border_arr[0] = (*quadrilateral)[0];
		border_arr[1] = (*quadrilateral)[1];
		border_arr[2] = (*quadrilateral)[2];
		border_arr[3] = (*quadrilateral)[3];
		four_point_transform(image, border_arr, warped);
#ifdef DISPLAY_INTERMEDIATE_IMAGES
		display_image("Corrected", warped);
#endif /* DISPLAY_INTERMEDIATE_IMAGES */
        Mat warped_grey;
        cvtColor(warped, warped_grey, COLOR_BGR2GRAY);
// #ifdef DISPLAY_INTERMEDIATE_IMAGES
        display_image("Corrected grey", warped_grey);
// #endif /* DISPLAY_INTERMEDIATE_IMAGES */
		vector<uchar> buf;
		imencode(".bmp", warped_grey, buf);
		Recogniser tess;
		tess.set_image_bmp(&buf[0]);
		tess.ocr();
	}
}

static int process_cmdline_args(int argc, char *argv[])
{
	char **arg;
	int ret = EXIT_SUCCESS;

	for (arg = &argv[1]; arg < &argv[argc]; arg++) {
		Mat input = imread(*arg);
		if (input.data) {
			process(input);
		} else {
			cerr << "Failed to load image from " << *arg << endl;
			ret = EXIT_FAILURE;
			break;
		}
	}

	return ret;
}

static int process_camera(void)
{
	VideoCapture cap(0);
	if (!cap.isOpened()) {
		return EXIT_FAILURE;
	}
	Mat frame;
	cap >> frame;
	process(frame);

	return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
	if (argc > 1) {
		return process_cmdline_args(argc, argv);
	}
	return process_camera();
}
