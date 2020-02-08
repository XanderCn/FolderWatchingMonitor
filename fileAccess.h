#ifndef FILEACCESS_H
#define FILEACCESS_H

#include "util.h"
#include <ctime>

struct fileInfo {

    enum FILE_OPERATION
        {
            ADDED,
            DELETED,
            MODIFIED,
            RENAMED_OLD,
            RENAMED_NEW,
            NONE
    } action;

    std::wstring sFilename;
    std::wstring sFullPath;
    time_t tModified;
    std::string sModified;
    std::string sSize;
    unsigned long long int fSize;
    fileInfo(std::wstring fPath, time_t tMod, unsigned long long int size, FILE_OPERATION act = NONE ) : action(act), sFullPath(fPath), tModified(tMod),  fSize(size)
    {
        for (auto it = fPath.end(); it != fPath.begin(); it --) { // quick cutting of fileName from fullPath
            if (*it == '/' || *it == '\\') {
                sFilename.assign(it+1, fPath.end());
                break;
            }
        }

        setDateModified(tMod);
        sSize.assign(std::to_string(size));
    }

    void setDateModified (time_t timeV) {
        tModified = timeV;
        char buff[20] = {0};
        strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&tModified));
        sModified.assign(buff);
    }
    void setSize(unsigned long long int fileSize) {
        fSize = fileSize;
        sSize.assign(std::to_string(fSize));
    }
};

typedef std::pair<time_t, unsigned long long int> FileAttrs;
FileAttrs fileInfoToFileAttrs(fileInfo& fI);

#endif // FILEACCESS_H
