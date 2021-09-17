// Day3: Allocating A Back Buffer
/*
IMPORTANT NOTE: For (Day 3) watch the stream. It will help more than the code.
*/

#include <Windows.h>

#define global_variable	static 
#define local_persist	static
#define internal		static// :--> @casey"One more meaning of *"static", What is does is basically, it can define a function as being local to the file that its in. So, it can be basically be used to say don't allow this function to be called from any other file other then the one that it is in. And by file I actually mean *"Translation Units" " global_variable bool Running; // NOTE: "static bool Running;" line is === to "bool Running = 0 or false;" 

//@casey -> "For now this is a global. They will not last long. We will make them local soon. Its just for testing purpose."
global_variable bool 		Running; //The following line is === to "bool Running = 0 or false;"
global_variable BITMAPINFO 	BitmapInfo;
global_variable void 		*BitmapMemory; //void *BitmapMemory; //@casey-> "BIG MOMENT: This is the actual bitmap memory that we are going receive back from Windows that we can finally draw into with our own renderer."
global_variable HBITMAP 	BitmapHandle;
global_variable HDC 		BitmapDeviceContext;


// @casey -> "DIB --> Device Independent Bitmap. That is the name that Windows uses to talk about things that we can write into as Bitmaps that it can then display using gdi."
internal void Win32_ResizeDIBSection(int Width, int Height) 
{
	//@casey -> "We will basically have to build the buffer using Windows to give us a buffer that we can draw into." "This is the part where we actually create the *BACK-BUFFER*, to put it to the window."

	//@casey (TODO): Bulletproof this :-> 
	//@casey -> Maybe don't free first, then free after, then free if that fails.

	//@casey (TODO): Free our DIBSection :->
	if (BitmapHandle) //i.e @casey-> "If we already have our BitmapHandle initialzed then delete/free "
	{
		DeleteObject(BitmapHandle);
	}
	if (!BitmapDeviceContext) //i.e @casey-> "If we don't have a BitmapDeviceContext"
	{
		//@casey (TODO): Should we recreate these under certain special conditions.
		BitmapDeviceContext = CreateCompatibleDC(0);
	}

	BitmapInfo.bmiHeader.biSize 		= sizeof(BitmapInfo.bmiHeader);
	BitmapInfo.bmiHeader.biWidth 		= Width;
	BitmapInfo.bmiHeader.biHeight 		= Height;
	BitmapInfo.bmiHeader.biPlanes 		= 1;
	BitmapInfo.bmiHeader.biBitCount 	= 32;
	BitmapInfo.bmiHeader.biCompression 	= BI_RGB;

	BitmapHandle =  CreateDIBSection(BitmapDeviceContext, //HDC hdc, 
									 &BitmapInfo,		  //const BITMAPINFO *pbmi, 
									 DIB_RGB_COLORS,	  //UINT usage, 
									 &BitmapMemory,		  //void **ppvBits, 
									 NULL,				  //HANDLE hSection, 
									 NULL);				  //DWORD offset)

}

internal void Win32_UpdateWindow(HDC DeviceContext, 
								 int X, 
								 int Y, 
								 int Width, 
								 int Height)
{
	//@casey -> "What this function basically does it takes our DIBSection that we created and blits it , but it also allows us to scale it."
	//"All this function does -> It is a rectangle to a reactangle copy, i.e Copies one reactangle to the other."
	StretchDIBits(DeviceContext,
				  X, Y, Width, Height,
				  X, Y, Width, Height,
				  BitmapMemory,
				  &BitmapInfo,
				  DIB_RGB_COLORS, 
				  SRCCOPY);
}

