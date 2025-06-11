// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "../client/OAIStoreApi.h"

#include <QtCore/qdebug.h>
#include <QtTest/qtest.h>

using namespace std::chrono_literals;

namespace OpenAPI {
const QDateTime TestDate(QDateTime::fromString("1.30.1", "M.d.s"));

class StoreApiTests : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void placeOrderTest();
    void getOrderByIdTest();
    void getInventoryTest();
    void deleteOrderTest();
    void timeoutTest();
};

void StoreApiTests::placeOrderTest() {
    OAIStoreApi api;
    bool orderPlaced = false;
    OAIOrder order;
    order.setId(500);
    order.setQuantity(10);
    order.setPetId(10000);
    order.setComplete(false);
    order.setStatus("shipping");
    order.setShipDate(QDateTime::currentDateTime());
    api.placeOrder(order, this, [&](QRestReply &reply, OAIOrder &respval) {
        if ((orderPlaced = reply.isSuccess())) {
            QCOMPARE(respval.getShipDate(), TestDate);
        } else {
            qDebug() << "Error happened while issuing request : " << reply.errorString();
        }
    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(orderPlaced, true, 14000);
}

void StoreApiTests::getOrderByIdTest() {
    OAIStoreApi api;
    api.setApiKey("api_key_2","testKey");
    bool orderFetched = false;
    api.getOrderById(500, nullptr, [&](QRestReply &reply, OAIOrder &respval) {
        if ((orderFetched = reply.isSuccess())) {
            QVERIFY(respval.getPetId() == 10000);
            QVERIFY(respval.getId() == 500);
        } else {
            qDebug() << "Error happened while issuing request : " << reply.errorString();
        }
    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(orderFetched, true, 14000);
}

void StoreApiTests::getInventoryTest() {
    OAIStoreApi api;
    api.setApiKey("api_key","special-key");
    bool inventoryFetched = false;
    api.getInventory(this, [&](QRestReply &reply, QMap<QString, qint32> respval) {
        if ((inventoryFetched = reply.isSuccess())) {
            for (const auto &key : respval.keys()) {
                qDebug() << (key) << " Quantities " << respval.value(key);
            }
        } else {
            qDebug() << "Error happened while issuing request : " << reply.errorString();
        }
    });

    QTRY_COMPARE_EQ_WITH_TIMEOUT(inventoryFetched, true, 14000);
}

void StoreApiTests::deleteOrderTest()
{
    OAIStoreApi api;
    bool orderPlaced = false;
    OAIOrder order;
    order.setId(600);
    order.setQuantity(10);
    order.setPetId(20000);
    order.setComplete(false);
    order.setStatus("shipping");
    order.setShipDate(QDateTime::currentDateTime());
    api.placeOrder(order, this, [&](QRestReply &reply, OAIOrder &respval) {
        if ((orderPlaced = reply.isSuccess())) {
            QCOMPARE(respval.getShipDate(), TestDate);
        } else {
            qDebug() << "Error happened while issuing request : " << reply.errorString();
        }
    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(orderPlaced, true, 14000);

    bool orderDeleted = false;
    // delete existing order
    api.deleteOrder(QString::number(order.getId()), this, [&](QRestReply &reply) {
        if (!(orderDeleted = reply.isSuccess())) {
            qDebug() << "Error happened while issuing request : " << reply.errorString();
        }
    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(orderDeleted, true, 14000);

    orderDeleted = false;
    // try to delete NOT existing order id = 33333
    api.deleteOrder("33333", this, [&](QRestReply &reply) {
        if (!(orderDeleted = reply.isSuccess())) {
            qDebug() << "Error happened while issuing request 'deleteOrder(33333)': " << reply.errorString();
        }
    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(orderDeleted, true, 14000);
}

void StoreApiTests::timeoutTest()
{
    OAIOrder order;
    order.setId(600);
    order.setQuantity(10);
    order.setPetId(20000);
    order.setComplete(false);
    order.setStatus("shipping");
    order.setShipDate(QDateTime::currentDateTime());

    OAIStoreApi api;
    bool orderPlaced = false;
    api.placeOrder(order, this, [&](QRestReply &summary) {
        orderPlaced = summary.isSuccess();
    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(orderPlaced, true, 14000);

    bool orderDeleted = false;
    QNetworkReply::NetworkError netError;
    QString errorStr;

    // set a 5 sec timeout, the test server should be able to answer
    // within the timeout (the server sleeps for 1 sec before sending the response)
    api.setTimeOut(5000ms);
    // try to delete NOT existing order id = 33333
    api.deleteOrder("33333", this, [&](QRestReply &reply) {
        if (!(orderDeleted = reply.isSuccess())) {
            qDebug() << "Error happened while issuing request: " << reply.errorString();
        }
    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(orderDeleted, true, 14000);

    orderDeleted = true;
    // decrease the timeout to 0,1 sec, the server shouldn't be able
    // to response in time (the server sleeps for 1 sec before sending the response)
    api.setTimeOut(100ms);
    // delete existing order
    api.deleteOrder(QString::number(order.getId()), this, [&](QRestReply &reply) {
        if (!(orderDeleted = reply.isSuccess())) {
            qDebug() << "Error happened while issuing request : " << reply.errorString();
            netError = reply.error();
            errorStr = reply.errorString();
        }
    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(orderDeleted, false, 14000);
    QVERIFY2(netError == QNetworkReply::OperationCanceledError,
             "Transfers are caneled if no bytes are transferred before the timeout expires.");
    QVERIFY2(errorStr == "Operation canceled", "Operation expected be canceled.");
}
} // OpenAPI

QTEST_MAIN(OpenAPI::StoreApiTests)
#include "StoreApiTests.moc"
