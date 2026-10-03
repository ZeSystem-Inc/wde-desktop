#include <QApplication>
#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDateTime>
#include <QTimer>
#include <QScreen>
#include <QProcess>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QWidget panel;
    panel.setWindowTitle("XFCE5 Panel");
    panel.setFixedHeight(40);
    panel.setStyleSheet("background-color: #2e3440; color: #eceff4;");
    
    QHBoxLayout *layout = new QHBoxLayout(&panel);
    layout->setContentsMargins(10, 0, 10, 0);

    QPushButton *menuBtn = new QPushButton("❖ XFCE5");
    menuBtn->setStyleSheet("font-weight: bold; padding: 6px 15px; background-color: #5e81ac; color: white; border-radius: 4px;");
    QObject::connect(menuBtn, &QPushButton::clicked, []() {
        QProcess::startDetached("rofi", QStringList() << "-show" << "drun");
    });

    QPushButton *termBtn = new QPushButton("💻 Terminal");
    termBtn->setStyleSheet("padding: 5px 10px; background-color: #434c5e; border-radius: 3px;");
    QObject::connect(termBtn, &QPushButton::clicked, []() {
        QProcess::startDetached("xterm", QStringList());
    });

    QLabel *clockLabel = new QLabel();
    clockLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #88c0d0;");
    
    QTimer *timer = new QTimer(&panel);
    QObject::connect(timer, &QTimer::timeout, [clockLabel]() {
        clockLabel->setText(QDateTime::currentDateTime().toString("dd MMM ddd  hh:mm:ss"));
    });
    timer->start(1000);

    layout->addWidget(menuBtn);
    layout->addWidget(termBtn);
    layout->addStretch();
    layout->addWidget(clockLabel);

    QScreen *primaryScreen = QGuiApplication::primaryScreen();
    if (primaryScreen) {
        panel.setFixedWidth(primaryScreen->geometry().width());
    }

    panel.show();
    return app.exec();
}
