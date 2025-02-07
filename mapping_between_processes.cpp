#ifdef _WIN32
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>
#include <cstring>
#endif

#include <iostream>

const wchar_t* shmName = L"Local\MySharedMemory";
const wchar_t* dataWrittenEventName = L"Local\DataWrittenEvent";
const wchar_t* dataReadEventName = L"Local\DataReadEvent";
const int BUF_SIZE = 512;

void WriteToSharedMemory()
{
#ifdef _WIN32
    HANDLE hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, BUF_SIZE, shmName);
    if (!hMapFile)
    {
        std::cerr << "CreateFileMapping error: " << GetLastError() << std::endl;
        return;
    }
    char* pBuf = static_cast<char*>(MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUF_SIZE));
#else
    int shmFd = shm_open(shmName, O_CREAT | O_RDWR, 0666);
    ftruncate(shmFd, BUF_SIZE);
    char* pBuf = static_cast<char*>(mmap(0, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0));
#endif

    if (!pBuf)
    {
        std::cerr << "Memory mapping error" << std::endl;
        return;
    }

#ifdef _WIN32
    HANDLE hDataWritten = CreateEventW(NULL, FALSE, FALSE, dataWrittenEventName);
    HANDLE hDataRead = CreateEventW(NULL, FALSE, FALSE, dataReadEventName);
#else
    sem_t* hDataWritten = sem_open(dataWrittenEventName, O_CREAT, 0666, 0);
    sem_t* hDataRead = sem_open(dataReadEventName, O_CREAT, 0666, 0);
#endif

    const char* message = "Hello from shared memory!";
#ifdef _WIN32
    strcpy_s(pBuf, BUF_SIZE, message);
#else
    strcpy(pBuf, message);
#endif
    std::cout << "Data written to shared memory. Waiting for reading..." << std::endl;

#ifdef _WIN32
    SetEvent(hDataWritten);
    WaitForSingleObject(hDataRead, INFINITE);
#else
    sem_post(hDataWritten);
    sem_wait(hDataRead);
#endif

#ifdef _WIN32
    UnmapViewOfFile(pBuf);
    CloseHandle(hDataWritten);
    CloseHandle(hDataRead);
    CloseHandle(hMapFile);
#else
    munmap(pBuf, BUF_SIZE);
    close(shmFd);
    shm_unlink(shmName);
    sem_close(hDataWritten);
    sem_close(hDataRead);
    sem_unlink(dataWrittenEventName);
    sem_unlink(dataReadEventName);
#endif
}

void ReadFromSharedMemory()
{
#ifdef _WIN32
    HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, shmName);
    char* pBuf = static_cast<char*>(MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUF_SIZE));
#else
    int shmFd = shm_open(shmName, O_RDONLY, 0666);
    char* pBuf = static_cast<char*>(mmap(0, BUF_SIZE, PROT_READ, MAP_SHARED, shmFd, 0));
#endif

    if (!pBuf)
    {
        std::cerr << "Memory mapping error" << std::endl;
        return;
    }

#ifdef _WIN32
    HANDLE hDataWritten = OpenEventW(EVENT_ALL_ACCESS, FALSE, dataWrittenEventName);
    HANDLE hDataRead = OpenEventW(EVENT_ALL_ACCESS, FALSE, dataReadEventName);
#else
    sem_t* hDataWritten = sem_open(dataWrittenEventName, 0);
    sem_t* hDataRead = sem_open(dataReadEventName, 0);
#endif

    std::cout << "Waiting for data..." << std::endl;
#ifdef _WIN32
    WaitForSingleObject(hDataWritten, INFINITE);
#else
    sem_wait(hDataWritten);
#endif

    std::cout << "Received data: " << pBuf << std::endl;
    memset(pBuf, 0, BUF_SIZE);

#ifdef _WIN32
    SetEvent(hDataRead);
    UnmapViewOfFile(pBuf);
    CloseHandle(hDataWritten);
    CloseHandle(hDataRead);
    CloseHandle(hMapFile);
#else
    sem_post(hDataRead);
    munmap(pBuf, BUF_SIZE);
    close(shmFd);
    sem_close(hDataWritten);
    sem_close(hDataRead);
#endif
}

int main()
{
#ifdef _WIN32
    HANDLE hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, shmName);
    bool isFirstProcess = (hMapFile == NULL);
#else
    int shmFd = shm_open(shmName, O_RDWR, 0666);
    bool isFirstProcess = (shmFd == -1);
#endif

    if (isFirstProcess)
    {
        WriteToSharedMemory();
    }
    else
    {
        ReadFromSharedMemory();
    }

    std::cout << (isFirstProcess ? "First process: exit" : "Second process: exit") << std::endl;
    return 0;
}
