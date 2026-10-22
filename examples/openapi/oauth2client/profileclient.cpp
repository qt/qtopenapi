// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "profileclient.h"

#include "userprofile.h"

#include <QtGui/QDesktopServices>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QRestReply>
#include <QtCore/QUrl>

using namespace Qt::StringLiterals;
using namespace QtOpenAPI;

// The demo server knows exactly one client. Client IDs of native applications
// are public by design (RFC 8252, section 8.5), so it is simply hardcoded.
static constexpr auto clientId = "qt-oauth2-example"_L1;

ProfileClient::ProfileClient(QObject *parent)
    : QObject(parent)
    // Listen on the loopback interface on a free port. The demo server accepts
    // any loopback redirect URI; a real authorization server usually requires
    // the exact URI, including the port, to be registered.
    , m_replyHandler(QHostAddress(QHostAddress::LocalHost), 0)
{
//! [oauth-configuration]
    // Endpoints and scopes come from the security scheme in oauth2client.yaml.
    // There is no setter for refreshUrl: refresh requests always go to tokenUrl.
    m_oauth.setAuthorizationUrl(QUrl(u"http://127.0.0.1:20500/oauth/authorize"_s));
    m_oauth.setTokenUrl(QUrl(u"http://127.0.0.1:20500/oauth/token"_s));
    m_oauth.setRequestedScopeTokens({ "profile:read"_ba, "email:read"_ba });
    m_oauth.setClientIdentifier(clientId);
    // Refresh the access token before it expires, as long as the server issued
    // a refresh token. The demo server issues tokens that live for 30 seconds.
    m_oauth.setAutoRefresh(true);
//! [oauth-configuration]

    m_replyHandler.setCallbackPath(u"/callback"_s);
    m_oauth.setReplyHandler(&m_replyHandler);

    // The user authorizes the application in the system browser.
    connect(&m_oauth, &QAbstractOAuth::authorizeWithBrowser, this, [](const QUrl &url) {
        QDesktopServices::openUrl(url);
    });

//! [token-handover]
    // Every new access token, from the initial grant or from a refresh, is
    // handed to the generated client. The client sends it as a Bearer token.
    connect(&m_oauth, &QAbstractOAuth::tokenChanged, &m_api, &UsersApi::setBearerToken);
//! [token-handover]

    connect(&m_oauth, &QAbstractOAuth::granted, this, [this] {
        emit message(tr("Authorization granted, the token expires at %1.")
                             .arg(m_oauth.expirationAt().toString(Qt::ISODate)));
        m_loggedIn = true;
        emit loggedIn();
    });

    // A failed refresh, or a token that is no longer usable, puts the flow back
    // to NotAuthenticated. Forget the old token and let the user log in again.
    connect(&m_oauth, &QAbstractOAuth::statusChanged, this, [this](QAbstractOAuth::Status status) {
        if (status == QAbstractOAuth::Status::NotAuthenticated)
            resetLogin(tr("The authorization is no longer valid, please log in again."));
    });

//! [oauth-errors]
    connect(&m_oauth, &QAbstractOAuth::requestFailed, this, [this](QAbstractOAuth::Error error) {
        emit message(tr("OAuth request failed with error %1.").arg(int(error)));
    });
    connect(&m_oauth, &QAbstractOAuth2::serverReportedErrorOccurred, this,
            [this](const QString &error, const QString &description, const QUrl &) {
                emit message(tr("Authorization server reported: %1 (%2)").arg(error, description));
            });
//! [oauth-errors]
}

void ProfileClient::resetLogin(const QString &reason)
{
    m_api.setBearerToken({});
    m_loggedIn = false;
    emit message(reason);
    emit loggedOut();
}

void ProfileClient::logIn()
{
    if (!m_replyHandler.isListening()) {
        emit message(tr("The redirect listener is not running."));
        return;
    }
    emit message(tr("Opening %1 in the browser, redirecting to %2.")
                         .arg(m_oauth.authorizationUrl().toString(), m_replyHandler.callback()));
    m_oauth.grant();
}

void ProfileClient::logOut()
{
    // Clearing the refresh token also stops the automatic refresh.
    m_oauth.setRefreshToken({});
    m_oauth.setToken({});
    resetLogin(tr("Logged out."));
}

//! [api-call]
void ProfileClient::fetchProfile()
{
    m_api.getCurrentUserProfile(this, [this](const QRestReply &reply,
                                             const UserProfile &profile) {
        if (reply.httpStatus() == 401) {
            // The server does not accept the access token any more.
            resetLogin(tr("The access token was rejected, please log in again."));
            return;
        }
        if (!reply.isSuccess()) {
            emit message(tr("getCurrentUserProfile failed with HTTP status %1: %2")
                                 .arg(reply.httpStatus())
                                 .arg(reply.errorString()));
            return;
        }
        const QString email = profile.getEmail().isEmpty() ? tr("not granted")
                                                           : profile.getEmail();
        emit message(tr("Profile of %1 (%2), email: %3, country: %4")
                             .arg(profile.getDisplayName(), profile.getId(), email,
                                  profile.getCountry()));
    });
}
//! [api-call]
