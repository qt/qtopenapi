// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef PROFILECLIENT_H
#define PROFILECLIENT_H

#include "usersapi.h"

#include <QtNetworkAuth/QOAuth2AuthorizationCodeFlow>
#include <QtNetworkAuth/QOAuthHttpServerReplyHandler>

#include <QtCore/QObject>
#include <QtCore/QString>

//! [class]
class ProfileClient : public QObject
{
    Q_OBJECT
public:
    explicit ProfileClient(QObject *parent = nullptr);

    void logIn();
    void logOut();
    bool isLoggedIn() const { return m_loggedIn; }
    void fetchProfile();

signals:
    void loggedIn();
    void loggedOut();
    void message(const QString &text);

private:
    void resetLogin(const QString &reason);

    QOAuth2AuthorizationCodeFlow m_oauth;
    QOAuthHttpServerReplyHandler m_replyHandler;
    QtOpenAPI::UsersApi m_api;
    bool m_loggedIn = false;
};
//! [class]

#endif // PROFILECLIENT_H
