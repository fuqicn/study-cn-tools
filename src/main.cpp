#include <QApplication>
#include <QSettings>
#include <QDir>
#include <QTimer>
#include <QLocalServer>
#include <QLocalSocket>
#include <QStandardPaths>
#include "mainwindow.h"
#include "firstrundialog.h"
#include "tutorialdialog.h"

static QString settingsFilePath()
{
    QString appDir = QApplication::applicationDirPath();
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/DefenseEdu";
    QString progDataPath = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/DefenseEdu";

    QString scopeFile = appDir + "/.install-scope";
    if (QFile::exists(scopeFile)) {
        QFile f(scopeFile);
        if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QString scope = QString::fromUtf8(f.readAll()).trimmed();
            f.close();
            if (scope == "all") {
                QDir().mkpath(progDataPath);
                return progDataPath + "/settings.ini";
            }
        }
    }

    QDir().mkpath(appDataPath);
    return appDataPath + "/settings.ini";
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("国防安全科普教育软件");
    app.setOrganizationName("DefenseEdu");
    app.setOrganizationDomain("defense-edu.local");

    // Single-instance check using QLocalServer — wake up existing window
    QString serverName = "defense-edu-single-instance-" + QString::fromLocal8Bit(qgetenv("USERNAME"));

    QLocalSocket socket;
    socket.connectToServer(serverName);
    if (socket.waitForConnected(500)) {
        socket.write("show");
        socket.waitForBytesWritten(500);
        socket.disconnectFromServer();
        return 0;
    }

    QLocalServer server;
    QLocalServer::removeServer(serverName);
    server.listen(serverName);

    // 检查是否为首次运行
    QString settingsPath = settingsFilePath();
    QSettings settings(settingsPath, QSettings::IniFormat);
    bool firstRun = !settings.value("firstRunCompleted", false).toBool();

    if (firstRun) {
        // 显示初次设置对话框
        FirstRunDialog setupDialog;
        int result = setupDialog.exec();

        // 如果用户没有完成设置（点击关闭/取消），则不写入任何配置，直接退出
        if (result != QDialog::Accepted) {
            return 0;
        }

        // 用户完成了设置流程 - 检查是否接受了许可协议
        if (setupDialog.licenseAccepted()) {
            // 保存首次运行设置
            settings.setValue("firstRunCompleted", true);
            settings.setValue("theme", setupDialog.theme());
            settings.sync();
        } else {
            // 用户未接受许可协议 - 不写入配置，直接退出
            return 0;
        }

        // 创建主窗口
        MainWindow window;
        window.show();

        QObject::connect(&server, &QLocalServer::newConnection, [&]() {
            QLocalSocket *client = server.nextPendingConnection();
            if (client) { client->readAll(); client->deleteLater(); }
            window.showNormal();
            window.activateWindow();
            window.raise();
        });

        // 首次运行完成后显示交互式教程
        QTimer::singleShot(800, &window, [&window]() {
            window.startTutorial();
        });

        return app.exec();
    }

    // 非首次运行，直接显示主窗口
    MainWindow window;
    window.show();

    QObject::connect(&server, &QLocalServer::newConnection, [&]() {
        QLocalSocket *client = server.nextPendingConnection();
        if (client) { client->readAll(); client->deleteLater(); }
        window.showNormal();
        window.activateWindow();
        window.raise();
    });

    return app.exec();
}
