#include "tabmanager.h"
#include <QWebEnginePage>

TabManager::TabManager(QWidget *parent)
    : QTabWidget(parent)
{
    setTabsClosable(true);
    setMovable(true);

    connect(this, &QTabWidget::tabCloseRequested,
        this, [this](int index) {
            QWidget *w = widget(index);
            removeTab(index);
            delete w; // ensures page is destroyed → fixes profile warning
        });
}

QWebEngineView* TabManager::current()
{
    return qobject_cast<QWebEngineView*>(currentWidget());
}

void TabManager::addTabPage(const QUrl &url, const QString &title, QWebEngineProfile *profile)
{
    QWebEngineView *view = new QWebEngineView();

    view->setPage(new QWebEnginePage(profile, view));
    view->setUrl(url);

    int index = addTab(view, title);
    setCurrentIndex(index);
}
