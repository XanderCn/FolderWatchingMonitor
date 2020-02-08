#include "fileObserver.h"

fileObserver::fileObserver () : fileAPI(nullptr){}

/*void fileObserver::updateFileInfo(struct fileInfo& fI) {
    std::scoped_lock<std::mutex> lock(notifyMutex);
    updateFileList(fI);
}*/

void fileObserver::updateFileInfo(struct fileInfo& fI) {
    std::scoped_lock<std::mutex> lock(notifyMutex);
    switch (fI.action) {
            case fileInfo::ADDED:
                fileList[fI.sFilename] = FileAttrs(fI.tModified, fI.fSize);
                fileAdded(fI);
                break;
            case fileInfo::DELETED:
                fileList.erase(fI.sFilename);
                fileDeleted(fI);
                break;
            case fileInfo::MODIFIED:
                fileList[fI.sFilename] = FileAttrs(fI.tModified, fI.fSize);
                fileModified(fI);
                break;
            case fileInfo::RENAMED_OLD:
                fileList.erase(fI.sFilename);
                fileRenamedOld(fI);
                break;
            case fileInfo::RENAMED_NEW:
                fileList[fI.sFilename] = FileAttrs(fI.tModified, fI.fSize);
                fileRanamedNew(fI);
                break;
            case fileInfo::NONE:
                fileList[fI.sFilename] = FileAttrs(fI.tModified, fI.fSize);
                fileListed(fI);
                break;
            default:
                break;
    }
}

void fileObserver::setFileWatcher (std::shared_ptr<fileWatcher> ptrFW) {
    fileAPI = ptrFW;
}

std::shared_ptr<fileWatcher> fileObserver::getFileWatcher () {
    return fileAPI;
}

std::map<std::wstring,FileAttrs>* fileObserver::getFileList () {
    return &fileList;
}

