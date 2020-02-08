#include "fileAccess.h"

FileAttrs fileInfoToFileAttrs(fileInfo& fI){
    return FileAttrs(fI.tModified, fI.fSize);
}
