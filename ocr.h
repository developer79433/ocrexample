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
	void delete_api(void) {
		delete api;
		api = NULL;
	};
public:
	Recogniser(const char *lang = "eng", const char *data_dir = NULL, const char *whitelist = NULL);
	virtual ~Recogniser(void);
	void set_image(Pix *image);
	void set_image(
		const unsigned char *imagedata,
		int width,
		int height,
		int bytes_per_pixel,
		int bytes_per_line
	);
	void set_image_bmp(const void *bmp_data);
	void ocr(void);
	void set_image(const char *filename);
};

}

#endif /* OCR_H_ */
