

#include <windows.h>

#include <time.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>

#define internal static
#define global_variable static
#define local_persist static

struct win32_buffer
{
	void *Memory;
	int Width;
	int Height;
	BITMAPINFO Info;
};




internal void
Win32DebugPrintFloat(float var);

