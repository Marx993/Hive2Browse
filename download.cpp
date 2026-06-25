#include "download.h"
#include <QFileDialog>
#include <QDebug>

DownloadManager::DownloadManager(QObject *parent)
    : QObject(parent)
{
}

void DownloadManager::handleDownload(QWebEngineDownloadRequest *download)
{
    QString path = QFileDialog::getSaveFileName(
        nullptr,
        "Save File",
        download->downloadFileName()
    );

    if (path.isEmpty()) {
        download->cancel();
        return;
    }

    download->setDownloadDirectory(QFileInfo(path).path());
    download->setDownloadFileName(QFileInfo(path).fileName());

    download->accept();

    qDebug() << "Downloading to:" << path;
}
