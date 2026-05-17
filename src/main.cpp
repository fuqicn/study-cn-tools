#include <QApplication>
#include <QSettings>
#include <QTimer>
#include <QLocalServer>
#include <QLocalSocket>
#include "mainwindow.h"
#include "firstrundialog.h"
#include "tutorialdialog.h"
#include "settingsmanager.h"

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
    if (!server.listen(serverName)) {
        qWarning("QLocalServer listen failed: %s", qPrintable(server.errorString()));
    }

    // 创建主窗口（复用同一实例，避免重复代码）
    MainWindow window;
    window.show();

    QObject::connect(&server, &QLocalServer::newConnection, [&]() {
        QLocalSocket *client = server.nextPendingConnection();
        if (client) { client->readAll(); client->deleteLater(); }
        window.showNormal();
        window.activateWindow();
        window.raise();
    });

    // 检查是否为首次运行
    QSettings settings(SettingsManager::defaultSettingsPath(), QSettings::IniFormat);
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

        // 首次运行完成后显示交互式教程
        QTimer::singleShot(800, &window, [&window]() {
            window.startTutorial();
        });
    }

    return app.exec();
}
