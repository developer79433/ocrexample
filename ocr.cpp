#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

#include "ocr.h"

#include <iostream>

using namespace std;

void ocr(const char *filename)
{
    tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
    // Initialize tesseract-ocr with English, without specifying tessdata path
    if (api->Init(NULL, "eng")) {
        cerr << "Could not initialize tesseract." << endl;
        exit(1);
    }

    // Open input image with leptonica library
    Pix *image = pixRead(filename);
    api->SetImage(image);
#ifdef SIMPLE
    // Get OCR result
    char *outText = api->GetUTF8Text();
    cout << "OCR output:" << endl << outText << endl;
    delete [] outText;
#else
    api->Recognize(0);
      tesseract::ResultIterator* ri = api->GetIterator();
      tesseract::PageIteratorLevel level = tesseract::RIL_WORD;
      if (ri != 0) {
        do {
          const char* word = ri->GetUTF8Text(level);
          float conf = ri->Confidence(level);
          int x1, y1, x2, y2;
          ri->BoundingBox(level, &x1, &y1, &x2, &y2);
          cout << "word: '" << word << "';  \tconf: " << conf << "; BoundingBox: " << x1 << "," << y1 << "," << x2 << "," << y2 << ";" << endl;
          delete[] word;
        } while (ri->Next(level));
      }
#endif

    // Destroy used object and release memory
    api->End();
    pixDestroy(&image);
}
