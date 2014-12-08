

#include "win32_game.h"


#include "game.cpp"


global_variable bool GlobalRunning;
global_variable win32_buffer Win32Buffer;
global_variable HDC CompatableDC;

global_variable int GlobalScore;
global_variable char *GlobalMessage;
global_variable char *GlobalMessage2;

global_variable HFONT GlobalScoreFont;
global_variable HFONT GlobalBigFont;
global_variable HFONT GlobalHighFont;





internal void 
Win32DispString(HWND hwnd, HDC DeviceContext, char* s, int x, int y, HFONT hfont, UINT alignmentOps)
{
	// HFONT hfont, oldFont;

	if (!s) return;

	// // hfont = (HFONT) GetStockObject(ANSI_VAR_FONT);
	// hfont = GlobalFont;

	if (SelectObject(DeviceContext, hfont))
	{
		SetTextAlign(DeviceContext, alignmentOps);
		SetTextColor(DeviceContext, (COLORREF)TextColor);
		SetBkMode(DeviceContext, TRANSPARENT);
		// SetTextColor(DeviceContext, (COLORREF)TextColor);
		// SetBkMode(DeviceContext, TRANSPARENT);
		TextOut(DeviceContext, x, y, s, strlen(s));

		// RECT r;
		// BeginPath(DeviceContext);
		// TextOut(DeviceContext, 10, 50, s, strlen(s));
		// EndPath(DeviceContext);
		// SelectClipPath(DeviceContext, RGN_AND);
		// FillRect(DeviceContext, &r, GetStockObject(GRAY_BRUSH));

	}
	else 
	{
		MessageBox(0, "Trouble displaying font.\n\nSorry, I'm new at this...", "^-^", MB_OK);
		exit(1);
	}

	// RECT pos = {x, y, 100, 100};

	// DrawText(
	//          DeviceContext,
	//          s,
	//          strlen(s),
	//          &pos,
	//          DT_CENTER);

}


// #define Win32DisplayBufferOnScreen Win32DisplayBufferOnScreenDouble
// #define Win32DisplayBufferOnScreen Win32DisplayBufferOnScreenSingle


internal void Win32DisplayBufferOnScreenDouble(win32_buffer *Buffer, int Score, 
                                               char *MainMessage1, char *MainMessage2, 
                                               HWND Window, HDC DeviceContext, HDC OffsDC)
{

	RECT clientRect;
	GetClientRect(Window, &clientRect);
	int WindowWidth = clientRect.right - clientRect.left;
	int WindowHeight = clientRect.bottom - clientRect.top;



	int result = StretchDIBits(
	              OffsDC,
	              0,
	              0,
	              WindowWidth,
	              WindowHeight,
	              0,
	              0,
	              Buffer->Width,
	              Buffer->Height,
	              Buffer->Memory,
	              &Buffer->Info,
	              DIB_RGB_COLORS,
				  SRCCOPY);
	if (!result)
	{
		DWORD msg = GetLastError();
		MessageBox(0, (char*)msg, " ", MB_OK);
	}


	char StringBuffer[256];
	sprintf(StringBuffer, "%d\n", Score);
	Win32DispString(Window, OffsDC, StringBuffer, 10, 5, GlobalScoreFont, TA_LEFT|TA_TOP);

	int cx = Buffer->Width / 2;// - 100;
	int cy = Buffer->Height / 2 - 20;
	Win32DispString(Window, OffsDC, MainMessage1, cx, cy, GlobalBigFont, TA_CENTER|TA_BOTTOM);

	cx = Buffer->Width / 2;// - 100;
	cy = (Buffer->Height / 2) + 100;
	Win32DispString(Window, OffsDC, MainMessage2, cx, cy, GlobalHighFont, TA_CENTER|TA_BOTTOM);

	BitBlt(
	       DeviceContext,
	       0,
	       0,
	       WindowWidth,
	       WindowHeight,
	       OffsDC,
	       0,
	       0,
	       SRCCOPY);



}


// internal void Win32DisplayBufferOnScreenSingle(win32_buffer *Buffer, int Score, char *BigMessage, HWND Window, HDC DeviceContext)
// {

