#include "winFileWatcher.h"
#include <string>
#include <list>
#include <future>

winFileWatcher::winFileWatcher(fileObserver * observer, std::wstring path) : fileWatcher(observer, path), pThLoop(nullptr), pQueRdrLoop(nullptr) {
        needToStop = true;
    };
winFileWatcher::winFileWatcher(fileObserver * observer) : fileWatcher(observer), pThLoop(nullptr), pQueRdrLoop(nullptr){
        needToStop = true;
    };

void winFileWatcher::stopMonitor () {
    needToStop = true;
    if (pThLoop != nullptr) {
        SetEvent(pFileWatchEvent);
        pThLoop->join();
        delete pThLoop;
        pThLoop = nullptr;
    }
    if (pQueRdrLoop != nullptr) {
        pQueRdrLoop->join();
        delete pQueRdrLoop;
        pQueRdrLoop = nullptr;
    }
}

int winFileWatcher::runMonitor(){
    if ( !dirExists(this->getPath()))
        return 1;

    pThLoop = new  std::thread(&winFileWatcher::monitorLoop, this); // Loop to react Windows events
    pQueRdrLoop = new std::thread(&winFileWatcher::notifyThreadSafe, this); // Loop to process events separately

    return 0;
}

int winFileWatcher::restartMonitor(){
    stopMonitor();
    return runMonitor();
}

int winFileWatcher::getFileAttrs (std::wstring path, fileInfo& retFi) {

   WIN32_FIND_DATA FindFileData;
   HANDLE hFind;
   LPCTSTR  lpFileName = path.c_str();

   hFind = FindFirstFile(lpFileName , &FindFileData);
   if (hFind == INVALID_HANDLE_VALUE) {
      return 1;
   }
   else {
      ULONGLONG FileSize = FindFileData.nFileSizeHigh;
      FileSize <<= sizeof( FindFileData.nFileSizeHigh ) * 8;
      FileSize |= FindFileData.nFileSizeLow;
      retFi.setSize(FileSize);
      retFi.setDateModified(fileTimeToTime_t(FindFileData.ftLastWriteTime));
      FindClose(hFind);
   }
   return 0;
}

bool winFileWatcher::dirExists(std::wstring path) {
    DWORD dwAttrib = GetFileAttributes(path.c_str());
    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
           (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

void winFileWatcher::initialNotification() { //to list all files in directory
    WIN32_FIND_DATA data;
        HANDLE hFind = FindFirstFile((this->getPath()+L"\\*.*").c_str(), &data);

        if ( hFind != INVALID_HANDLE_VALUE ) {
            do {
                if ( !(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {
                    fileInfo newFI (this->getPath()+L"\\"+data.cFileName,fileTimeToTime_t(data.ftLastWriteTime)
                                    , winfSizeToLL(data.nFileSizeLow, data.nFileSizeHigh)
                                    , fileInfo::NONE);
                    notifyObserver(newFI);
                }
            } while (FindNextFile(hFind, &data));
            FindClose(hFind);
        }
    };

void winFileWatcher::monitorLoop() {
    needToStop = false;

    HANDLE hDir = CreateFile(this->getPath().c_str(),
            GENERIC_READ,
            FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
            NULL
            );

    OVERLAPPED o = {};

    pFileWatchEvent = CreateEvent(0, FALSE, FALSE, 0);
    //pFileWatchEvent = CreateEvent(0, TRUE, FALSE, 0);
    o.hEvent = pFileWatchEvent;

    DWORD nBufferLength = 60*1024;
    BYTE* lpBuffer = new BYTE[nBufferLength];

    while(!needToStop) {

        DWORD returnedBytes = 0;
        ReadDirectoryChangesW(hDir, lpBuffer, nBufferLength, TRUE,
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_CREATION ,
            &returnedBytes, &o, 0);

        DWORD dwWaitStatus = WaitForSingleObject(o.hEvent, INFINITE); // If wait is not INFINITE, some events are missed by watcher

        if (needToStop) break;

        switch(dwWaitStatus) {
        case WAIT_OBJECT_0: {
                DWORD seek = 0;
                while(seek < nBufferLength) {
                    PFILE_NOTIFY_INFORMATION pNotify = PFILE_NOTIFY_INFORMATION(lpBuffer + seek);
                    seek += pNotify->NextEntryOffset;

                    fileInfo fI = notifyToFileInfo(pNotify);

                    notifyMutex.lock();
                    notifyQueue.push(fI);
                    notifyMutex.unlock();

                    if(pNotify->NextEntryOffset == 0)
                        break;
                }
            }
            break;
        case WAIT_TIMEOUT:
            break;
        default:
            needToStop = true;
            break;
        }
    }

    CloseHandle(o.hEvent);
    delete [] lpBuffer;
}

void winFileWatcher::notifyThreadSafe(){ // notification queue extracting function
    while (!needToStop) {
        if (!notifyQueue.empty()) {
            notifyMutex.lock();
            if (needToStop) { notifyMutex.unlock(); return; }
            fileInfo newFI = notifyQueue.front();
            notifyQueue.pop();
            notifyMutex.unlock();
            getFileAttrs(newFI.sFullPath, newFI);
            this->notifyObserver(newFI);
        } else {
            Sleep(500);
        }
    }
}

int winFileWatcher::renameFile(std::wstring& oldName, std::wstring& newName){ // return 0 on success
    return (! MoveFile( (this->getPath() + L"\\" + oldName).c_str()
                      , (this->getPath() + L"\\" + newName).c_str()));

}

fileInfo winFileWatcher::notifyToFileInfo(PFILE_NOTIFY_INFORMATION pNotify) {
    WCHAR szwFileName[MAX_PATH];

    ULONG ulCount = std::min(pNotify->FileNameLength/2, (unsigned long)(MAX_PATH-1));
    wcsncpy_s (szwFileName, pNotify->FileName, ulCount);
    szwFileName[ulCount] = L'\0';

    std::wstring wstrName(szwFileName);
    std::wstring fullName (this->getPath()+L"\\"+wstrName);
    struct fileInfo newFI(fullName, 0,0);

    switch (pNotify->Action ) {
    case FILE_ACTION_ADDED:
        newFI.action = fileInfo::ADDED;
        break;
    case FILE_ACTION_REMOVED:
        newFI.action = fileInfo::DELETED;
        break;
    case FILE_ACTION_MODIFIED:
        newFI.action = fileInfo::MODIFIED;
        break;
    case FILE_ACTION_RENAMED_OLD_NAME:
        newFI.action = fileInfo::RENAMED_OLD;
        break;
    case FILE_ACTION_RENAMED_NEW_NAME:
        newFI.action = fileInfo::RENAMED_NEW;
        break;
    default:
        break;
    }
    return newFI; //returnin newFi with no attributes set
}

winFileWatcher::~winFileWatcher() {
    stopMonitor();
    // TODO: make monitor loop not leak memory!
}

time_t  winFileWatcher::fileTimeToTime_t(FILETIME const& ft)  {
    ULARGE_INTEGER ull;
    ull.LowPart = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;
    return ull.QuadPart / 10000000ULL - 11644473600ULL;
}

unsigned long long int winFileWatcher::winfSizeToLL (DWORD low, DWORD high) {
    ULARGE_INTEGER ull;
    ull.LowPart = low;
    ull.HighPart = high;
    return ull.QuadPart;
}
