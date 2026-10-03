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
    qputenv("QT_QPA_PLATFORM", "wayland");

    QApplication app(argc, argv);

    QWidget panel;
    panel.setWindowTitle("XFCE5 Panel");
    panel.setFixedHeight(40);
    panel.setStyleSheet("background-color: #2e3440; color: #eceff4; font-family: sans-serif;");
    
    QHBoxLayout *layout = new QHBoxLayout(&panel);
    layout->setContentsMargins(10, 0, 10, 0);

    QPushButton *menuBtn = new QPushButton("❖ XFCE5 Menu");
    menuBtn->setStyleSheet(
        "QPushButton { background-color: #5e81ac; color: white; font-weight: bold; padding: 6px 15px; border-radius: 4px; border: none; }"
        "QPushButton:hover { background-color: #81a1c1; }"
    );
    QObject::connect(menuBtn, &QPushButton::clicked, []() {
        QProcess::startDetached("rofi", QStringList() << "-show" << "drun");
    });

    QPushButton *termBtn = new QPushButton("💻 Terminal");
    termBtn->setStyleSheet(
        "QPushButton { background-color: #434c5e; color: white; padding: 5px 12px; border-radius: 4px; border: none; }"
        "QPushButton:hover { background-color: #4c566a; }"
    );
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