// 	RECT clientRect;
// 	GetClientRect(Window, &clientRect);
// 	int WindowWidth = clientRect.right - clientRect.left;
// 	int WindowHeight = clientRect.bottom - clientRect.top;

// 	int result = StretchDIBits(
// 	              DeviceContext,
// 	              0,
// 	              0,
// 	              WindowWidth,
// 	              WindowHeight,
// 	              0,
// 	              0,
// 	              Buffer->Width,
// 	              Buffer->Height,
// 	              Buffer->Memory,
// 	              &Buffer->Info,
// 	              DIB_RGB_COLORS,
// 				  SRCCOPY);
// 	if (!result)
// 	{
// 		DWORD msg = GetLastError();
// 		MessageBox(0, (char*)msg, " ", MB_OK);
// 	}

// 	char StringBuffer[256];
// 	sprintf(StringBuffer, "%d\n", Score);
// 	Win32DispString(Window, DeviceContext, StringBuffer, 10, 10, GlobalScoreFont, TA_LEFT|TA_TOP);

// 	int cx = WindowWidth / 2 - 100;
// 	int cy = WindowHeight / 2 - 100;
// 	Win32DispString(Window, DeviceContext, BigMessage, cx, cy, GlobalBigFont, TA_CENTER|TA_BOTTOM);

// 	// cx = Buffer->Width / 2;
// 	// cy = Buffer->Height / 2 + 10;
// 	// Win32DispString(Window, DeviceContext, GlobalMessage2, cx, cy, GlobalBigFont);

// 	// // help with flickering?
// 	// GdiFlush();
// }



internal void 
Win32ResizeBuffer(win32_buffer *Buffer, int wid, int hei)
{
	if (Buffer->Memory)
	{
		VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
	}

	Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info);
	Buffer->Info.bmiHeader.biWidth = wid;
	Buffer->Info.bmiHeader.biHeight = -hei;  // negative to set origin in top left
	Buffer->Info.bmiHeader.biPlanes = 1;
	Buffer->Info.bmiHeader.biBitCount = 32;
	Buffer->Info.bmiHeader.biCompression = BI_RGB;

	Buffer->Width = wid;
	Buffer->Height = hei;
	int BufferSize = Buffer->Width * Buffer->Height * 4;

	Buffer->Memory = VirtualAlloc(0, BufferSize, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
}



internal LRESULT CALLBACK 
Win32WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int result = 1; // 0 meaning 'i took care of things' ? i dont even know any more
	switch(uMsg)
	{
		case WM_CLOSE:
		{
			GlobalRunning = false;
		} break;


		// this helps the flicker, i think
		// case WM_ERASEBKGND:
		// {
		// 	return 1; 
		// } break;

		// case WM_SIZING:
		// {
		// 	// don't resize
		// } break;

		// uncomment this to resize the backbuffer on window resize
		// case WM_SIZE:
		// {
		// 	RECT clientRect;
		// 	GetClientRect(hwnd, &clientRect);
		// 	int WindowWidth = clientRect.right - clientRect.left;
		// 	int WindowHeight = clientRect.bottom - clientRect.top;
		// 	Win32ResizeBuffer(&Win32Buffer, WindowWidth, WindowHeight);
		// } break;

		// added to try to handle the text flickering
		//don't really need to handle paint?
		// on further invest, does not seem to be source of flicker
		// case WM_PAINT:
		// {
		// 	PAINTSTRUCT PaintStruct;
		// 	HDC PaintingDeviceContext = BeginPaint(hwnd, &PaintStruct);
		// 	Win32DisplayBufferOnScreenDouble(&Win32Buffer, GlobalScore, GlobalMessage, hwnd, PaintingDeviceContext);
		// 	EndPaint(hwnd, &PaintStruct);
		// } break;

		default:
		{
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
	}
	return result;
}


internal game_input 
Win32ReadKeyboardState()
{
	game_input Result = {};
	if (GetAsyncKeyState('W')) 		Result.w = true;
	if (GetAsyncKeyState('A'))		Result.a = true;
	if (GetAsyncKeyState('S')) 		Result.s = true;
	if (GetAsyncKeyState('D')) 		Result.d = true;
	if (GetAsyncKeyState(VK_UP))  	Result.up = true;
	if (GetAsyncKeyState(VK_DOWN)) 	Result.down = true;
	if (GetAsyncKeyState(VK_LEFT)) 	Result.left = true;
	if (GetAsyncKeyState(VK_RIGHT))	Result.right = true;
	return Result;
}


