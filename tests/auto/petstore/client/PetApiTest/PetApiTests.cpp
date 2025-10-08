// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "../client/QtOAIPetApi.h"
#include "../client/QtOAIStoreApi.h"
#include "../client/QtOAIUserApi.h"

#include <QtCore/qobject.h>
#include <QtGui/qimage.h>
#include <QtNetwork/qnetworkrequestfactory.h>
#include <QtNetwork/qrestaccessmanager.h>
#include <QtTest/qtest.h>

namespace QtOpenAPI {
const int REPLY_OK = 200;

class PetApiTests : public QObject {
    Q_OBJECT

    QtOAIPet createRandomPet(const QString &status = "freaky", const QString &name = "monster");
    void connectAddPetApi(QtOAIPetApi *petApi, bool &petCreated);

private Q_SLOTS:
    void findPetsByStatusTest();
    void createAndGetPetTest();
    void updatePetTest();
    void updatePetWithFormTest();
    void deleteCreatedPetByBearerTest();
    void deleteCreatedPetNoBearerTest();
    void uploadPetFileTest();
    void mixedApiCallsTest();
    void getFilesFromServerTest();
    void sslConfigurationTest();
    void setNoBasicLoginAndPasswordTest();
    void getPatientPetsTest();
};

const QString user("User1");
const QString password("1234");

class QtOAIPetApiInheritageTest: public QtOAIPetApi
{
public:
    QtOAIPetApiInheritageTest(QObject *parent = nullptr)
        : QtOAIPetApi(parent){}

    bool m_testCheck = false;
private:
    // Template operation function will call this overriten method
    void addPetWithDataImpl(const QtOAIPet &QtOAIPet, const QObject *context, QtPrivate::QSlotObjectBase *slot) override
    {
        m_testCheck = true;
        QtOAIPetApi::addPetWithDataImpl(QtOAIPet, context, slot);
    }
};

QtOAIPet PetApiTests::createRandomPet(const QString &status, const QString &name) {
    QtOAIPet pet;
    qint64 id = static_cast<long long>(rand());
    pet.setName(name);
    pet.setId(id);
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
    pet.setStatus(status);
QT_WARNING_POP
    pet.setAge(10);
    pet.setPatience(9);
    return pet;
}

void PetApiTests::findPetsByStatusTest() {
    QtOAIPetApi api;
    bool petFound = false;
    bool petCreated = false;
    api.setUsername(user);
    api.setPassword(password);
    api.setApiKey("api_key","special-key");

    QtOAIPet randomPet = createRandomPet();
    QtOAIPet availablePet = createRandomPet("available", "avaialble_pet");
    availablePet.setId(1111);
    QtOAIPet sold_pet = createRandomPet("sold", "sold_pet");
    sold_pet.setId(2222);

    connectAddPetApi(&api, petCreated);
    api.addPet(randomPet);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petCreated, true, 5000);

    petCreated = false;
    api.addPet(availablePet);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petCreated, true, 5000);

    petCreated = false;
    api.addPet(sold_pet);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petCreated, true, 5000);

    connect(&api, &QtOAIPetApi::findPetsByStatusFinished,
            this, [&](QList<QtOAIPet> pets) {
        petFound = true;
        QVERIFY(!pets.isEmpty());
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
        for (const QtOAIPet &pet: pets) {
            qDebug() << "Pet id = " << pet.getId() << "status = " << pet.getStatus();
            QVERIFY(pet.getStatus() == "available" || pet.getStatus() == "sold");
        }
QT_WARNING_POP
    });
    connect(&api, &QtOAIPetApi::findPetsByStatusErrorOccurred,
            this, [&](QNetworkReply::NetworkError, const QString &errorStr) {
        qDebug() << "Error happened while issuing request : " << errorStr;
    });

    api.findPetsByStatus({"available", "sold"});
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petFound, true, 5000);
}

