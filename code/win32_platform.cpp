// Day4: Animating The Backbuffer

#include <Windows.h>
#include <stdint.h>

#define internal		static
#define global_variable static
#define local_variable	static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

global_variable bool		Running;
global_variable BITMAPINFO	BitmapInfo;
global_variable void*		BitmapMemory;// ** imp **
global_variable int BytesPerPixel = 4;//in Bytes, i.e 32 bits

global_variable int BitmapWidth;
global_variable int BitmapHeight;

internal void RenderWeirdGradient(int BlueOffset, 
								  int GreenOffset)
{
	
	int Width = BitmapWidth;
	int Height = BitmapHeight;
	
	int Pitch = Width * BytesPerPixel;
	uint8* Row = (uint8*) BitmapMemory;

	for(int Y = 0; Y < BitmapHeight; Y++)
	{
		//** Writing 32 bits is faster than writing 8 bits. Although the compiler can do that stuff for us.**
		uint32* Pixel = (uint32 *)Row; //uint8* Pixel = Row;
		
		for(int X = 0; X < BitmapWidth; X++)
		{
			/*
			 * NOTE:  XX === Padding
			 *
			 * Memory   : BB GG RR XX
			 * Register : XX RR BB GG
			 *
			*/
			
			//Blue
			uint8 Blue = (X + BlueOffset);
			
			//Green
			uint8 Green = (Y + GreenOffset);
			
			//Red
			uint8 Red = 0;
			
			//Padding
			//uint8 Padding = 0;
			
			//Pixel Representation:-> "Performing the bitwise operations as follows b/c of how they will be stored in the register."
			*Pixel++ = (Red) | (Green << 8) | (Blue); // Equivlent to :*Pixel++ = (Blue) | (Red) | (Green << 8);  SAME THING
		}
		
		Row += Pitch;
	}
}

internal void Win32_ResizeDIBSection(int Width, 
									 int Height)
{
	//TODO: (@casey):-> Bulletproof this (later)
	//Maybe don't free first, free after, then free first if that fails. 

	if (BitmapMemory)
	{
		VirtualFree(BitmapMemory, 
					0, 
					MEM_RELEASE);
	}
	
	BitmapWidth = Width;
	BitmapHeight = Height;

	BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
	BitmapInfo.bmiHeader.biWidth = BitmapWidth;
	BitmapInfo.bmiHeader.biHeight = -BitmapHeight; 
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 32; 
	BitmapInfo.bmiHeader.biCompression = BI_RGB;

	/* 
	 * WE CAN DO THE MEMORY ALLOCATION OURSELVES HOWEVER WE WANT TO 
	 * AS LONG AS IT IS OF THE RIGHT SIZE AND WORKS OK, WINDOWS WILL
	 * GO AHEAD AND DO THE RIGHT THING.
	 *
	 */

	int BitmapMemorySize = (BitmapWidth * BitmapHeight) * BytesPerPixel; //size in (Bytes)
	BitmapMemory = VirtualAlloc(0, 
								BitmapMemorySize, 
								MEM_COMMIT, 
								PAGE_READWRITE);

	// #1	
	//Drawing Function :->
	//RenderWeirdGradient(0, 0); //reproducing the previous drawing.
	
	// #2
	// TODO(casey): Probably clear this to black 
}

internal void Win32_UpdateDIBSection(HDC DeviceContext,
									 RECT *ClientRect,
									 int X, 
									 int Y, 
									 int Width, 
									 int Height)
{
	int WindowWidth = ClientRect->right - ClientRect->left;
	int WindowHeight = ClientRect->bottom - ClientRect->top;
	StretchDIBits(DeviceContext,
				  /*X, Y, Width, Height, //dest
				  X, Y, Width, Height, //src */
				  0, 0, BitmapWidth, BitmapHeight,
				  0, 0, WindowWidth, WindowHeight,
				  BitmapMemory,
				  &BitmapInfo,
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

		case WM_SIZE:
			{
				OutputDebugStringA("WM_SIZE\n");

				RECT ClientRect;
				GetClientRect(Window, &ClientRect); 

				int Width = ClientRect.right - ClientRect.left;
				int Height = ClientRect.bottom - ClientRect.top;

				Win32_ResizeDIBSection(Width, Height);

			}
			break;

			//TODO
		case WM_PAINT:
			{
				OutputDebugStringA("WM_PAINT\n");

				PAINTSTRUCT Paint;
				HDC DeviceContext = BeginPaint(Window, &Paint);

				int X = Paint.rcPaint.left;
				int Y = Paint.rcPaint.top;
				int Width = Paint.rcPaint.right - Paint.rcPaint.left;
				int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

				//temporary code:->
				RECT ClientRect;
				GetClientRect(Window, &ClientRect);
				//

				Win32_UpdateDIBSection(DeviceContext,
									   &ClientRect,
									   X, 
									   Y, 
									   Width, 
									   Height);

				EndPaint(Window, &Paint);

			}
			break;

		/*
		 * "Leaving the Following for later "
		 * "Since there are some things to do."
		 * WILL DO IT LATER.
		 
		 case WM_SETCURSOR: //Staring-Time: (1:22:06)
			{
				SetCursor(0); //put it, in order to remove the cursor and the loading symobol from the top of our Window
			}
			break;
		*/

		case WM_CLOSE:
			{
				Running = false;
			}
			break;

		case WM_DESTROY:
			{
				Running = false;
			}
			break;

		default:
			{
				Result = DefWindowProcA(Window, Message, WParam, LParam);
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
			
			Running = true;
			while (Running)
			{
				/*
				 * PROBLEMS WITH "GetMessageA(.....) function", is that 
				 * It will block. So, once we run of out Messages we will
				 * block right there. Windows will shut us down and it will
				 * wait for another message to come our queue and will do 
				 * other things that it needs to do, like switching to another 
				 * process etc , use that cpu time effectively.
				 * BUT WE DON'T WANT THAT. We want to use that cpu time to ourselves.
				 * 
				 * So, we will use "PeekMessageA(.....) function".
				 * 
				 * PeekMessageA(....) does the same thing as GetMessageA(....). BUT 
				 * when there is no message, it will allow us to keep on running, instead
				 * of blocking.
				 */

				//Message Loop
				MSG Message;
				while (PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
				{
					if (Message.message == WM_QUIT)
					{
						Running = false;
					}
					
					TranslateMessage(&Message);
					DispatchMessageA(&Message);
				}

				//Rendered to the bitmap through this function
				RenderWeirdGradient(BlueOffset, GreenOffset); //BlueOffset & GreenOffset are initialized in the outer loop of the Running Loop.
				
				//Blitting on the screen from the bitmap
				HDC DeviceContext = GetDC(Window);
				RECT ClientRect;
				GetClientRect(Window, &ClientRect);
				int WindowWidth = ClientRect.right - ClientRect.left;
				int WindowHeight = ClientRect.bottom - ClientRect.top;
				Win32_UpdateDIBSection(DeviceContext, 
									   &ClientRect, 
									   0, 
									   0, 
									   WindowWidth, 
									   WindowHeight);
				ReleaseDC(Window, DeviceContext);

				++(BlueOffset);
				//++(GreenOffset);
			}
		}
		else //TODO
		{

		}
	}
	else //TODO
	{

	}

	return 0;
}


