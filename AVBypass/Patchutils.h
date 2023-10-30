#pragma once

#pragma warning (disable : 26451) // 4byte to 8byte

#include <windows.h>
#include <iostream>
#include <fstream>

size_t SigScan(std::fstream* fs, const char* sig, const char* mask, int length, int index)
{
	char* buffer = new char[length];
	int currentindex = 1;

	for (size_t i = 0; i < 0x100000; i++)
	{
		fs->seekg(i, std::ios::beg);
		fs->read(buffer, length);

		for (size_t u = 0; u < length; u++)
		{
			if (sig[u] != buffer[u] && mask[u] == 'x')
			{
				break;
			}

			if (u == length - 1)
			{
				if (currentindex < index)
				{
					currentindex++;
					break;
				}

				delete[] buffer;
				return i;
			}
		}
	}

	delete[] buffer;
	return 0xdeadbeef;
}

void PatchBytes(std::fstream* fs, int offset, char* bytes, int len)
{
	fs->seekg(offset, std::ios::beg);
	fs->write(bytes, len);
}

void CoutBytes(std::fstream* fs, int offset, int length)
{
	fs->seekg(offset, std::ios::beg);

	char* buffer = new char[length + 1];

	buffer[length] = '\0';
	fs->read(buffer, length);
	std::cout << std::hex << (int)buffer << std::dec << std::endl;

	delete[] buffer;
}


// file to memory function, copies byte from the file, and sets it or something
void f2m(std::fstream* fs, char* buffer, uintptr_t offset, size_t length)
{
	fs->seekg(offset, std::ios::beg);

	buffer[length] = '\0';
	fs->read(buffer, length);
}
