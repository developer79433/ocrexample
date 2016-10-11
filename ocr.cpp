#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

#include "ocr.h"

#include <iostream>

using namespace std;

void ocr(const char *filename)
{
    char *outText;

    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
    // Initialize tesseract-ocr with English, without specifying tessdata path
    if (api->Init(NULL, "eng")) {
        cerr << "Could not initialize tesseract." << endl;
        exit(1);
    }

    // Open input image with leptonica library
    Pix *image = pixRead(filename);
    api->SetImage(image);
    // Get OCR result
    outText = api->GetUTF8Text();
    cout << "OCR output:" << endl << outText << endl;

    // Destroy used object and release memory
    api->End();
    delete [] outText;
    pixDestroy(&image);
}
