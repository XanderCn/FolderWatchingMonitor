#include "simpleGUI.h"

simpleGUI::simpleGUI(QWidget *parent) : QMainWindow(parent){

    x = 800;// yes, app window layout is hardcoded
    y = 600;
    this->resize(x,y);
    pExitBtn = new QPushButton( "Bye bye, world!", this);
    pExitBtn->setGeometry(x - 170, y - 50, 150, 20);
    QObject::connect(pExitBtn, SIGNAL (clicked()), QApplication::instance(), SLOT (quit()));

    pFileCountText = new QLabel(this);
    pFileCountText -> setGeometry(x - 170, y - 120, 150, 30);
    pFileCountText -> setText("Total: 0 files");

    pWatchDiretory = new QTextEdit(this);
    pWatchDiretory ->setGeometry(10, 10,570, 30);

    pNewDir = new QPushButton ("Watch this!", this);
    pNewDir -> setGeometry(630, 10, 150, 30);

    QObject::connect(pNewDir, SIGNAL (clicked()), this, SLOT (newDirWatch()));

    pLogText = new QPlainTextEdit(this);
    pLogText -> setReadOnly(true);
    pLogText ->setGeometry(10, 470, 600, 100);

    QStringList headers;
    headers.append("Name");
    headers.append("Date Modified");
    headers.append("Size, bytes");
    pTableWidget = new QTableWidget(0,3, this);

    pTableWidget->setGeometry(10, 60, 750, 390);
    pTableWidget->setShowGrid(true);
    pTableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    pTableWidget->setHorizontalHeaderLabels(headers);
    pTableWidget->horizontalHeader()->setStretchLastSection(true);
    pTableWidget->setColumnWidth(0,330);
    pTableWidget->setColumnWidth(1,120);
    QObject::connect(pTableWidget, SIGNAL (cellDoubleClicked(int,int)), this, SLOT (fileNameClicked(int, int)));
    QObject::connect(pTableWidget, SIGNAL (cellChanged(int, int)), this, SLOT (fileNameChanged(int, int)));
}

simpleGUI::~simpleGUI(){
    delete pExitBtn;
    delete pWatchDiretory;
    delete pLogText;
    delete pFileCountText;
    delete pTableWidget;
    delete pNewDir;
}

void simpleGUI::logMessage(std::wstring msg){
    std::lock_guard<std::mutex> lock(logMutex);
    pLogText->appendPlainText(QString::fromStdWString(msg));
    pLogText->ensureCursorVisible();
}


void simpleGUI::fileAdded(struct fileInfo& fI) {
    insertRow(fI);
    updateFileNum();
}

void simpleGUI::fileDeleted(struct fileInfo& fI) {
    auto found = pTableWidget->findItems(QString::fromStdWString(fI.sFilename), Qt::MatchFixedString);
    if (!found.isEmpty()) {
        int row = found.first()->row();
        if (row != -1) {
            pTableWidget->removeRow(row);
            updateFileNum();
        }
    }
}

void simpleGUI::fileModified(struct fileInfo& fI) {
    auto found = pTableWidget->findItems(QString::fromStdWString(fI.sFilename), Qt::MatchFixedString);
    if (!found.isEmpty()) {
        int row = found.first()->row();
        if (row != -1) {
            pTableWidget->setItem(row,1, new QTableWidgetItem(fI.sModified.c_str()));
            pTableWidget->setItem(row,2, new QTableWidgetItem(fI.sSize.c_str()));
        }
    }
}

void simpleGUI::fileRenamedOld(struct fileInfo& fI) {
    auto found = pTableWidget->findItems(QString::fromStdWString(fI.sFilename), Qt::MatchFixedString);
    if (!found.isEmpty()) {
        int row = found.first()->row();
        if (row != -1) {
            pTableWidget->setItem(row,0, new QTableWidgetItem("???"));
        }
    }
}

void simpleGUI::fileRanamedNew(struct fileInfo& fI) {
    auto found = pTableWidget->findItems("???", Qt::MatchFixedString);
    if (!found.isEmpty()) {
        int row = found.first()->row();
        if (row != -1) {
            pTableWidget->setItem(row,0, new QTableWidgetItem(QString::fromStdWString(fI.sFilename)));
            logMessage(L"File renamed by user: " + fI.sFilename);
        }
    }
}

void simpleGUI::fileListed(struct fileInfo& fI) {
    insertRow(fI);
    updateFileNum();
}

void simpleGUI::insertRow (fileInfo &pFI) {
    pTableWidget->insertRow ( pTableWidget->rowCount() );

    pTableWidget->setItem   ( pTableWidget->rowCount()-1, 0, new QTableWidgetItem(QString::fromStdWString(pFI.sFilename)));

    QTableWidgetItem * column2 = new QTableWidgetItem(pFI.sModified.c_str());
    column2->setFlags(column2->flags() ^ Qt::ItemIsEditable);
    pTableWidget->setItem   ( pTableWidget->rowCount()-1, 1,column2);
    QTableWidgetItem * column3 = new QTableWidgetItem(pFI.sSize.c_str());
    column3->setFlags(column3->flags() ^ Qt::ItemIsEditable);
    pTableWidget->setItem   ( pTableWidget->rowCount()-1, 2, column3);

}

void simpleGUI::updateFileNum() {
    std::string countInfo("Total: " + std::to_string(pTableWidget->rowCount()) + " files");
    pFileCountText->setText(countInfo.c_str());
}

void simpleGUI::fileNameChanged (int row, int column) { // catch here file renamed by users
    if (fileRenamed.empty()) return; //if no name buffered, then it's updateInfo from fileWatcher
    std::wstring newFileName = pTableWidget->item(row, column)->text().toStdWString();

    if ( this->getFileWatcher()->renameFile(fileRenamed, newFileName) ) { //if fails
        pTableWidget->setItem(row,column, new QTableWidgetItem(QString::fromStdWString(fileRenamed)));
        logMessage(L"Failed to rename file to " + newFileName);
    }
    fileRenamed.clear();
}

void simpleGUI::fileNameClicked (int row, int column ) {
    fileRenamed = pTableWidget->item(row, column)->text().toStdWString();
}

void simpleGUI::newDirWatch() {

    if (this->getFileWatcher() == nullptr) {
        logMessage(L"fileWatcher object not set, it wouldn't ever work.");
        return;
    }

    std::wstring path(pWatchDiretory->toPlainText().toStdWString());
    pTableWidget->setRowCount(0);

    this->getFileWatcher()->setPath(path);
    if ( this->getFileWatcher()->dirExists(path) == false ) {
        logMessage(L"Specified directory is NOT AVAILABLE");
        return;
    }

    this->getFileWatcher()->initialNotification();
    if (this->getFileWatcher()->restartMonitor()) {
        logMessage(L"Failed to setup directory watch");
    } else { // to change directory - PLS restart programm
        logMessage(L"Watching directory: " + path);
        pNewDir->setVisible(false);
        pWatchDiretory->setReadOnly(true);
    }
}
