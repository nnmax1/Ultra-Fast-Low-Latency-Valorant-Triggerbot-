#include <windows.h>
#include <iostream>
#include <thread>
#include <chrono>

// Toggle key
#define TOGGLE_KEY VK_F8

// Valorant enemy outline yellow range (RGB)
// Adjust these based on your monitor calibration, in-game brightness, and post-processing
const int R_MIN = 210, R_MAX = 255;
const int G_MIN = 160, G_MAX = 255;
const int B_MIN = 0,   B_MAX = 80;

void RunTriggerBot() {
    HDC hScreen = GetDC(NULL);
    HDC hDC = CreateCompatibleDC(hScreen);
    
    int width = 1, height = 1;
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // Top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    unsigned char* pixels = nullptr;
    HBITMAP hBitmap = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, (void**)&pixels, NULL, 0);
    SelectObject(hDC, hBitmap);

    std::cout << "[+] Triggerbot initialized. Press F8 to toggle.\n";
    bool active = false;

    // Pre-allocate INPUT structure to avoid allocation overhead in loop
    INPUT fireInput = {0};
    fireInput.type = INPUT_MOUSE;
    fireInput.mi.dwFlags = MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP;

    while (true) {
        if (GetAsyncKeyState(TOGGLE_KEY) & 1) {
            active = !active;
            std::cout << (active ? "[*] Active\n" : "[*] Inactive\n");
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        if (!active) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }

        int cx = GetSystemMetrics(SM_CXSCREEN) / 2;
        int cy = GetSystemMetrics(SM_CYSCREEN) / 2;

        // Capture only the 1x1 pixel at screen center
        BitBlt(hDC, 0, 0, width, height, hScreen, cx, cy, SRCCOPY);

        unsigned char r = pixels[2];
        unsigned char g = pixels[1];
        unsigned char b = pixels[0];

        if (r >= R_MIN && r <= R_MAX && g >= G_MIN && g <= G_MAX && b >= B_MIN && b <= B_MAX) {
            SendInput(1, &fireInput, sizeof(INPUT));
            // Minimal cooldown to prevent full-auto spray on semi-auto weapons
            std::this_thread::sleep_for(std::chrono::milliseconds(15));
        }

        std::this_thread::yield(); // Yield to OS scheduler for max polling speed
    }

    delete[] pixels;
    DeleteObject(hBitmap);
    DeleteDC(hDC);
    ReleaseDC(NULL, hScreen);
}

int main() {
    // Disable Windows timer resolution throttling
    timeBeginPeriod(1);
    
    // Pin to a single CPU core to avoid cache misses and context switching
    SetThreadAffinityMask(GetCurrentThread(), 1);
    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    
    // Disable DPI scaling artifacts
    SetProcessDPIAware();

    RunTriggerBot();
    return 0;
}