void connectPetByIdApi(QtOAIPetApi *petApi, bool &petFetched, QtOAIPet &petToCheck)
{
    QObject::connect(petApi, &QtOAIPetApi::getPetByIdFinished,
                     petApi, [&petFetched, &petToCheck](QtOAIPet summary) {
        // pet created
        petFetched = true;
        petToCheck = summary;
    });
    QObject::connect(petApi, &QtOAIPetApi::getPetByIdErrorOccurred, petApi,
                     [&](QNetworkReply::NetworkError, const QString &errorStr) {
                         qDebug() << "Error happened while issuing request : " << errorStr;
                     });
}

void PetApiTests::createAndGetPetTest() {
    QtOAIPetApi api;
    api.setUsername(user);
    api.setPassword(password);
    api.setApiKey("api_key","special-key");
    bool petCreated = false;

    const QString petName("Exclusive name");
    QtOAIPet pet = createRandomPet("available", petName);
    qint64 id = pet.getId();

    connect(&api, &QtOAIPetApi::addPetFinished, this, [&](QtOAIPet summary) {
        // pet created
        petCreated = true;
        QCOMPARE(pet, summary);
    });
    connect(&api, &QtOAIPetApi::addPetErrorOccurred, this,
            [&](QNetworkReply::NetworkError, const QString &errorStr) {
        qDebug() << "Error happened while issuing request : " << errorStr;
    });
    api.addPet(pet);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petCreated, true, 14000);

    bool petFetched = false;
    QtOAIPet petToCheck;
    connectPetByIdApi(&api, petFetched, petToCheck);
    api.getPetById(id);

    QTRY_COMPARE_EQ_WITH_TIMEOUT(petFetched, true, 14000);
    QVERIFY2(petToCheck.getName().compare(petName) == 0, "pet isn't found.");

    QtOAIPetApiInheritageTest mockedApi;
    mockedApi.setUsername(user);
    mockedApi.setPassword(password);
    mockedApi.setApiKey("api_key","special-key");
    petCreated = false;
    mockedApi.addPet(pet, this, [&](const QRestReply &reply, const QtOAIPet &newPet) {
        if (!(petCreated = reply.isSuccess())) {
            qWarning() << "Not successful" << reply.errorString();
        } else {
            QCOMPARE(pet, newPet);
        }
    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petCreated, true, 14000);
    QVERIFY2(mockedApi.m_testCheck, "template isn't called with re-implemented impl()");
}

void PetApiTests::updatePetTest() {
    QtOAIPetApi api;
    api.setUsername(user);
    api.setPassword(password);
    QtOAIPet pet = createRandomPet();
    QtOAIPet petToCheck;
    qint64 id = pet.getId();
    bool petAdded = false;

    connect(&api, &QtOAIPetApi::addPetFinished, this, [&](QtOAIPet summary) {
        petAdded = true;
        QCOMPARE(pet, summary);
    });
    connect(&api, &QtOAIPetApi::addPetErrorOccurred,
            this, [&](QNetworkReply::NetworkError, const QString &errorStr) {
        qDebug() << "Error happened while issuing request : " << errorStr;
    });
    // create pet
    api.addPet(pet);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petAdded, true, 5000);

    // fetch it
    bool petFetched = false;
    connectPetByIdApi(&api, petFetched, petToCheck);
    // create pet
    api.getPetById(id);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petFetched, true, 5000);

    // update it
    bool petUpdated = false;
    connect(&api, &QtOAIPetApi::updatePetFinished, this, [&](QtOAIPet summary) {
        petUpdated = true;
        pet = summary;
    });
    connect(&api, &QtOAIPetApi::updatePetErrorOccurred,
            this, [&](QNetworkReply::NetworkError, const QString &errorStr) {
        qDebug() << "Error happened while issuing request : " << errorStr;
    });

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
    // update pet
    petToCheck.setStatus(QString("scary"));
