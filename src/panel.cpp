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
    
    QHBoxLayout *layout = new QHBoxLayout(&panel);
    layout->setContentsMargins(10, 0, 10, 0);

    QPushButton *menuBtn = new QPushButton("XFCE5 Menu");
    menuBtn->setStyleSheet("font-weight: bold; padding: 5px 15px;");
    QObject::connect(menuBtn, &QPushButton::clicked, []() {
        QProcess::startDetached("rofi", QStringList() << "-show" << "drun");
    });

    QLabel *clockLabel = new QLabel();
    clockLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
    
    QTimer *timer = new QTimer(&panel);
    QObject::connect(timer, &QTimer::timeout, [clockLabel]() {
        clockLabel->setText(QDateTime::currentDateTime().toString("dd MMM hh:mm:ss"));
    });
    timer->start(1000);

    layout->addWidget(menuBtn);
    layout->addStretch(); // Ortadaki boşluk
    layout->addWidget(clockLabel);

    QScreen *primaryScreen = QGuiApplication::primaryScreen();
    if (primaryScreen) {
        int screenWidth = primaryScreen->geometry().width();
        panel.setFixedWidth(screenWidth);
    }

    panel.show();

    return app.exec();
}
