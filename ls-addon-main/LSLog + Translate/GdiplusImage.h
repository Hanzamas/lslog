#ifndef GDIPLUSIMAGE_H
#define GDIPLUSIMAGE_H

#include <Windows.h>
#include <gdiplus.h>

#pragma comment (lib, "gdiplus.lib")

class GdiplusImage {
public:
    GdiplusImage();
    ~GdiplusImage();

    bool LoadImageFromFile(const WCHAR* filename);
    bool SaveImageToFile(const WCHAR* filename, const CLSID& clsidEncoder);
    void DrawImage(HDC hdc, int x, int y);
    void ResizeImage(UINT width, UINT height);

private:
    Gdiplus::Image* image;
    UINT originalWidth;
    UINT originalHeight;

    void InitializeGdiplus();
    void ShutdownGdiplus();
};

#endif // GDIPLUSIMAGE_H
