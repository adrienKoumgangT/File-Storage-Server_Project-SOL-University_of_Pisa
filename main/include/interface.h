#if !defined(INTERFACE_FOR_SERVER)
#define INTERFACE_FOR_SERVER

int closeConnection(const char* sockname);

int openFile(const char* pathname, int flags);

int readFile(const char* pathname, void** buf, size_t* size);

int appendToFile(const char* pathname, void* buf, size_t size, const char* dirname);

int lockFile(const char* pathname);

int unlockFile(const char* pathname);

int closeFile(const char* pathname);

int removeFile(const char* pathname);

#endif