#ifndef SIMPLEGUI_H
#define SIMPLEGUI_H

#include "fileAccess.h"
#include "fileObserver.h"
#include <QTextEdit>
#include <QList>
#include <QTableWidgetItem>
#include <QLabel>
#include <winFileWatcher.h>
#include <memory>
#include <QPlainTextEdit>

class simpleGUI: public QMainWindow, public fileObserver {
    Q_OBJECT
public:
    simpleGUI(QWidget *parent = 0);
    virtual ~simpleGUI();

    void logMessage(std::wstring msg);
    void fileAdded(struct fileInfo& fI);
    void fileDeleted(struct fileInfo& fI);
    void fileModified(struct fileInfo& fI);
    void fileRenamedOld(struct fileInfo& fI);
    void fileRanamedNew(struct fileInfo& fI);
    void fileListed(struct fileInfo& fI);

    void insertRow (struct fileInfo& pFI);
    void updateFileNum();

public slots:
    void fileNameChanged (int row, int column);
    void fileNameClicked (int row, int column );
    void newDirWatch();

private:
    int x;
    int y;
    std::wstring fileRenamed;
    QTextEdit * pWatchDiretory;
    QPushButton * pNewDir;
    QPushButton * pExitBtn;
    QPlainTextEdit * pLogText;
    QLabel * pFileCountText;
    QTableWidget * pTableWidget;
    std::mutex logMutex;
};


#endif // SIMPLEGUI_H

