// Day5: Windows Graphics Review

#include <Windows.h>
#include <stdint.h>

#define internal_func	static
#define global_variable static
#define local_variable	static

//signed
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
//unsigned
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

struct Win32_OffScreenBuffer
{
	//NOTE: @casey:-> "For now, removed (BytesPerPixel) since Pixels are always set/considered here to be of 4 Bytes, i.e 32 bits. i.e:-> Memory Order: (BB GG RR XX)"
	
	BITMAPINFO Info;
	void* Memory;
	int Width;
	int Height;
	int Pitch;
};


//Global variables
global_variable bool GlobalRunning;
global_variable Win32_OffScreenBuffer GlobalBackBuffer;


struct Win32_WindowDimensions
{
	int Width;
	int Height;
};


Win32_WindowDimensions Win32_GetWindowDimensions(HWND Window)
{
	Win32_WindowDimensions Result;

	RECT ClientRect;
	GetClientRect(Window, &ClientRect);
	Result.Width = ClientRect.right - ClientRect.left;
	Result.Height = ClientRect.bottom - ClientRect.top;
	
	return Result;
}


internal_func 
void RenderWeirdGradient(Win32_OffScreenBuffer Buffer, 
						 int BlueOffset, 
						 int GreenOffset)
{
	//TODO: Let's see what the optimizer does	
	uint8* Row = (uint8*) Buffer.Memory;

	for(int Y = 0; 
		Y < Buffer.Height; 
		Y++)
	{
		uint32* Pixel = (uint32* )Row; 
		
		for(int X = 0; 
			X < Buffer.Width; 
			X++)
		{
			//Blue
			uint8 Blue = (X + BlueOffset);

			//Green
			uint8 Green = (Y + GreenOffset);
			
			//Red
			uint8 Red = 0; 
			
			//Padding
			//uint8 Padding = 0;
			
			*Pixel++ = (Red << 16) | (Green << 8) | (Blue); 
		}

		Row += Buffer.Pitch; // I setted "Buffer.Pitch" value in Win32_ResizeDIBSection(), where I was setting up all the important parameters releated to the bitmap.
	}
}


internal_func 
void Win32_ResizeDIBSection(Win32_OffScreenBuffer *Buffer,
							int Width, 
							int Height)
{
	if (Buffer->Memory)
	{
		VirtualFree(Buffer->Memory, 
					0, 
					MEM_RELEASE);
	}
	
	Buffer->Width = Width;
	Buffer->Height = Height;
	Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
	Buffer->Info.bmiHeader.biWidth = Buffer->Width;
	Buffer->Info.bmiHeader.biHeight = -Buffer->Height; 
	Buffer->Info.bmiHeader.biPlanes = 1;
	Buffer->Info.bmiHeader.biBitCount = 32; 
	Buffer->Info.bmiHeader.biCompression = BI_RGB;

	int BytesPerPixel = 4;
	int BitmapMemorySize = ((Buffer->Width) * (Buffer->Height)) * BytesPerPixel; //size in (Bytes)

	Buffer->Memory = VirtualAlloc(0, 
								  BitmapMemorySize, 
								  MEM_COMMIT, 
								  PAGE_READWRITE);
	
	Buffer->Pitch = (Buffer->Width) * (BytesPerPixel);
}


internal_func 
void Win32_DisplayBufferInWindow(HDC DeviceContext,
								 int WindowWidth,
								 int WindowHeight,
								 Win32_OffScreenBuffer Buffer)
{
	//TODO(@casey):-> "Aspect ratio correction"
	//TODO(@casey):-> "Play with stretch modes"
	StretchDIBits(DeviceContext,
				  0, 0, WindowWidth, WindowHeight,	 // destn
				  0, 0, Buffer.Width, Buffer.Height, // src
				  Buffer.Memory,
				  &Buffer.Info,
				  DIB_RGB_COLORS,
				  SRCCOPY);
}


