#pragma once

#include <QObject>
#include <QWebEngineDownloadRequest>

class DownloadManager : public QObject {
public:
    DownloadManager(QObject *parent = nullptr);

public slots:
    void handleDownload(QWebEngineDownloadRequest *download);
};
