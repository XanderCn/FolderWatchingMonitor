#ifndef WINFILEWATCHER_H
#define WINFILEWATCHER_H

#include "fileWatcher.h"

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <algorithm>
#include <thread>
#include <ctime>
#include <string>
#include "fileAccess.h"
#include <queue>
#include <mutex>

typedef std::basic_string<TCHAR> tstring;

class winFileWatcher : public fileWatcher {
public:
    winFileWatcher(fileObserver * observer, std::wstring path);
    winFileWatcher(fileObserver * observer);

    int getFileAttrs (std::wstring path, fileInfo& retFi);
    bool dirExists(std::wstring path);

    void initialNotification(); // send listing of specified directory to observer set

    void monitorLoop();

    void stopMonitor ();
    int runMonitor();
    int restartMonitor();

    void winNotifyObserver(PFILE_NOTIFY_INFORMATION pNotify);
    void winNotifyObserver(FILE_NOTIFY_INFORMATION& pNotify);

    fileInfo notifyToFileInfo(PFILE_NOTIFY_INFORMATION pNotify);

    int renameFile(std::wstring& oldName, std::wstring& newName);

    static time_t  fileTimeToTime_t(FILETIME const& ft);
    static unsigned long long int winfSizeToLL (DWORD low, DWORD high);

    virtual ~winFileWatcher();
private:
    bool needToStop;
    std::thread * pThLoop;
    std::thread * pQueRdrLoop;

    //OVERLAPPED * o;
    HANDLE pFileWatchEvent;


    std::queue<fileInfo> notifyQueue;
    std::mutex notifyMutex;
    void notifyThreadSafe();



};



#endif // WINFILEWATCHER_H
