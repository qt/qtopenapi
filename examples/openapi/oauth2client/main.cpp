// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "profileclient.h"

#include <QtCore/QProcess>
#include <QtWidgets/QApplication>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName(u"The Qt Company"_s);
    app.setOrganizationDomain(u"qt.io"_s);
    app.setApplicationName(u"Qt OpenAPI OAuth 2.0 client example"_s);

    ProfileClient client;

    QWidget window;
    window.setWindowTitle(QApplication::applicationName());
    auto *layout = new QVBoxLayout(&window);
    auto *logInButton = new QPushButton(QApplication::tr("Log in"), &window);
    auto *profileButton = new QPushButton(QApplication::tr("Get profile"), &window);
    auto *log = new QPlainTextEdit(&window);
    log->setReadOnly(true);
    layout->addWidget(logInButton);
    layout->addWidget(profileButton);
    layout->addWidget(log);

    // The same button logs in and out. Get profile stays enabled, so that
    // requesting the profile without authorization can be tried as well.
    QObject::connect(logInButton, &QPushButton::clicked, &client, [&client] {
        if (client.isLoggedIn())
            client.logOut();
        else
            client.logIn();
    });
    QObject::connect(&client, &ProfileClient::loggedIn, logInButton, [logInButton] {
        logInButton->setText(QApplication::tr("Log out"));
    });
    QObject::connect(&client, &ProfileClient::loggedOut, logInButton, [logInButton] {
        logInButton->setText(QApplication::tr("Log in"));
    });
    QObject::connect(profileButton, &QPushButton::clicked, &client, &ProfileClient::fetchProfile);
    QObject::connect(&client, &ProfileClient::message, log, &QPlainTextEdit::appendPlainText);

    // The demo authorization and resource server is a Go program in the
    // server subdirectory. CMake builds it if Go is available and passes its
    // location as SERVER_PATH. Its log output ends up in the window.
    QProcess server;
#ifdef SERVER_PATH
    server.setWorkingDirectory(QStringLiteral(SERVER_FOLDER));
    server.setProcessChannelMode(QProcess::MergedChannels);
    QObject::connect(&server, &QProcess::readyReadStandardOutput, log, [&server, log] {
        log->appendPlainText(QString::fromUtf8(server.readAllStandardOutput()).trimmed());
    });
    server.start(QStringLiteral(SERVER_PATH));
    if (!server.waitForStarted())
        log->appendPlainText(QApplication::tr("Could not start the demo server: %1")
                                     .arg(server.errorString()));
#else
    log->appendPlainText(QApplication::tr("Go was not found when the example was configured. "
                                          "Run 'go run .' in the server directory first."));
#endif

    window.resize(560, 400);
    window.show();
    const int result = app.exec();

    if (server.state() == QProcess::Running) {
        server.kill();
        server.waitForFinished();
    }
    return result;
}
