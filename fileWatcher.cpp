#include "fileObserver.h"
#include "fileWatcher.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <thread>

//file API to monitor changes in directory or rename files
fileWatcher::fileWatcher(fileObserver* observer, std::wstring path) : pFo(observer), sPath(path) {}
fileWatcher::fileWatcher(fileObserver* observer) : pFo(observer) {}

void fileWatcher::notifyObserver (struct fileInfo& fI) {
    pFo->updateFileInfo(fI);
}

std::wstring fileWatcher::getPath () {
    return sPath;
}

void fileWatcher::setPath (std::wstring newPath){
    sPath = newPath;
}

fileWatcher::~fileWatcher(){
}