QT_WARNING_POP
    api.updatePet(petToCheck);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petUpdated, true, 5000);

    // api already connected above by connectPetByIdApi
    petFetched = false;
    api.getPetById(id);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petFetched, true, 5000);
    QVERIFY2(pet.getId() == petToCheck.getId(), "pet isn't found");
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
    QVERIFY2(pet.getStatus().compare(petToCheck.getStatus()) == 0, "status isn't updated");
QT_WARNING_POP
}

void PetApiTests::updatePetWithFormTest() {
    QtOAIPetApi api;
    api.setUsername(user);
    api.setPassword(password);
    QtOAIPet pet = createRandomPet();
    QtOAIPet petToCheck;
    qint64 id = pet.getId();

    // create pet
    bool petAdded = false;
    connect(&api, &QtOAIPetApi::addPetFinished, this, [&](QtOAIPet summary) {
        petAdded = true;
        QCOMPARE(pet, summary);
    });
    connect(&api, &QtOAIPetApi::addPetErrorOccurred,
            this, [&](QNetworkReply::NetworkError, const QString &errorStr) {
        qDebug() << "Error happened while issuing request : " << errorStr;
    });

    api.addPet(pet);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petAdded, true, 5000);

    // fetch it
    bool petFetched = false;
    connectPetByIdApi(&api, petFetched, petToCheck);
    api.getPetById(id);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petFetched, true, 5000);

    // update it
    bool petUpdated = false;
    connect(&api, &QtOAIPetApi::updatePetWithFormFinished, this, [&]() {
        petUpdated = true;
    });
    connect(&api, &QtOAIPetApi::updatePetWithFormErrorOccurred,
            this, [&](QNetworkReply::NetworkError, const QString &errorStr) {
        qDebug() << "Error happened while issuing request : " << errorStr;
    });

    QString name("gorilla");
    api.updatePetWithForm(id, name);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petUpdated, true, 5000);

    // fetch it
    petFetched = false;
    api.getPetById(id);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petFetched, true, 5000);
}

// Bearer Authentication:
// see https://swagger.io/docs/specification/v3_0/authentication/bearer-authentication
void PetApiTests::deleteCreatedPetByBearerTest()
{
    QtOAIPetApi api;
    api.setUsername(user);
    api.setPassword(password);
    api.setBearerToken("BEARER-TOKEN");
    QtOAIPet pet = createRandomPet();
    QtOAIPet petToCheck;
    qint64 id = pet.getId();

    // create pet
    bool petAdded = false;
    connect(&api, &QtOAIPetApi::addPetFinished, this, [&](QtOAIPet summary) {
        petAdded = true;
        QCOMPARE(pet, summary);
    });
    connect(&api, &QtOAIPetApi::addPetErrorOccurred,
            this, [&](QNetworkReply::NetworkError, const QString &errorStr) {
        qDebug() << "Error happened while issuing request : " << errorStr;
    });

    api.addPet(pet);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petAdded, true, 5000);

    // delete created pet
    bool petDeleted = false;
    connect(&api, &QtOAIPetApi::deletePetFinished, this, [&]() {
        petDeleted = true;
    });
    connect(&api, &QtOAIPetApi::deletePetErrorOccurred,
            this, [&](QNetworkReply::NetworkError, const QString &errorStr) {
        qDebug() << "Error happened while issuing request : " << errorStr;
    });

    api.deletePet(id);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petDeleted, true, 5000);
}

// Bearer Authentication:
// see https://swagger.io/docs/specification/v3_0/authentication/bearer-authentication
void PetApiTests::deleteCreatedPetNoBearerTest()
{
    QtOAIPetApi apiNoBearer;
    // delete created pet
    bool petNotDeleted = false;
    int errorOccurredCounter = 0;
    connect(&apiNoBearer, &QtOAIPetApi::deletePetFinished, this, [&]() {
        qDebug() << "deletePetFinished: No error happened";
    });
    connect(&apiNoBearer, &QtOAIPetApi::deletePetErrorOccurred,
            this, [&](QNetworkReply::NetworkError, const QString &errorStr) {
                qDebug() << "deletePetErrorOccurred: expected error happened while issuing the request : " << errorStr;
                petNotDeleted = true;
                errorOccurredCounter++;
            });

    apiNoBearer.deletePet(0);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petNotDeleted, true, 5000);
    // The slot is called only once, no duplication anymore
    QTRY_COMPARE_EQ_WITH_TIMEOUT(errorOccurredCounter, 1, 5000);
}

