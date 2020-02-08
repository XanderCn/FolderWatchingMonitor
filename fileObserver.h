#ifndef FILEOBSERVER_H
#define FILEOBSERVER_H


#include "fileAccess.h"
#include "fileWatcher.h"
#include <mutex>
#include <map>

 //fileObserver - class to react on any changes, that can come from fileWatcher API, described in class fileWatcher
class fileObserver {
public:
    fileObserver ();

    void updateFileInfo(struct fileInfo& fI);

    virtual void fileAdded(struct fileInfo& fI) = 0;
    virtual void fileDeleted(struct fileInfo& fI) = 0;
    virtual void fileModified(struct fileInfo& fI) = 0;
    virtual void fileRenamedOld(struct fileInfo& fI) = 0;
    virtual void fileRanamedNew(struct fileInfo& fI) = 0;
    virtual void fileListed(struct fileInfo& fI) = 0;

    void setFileWatcher (std::shared_ptr<fileWatcher> watcher);
    std::shared_ptr<fileWatcher> getFileWatcher ();
    std::map<std::wstring,FileAttrs>* getFileList ();


private:
    std::mutex notifyMutex;
    std::wstring sPath;
    std::shared_ptr<fileWatcher> fileAPI;
    std::map<std::wstring,FileAttrs> fileList;
};


#endif // FILEOBSERVER_H
