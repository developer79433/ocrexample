#ifndef OCR_H_
#define OCR_H_

#include <cstdlib>
#include <tesseract/baseapi.h>

using namespace std;

namespace ocr {

class Recogniser {
private:
	tesseract::TessBaseAPI *api;
	Pix *image;
public:
	Recogniser(const char *lang = "eng");
	virtual ~Recogniser(void);
	void set_image(Pix *image);
	void set_image(
		const unsigned char *imagedata,
		int width,
		int height,
		int bytes_per_pixel,
		int bytes_per_line
	);
	void ocr(void);
	void set_image(const char *filename);
};

}

#endif /* OCR_H_ */
