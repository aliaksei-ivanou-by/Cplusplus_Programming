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

class SharedMemoryManager {
public:
    SharedMemoryManager(bool isWriter) : isWriter(isWriter) {
#ifdef _WIN32
        if (isWriter) {
            hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, BUF_SIZE, shmName);
        }
        else {
            hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, shmName);
        }
        if (!hMapFile) {
            std::cerr << "File mapping error: " << GetLastError() << std::endl;
            return;
        }
        pBuf = static_cast<char*>(MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUF_SIZE));
        hDataWritten = CreateEventW(NULL, FALSE, FALSE, dataWrittenEventName);
        hDataRead = CreateEventW(NULL, FALSE, FALSE, dataReadEventName);
#else
        if (isWriter) {
            shmFd = shm_open(shmName, O_CREAT | O_RDWR, 0666);
            ftruncate(shmFd, BUF_SIZE);
        }
        else {
            shmFd = shm_open(shmName, O_RDWR, 0666);
        }
        pBuf = static_cast<char*>(mmap(0, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0));
        hDataWritten = sem_open(dataWrittenEventName, O_CREAT, 0666, 0);
        hDataRead = sem_open(dataReadEventName, O_CREAT, 0666, 0);
#endif
    }

    ~SharedMemoryManager() {
#ifdef _WIN32
        UnmapViewOfFile(pBuf);
        CloseHandle(hMapFile);
        CloseHandle(hDataWritten);
        CloseHandle(hDataRead);
#else
        munmap(pBuf, BUF_SIZE);
        close(shmFd);
        sem_close(hDataWritten);
        sem_close(hDataRead);
        if (isWriter) {
            shm_unlink(shmName);
            sem_unlink(dataWrittenEventName);
            sem_unlink(dataReadEventName);
        }
#endif
    }

    void Write(const char* message) {
        if (!isWriter) return;
#ifdef _WIN32
        strcpy_s(pBuf, BUF_SIZE, message);
        SetEvent(hDataWritten);
        WaitForSingleObject(hDataRead, INFINITE);
#else
        strcpy(pBuf, message);
        sem_post(hDataWritten);
        sem_wait(hDataRead);
#endif
    }

    void Read() {
        if (isWriter) return;
#ifdef _WIN32
        WaitForSingleObject(hDataWritten, INFINITE);
#else
        sem_wait(hDataWritten);
#endif
        std::cout << "Received data: " << pBuf << std::endl;
        memset(pBuf, 0, BUF_SIZE);
#ifdef _WIN32
        SetEvent(hDataRead);
#else
        sem_post(hDataRead);
#endif
    }

private:
    bool isWriter;
#ifdef _WIN32
    HANDLE hMapFile, hDataWritten, hDataRead;
#else
    int shmFd;
    sem_t* hDataWritten, * hDataRead;
#endif
    char* pBuf;
};

int main() {
#ifdef _WIN32
    bool isFirstProcess = (OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, shmName) == NULL);
#else
    bool isFirstProcess = (shm_open(shmName, O_RDWR, 0666) == -1);
#endif

    SharedMemoryManager shmManager(isFirstProcess);
    if (isFirstProcess) {
        shmManager.Write("Hello from shared memory!");
    }
    else {
        shmManager.Read();
    }

    std::cout << (isFirstProcess ? "First process: exit" : "Second process: exit") << std::endl;
    return 0;
}