void PetApiTests::uploadPetFileTest()
{
    QtOAIPetApi api;
    api.setUsername(user);
    api.setPassword(password);
    QtOAIPet pet = createRandomPet();
    QtOAIPet petToCheck;
    qint64 id = pet.getId();

    // create pet
    bool petAdded = false;
    connect(&api, &QtOAIPetApi::addPetFinished, this, [&](QtOAIPet summary) {
        petAdded = true;
        QCOMPARE(pet, summary);
    });
    connect(&api, &QtOAIPetApi::addPetErrorOccurred,
            this, [&](QNetworkReply::NetworkError, const QString &errorStr) {
        qDebug() << "Error happened while issuing request : " << errorStr;
    });

    api.addPet(pet);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petAdded, true, 5000);

    // delete created pet
    bool petFileUploaded = false;
    QString type, message;
    qint32 code = -100;
    connect(&api, &QtOAIPetApi::uploadFileFinished, this, [&](QtOAIApiResponse response) {
        petFileUploaded = true;
        type = response.getType();
        code = response.getCode();
        message = response.getMessage();
        qWarning() << type << code << message;
    });
    connect(&api, &QtOAIPetApi::uploadFileErrorOccurred,
            this, [&](QNetworkReply::NetworkError, const QString &errorStr) {
        qDebug() << "Error happened while issuing request : " << errorStr;
    });

    QtOAIHttpFileElement fileElement;
    fileElement.setFileName(":/PetStore/file-for-uploading.txt");
    fileElement.setMimeType("txt");
    fileElement.setVariableName("Variable=100");
    fileElement.setRequestFileName(":/PetStore/file-for-uploading.txt");

    api.uploadFile(id, QString("metadata-info"), fileElement);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petFileUploaded, true, 5000);
    QVERIFY2(code != -100, "didn't finish within timeout");
}

void PetApiTests::connectAddPetApi(QtOAIPetApi *petApi, bool &petCreated)
{
    QObject::connect(petApi, &QtOAIPetApi::addPetFinished, this, [&petCreated]() {
        // pet created
        petCreated = true;
    });
    QObject::connect(petApi, &QtOAIPetApi::addPetErrorOccurred,
                     this, [&] (QNetworkReply::NetworkError, const QString &errorStr) {
                         qDebug() << "Error happened while issuing request : " << errorStr;
                     });
}

