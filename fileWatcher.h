#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <sys/types.h>
#include <sys/stat.h>
#include <thread>

class fileObserver; // forward declaration of class

//file API to monitor changes in directory or rename files
class fileWatcher {
public:
    fileWatcher(fileObserver* observer, std::wstring path);
    fileWatcher(fileObserver* observer);

    void notifyObserver (struct fileInfo& fI);
    std::wstring getPath ();
    void setPath (std::wstring newPath);

    virtual int getFileAttrs (std::wstring path, fileInfo& retFi)=0;
    virtual int renameFile(std::wstring& oldName, std::wstring& newName) = 0; //return 0 on success
    virtual void initialNotification() = 0; // file Listing
    virtual void stopMonitor () = 0;
    virtual int runMonitor() = 0;
    virtual int restartMonitor() = 0;
    virtual bool dirExists(std::wstring path) = 0;
    virtual ~fileWatcher();

private:
    fileObserver * pFo;
    std::wstring sPath;
};

#endif // FILEWATCHER_H
