#include <QApplication>
#include <QSettings>
#include <QDesktopServices>
#include <QUrl>
#include <QStandardPaths>
#include <QWidget>
#include <QDockWidget>
#include <QWebEngineProfile>
#include <QWebEnginePage>
#include <QApplication>
#include <QMainWindow>
#include <QWebEngineView>
#include <QToolBar>
#include <QLineEdit>
#include <QAction>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QTimer>
#include <QVariantMap>
#include <QCloseEvent>

#include "download.h"
#include "tabmanager.h"

class HiveBrowser : public QMainWindow {
public:
    HiveBrowser() {

        // ===== PROFILE (FIXED PERSISTENCE) =====
        profile = new QWebEngineProfile("hive_profile", this);

        QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

        profile->setPersistentStoragePath(path + "/storage");
        profile->setCachePath(path + "/cache");
        profile->setPersistentCookiesPolicy(QWebEngineProfile::AllowPersistentCookies);

        // downloads hook (IMPORTANT)
        downloads = new DownloadManager(this);
        connect(profile, &QWebEngineProfile::downloadRequested,
                downloads, &DownloadManager::handleDownload);

        // ===== UI =====
        QWidget *container = new QWidget;
        QVBoxLayout *layout = new QVBoxLayout(container);
        layout->setContentsMargins(0,0,0,0);

        tabs = new TabManager(this);
        layout->addWidget(tabs);

        setCentralWidget(container);

        // ===== TOOLBAR =====
        QToolBar *bar = addToolBar("nav");

        auto addBtn = [&](const QString &text, auto func) {
            QAction *a = bar->addAction(text);
            connect(a, &QAction::triggered, this, func);
        };

        addBtn("◀", [this](){ if (tabs->current()) tabs->current()->back(); });
        addBtn("▶", [this](){ if (tabs->current()) tabs->current()->forward(); });
        addBtn("⟳", [this](){ if (tabs->current()) tabs->current()->reload(); });
        addBtn("⌂", [this](){
            if (tabs->current())
                tabs->current()->setUrl(QUrl("https://duckduckgo.com/html"));
        });

        addBtn("+", [this](){
            tabs->addTabPage(QUrl("https://duckduckgo.com/html"), "New Tab", profile);
        });

        // URL bar
        urlBar = new QLineEdit();
        bar->addWidget(urlBar);

        QAction *go = bar->addAction("Go");
        connect(go, &QAction::triggered, this, &HiveBrowser::loadPage);
        connect(urlBar, &QLineEdit::returnPressed, this, &HiveBrowser::loadPage);

        // ===== FIXED SIGNALS (NO UNIQUE CONNECTION BUG) =====
        connect(tabs, &QTabWidget::currentChanged, this, [this](int){
            auto view = tabs->current();
            if (!view) return;

            urlBar->setText(view->url().toString());

            connect(view, &QWebEngineView::urlChanged, this,
                [this](const QUrl &url) {
                    urlBar->setText(url.toString());
                });

            connect(view, &QWebEngineView::titleChanged, this,
                [this, view](const QString &title) {
                    int i = tabs->indexOf(view);
                    if (i == -1) return;

                    QString t = title;
                    if (t.length() > 20)
                        t = t.left(17) + "...";

                    tabs->setTabText(i, t);
                });
        });

        // ===== SCROLLBAR =====
        scrollBar = new QScrollBar(Qt::Vertical);
        scrollBar->setFixedWidth(18);

        QDockWidget *dock = new QDockWidget("", this);
        dock->setWidget(scrollBar);
        dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
        addDockWidget(Qt::RightDockWidgetArea, dock);

        connect(scrollBar, &QScrollBar::valueChanged, this, [this](int v) {
            if (tabs->current())
                tabs->current()->page()->runJavaScript(
                    QString("window.scrollTo(0,%1);").arg(v));
        });

        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [this]() {
            if (!tabs->current()) return;

            tabs->current()->page()->runJavaScript(R"(
                ({
                    scrollTop: document.documentElement.scrollTop || document.body.scrollTop,
                    scrollHeight: document.documentElement.scrollHeight,
                    clientHeight: document.documentElement.clientHeight
                });
            )", [this](const QVariant &res) {

                auto m = res.toMap();

                int max = m["scrollHeight"].toInt() - m["clientHeight"].toInt();
                if (max < 0) max = 0;

                scrollBar->setMaximum(max);
                scrollBar->setPageStep(m["clientHeight"].toInt());
                scrollBar->setValue(m["scrollTop"].toInt());
            });
        });

        timer->start(30);

        loadSession();
    }

protected:
    void closeEvent(QCloseEvent *e) override {
        saveSession();
        QMainWindow::closeEvent(e);
    }

private:
    TabManager *tabs;
    QLineEdit *urlBar;
    QScrollBar *scrollBar;
    QWebEngineProfile *profile;
    DownloadManager *downloads;

    void loadPage() {
        QString input = urlBar->text();
        if (!input.startsWith("http"))
            input = "https://duckduckgo.com/html/?q=" + input;

        if (tabs->current())
            tabs->current()->setUrl(QUrl(input));
    }

    void saveSession() {
        QSettings s("HiveBrowser","session");

        s.beginWriteArray("tabs");
        int i=0;

        for (int t=0; t<tabs->count(); t++) {
            auto v = qobject_cast<QWebEngineView*>(tabs->widget(t));
            if (!v) continue;

            s.setArrayIndex(i++);
            s.setValue("url", v->url().toString());
        }

        s.endArray();
    }

    void loadSession() {
        QSettings s("HiveBrowser","session");

        int size = s.beginReadArray("tabs");

        if (size == 0) {
            tabs->addTabPage(QUrl("https://duckduckgo.com/html"), "Start", profile);
            return;
        }

        for (int i=0;i<size;i++) {
            s.setArrayIndex(i);
            tabs->addTabPage(QUrl(s.value("url").toString()), "Tab", profile);
        }

        s.endArray();
    }
};
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QApplication::setStyle("Windows");
	QPalette p;
	p.setColor(QPalette::Window, QColor("#C0C0C0"));
	
	p.setColor(QPalette::Base, QColor("#FFFFFF"));
	
	p.setColor(QPalette::Button, QColor("#C0C0C0"));
	
	p.setColor(QPalette::Light, Qt::white);
	
	p.setColor(QPalette::Dark, QColor("#404040"));
	
	p.setColor(QPalette::Text, Qt::black);
	
	p.setColor(QPalette::ButtonText, Qt::black);

	QApplication::setPalette(p);

    HiveBrowser w;
    w.resize(900, 600);
    w.show();

    return app.exec();
}
