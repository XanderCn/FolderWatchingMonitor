
#include "util.h"
#include "simpleGUI.h"
#include "fileAccess.h"
#ifdef Q_OS_WINDOWS
#include "winFileWatcher.h"
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    simpleGUI sG;
    std::shared_ptr<fileWatcher> sptrFW(new winFileWatcher(&sG));
    sG.setFileWatcher(sptrFW);

    sG.show();

    return a.exec();
}