LRESULT Win32_MainWindowCallback(HWND Window,
								 UINT Message,
								 WPARAM WParam,
								 LPARAM LParam)
{
	LRESULT Result = 0;

	switch (Message)
	{
		case WM_ACTIVATEAPP:
			{
				OutputDebugStringA("WM_ACTIVATEAPP\n");
			}
			break;

		/*
		case WM_SIZE\:
			{
				OutputDebugStringA("WM_SIZE\n");

				Win32_WindowDimensions Dimensions = Win32_GetWindowDimensions(Window);
				Win32_ResizeDIBSection(&GlobalBackBuffer,
									   Dimensions.Width, 
									   Dimensions.Height);
			}
			break;
		*/


		case WM_PAINT:
			{
				OutputDebugStringA("WM_PAINT\n");

				PAINTSTRUCT Paint;
				HDC DeviceContext = BeginPaint(Window, &Paint); //An application should not call BeginPaint except in response to a WM_PAINT message. Same goes for EndPaint().

				int X = Paint.rcPaint.left;
				int Y = Paint.rcPaint.top;
				int Width = Paint.rcPaint.right - Paint.rcPaint.left;
				int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

				Win32_WindowDimensions Dimensions = Win32_GetWindowDimensions(Window);

				Win32_DisplayBufferInWindow(DeviceContext,
											Dimensions.Width,
											Dimensions.Height,
											GlobalBackBuffer);

				EndPaint(Window, &Paint);
			}
			break;

		case WM_CLOSE:
			{
				GlobalRunning = false;
			}
			break;

		case WM_DESTROY:
			{
				GlobalRunning = false;
			}
			break;

		default:
			{
				Result = DefWindowProcA(Window, 
										Message, 
										WParam, 
										LParam);
			}
	}

	return Result;
}


int CALLBACK WinMain(HINSTANCE Instance,
					 HINSTANCE PrevInstance,
					 LPSTR CmdLine,
					 int ShowCmd)
{
	WNDCLASSA WindowClass = {};

	Win32_ResizeDIBSection(&GlobalBackBuffer,
						   1280,
						   720);
						   

	WindowClass.lpszClassName = "HandmadeHeroWindowClass";
	WindowClass.hInstance = Instance;
	WindowClass.lpfnWndProc = Win32_MainWindowCallback;
	WindowClass.style = (CS_OWNDC | CS_HREDRAW | CS_VREDRAW);

	if (RegisterClassA(&WindowClass))
	{
		HWND Window = CreateWindowExA(0,
						              WindowClass.lpszClassName,
						              "Handmade Hero",
						              (WS_VISIBLE | WS_OVERLAPPEDWINDOW),
						              CW_USEDEFAULT,
						              CW_USEDEFAULT,
						              CW_USEDEFAULT,
						              CW_USEDEFAULT,
						              0,
						              0,
						              Instance,
									  0);
		if (Window)
		{
			int BlueOffset = 0;
			int GreenOffset = 0;
			
			GlobalRunning = true;
			while (GlobalRunning)
			{
				//Message Loop
				MSG Message;
				while (PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
				{
					if (Message.message == WM_QUIT)
					{
						GlobalRunning = false;
					}
					
					TranslateMessage(&Message);
					DispatchMessageA(&Message);
				}

				//Rendered to the bitmap through this function
				RenderWeirdGradient(GlobalBackBuffer,
									BlueOffset, 
									GreenOffset); 
				++(BlueOffset);
				++(GreenOffset);

				//Blitting on the screen from the bitmap
				HDC DeviceContext = GetDC(Window);
				
				Win32_WindowDimensions Dimensions = Win32_GetWindowDimensions(Window);
				Win32_DisplayBufferInWindow(DeviceContext, 
											Dimensions.Width, 
											Dimensions.Height, 
											GlobalBackBuffer);
				
				ReleaseDC(Window, DeviceContext);
			}
		}
		else //TODO
		{
			OutputDebugStringA("Window Creation Failed\n");
		}
	}
	else //TODO
	{
		OutputDebugStringA("RegisterClassA() failed\n");
	}

	return 0;
}


