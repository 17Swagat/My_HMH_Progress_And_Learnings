//Day 3 stuff :-> Allocating a back buffer
//(VERSION 2)

#include <Windows.h>

#define internal static
#define global_variable static
#define local_variable static

global_variable bool Running;
global_variable BITMAPINFO BitmapInfo;
global_variable void* BitmapMemory;// ** imp **
global_variable HBITMAP BitmapHandle; 
global_variable HDC BitmapDeviceContext; 

internal void Win32_ResizeDIBSection(int Width, 
									 int Height)

{
	//TODO: (@casey):-> Bulletproof this (later)
	//Maybe don't free first, free after, then free first if that fails. 
	//(WILL LOOK INTO THIS MATTER LATER IN TH SERIES)
	
	//TODO: Free our DIBSection
	if(BitmapHandle)
	{
		DeleteObject(BitmapHandle);
	}

	if (!BitmapDeviceContext)
	{
		//TODO: @casey:->"Should we recreate these under certain special circumstances or not."
		BitmapDeviceContext = CreateCompatibleDC(0);
	}

	BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
	BitmapInfo.bmiHeader.biWidth = Width;
	BitmapInfo.bmiHeader.biHeight = Height;
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 32; /*
										   bits per pixel. //We use 8 bits for Red, 8 bits for Green, 8 bits for Blue, and EXTRA 8 bits for spacing; B/C (RGB) requires 
										   total bits of 24 bits, which is not DWORD Aligned. While 32 bit is DWORD Aligned.
										   
										   "A DWORD, which is short for "double word," is a DATA-TYPE definition that is specific to Microsoft Windows. When defined in 
										   the file windows.h, a dword is an unsigned, 32-bit unit of data. It can contain an integer value in the range 0 through 4,
										   294,967,295"
										   */
	BitmapInfo.bmiHeader.biCompression = BI_RGB;
	/*
	 * THESE PARAMETERS ARE AUTOMATICALLY SET TO 0 : Since (BITMAPINFO BitmapInfo) is now a global static variable.Which is automatically set to 0 if nothing mentioned.
	BitmapInfo.bmiHeader.biSizeImage = 0; //since the above parameter is "BI_RGB"
	BitmapInfo.bmiHeader.biClrUsed = 0; 
	BitmapInfo.bmiHeader.biXPelsPerMeter = 0; //completely useless parameters for us (@casey is also not sure whether we can set them to 0 or not.But for now its 0)
	BitmapInfo.bmiHeader.biYPelsPerMeter = 0; //completely useless parameters for us (@casey is also not sure whether we can set them to 0 or not.But for now its 0)
	BitmapInfo.bmiHeader.biClrUsed = 0; //since we don't use the color table (Refer:-> The Docs for more info. Its useful)
	BitmapInfo.bmiHeader.biClrImportant = 0; //Refer Docs for info. Not imp to us, since we don't use any of the (index-palette) stuff.
	*/
	

	BitmapHandle = CreateDIBSection(BitmapDeviceContext,//With this device-context, the function is going to create a Bitmap that is suitable for use with this DeviceContext.
									&BitmapInfo,
									DIB_RGB_COLORS,
									&BitmapMemory, //this will receive back from Windows 
									0, //@casey -> "Never used this parameter"
									0); //this parameter is relatited to the above one
}

internal void Win32_UpdateDIBSection(HDC DeviceContext,
									 int X, 
									 int Y, 
									 int Width, 
									 int Height)

{
	StretchDIBits(DeviceContext,
				  X, Y, Width, Height, //dest
				  X, Y, Width, Height, //src :--> "since our Backbuffer will be of the same size as of our Window(client)"
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
			GetClientRect(Window, &ClientRect); //**

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

			Win32_UpdateDIBSection(DeviceContext, X, Y, Width, Height);

			EndPaint(Window, &Paint);

		}
		break;

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
		HWND WindowHandle = CreateWindowExA(0,
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
		if (WindowHandle)
		{
			Running = true;
			while (Running)
			{
				MSG Message;
				BOOL MessageResult = GetMessageA(&Message,
												 0,
												 0,
												 0);
				if (MessageResult > 0)
				{
					TranslateMessage(&Message);
					DispatchMessageA(&Message);
				}
				else
				{
					break;
				}
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