void PetApiTests::mixedApiCallsTest()
{
    std::shared_ptr<QNetworkAccessManager> manager = std::make_shared<QNetworkAccessManager>();
    std::shared_ptr<QRestAccessManager> restManager
        = std::make_shared<QRestAccessManager>(manager.get(), this);
    std::shared_ptr<QNetworkRequestFactory> factory = std::make_shared<QNetworkRequestFactory>();;

    QtOAIPetApi api1, api2;
    api1.setUsername(user);
    api1.setPassword(password);
    api2.setUsername(user);
    api2.setPassword(password);
    QtOAIStoreApi apiStore;
    QtOAIUserApi apiUser;

    // test resource re-setting
    apiStore.setNetworkAccessResources(manager, restManager);
    apiStore.setNetworkRequestFactory(factory);
    apiUser.setNetworkAccessResources(manager, restManager);
    apiUser.setNetworkRequestFactory(factory);
    QtOAIPet pet1 = createRandomPet();
    QtOAIPet pet2 = createRandomPet();
    QtOAIPet petToCheck;

    api1.setApiKey("api_key","special-key");
    bool petCreated = false;
    connectAddPetApi(&api1, petCreated);
    connectAddPetApi(&api2, petCreated);

    api1.addPet(pet1);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petCreated, true, 14000);

    petCreated = false;
    api2.addPet(pet2);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petCreated, true, 14000);

    bool inventoryFetched = false;
    connect(&apiStore, &QtOAIStoreApi::getInventoryFinished,
            this, [&](QMap<QString, qint32> status) {
        inventoryFetched = true;
        for (const auto &key : status.keys()) {
            qDebug() << (key) << " Quantities " << status.value(key);
        }
    });
    connect(&apiStore, &QtOAIStoreApi::getInventoryErrorOccurred,
            this, [&] (QNetworkReply::NetworkError, const QString &errorStr) {
                qDebug() << "Error happened while issuing request : " << errorStr;
            });

    apiStore.getInventory();
    QTRY_COMPARE_EQ_WITH_TIMEOUT(inventoryFetched, true, 14000);

    bool userLoggedOut = false;
    connect(&apiUser, &QtOAIUserApi::logoutUserFinished, this, [&]() {
        userLoggedOut = true;
    });
    connect(&apiUser, &QtOAIUserApi::logoutUserErrorOccurred,
            this, [&] (QNetworkReply::NetworkError, const QString &errorStr) {
                qDebug() << "Error happened while issuing request : " << errorStr;
            });

    apiUser.logoutUser(QJsonValue("johndoe"));
    QTRY_COMPARE_EQ_WITH_TIMEOUT(userLoggedOut, true, 14000);
}

void PetApiTests::getFilesFromServerTest()
{
    QtOAIPetApi api;
    api.setUsername(user);
    api.setPassword(password);
    QtOAIPet pet = createRandomPet();
    QtOAIPet petToCheck;
    qint64 id = pet.getId();

    // create pet
    bool petAdded = false;
    connect(&api, &QtOAIPetApi::addPetFinished, this, [&](QtOAIPet summary) {
        petAdded = true;
        QCOMPARE(pet, summary);
    });
    connect(&api, &QtOAIPetApi::addPetErrorOccurred,
            this, [&](QNetworkReply::NetworkError, const QString &errorStr) {
        qDebug() << "Error happened while issuing request : " << errorStr;
    });

    api.addPet(pet);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petAdded, true, 14000);

    // get pet json info file
    bool petFileDownloaded = false;
    connect(&api, &QtOAIPetApi::getJsonFileFinished,
            this, [&](QtOAIHttpFileElement summary) {
        petFileDownloaded = true;
        QCOMPARE("response.json", summary.m_requestFilename);
        QJsonObject fileContent = summary.asJsonValue().toObject();
        QCOMPARE(fileContent.value("file-name").toString(), QString("Hi, I am a response!"));
        QCOMPARE(fileContent.value("value").toInt(), 81);
    });
    connect(&api, &QtOAIPetApi::getJsonFileErrorOccurred,
            this, [&](QNetworkReply::NetworkError, const QString &errorStr) {
        qDebug() << "Error happened while issuing request : " << errorStr;
    });

    api.getJsonFile(id);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petFileDownloaded, true, 5000);

    // get PNG file
    bool petPngDownloaded = false;
    connect(&api, &QtOAIPetApi::findPetsImageByIdFinished, this, [&](QString summary) {
        petPngDownloaded = true;
        // test file size
        QCOMPARE(summary.size(), 8868);
        // test we can load it normally into QImage object
        QVERIFY2(!QImage::fromData(QByteArray::fromBase64(summary.toUtf8()), "png").isNull(), "Image isn't loaded.");
    });
    connect(&api, &QtOAIPetApi::findPetsImageByIdErrorOccurred,
            this, [&](QNetworkReply::NetworkError, const QString &errorStr) {
        qDebug() << "Error happened while issuing request : " << errorStr;
    });

    api.findPetsImageById(id);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petPngDownloaded, true, 5000);
}

