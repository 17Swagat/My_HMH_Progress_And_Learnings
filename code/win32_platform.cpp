//(1) Day2: Opening a Win32 Window

#include <Windows.h>

//WindowProc for WindowClass
LRESULT CALLBACK MainWindowCallback(HWND Window,
				    UINT Message,
				    WPARAM WParam,
				    LPARAM LParam)
{
	LRESULT Result = 0; 
	
	// @casey "For now we are going not handle the messages. We're just gonna see when functions are called."
	switch(Message)
	{
		case WM_ACTIVATEAPP: 
		{
			OutputDebugStringA("WM_ACTIVATEAPP\n");
			
		} break;

		case WM_CLOSE:
		{
			OutputDebugStringA("WM_CLOSE\n");

		} break;

		case WM_DESTROY:
		{
			OutputDebugStringA("WM_DESTROY\n");

		} break;

		case WM_SIZE:
		{
			OutputDebugStringA("WM_SIZEX\n");

		} break;

		case WM_PAINT:
		{
			PAINTSTRUCT Paint;
			HDC DeviceContext = BeginPaint(Window, &Paint);

			int X = Paint.rcPaint.left;
			int Y = Paint.rcPaint.top;
			int Width = Paint.rcPaint.right - Paint.rcPaint.left;
			int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

			//NOTE: For PatBlt function we imported "gdi32.lib" in our build.bat 


			//###0) On not using PatBlt Function, we will get a broken sort of "black-white screen" on resizing.


			//###1) Will get a white screen
			
			PatBlt(DeviceContext,//HDC hdc 
			       X, 	     //int x, 
			       Y, 	     //int y, 
			       Width, 	     //int w, 
			       Height, 	     //int h, 
			       WHITENESS);   //DWORD rop
			


			//###2) Will get black-white flicker screen
			

			/*static DWORD Operation = WHITENESS;
			PatBlt(DeviceContext, X, Y, Width, Height, Operation);
			if (Operation == WHITENESS)
				Operation = BLACKNESS;
			else 
				Operation = WHITENESS;
			*/

			EndPaint(Window, &Paint);
			
		} break;

		default:
		{
			//OutputDebugStringA("Default\n");
			Result = DefWindowProcA(Window, Message, WParam, LParam);
		} 
	}

	return (Result);
}

//Entry-Point
int CALLBACK WinMain(HINSTANCE Instance, 
		     HINSTANCE PrevInstance, 
		     LPSTR CmdLine, 
		     int ShowCmd)
{
	WNDCLASS WindowClass = {};
	WindowClass.hInstance = Instance;
	WindowClass.lpfnWndProc = MainWindowCallback; //(WindowProc)This programmerDefined Funtion: "Handles all the messages coming from windows"
	WindowClass.lpszClassName = "HandmadeHeroWindowClass";
	//WindowClass.icon;
	WindowClass.style = (CS_HREDRAW | CS_VREDRAW | CS_OWNDC);  //TODO(casey): "Will have check later whether CS_HREDRAW, CS_VREDRAW, CS_OWNDC still matters nowadays." --> ANS: They doesn't matter.
	/*
	 *
	 * CS_CLASSDC -> casey@ "It says I want my own device-context for any window thats created with the this window class. @docs "Allocates one device context to be shared by all windows in the class"
	 * CS_OWNDC   -> casey@ "I want my own Dc for every individual window. This is techinically the one that we will be using. It probably not necessary these days to put this put still I'm putting it for old time sake. What that basically means is that we will not have to get and release DCs for our windows if we don't want to , becoz we will always have our own DC that we can just use. Although CS_CLASSDC would have also done the same thing since we are going to create only one window." 
	 *
	 * */


	if (RegisterClassA(&WindowClass)) 
	{
		HWND WindowHandle = CreateWindowExA(0, //DWORD dwExStyle, 
						    WindowClass.lpszClassName, //LPCSTR lpClassName, 
						    "Handmade Hero", //LPCSTR lpWindowName, 
						    (WS_OVERLAPPEDWINDOW | WS_VISIBLE), //DWORD dwStyle, 
						    CW_USEDEFAULT, //int X, 
						    CW_USEDEFAULT, //int Y, 
						    CW_USEDEFAULT, //int nWidth, 
						    CW_USEDEFAULT, //int nHeight, 
						    0, //HWND hWndParent, 
						    0, //HMENU hMenu, 
						    Instance, //HINSTANCE hInstance, 
						    0);//LPVOID lpParam

		if (WindowHandle != NULL) //@casey: Do not write != NULL. Since NULL === 0. But I like it.
		{
			for(;;) //@casey: "If we don'twrite the infinite-loop, then The program will start and instantly close. We basically need to sit in an infinite loop processing messages until windows actually tells us that we are done." "We are using for(;;) as our infinite loop, rather than while(1) or while(true) becoz the if we set the compiler flag to show us all warnings(/Wall) then it will show us few-warnings since 1 and true will never change inside while loop condition.
			{
				MSG Message;
				BOOL MessageResult = GetMessageA(&Message,//LPMSG lpMsg :-> receives message information from the thread's message queue
								 0, //HWND hWnd :->If hWnd is NULL, GetMessage retrieves messages for any window that belongs to the current thread, and any messages on the current thread's message queue whose hwnd value is NULL (see the MSG structure). Therefore if hWnd is NULL, both window messages and thread messages are processed.  
								 0,  //UINT wMsgFilterMin
								 0); //UINT wMsgFilterMax // ["If wMsgFilterMin and wMsgFilterMax are both zero, GetMessage returns all available messages"]

				if (MessageResult > 0)
				{
					TranslateMessage(&Message);
					DispatchMessageA(&Message); //Each time the program calls the DispatchMessage function, it indirectly causes Windows to invoke the WindowProc function, once for each message.
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
	else //When Registering Fails. @casey"Generally it never happens in practise" 
	{
		//@casey(TODO): Logging
	}

	

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

// (2) Tutorial: (https://docs.microsoft.com/en-us/windows/win32/learnwin32/your-first-windows-program)

