#include <windows.h>
#include <iostream>
#include <cstring>

const wchar_t* shmName = L"Local\MySharedMemory";
const wchar_t* dataWrittenEventName = L"Local\DataWrittenEvent";
const wchar_t* dataReadEventName = L"Local\DataReadEvent";
const int BUF_SIZE = 512;

void WriteToSharedMemory(HANDLE hMapFile)
{
    char* pBuf = static_cast<char*>(MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUF_SIZE));
    if (!pBuf)
    {
        std::cerr << "MapViewOfFile error: " << GetLastError() << std::endl;
        return;
    }

    HANDLE hDataWritten = CreateEventW(NULL, FALSE, FALSE, dataWrittenEventName);
    HANDLE hDataRead = CreateEventW(NULL, FALSE, FALSE, dataReadEventName);
    if (!hDataWritten || !hDataRead)
    {
        std::cerr << "CreateEvent error: " << GetLastError() << std::endl;
        UnmapViewOfFile(pBuf);
        return;
    }

    const char* message = "Hello from shared memory!";
    strcpy_s(pBuf, BUF_SIZE, message);
    std::cout << "Data written to shared memory. Waiting for reading..." << std::endl;

    SetEvent(hDataWritten);
    WaitForSingleObject(hDataRead, INFINITE);

    UnmapViewOfFile(pBuf);
    CloseHandle(hDataWritten);
    CloseHandle(hDataRead);
}

void ReadFromSharedMemory(HANDLE hMapFile)
{
    char* pBuf = static_cast<char*>(MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUF_SIZE));
    if (!pBuf)
    {
        std::cerr << "MapViewOfFile error: " << GetLastError() << std::endl;
        return;
    }

    HANDLE hDataWritten = OpenEventW(EVENT_ALL_ACCESS, FALSE, dataWrittenEventName);
    HANDLE hDataRead = OpenEventW(EVENT_ALL_ACCESS, FALSE, dataReadEventName);
    if (!hDataWritten || !hDataRead)
    {
        std::cerr << "OpenEvent error: " << GetLastError() << std::endl;
        UnmapViewOfFile(pBuf);
        return;
    }

    std::cout << "Waiting for data..." << std::endl;
    WaitForSingleObject(hDataWritten, INFINITE);

    std::cout << "Received data: " << pBuf << std::endl;
    ZeroMemory(pBuf, BUF_SIZE);

    SetEvent(hDataRead);

    UnmapViewOfFile(pBuf);
    CloseHandle(hDataWritten);
    CloseHandle(hDataRead);
}

int main()
{
    HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, shmName);
    bool isFirstProcess = (hMapFile == NULL);

    if (isFirstProcess)
    {
        hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, BUF_SIZE, shmName);
        if (!hMapFile)
        {
            std::cerr << "CreateFileMapping error: " << GetLastError() << std::endl;
            return 1;
        }
        WriteToSharedMemory(hMapFile);
    }
    else
    {
        ReadFromSharedMemory(hMapFile);
    }

    CloseHandle(hMapFile);
    std::cout << (isFirstProcess ? "First process: exit" : "Second process: exit") << std::endl;
    return 0;
}
