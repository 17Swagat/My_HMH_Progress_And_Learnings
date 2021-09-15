#include <Windows.h>

int CALLBACK WinMain(HINSTANCE hInstance, 
		     HINSTANCE hPrevInstance, // Its just legacy-code now
		     LPSTR lpCmdLine, 
		     int nShowCmd)
{
	MessageBoxA(0, "This is handmade hero", "Handmade Hero", (MB_OK | MB_ICONINFORMATION));
	/*

	NOTE: Here, when we call "cl", by default its in "ANSI" mode not "utf-16 mode"
	
	Its defined as -> 
	
	#define MessageBox MessageBoxA

	*/

	OutputDebugString("Hello World"); //Here, also -> (#define OutputDebugString OutputDebugStringA)

	return 0;
}
