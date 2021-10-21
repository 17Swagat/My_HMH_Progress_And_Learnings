// Day7: Initializing DirectSound 

#include <Windows.h>
#include <stdint.h>
#include <Xinput.h>
#include <dsound.h>

#define internal_func	static
#define global_variable static
#define local_variable	static

//signed
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef int32 bool32;
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


//XInputGetState and XInputSetState settings
//**************************************************************************************************

/*
 * XInputGetState stuff
 */
#define  X_INPUT_GET_STATE(func_name)				\
	DWORD WINAPI func_name(DWORD dwUserIndex,		\
						   XINPUT_STATE* pVibration)
typedef X_INPUT_GET_STATE(x_input_get_state); 
X_INPUT_GET_STATE(XInputGetStateStub) 
{
	return ERROR_DEVICE_NOT_CONNECTED;
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub; 
#define XInputGetState XInputGetState_

/*
 * XInputSetState stuff
 */

#define	 X_INPUT_SET_STATE(func_name)					\
	DWORD WINAPI func_name(DWORD dwUserIndex,			\
						   XINPUT_VIBRATION* pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
	return ERROR_DEVICE_NOT_CONNECTED;
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_


internal_func
void Win32_LoadXInput()
{
	HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
	if (!XInputLibrary)
	{
		//TODO: @casey:-> Diagnostic
		HMODULE XInputLibrary = LoadLibraryA("xinput1_3.dll"); //if xinput1_4.dll is not present in the Windows system of the user, then load xinput1_3.dll
	}
	if(XInputLibrary)
	{
		XInputGetState = (x_input_get_state *) GetProcAddress(XInputLibrary, "XInputGetState");
		if (!XInputGetState) 
		{
			XInputGetState = XInputGetStateStub;
		}
		
		XInputSetState = (x_input_set_state *) GetProcAddress(XInputLibrary, "XInputSetState");
		if (!XInputSetState) 
		{
			XInputSetState = XInputSetStateStub;
		}

		//TODO: @casey:-> Diagnostic
	}
	else 
	{
		//TODO: @casey:-> Diagnostic
	}
}

//**************************************************************************************************


/*
 * DirectSound Stuff
 * */

#define  DIRECT_SOUND_CREATE(func_name)				\
	HRESULT WINAPI func_name(LPCGUID pcGuidDevice,  \
							 LPDIRECTSOUND *ppDS,   \
							 LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

internal_func
void Win32_InitDSound(HWND Window, 
					  int32 SamplesPerSecond,
					  int32 BufferSize)
{
	//(1st): Load the library
	HMODULE DSoundLibaray = LoadLibraryA("dsound.dll");
	if (DSoundLibaray)
	{
		//(2nd): Get a DirectSound object! (cooperative mode)  b/c DirectSound is based on OOP
		direct_sound_create *DirectSoundCreate = (direct_sound_create *) GetProcAddress(DSoundLibaray, "DirectSoundCreate");
		LPDIRECTSOUND DirectSound;
		if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(NULL,         //assumming we get the DirectSoundCreate() address, we are going to run it in here 
															 &DirectSound,
															 NULL)))
		{
			WAVEFORMATEX WaveFormat                     =   {};
						 WaveFormat.wFormatTag          =   WAVE_FORMAT_PCM;
						 WaveFormat.nChannels           =   2;
						 WaveFormat.nSamplesPerSec      =   SamplesPerSecond;
						 WaveFormat.wBitsPerSample      =   16;
						 WaveFormat.nBlockAlign         =   (WaveFormat.nChannels * WaveFormat.wBitsPerSample)/8;
						 WaveFormat.nAvgBytesPerSec     =   (WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign);
						 WaveFormat.cbSize              =   0;
			
			if (SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
			{
				DSBUFFERDESC BufferDescription = {};
				BufferDescription.dwSize = sizeof(BufferDescription);
				BufferDescription.dwFlags =	DSBCAPS_PRIMARYBUFFER; //TODO: @casey-> DSBCAPS_GLOBALFOCUS
				
				//(3rd): Create a primary buffer, where we will basically set the mode of that in the old school way of doing things
				LPDIRECTSOUNDBUFFER PrimaryBuffer;

				if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, 
														 	 &PrimaryBuffer, 
															 NULL)))
				{
					if (SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat)))
					{
						//Now, we have finally set the format	

						OutputDebugStringA("PrimaryBuffer format was set\n"); //Test
					}
					else 
					{
						//TODO: @casey-> Diagnostic
					}
				}
			}
			else 
			{
				//TODO: @casey-> Diagnostic
			}
			
			//(4th): Create a secondary buffer, where we are basically going to write into	
			
			DSBUFFERDESC BufferDescription                 =  {};
						 BufferDescription.dwSize          =  sizeof(BufferDescription);
						 BufferDescription.dwFlags         =  0;
						 BufferDescription.dwBufferBytes   =  BufferSize;
						 BufferDescription.lpwfxFormat     =  &WaveFormat;
			
			LPDIRECTSOUNDBUFFER SecondaryBuffer;
			
			if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, 
														 &SecondaryBuffer, 
														 NULL)))
			{
				//(5th): Start it playing!

				OutputDebugStringA("SecondaryBuffer created successfully.\n"); //Test
			}
			
		}
		else 
		{
			//TODO: @casey:-> Diagnostic
		}
	}
}

