// DAY 4: Animating The Back Buffer
//
//#include <Windows.h>
//#include <stdint.h>
//
//#define global_variable	static
//#define local_persist	static
//#define internal static // :--> @casey"One more meaning of *"static", What is does is basically, it can define a function as being local to the file that its in. So, it can be basically be used to say don't allow this function to be called from any other file other then the one that it is in. And by file I actually mean *"Translation Units" " global_variable bool Running; // NOTE: "static bool Running;" line is === to "bool Running = 0 or false;"
//
//typedef int8_t  int8;
//typedef int16_t int16;
//typedef int32_t int32;
//typedef int64_t int64;
//
//typedef uint8_t  uint8;
//typedef uint16_t uint16;
//typedef uint32_t uint32;
//typedef uint64_t uint64;
//
//global_variable bool 	   Running;
//
//global_variable BITMAPINFO BitmapInfo;
//global_variable void 	  *BitmapMemory;
//global_variable int 	   BitmapWidth;
//global_variable int 	   BitmapHeight;
//
//
//// @casey -> "DIB --> Device Independent Bitmap. That is the name that Windows uses to talk about things that we can write into as Bitmaps that it can then display using gdi."
//
//internal void Win32_ResizeDIBSection(int Width,
//int Height
//)
//{
//
//if (BitmapMemory) //@casey -> "If we should have to resize our DIB section and bitmap memory was already allocated than we want to just make sure that quickly before we do the following things that we have done below, we go ahead and free that memory.
//{
//VirtualFree(BitmapMemory, 0, MEM_RELEASE);
//}
//
//BitmapWidth  = Width;
//BitmapHeight = Height;
//
//BitmapInfo.bmiHeader.biSize        = sizeof(BitmapInfo.bmiHeader);
//BitmapInfo.bmiHeader.biWidth       = BitmapWidth;
//BitmapInfo.bmiHeader.biHeight      = -BitmapHeight; //The height of the bitmap, in pixels. If biHeight is positive, the bitmap is a bottom-up DIB and its origin is the lower-left corner. If biHeight is negative, the bitmap is a top-down DIB and its origin is the upper-left corner. If biHeight is negative, indicating a top-down DIB, biCompression must be either BI_RGB or BI_BITFIELDS. Top-down DIBs cannot be compressed. If biCompression is BI_JPEG or BI_PNG, the biHeight member specifies the height of the decompressed JPEG or PNG image file, respectively.
//
//BitmapInfo.bmiHeader.biPlanes      = 1;
//BitmapInfo.bmiHeader.biBitCount    = 32; //@casey -> "We have asked for 32 bits of memory per pixel. 8-bits for RED, 8-bits for BLUE , 8-bits for GREEN and 8-bits for padding(called Padding-Byte). Reason for the padding byte: In general on the x86 architecture often times there is a penelty for doing whats called unaligned accessing. REFERE FOR MORE INFO.->https://youtu.be/hNKU8Jiza2g?t=334 "
//BitmapInfo.bmiHeader.biCompression = BI_RGB;
//
//int BytesPerPixel    = 4;
//int BitmapMemorySize = (BitmapWidth * BitmapHeight) * BytesPerPixel; // = (Area of the Reactangle) * (No of Bytes Per Pixel)
//BitmapMemory     = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE); // @casey-> " We can allocate memory using "VirtualAlloc()" and "HeapAlloc". We are gonna stick with VirtualAlloc() mostly. Basically what VirtualAlloc() does is that it goes ahead and allocates a certain number of memory-pages and basically what happens is that when we allocate memory directly from the system it is generally allocated in Pages and Pages are of certain size. And in Windows they are often 4096 Bytes, sometimes they are 64KB. It depends whether they are small pages or large pages, sort of a OS thing that we will potentially talk later about it. But basically there are 2 ways we can allocate memory: We could do it via HeapAlloc(). In HeapAlloc(), its asking the system to suballocate out of the pages for you, So we can pass any size you want here and you will get a pointer to that. Whereas "VirtualAlloc()" is little more raw. VirtualAlloc() must give you back Pages. So, you can't for example ask for something that is less than 4096 KB and expected to give it back to you what it actually will do is it will give you back the whole 4096 KB and the rest will just be wasted. I wanna get you used to allocating memory from this becoz we are gonna we doing almost all of our own in this project. So, I just want to get started with this VirtualAlloc() right away so you are used to calling it and it does not frighten you. "
//
//
//int Pitch = Width * BytesPerPixel; //@casey -> "This is how big each individual row is."
//uint8 *Row = (uint8 *)BitmapMemory;
//
//for (int Y = 0;
//(Y < BitmapHeight);
//++Y)
//{
//uint8 *Pixel = (uint8 *)Row;
//for (int X = 0;
//(X < BitmapWidth);
//++X)
//{
////"Writing Each Byte in Memory
//
///*
//* Pixel in memory -->
//PIXEL IN MEMORY: RR GG BB xx
//0x xxBBGGRR //Becoz of "little endian architecture"
//
//In Windows, they did --> Pixel in memory : 0x BBGGRRxx
//*/
//
////Byte 1 :--> BB
//*Pixel = (uint8) X;//255; /* Max value of uint8 is (255) */
//++Pixel;
//
////Byte 2 :--> GG
//*Pixel = (uint8) Y;
//++Pixel;
//
////Byte 3 :--> RR
//*Pixel = 200;
//++Pixel;
//
////Byte 4 :--> XX
//*Pixel = 0;
//++Pixel;
//}
//
//Row += Pitch;
//}
//
//}
//
//internal void Win32_UpdateWindow(HDC   DeviceContext,
//RECT *WindowRect, //Here we are passing a WindowRect, that will tell us basically how big the Window is.
//int   X,
//int   Y,
//int   Width,
//int   Height
//)
//{
////@casey -> "What this function basically does it takes our DIBSection that we created and blits it , but it also allows us to scale it."
////"All this function does -> It is a rectangle to a reactangle copy, i.e Copies one reactangle to the other."
//
///*
//* @casey --> "What we are gonna do here essentially is called something *Dirty Rectangle Update*. We pass Win32_UpdateWindow() function the region of our window that Windows asks us to repaint.
//*
//* But we can introduce/encounter some bugs here and make our stuff hard to debug by actually using the *Dirty Rectangle* first time out.
//* So, instead of doing that we are gonna StretchDIBits() just fill the whole window first, to make sure that everthing is working properly and then we will worry about doing
//* a sub region that Windows wants us to do."*/
//
//// #1 . Applying StretchDIBits() to the whole Window
//int WindowWidth = WindowRect->right - WindowRect->left;
//int WindowHeight = WindowRect->bottom - WindowRect->top;
//
//StretchDIBits(DeviceContext,
//0, 0, BitmapWidth, BitmapHeight,//src
//X, Y, WindowWidth, WindowHeight, //destn
//BitmapMemory,
//&BitmapInfo,
//DIB_RGB_COLORS,
//SRCCOPY);
//}
//
//LRESULT CALLBACK Win32_MainWindowCallback(HWND   Window,
//UINT   Message,
//WPARAM WParam,
//LPARAM LParam
//)
//{
//LRESULT Result = 0;
//
//switch(Message)
//{
//case WM_ACTIVATEAPP:
//{
//OutputDebugStringA("WM_ACTIVATEAPP\n");
//
//} break;
//
//case WM_SIZE:
//{
//RECT ClientRect;
//GetClientRect(Window, &ClientRect); //Docs: [Retrieves the coordinates of a window's client area. The client coordinates specify the upper-left and lower-right corners of the client area. Because client coordinates are relative to the upper-left corner of a window's client area, the coordinates of the upper-left corner are (0,0).] The "left" and "top" members are 0.The right and bottom members contain the width and height of the window.). "IN MY OPINION, to be honest there is no need to specify ClientRect.left and ClientRect.top in Width and Height."
//int Width  = ClientRect.right - ClientRect.left;
//int Height = ClientRect.bottom - ClientRect.top;
//
//Win32_ResizeDIBSection(Width, Height);
//
//OutputDebugStringA("WM_SIZEX\n");
//
//} break;
//
////@casey -> "WM_PAINT is one of the place where we will paint. But its not just the only region where we will paint."
//case WM_PAINT:
//{
//PAINTSTRUCT Paint;
//HDC DeviceContext = BeginPaint(Window, &Paint);
//
//int X      = Paint.rcPaint.left;
//int Y      = Paint.rcPaint.top;
//int Width  = Paint.rcPaint.right - Paint.rcPaint.left;
//int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
//
////**I would have prefered WindowRect, but @casey used ClientRect.
//RECT ClientRect;
//GetClientRect(Window, &ClientRect);
//Win32_UpdateWindow(DeviceContext,//HDC DeviceContext,
//&ClientRect,//RECT *WindowRect,
//X, //int X,
//Y, //int Y,
//Width, //int Width,
//Height); //int Height)
//
//EndPaint(Window, &Paint);
//
//} break;
//
//
//case WM_CLOSE:
//{
//Running = false;
//OutputDebugStringA("WM_CLOSE\n");
//
//} break;
//
//case WM_DESTROY:
//{
//Running = false;
//OutputDebugStringA("WM_DESTROY\n");
//
//} break;
//
//default:
//{
////OutputDebugStringA("Default\n");
//Result = DefWindowProcA(Window, Message, WParam, LParam);
//}
//}
//
//return (Result);
//}
//
////Win32 Entry Point
//int CALLBACK WinMain(HINSTANCE Instance,
//HINSTANCE PrevInstance,
//LPSTR     CmdLine,
//int       ShowCmd
//)
//{
//WNDCLASSA WindowClass               = {};
//WindowClass.hInstance     = Instance;
//WindowClass.lpfnWndProc   = Win32_MainWindowCallback;
//WindowClass.lpszClassName = "HandmadeHeroWindowClass";
//WindowClass.style         = (CS_HREDRAW | CS_VREDRAW | CS_OWNDC);
//
//if (RegisterClassA(&WindowClass))
//{
//HWND WindowHandle = CreateWindowExA(0,
//WindowClass.lpszClassName,
//"Handmade Hero",
//(WS_OVERLAPPEDWINDOW | WS_VISIBLE),
//CW_USEDEFAULT,
//CW_USEDEFAULT,
//CW_USEDEFAULT,
//CW_USEDEFAULT,
//0,
//0,
//Instance,
//0);
//
//if (WindowHandle != NULL)
//{
//Running = true;
//while (Running)
//{
//MSG Message;
//BOOL MessageResult = GetMessageA(&Message,
//0,
//0,
//0);
//if (MessageResult > 0)
//{
//TranslateMessage(&Message);
//DispatchMessageA(&Message);
//}
//else
//{
//break;
//}
//}
//}
//else
//{
////@casey(TODO): Logging
//}
//}
//else
//{
////@casey(TODO): Logging
//}
//
//return 0;
//}
//

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
