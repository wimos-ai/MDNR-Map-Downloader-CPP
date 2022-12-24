#include "EasyBitmap.h"
#include <stdexcept>

EasyBitmap::EasyBitmap(Gdiplus::Bitmap* bmp): bmp(bmp)
{
	Gdiplus::Rect rc(0, 0, bmp->GetWidth(), bmp->GetHeight());

	if (bmp->LockBits(&rc, Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeWrite, PixelFormat32bppARGB, &data) != Gdiplus::Status::Ok) {
		throw std::logic_error("Locking the file failed");
	}

}

ARGB_Pixel* EasyBitmap::pixelAt(int xidx, int yidx)
{
#ifdef _DEBUG
	if (xidx > this->width() || xidx < 0) {
		throw std::logic_error("xidx is out of bounds");
	}

	if (yidx > this->height() || yidx < 0)
	{
		throw std::logic_error("yidx is out of bounds");
	}
#endif // _DEBUG

	ARGB_Pixel* dat = static_cast<ARGB_Pixel*>(data.Scan0);

	ARGB_Pixel* pix = &(dat[yidx * std::abs(data.Stride) / 4 + xidx]);

	auto r = pix->red;

	pix->red = 0;

	pix->red = r;

	return pix;

	//return ((ARGB_Pixel*)data.Scan0)[xidx + (yidx * data.Width)];
}

EasyBitmap::~EasyBitmap()
{
	if (bmp->UnlockBits(&data) != Gdiplus::Ok) {
		throw std::logic_error("Error unlocking bitmap");
	}
}

int EasyBitmap::width()
{
	return data.Width;
}

int EasyBitmap::height()
{
	return data.Height;
}