//**************************************************************************************************
//**************************************************************************************************

internal_func
Win32_WindowDimensions Win32_GetWindowDimensions(HWND Window)
{
	Win32_WindowDimensions Result;

	RECT ClientRect;
	GetClientRect(Window, &ClientRect);
	Result.Width    =  ClientRect.right - ClientRect.left;
	Result.Height   =  ClientRect.bottom - ClientRect.top;
	
	return Result;
}


internal_func 
void RenderWeirdGradient(Win32_OffScreenBuffer *Buffer, 
						 int XOffset, 
						 int YOffset)
{
	uint8* Row   =  (uint8*) Buffer->Memory;

	for(int Y = 0; 
		Y < Buffer->Height; 
		Y++)
	{
		uint32* Pixel = (uint32* )Row; 
		
		for(int X = 0; 
			X < Buffer->Width; 
			X++)
		{
			//Blue            
			uint8 Blue        =   (X + XOffset);
			//Green           
			uint8 Green       =   (Y + YOffset);
			//Red             
			uint8 Red         =   0; 
			//Padding         
			//uint8 Padding   =   0;
			*Pixel++          =  (Red << 16) | (Green << 8) | (Blue); 
		}

		Row += Buffer->Pitch; 
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
	
	Buffer->Width                          =  Width;
	Buffer->Height                         =  Height;
	Buffer->Info.bmiHeader.biSize          =  sizeof(Buffer->Info.bmiHeader);
	Buffer->Info.bmiHeader.biWidth         =  Buffer->Width;
	Buffer->Info.bmiHeader.biHeight        =  -Buffer->Height; 
	Buffer->Info.bmiHeader.biPlanes        =  1;
	Buffer->Info.bmiHeader.biBitCount      =  32; 
	Buffer->Info.bmiHeader.biCompression   =  BI_RGB;

	int BytesPerPixel      =  4;
	int BitmapMemorySize   =  ((Buffer->Width) * (Buffer->Height)) * BytesPerPixel; //size in (Bytes)

	Buffer->Memory = VirtualAlloc(0, 
								  BitmapMemorySize, 
								  MEM_RESERVE | MEM_COMMIT, 
								  PAGE_READWRITE);
	
	Buffer->Pitch = (Buffer->Width) * (BytesPerPixel);
}


internal_func 
void Win32_DisplayBufferInWindow(Win32_OffScreenBuffer *Buffer,
								 HDC DeviceContext,
								 int WindowWidth,
								 int WindowHeight)
{
	//TODO(@casey):-> "Aspect ratio correction"
	//TODO(@casey):-> "Play with stretch modes"
	StretchDIBits(DeviceContext,
				  0, 0, WindowWidth, WindowHeight,	 // destn
				  0, 0, Buffer->Width, Buffer->Height, // src
				  Buffer->Memory,
				  &Buffer->Info,
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

		case WM_PAINT:
			{
				OutputDebugStringA("WM_PAINT\n");

				PAINTSTRUCT Paint;
				HDC DeviceContext = BeginPaint(Window, &Paint); 
				Win32_WindowDimensions Dimensions = Win32_GetWindowDimensions(Window);
				Win32_DisplayBufferInWindow(&GlobalBackBuffer,
											DeviceContext,
											Dimensions.Width,
											Dimensions.Height);
				EndPaint(Window, &Paint);
			}
			break;

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			uint32 VKcode = WParam; //Don't know why (uint32) has been used.
			#define KeyMessageWasDownBit (1 << 30)
			#define KeyMessageIsDownBit (1 << 31)
			bool WasDown = ((LParam & KeyMessageWasDownBit) != 0); //Don't know why 29 didn't work
			bool IsDown = ((LParam & KeyMessageIsDownBit) == 0); //Don't knwo why 30 didn't work
			
			if (WasDown != IsDown)
			{
				if (VKcode == 'W')
				{
					OutputDebugStringA("W\n");
				}
				else if (VKcode == 'A')
				{}
				else if (VKcode == 'S')
				{}
				else if (VKcode == 'D')
				{}
				else if (VKcode == 'Q')
				{}
				else if (VKcode == 'E')
				{}
				else if (VKcode == VK_UP)
				{}
				else if (VKcode == VK_DOWN)
				{}
				else if (VKcode == VK_RIGHT)
				{}
				else if (VKcode == VK_LEFT)
				{}
				else if (VKcode == VK_SPACE)
				{}
				else if (VKcode == VK_ESCAPE)
				{
					OutputDebugStringA("ESCAPE: ");
					if (IsDown)
					{
						OutputDebugStringA("IsDown");
					}
					if (WasDown)
					{
						OutputDebugStringA("WasDown");
					}
					OutputDebugStringA("\n");
				}
			}
			
			bool32 AltKeyWasDown = (LParam & (1 << 29)); //((LParam & (1 << 29)) != 0);
			if ((VKcode == VK_F4) && AltKeyWasDown)
			{
				GlobalRunning = false;
			}
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
	Win32_LoadXInput(); //@casey:-> "Unlike DirectSound, we can load xinput anytime and anywhere we want."

	WNDCLASSA WindowClass = {};

	Win32_ResizeDIBSection(&GlobalBackBuffer,
						   1280,
						   720);

	WindowClass.lpszClassName   =  "HandmadeHeroWindowClass";
	WindowClass.hInstance       =  Instance;
	WindowClass.lpfnWndProc     =  Win32_MainWindowCallback;
	WindowClass.style           =  (CS_OWNDC | CS_HREDRAW | CS_VREDRAW);

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
			//Initializing - DirectSound
			Win32_InitDSound(Window,					//@casey:-> "I think we cannot get our directSound untill and we have our Window set" 
							 48000,
							 48000 * sizeof(int16) * 2); 

			int XOffset = 0;
			int YOffset = 0;
			GlobalRunning = true;
			while (GlobalRunning)
			{
				//Windows Message Processing Loop
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

				//XInput
				//*******************************************************************************
				//TODO: @casey:-> "Should we poll this more frequently"
				for (DWORD ControllerIndex = 0;
					 ControllerIndex < XUSER_MAX_COUNT;
					 ++ControllerIndex)
				{
					XINPUT_STATE ControllerState;
					
					if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
					{
						//This controller is plugged in
						//TODO: @casey -> "See if ControllerState.dwPacketNumber increments too rapidly."
						
						XINPUT_GAMEPAD* Pad = &ControllerState.Gamepad; //for syntactic convinience

						bool Up                 =   (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
						bool Down               =   (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
						bool Left               =   (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
						bool Right              =   (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
						bool Start              =   (Pad->wButtons & XINPUT_GAMEPAD_START);
						bool Back               =   (Pad->wButtons & XINPUT_GAMEPAD_BACK);
						bool LeftShoulder       =   (Pad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
						bool RightShoulder      =   (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
						bool AButton            =   (Pad->wButtons & XINPUT_GAMEPAD_A);
						bool BButton            =   (Pad->wButtons & XINPUT_GAMEPAD_B);
						bool XButton            =   (Pad->wButtons & XINPUT_GAMEPAD_X);
						bool YButton            =   (Pad->wButtons & XINPUT_GAMEPAD_Y);

						int16 StickX = Pad->sThumbLX;
						int16 StickY = Pad->sThumbLY;
						
						//simple test
						
						XOffset += StickX >> 12;
						YOffset += StickY >> 12;	

						if (AButton)
						{
							//(YOffset) += 5;
						}
					}
					else 
					{
						//The controller is not availabe
					}
				}
				//*******************************************************************************
				

				//Rendered to the bitmap through this function
				RenderWeirdGradient(&GlobalBackBuffer,
									XOffset, 
									YOffset); 
				
				//Blitting on the screen from the bitmap
				HDC DeviceContext                   =  GetDC(Window);
				Win32_WindowDimensions Dimensions   =  Win32_GetWindowDimensions(Window);
				Win32_DisplayBufferInWindow(&GlobalBackBuffer,
											DeviceContext, 
											Dimensions.Width, 
											Dimensions.Height);
				++(XOffset);
				//++(YOffset);
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
