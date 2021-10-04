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

global_variable int BitmapWidth;
global_variable int BitmapHeight;

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
	BitmapInfo.bmiHeader.biHeight = -BitmapHeight; /* If biHeight is positive, the bitmap is a bottom-up DIB and its origin is the lower-left corner. 
												      If biHeight is negative, the bitmap is a top-down DIB and its origin is the upper-left corner.
												      "Thats the reason why We put a -ve sign before BitmapHeight."
												   */
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 32; /*  We have asked for 32 bits per Pixel, of which 8 bits of padding. 
											  B/c In general on the x86 architecture often times there is a 
											  penalty for doing whats called unaligned accessing. So, if you are 
											  trying to operate on a value that say 8 bits, 16 bits, 32 bits, 64 bits,
											  if you are operating on say 32 bits values, then they should be aligned on 
											  32 bits boundaries. So, here to simply things and to not get a unnecessary
											  penalty We want to make sure that we are always touching a pixel on an exact
											  4 Byte boundary. So, if we were to ask for the bits that we actally need for RBG,
											  that would be 8 bits, 8bits, 8bits or 24 bits which is 3 Bytes, which is in no way aligned with 4.
											  So, the extra 8 bits (1 Byte) that we use will be just for padding. They mean nothing else and won't 
											  be used but we are just gonna do that so that we know that our pixels are always aligned on 4 Byte boundary.
										   */
	BitmapInfo.bmiHeader.biCompression = BI_RGB;

	/* 
	 * WE CAN DO THE MEMORY ALLOCATION OURSELVES HOWEVER WE WANT TO 
	 * AS LONG AS IT IS OF THE RIGHT SIZE AND WORKS OK, WINDOWS WILL
	 * GO AHEAD AND DO THE RIGHT THING.
	 *
	 */

	int BytesPerPixel = 4;//in Bytes, i.e 32 bits
	int BitmapMemorySize = (BitmapWidth * BitmapHeight) * BytesPerPixel; //size in (Bytes)
	BitmapMemory = VirtualAlloc(0, 
								BitmapMemorySize, 
								MEM_COMMIT, 
								PAGE_READWRITE);
	
	/*Drawing Pixels to the bitmap
	  Will draw (row by row) ; (top to bottom)
	  //(https://discord.com/channels/239737791225790464/328835318180478979/894230152270213130)
	*/
	int Pitch = Width * BytesPerPixel; /* Pitch = No. of Bytes that one needs to add to the address 
												  in the first pixel of a row in order to go to the 
												  address of the 1st pixel of the next row. It depends
												  on how an image is represented in memory & is measured
												  in Bytes.
										*/
	uint8* Row = (uint8*) BitmapMemory;

	for(int Y = 0; Y < BitmapHeight; Y++)//here we are going row by row. 
	{
		uint8* Pixel = Row;//(uint8* )Row; A/c to me since its "Row" has been already typecasted to (uint8*)
		
		for(int X = 0; X < BitmapWidth; X++)//for each individual row we are going to go ahead and go over each pixel.	
		{
			/*
			 * "The reason why here the first byte of the Pixel is corresponding to BLUE" :-->
			 *
			 * Due to "Little ENDIAN Architecture", When bytes are loads out of memory to form a big bigger
			 * value, the processor will actually put the first byte in the lowest part of those 32 bits, the 
			 * second Byte will go in the next part, the next to the next Byte and higest will be the fourth byte
			 * over. So, basically what happens is that 
			 * So, if these are pixels in memeory are like this
												
				Pixel in memory(in hexadecimal) :  RR GG BB XX ; where XX :-> referring to the padding 
				
				When we are to load them into uint32, you would load them as follows in the register 
				
				0x (XX BB GG RR) ==> 32 bit value. {Since,  0x 00 00 00 00 ; (Since in hexadecimal, 00 === 8 bits === 1 Byte) }

				Now, well the first people, who wrote windows didn't like this, so when they loaded a 32 bits value and thought
				of it as a pixel, they wanted it to look like it was in the right order, when they are looking at the value in 
				the register of the cpu.

				So, what they did was they defined the memory order to be backwards, i.e :-> 0x (XX RR GG BB). So, it get gets 
				swapped in the memory as follows :-> 

				Pixel in memory(in hexadecimal) :  BB GG RR XX ; where XX :-> referring to the padding 
			*/
		
			//writing each byte in memory individually, i.e writing R, G, B individually

			//Blue
			*Pixel = (uint8 )Y;//(uint8 )X; //just taking the value of the lower bits of X. Trunkated or we can say chopped off.
			++Pixel;

			//Green
			*Pixel = 200;//(uint8 )Y;
			++Pixel;

			//Red
			*Pixel = (uint8 )X;
			++Pixel;

			//Padding
			*Pixel = 0;
			++Pixel;
		}
		
		/*
		 * moving to the next row,{since (Row) is still pointing to the first Byte of BitmapMemory 
		 * and that also means that it is pointing to the first pixel-byte of Row 1.
		 * and on adding (Pitch) to it, the (Row) pointer points to the first Byte of the next Row 
		 * and the process keeps on repeating.}
		 * Row += Pitch;
		 */
		Row += Pitch;
	}
}

internal void Win32_UpdateDIBSection(HDC DeviceContext,
									 RECT *WindowRect,
									 int X, 
									 int Y, 
									 int Width, 
									 int Height)
{
	/* StretchDIBits() used to be slower in comparison to BitBlt(), b/c BitBlt(), since Windows could do 
	 * the allocation of memory itself and had the bitmap already selected It was a faster path. 
	 * But since we starting from making our own renderer (later will learn to use opengl) and performance
	 * difference may not exist anymore between using StretchDIBits() and BitBlt(). Performace difference
	 * used to there way back when things were not going through a 3D card under the hood and all sorts of things,
	 * there is probably no reason to use BitBlt(). StretchDIBits() is going to be good for now. 
	 * "B/c eventually what we will do when we optimize things we will want to create an opengl context 
	 * and actually write directly to a texture with overlap downloads and all these sorts of stuff.
	 * (WILL DICUSS LATER IN THE SERIES) "*/
	
	int WindowWidth = WindowRect->right - WindowRect->left;
	int WindowHeight = WindowRect->bottom - WindowRect->top;
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


