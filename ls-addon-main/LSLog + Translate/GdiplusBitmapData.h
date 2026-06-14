#ifndef GDIPLUSBITMAPDATA_H
#define GDIPLUSBITMAPDATA_H

#include <Windows.h>
#include <gdiplus.h>

class GdiplusBitmapData {
public:
    GdiplusBitmapData(Gdiplus::Bitmap* bitmap);
    ~GdiplusBitmapData();

    bool Lock();
    void Unlock();
    BYTE* GetPixelData() const;

    UINT GetWidth() const;
    UINT GetHeight() const;
    UINT GetStride() const;

private:
    Gdiplus::Bitmap* bitmap;
    Gdiplus::BitmapData bitmapData;
    bool isLocked;

    // Prevent copying
    GdiplusBitmapData(const GdiplusBitmapData&) = delete;
    GdiplusBitmapData& operator=(const GdiplusBitmapData&) = delete;
};

#endif // GDIPLUSBITMAPDATA_H