internal void
Win32DebugPrintFloat(float var)
{
	char Buffer[256];
	sprintf(Buffer, "%.2f\n", var);
	OutputDebugString(Buffer);
}


internal int CALLBACK 
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	WNDCLASS wc = {};
	//wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = Win32WindowProc;
	wc.hInstance = hInstance;
	// Class.hIcon = static_cast<HICON>(LoadImage(hInstance,
	//                                  MAKEINTRESOURCE(IDI_ANATWND),
	//                                  IMAGE_ICON,
	//                                  32,
	//                                  32,
	//                                  LR_DEFAULTSIZE));
	//Class.hCursor;
	wc.hCursor = LoadCursor (NULL, IDC_ARROW);
	wc.lpszClassName = "LD31SnowClassName";
	//Class.hIconSm;

	if (RegisterClass(&wc))
	{

		RECT NeededDims = {};
		NeededDims.right = 960;
		NeededDims.bottom = 720;
		AdjustWindowRectEx(&NeededDims, WS_OVERLAPPEDWINDOW, 0, 0);

		// ☃

		HWND Window = CreateWindowEx(0,
		                             wc.lpszClassName,
		                             //"Attack Of The Maybe Killer (But More Like Dispositionally-Ambiguous) Snowpeople",
		                             "Attack Of The Maybe Killer (But More Like Motivationally-Ambiguous) Snowpeople",
		                             (WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX) | WS_VISIBLE ,
		                             CW_USEDEFAULT,
		                             CW_USEDEFAULT,
		                             NeededDims.right - NeededDims.left,
		                             NeededDims.bottom - NeededDims.top,
		                             0,
		                             0,
		                             hInstance,
		                             0);

		if (Window)
		{
			

			RECT clientRect;
			GetClientRect(Window, &clientRect);
			int WindowWidth = clientRect.right - clientRect.left;
			int WindowHeight = clientRect.bottom - clientRect.top;
			Win32ResizeBuffer(&Win32Buffer, WindowWidth, WindowHeight);



			HDC DeviceContext = GetDC(Window);

			// ARGGgg
			HDC OffsDC = CreateCompatibleDC(DeviceContext);
			HBITMAP mbmp =  CreateBitmap(WindowWidth,WindowHeight,1,32,NULL);
			HBITMAP moldbmp = (HBITMAP)SelectObject(OffsDC,mbmp);



			// buffer 1, main buffer

			if (!Win32Buffer.Memory)
			{
				MessageBox(0, "No memory for screen!\n\nSorry, I'm new at this.","^-^", MB_OK);
				exit(1);
			}
			for (int y = 0; y < Win32Buffer.Height; y++)
			{
				for (int x = 0; x < Win32Buffer.Width; x++)
				{
					uint32_t Color = 0x55ff5500;
					int pixelOffset = y*Win32Buffer.Width + x;
					*(((uint32_t*)Win32Buffer.Memory) + pixelOffset) = Color;
				}
			}

			// wow totally not used
			// buffer 2, for baking particles into BG
			win32_buffer Win32BGBuffer;
			Win32ResizeBuffer(&Win32BGBuffer, WindowWidth, WindowHeight);
			if (!Win32BGBuffer.Memory)
			{
				MessageBox(0, "No memory for screen!\n\nSorry, I'm new at this.","^-^", MB_OK);
				exit(1);
			}
			for (int y = 0; y < Win32BGBuffer.Height; y++)
			{
				for (int x = 0; x < Win32BGBuffer.Width; x++)
				{
					uint32_t Color = 0x55ff5500;
					int pixelOffset = y*Win32BGBuffer.Width + x;
					*(((uint32_t*)Win32BGBuffer.Memory) + pixelOffset) = Color;
				}
			}



			GlobalScoreFont = CreateFont(
			                 36, //48,
			                 0,
			                 0,
			                 0,
			                 FW_BOLD,
			                 0,
			                 0,
			                 0,
			                 ANSI_CHARSET,
			                 0, //DWORD fdwOutputPrecision,
			                 0,
			                 ANTIALIASED_QUALITY,
			                 FF_MODERN,
			                 // "Calibri"
			                 // "Verdana"
			                 // "Candara"
			                 "Trebuchet MS"
			                 );

			GlobalBigFont = CreateFont(
			                 128, //96, //48,
			                 0,
			                 0,
			                 0,
			                 // FW_BOLD,
			                 FW_THIN,
			                 0,
			                 0,
			                 0,
			                 ANSI_CHARSET,
			                 0, //DWORD fdwOutputPrecision,
			                 0,
			                 ANTIALIASED_QUALITY,
			                 FF_SWISS,
			                 // "Calibri"
			                 "Verdana"
			                 // "Candara"
			                 // "Trebuchet MS"
			                 // "Segoe UI Semibold"
			                 // "Microsoft NeoGothic"
			                 // "Copperplate Gothic"
			                 // "Impact"
			                 // 0
			                 );

			GlobalHighFont = CreateFont(96,0,0,0,
			                           FW_THIN,0,0,0,
			                           ANSI_CHARSET,0,0,
			                           ANTIALIASED_QUALITY,
			                           FF_SWISS,
			                           "Verdana");

			// Win32InitTextRendering(Window, DeviceContext, GlobalScoreFont);


			// int Score = 0;

			LARGE_INTEGER CounterFrequency;
			QueryPerformanceFrequency(&CounterFrequency);
			LARGE_INTEGER LastCounter;
			LARGE_INTEGER ThisCounter;
			LARGE_INTEGER TicksElapsed;
			QueryPerformanceCounter(&LastCounter);


			GlobalRunning = true;
			while (GlobalRunning) {
				MSG Message;
				while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
				{
					TranslateMessage(&Message);
					DispatchMessage(&Message);
				}

				game_input InputThisFrame = Win32ReadKeyboardState();

				game_buffer RefToBufferForGameToFill = {};
				RefToBufferForGameToFill.Memory = Win32Buffer.Memory; // pointer will point to the same memory
				RefToBufferForGameToFill.Width = Win32Buffer.Width;
				RefToBufferForGameToFill.Height = Win32Buffer.Height;

				game_buffer BGBuffer = {};
				BGBuffer.Memory = Win32BGBuffer.Memory; // pointer will point to the same memory
				BGBuffer.Width = Win32BGBuffer.Width;
				BGBuffer.Height = Win32BGBuffer.Height;


				QueryPerformanceCounter(&ThisCounter);
				TicksElapsed.QuadPart = ThisCounter.QuadPart - LastCounter.QuadPart;
				float dt = (float)(TicksElapsed.QuadPart * 1000) / CounterFrequency.QuadPart;
				LastCounter = ThisCounter;

				// char* BigMessage;
				char GlobalMessage[256] = {};
				char GlobalMessage2[256] = {};

				MainGameLoop(&RefToBufferForGameToFill, &BGBuffer, InputThisFrame, 
				             &GlobalScore, GlobalMessage, GlobalMessage2, dt);


				Win32DisplayBufferOnScreenDouble(&Win32Buffer, GlobalScore, GlobalMessage, GlobalMessage2, 
				                                 Window, DeviceContext, OffsDC);
				// Win32DisplayBufferOnScreenSingle(&Win32Buffer, GlobalScore, GlobalMessage, 
				//                                  Window, DeviceContext);



				LARGE_INTEGER LastCounter;
				LARGE_INTEGER ThisCounter;
				QueryPerformanceCounter(&LastCounter);

				Win32DebugPrintFloat(dt);

				// char StringBuffer[256];
				// sprintf(StringBuffer, "%.2f\n", dt);
				// Win32DispString(Window, DeviceContext, StringBuffer, 10, 10);


			} // done running (globalrunning false)

	// not really needed?
	SelectObject(OffsDC,moldbmp);
	DeleteObject(mbmp);
	DeleteDC(OffsDC);

		}


		else
		{
			MessageBox(0, "Window couldn't open.\n\nSorry, I'm new at this...", "^-^", MB_OK);
		}
	}
	else
	{
		MessageBox(0, "Couldn't register class.\n\nThat doesn't seem good.\n\nI probably messed something up...", "^-^", MB_OK);
	}




	return 0;
}