void PetApiTests::sslConfigurationTest()
{
#if QT_CONFIG(ssl)
    QtOAIPetApi api;
    std::shared_ptr<QNetworkRequestFactory> factory = std::make_shared<QNetworkRequestFactory>();
    auto config = QSslConfiguration::defaultConfiguration();
    config.setProtocol(QSsl::TlsV1_2OrLater);
    api.setNetworkRequestFactory(factory);
    api.setSslConfiguration(config);
    QCOMPARE(factory->sslConfiguration(), config);
#else
    QSKIP("Skipping SSL test, not supported by current Qt configuration");
#endif // ssl
}

// Basic login:password - headers authentication,
// see https://swagger.io/docs/specification/v3_0/authentication/basic-authentication
// Testing if no data provided
void PetApiTests::setNoBasicLoginAndPasswordTest()
{
    QtOAIPetApi apiNoData;
    QtOAIPet pet = createRandomPet();
    bool petNotAdded = false;
    connect(&apiNoData, &QtOAIPetApi::addPetFinished, this, [&]() {
        qDebug() << "addPetFinished: no error happened.";
    });
    connect(&apiNoData, &QtOAIPetApi::addPetErrorOccurred,
            this, [&](QNetworkReply::NetworkError, const QString &errorStr) {
        petNotAdded = true;
        qDebug() << "addPetErrorOccurred: expected error happened while issuing the request : " << errorStr;
    });

    apiNoData.addPet(pet);
    QTRY_COMPARE_EQ_WITH_TIMEOUT(petNotAdded, true, 5000);
}

void PetApiTests::getPatientPetsTest()
{
    QtOAIPetApi api;
    api.setUsername(user);
    api.setPassword(password);

    QtOAIPet pet1 = createRandomPet("sold", "Poops_Baraboops");
    pet1.setAge(100); // oddly old cat though

    QtOAIPet pet2 = createRandomPet("notsold", "TheThing");
    pet2.setAge(33); // also oddly old cat though
    pet2.setPatience(0);

    // create pet1
    bool operationStatus = false;
    api.addPet(pet1, nullptr, [&](const QRestReply &reply, const QtOAIPet &summary) {
        if (!(operationStatus = reply.isSuccess()))
            qDebug() << "Error happened while issuing request : " << reply.errorString();

        QVERIFY(operationStatus);
        QCOMPARE(summary, pet1);

    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(operationStatus, true, 14000);

    // create pet2
    operationStatus = false;
    api.addPet(pet2, nullptr, [&](const QRestReply &reply, const QtOAIPet &summary) {
        if (!(operationStatus = reply.isSuccess()))
            qDebug() << "Error happened while issuing request : " << reply.errorString();

        QVERIFY(operationStatus);
        QCOMPARE(summary, pet2);

    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(operationStatus, true, 14000);

    // found pets with same data
    QList<qint32> petData;
    petData.append(pet1.getAge());
    petData.append(pet1.getPatience());
    petData.append(pet2.getAge());
    petData.append(pet2.getPatience());
    operationStatus = false;
    api.findPetsByAgeAndPatience(petData, this, [&](const QRestReply &reply, const QList<QtOAIPet> &summary) {
        if (!(operationStatus = reply.isSuccess()))
            qDebug() << "Error happened while issuing request : " << reply.errorString();

        QVERIFY(operationStatus);
        QCOMPARE(REPLY_OK, reply.httpStatus());
        QVERIFY(summary.count() == 2);
        const std::array<QtOAIPet, 2> expectedPets = {pet1, pet2};
        QVERIFY(std::is_permutation(summary.cbegin(), summary.cend(),
                                    expectedPets.cbegin(), expectedPets.cend()));

    });
    QTRY_COMPARE_EQ_WITH_TIMEOUT(operationStatus, true, 14000);
}
} // QtOpenAPI

QTEST_MAIN(QtOpenAPI::PetApiTests)
#include "PetApiTests.moc"
