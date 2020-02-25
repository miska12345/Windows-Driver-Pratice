// test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <Windows.h>
int main()
{
    HANDLE hDevice = ::CreateFile(L"\\\\.\\Zerox", GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hDevice == INVALID_HANDLE_VALUE) {
        return -1;
    }

    BYTE buffer[64];
    for (int i = 0; i < sizeof(buffer); i++) {
        buffer[i] = i;
    }
    DWORD bytes;
    BOOL ok = ::ReadFile(hDevice, buffer, sizeof(buffer), &bytes, nullptr);
    if (!ok) {
        return -1;
    }
    if (bytes != sizeof(buffer)) {
        return -1;
    }

    long total = 0;
    for (auto n : buffer) {
        total += n;
    }
    if (total != 0) {
        printf("sum not equal 0\n");
    }

    BYTE buffer2[1024];
    ok = ::WriteFile(hDevice, buffer2, sizeof(buffer2), &bytes, nullptr);
    if (!ok) {
        return -1;
    }
    if (bytes != sizeof(buffer2)) {
        printf("wrong bytes write\n");
    }
    printf("done\n");
    ::CloseHandle(hDevice);
}
