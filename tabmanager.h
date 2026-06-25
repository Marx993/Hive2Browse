#pragma once

#include <QTabWidget>
#include <QWebEngineView>
#include <QWebEngineProfile>

class TabManager : public QTabWidget {
public:
    TabManager(QWidget *parent = nullptr);

    QWebEngineView* current();

    void addTabPage(const QUrl &url, const QString &title, QWebEngineProfile *profile);
};
