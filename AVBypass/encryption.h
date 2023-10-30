#pragma once

#include "PatchUtils.h"

static char funcstartsig[] =
{ 0x48, 0xc7, 0x44, 0x24, 0x20, 0x77, 0x07, 0x00, 0x00, 0xeb, 0x0d, 0x48 };
const int funcstartoffset = 37;
const int funcstartsize = 11;

static char funcendsig[] =
{ 0x48, 0xc7, 0x44, 0x24, 0x30, 0x00, 0x10, 0x00, 0x00, 0xeb, 0x0d, 0x48 };
const int funcendoffset = -1;
const int funcendsize = 9;


static char funcstartmask[] = "xxxx?xxxxxxx";
static char funcendmask[] =   "xxxx?xxxxxxx";

// ayo the key be 8 byte rn so change dat if u feelin' that way, u will need to change some other part o' da code as well i tink
void xormem(ULONG64 StartAddr, ULONG64 Size, char key[8 + 1])
{
    // x64 xormem, no inline asm
    BYTE* ecx = reinterpret_cast<BYTE*>(StartAddr + Size);
    BYTE* eax = reinterpret_cast<BYTE*>(StartAddr);

    // crypt_loop
    int i = 0;
    for (; eax < ecx; eax += sizeof(BYTE))
    {
        *eax ^= key[i % 8];
        i++;
    }
}

bool xorsigmem(char key[8 + 1], uintptr_t func)
{
    uintptr_t funcstart = 0, funcend = 0; // address results from sigscan

    // sigscan
    for (BYTE* currentaddr = reinterpret_cast<BYTE*>(func);
        currentaddr < reinterpret_cast<BYTE*>(func + 0x10000); // max size of func is 65535 rn
        currentaddr++)
    {
        for (size_t i = 0; i < funcstartsize; i++)
        {
            if (funcstartsig[i] != (char)currentaddr[i] && funcstartmask[i] == 'x') {
                break;
            }

            if (i == funcstartsize - 1)
            {
                funcstart = (uintptr_t)currentaddr + funcstartoffset;
            }
        }

        for (size_t i = 0; i < funcendsize; i++)
        {
            if (funcendsig[i] != (char)currentaddr[i] && funcendmask[i] == 'x') {
                break;
            }

            if (i == funcendsize - 1)
            {
                funcend = (uintptr_t)currentaddr + funcendoffset + 1;
            }
        }

        if (funcend != 0 && funcstart != 0)
        {
            break;
        }
    }

    std::cout << "funcstart: 0x" << std::hex << funcstart << '\n';
    std::cout << "funcend:   0x" << funcend << std::dec << '\n';

    // self-modifying code
    if (funcstart == 0 || funcend == 0)
    {
        std::cout << "sig not found\n";
        Sleep(3000);
        return false;
    }
    else
    {
        DWORD dwOldProtect;
        uintptr_t FunctionSize = funcend - funcstart;
        VirtualProtect((uintptr_t*)func, FunctionSize, PAGE_EXECUTE_READWRITE, &dwOldProtect);

        int i = 0;
        for (uintptr_t currentaddr = funcstart; currentaddr < funcend; currentaddr++)
        {
            //std::cout << "CurrentByte: 0x" << std::hex << (int)(*(BYTE*)currentaddr) << '\n';
            *(char*)currentaddr ^= key[i % 8]; //0x01
            //std::cout << "After  Byte: 0x" << std::hex << (int)(*(BYTE*)currentaddr) << '\n';
            i++;
        }

        return true;
    }
}

// dis also 8 byte rn fr
void xorpatch(std::fstream* fs, char key[8 + 1])
{
    uintptr_t startAddr = SigScan(fs, funcstartsig, funcstartmask, funcstartsize, 1) + funcstartoffset;
    uintptr_t endAddr = SigScan(fs, funcendsig, funcendmask, funcendsize, 1) + funcendoffset + 1;
    size_t sizeOfFunc = endAddr - startAddr;

    char* buffer = new char[sizeOfFunc + 1];
    f2m(fs, (char*)buffer, startAddr, sizeOfFunc);

    for (size_t i = 0; i < sizeOfFunc; i++)
    {
        buffer[i] ^= key[i % 8];
    }

    PatchBytes(fs, startAddr, buffer, sizeOfFunc);

    delete[] buffer;
    Sleep(1000);
}