LRESULT CALLBACK Win32_MainWindowCallback(HWND   Window,
										  UINT 	Message,
										  WPARAM WParam,
										  LPARAM LParam)
{
	LRESULT Result = 0;

	switch(Message)
	{
		case WM_ACTIVATEAPP: 
				{
					OutputDebugStringA("WM_ACTIVATEAPP\n");
					
				} break;

		case WM_SIZE:
				{
					RECT ClientRect;
					GetClientRect(Window, &ClientRect); //Docs: [Retrieves the coordinates of a window's client area. The client coordinates specify the upper-left and lower-right corners of the client area. Because client coordinates are relative to the upper-left corner of a window's client area, the coordinates of the upper-left corner are (0,0).] The "left" and "top" members are 0.The right and bottom members contain the width and height of the window.). "IN MY OPINION, to be honest there is no need to specify ClientRect.left and ClientRect.top in Width and Height."
					int Width  = ClientRect.right - ClientRect.left; 
					int Height = ClientRect.bottom - ClientRect.top;

					Win32_ResizeDIBSection(Width, Height);

					OutputDebugStringA("WM_SIZEX\n");

				} break;


		//@casey -> "WM_PAINT is one of the place where we will paint. But its not just the only region where we will paint."
		case WM_PAINT:
			{
				PAINTSTRUCT Paint;
				HDC DeviceContext = BeginPaint(Window, &Paint);

				int X 		= Paint.rcPaint.left;
				int Y 		= Paint.rcPaint.top;
				int Width 	= Paint.rcPaint.right - Paint.rcPaint.left;
				int Height 	= Paint.rcPaint.bottom - Paint.rcPaint.top;

				Win32_UpdateWindow(DeviceContext, 
								   X, 
								   Y, 
								   Width, 
								   Height);

				EndPaint(Window, &Paint);
				
			} break;

		case WM_CLOSE:
			{
				// ##1
				//PostQuitMessage(0);
				
				// ##2 : Better then ##1
				//PostQuitMessage(0);
				
				// ##3
				//DestroyWindow(Window); //"It just destroyes the Window, not closes the application.Therefore we used PostQuitMessage(0) inside WM_DESTROY"

				// ##4 (@casey "Recommanded in HMH)
				// @casey: TODO: "Handle this with a message to the user?"
				Running = false;

				OutputDebugStringA("WM_CLOSE\n");

			} break;

		case WM_DESTROY:
			{
				// ##1
				// //Nothing is written.
				
				// ##2 (Better then ##1 & connected to the #2 of WM_CLOSE)
				//PostQuitMessage(0);
				
				// ##3 (connected to the #3 of WM_CLOSE)
				//PostQuitMessage(0);

				// ##4 (@casey "Recommanded in HMH)
				// @casey: TODO: "Handle this as an error - recreate window?"
				Running = false;

				OutputDebugStringA("WM_DESTROY\n");

			} break;

		default:
			{
				//OutputDebugStringA("Default\n");
				Result = DefWindowProcA(Window, Message, WParam, LParam);
			} 
	}

	return (Result);
}

//Win32 Entry Point
int CALLBACK WinMain(HINSTANCE Instance, 
					 HINSTANCE PrevInstance, 
					 LPSTR CmdLine, 
					 int ShowCmd)
{
	WNDCLASSA WindowClass 		= {};
	WindowClass.hInstance 		= Instance;
	WindowClass.lpfnWndProc 	= Win32_MainWindowCallback; 
	WindowClass.lpszClassName 	= "HandmadeHeroWindowClass";
	WindowClass.style 			= (CS_HREDRAW | CS_VREDRAW | CS_OWNDC);  

	if (RegisterClassA(&WindowClass)) 
	{
		HWND WindowHandle = CreateWindowExA(0, 
						    				WindowClass.lpszClassName, 
						    				"Handmade Hero", 		   
						    				(WS_OVERLAPPEDWINDOW | WS_VISIBLE), 
						    				CW_USEDEFAULT, 
						    				CW_USEDEFAULT, 
						    				CW_USEDEFAULT,
						    				CW_USEDEFAULT,
						    				0, 
						    				0,
						    				Instance, 
						    				0);

		if (WindowHandle != NULL) 
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
		else 
		{
			//@casey(TODO): Logging
		}
	}
	else 
	{
		//@casey(TODO): Logging
	}

	return 0;
}

