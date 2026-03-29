// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qoaicommonglobal.h"

#include "../basicSchemaAlternatives/client/storeapi.h"

#include <QtCore/qbuffer.h>
#include <QtCore/qobject.h>
#include <QtNetwork/qnetworkrequestfactory.h>
#include <QtNetwork/qrestaccessmanager.h>
#include <QtTest/qtest.h>

using namespace Qt::StringLiterals;
using namespace QtOpenApiCommon;

class LoggingNetworkAccessManager : public QNetworkAccessManager
{
public:
    LoggingNetworkAccessManager(QObject *parent = nullptr)
        : QNetworkAccessManager(parent)
    {}
    ~LoggingNetworkAccessManager() override
    {}

protected:
    QNetworkReply *createRequest(QNetworkAccessManager::Operation op,
                                 const QNetworkRequest &originalReq,
                                 QIODevice *outgoingData = nullptr) override;

public:
    QByteArray m_content;
};

QNetworkReply *LoggingNetworkAccessManager::createRequest(QNetworkAccessManager::Operation op,
                                                          const QNetworkRequest &originalReq,
                                                          QIODevice *outgoingData)
{
    // The QNAM::put/post/sendCustomRequest operations that take QByteArray
    // as a data parameter use QBuffer to wrap the byte array.
    // It is later passed as a QIODevice* to this method.
    // We cast the QIODevice to QBuffer, and store its content to intercept
    // the transferred data.
    QBuffer *buffer = qobject_cast<QBuffer*>(outgoingData);
    m_content = buffer ? buffer->data() : QByteArray();

    return QNetworkAccessManager::createRequest(op, originalReq, outgoingData);
}

class OneOfTest : public QObject {
    Q_OBJECT

    enum class StoreApiMode {
        UseCat,
        UseDog,
        UsePet
    };

private:
    void generateAlternativeSchemasTestData();

private Q_SLOTS:
    void testAlternativeSchemasFunctions_data();
    void testAlternativeSchemasFunctions();
    void testAlternativeSchemasOptional_data();
    void testAlternativeSchemasOptional();
    void testAlternativeSchemasRequired_data();
    void testAlternativeSchemasRequired();
};

void OneOfTest::testAlternativeSchemasFunctions_data()
{
    QTest::addColumn<SchemasModelsOneOf::Pet>("pet");
    QTest::addColumn<QString>("expectedJson");
    QTest::addColumn<QJsonValue>("expectedJsonValue");
    QTest::addColumn<QString>("oneOfCatExpectedJson");
    QTest::addColumn<QString>("oneOfDogExpectedJson");
    QTest::addColumn<SchemasModelsOneOf::Cat>("getOneOfCat");
    QTest::addColumn<SchemasModelsOneOf::Dog>("getOneOfDog");
    QTest::addColumn<bool>("isOneOfCatValid");
    QTest::addColumn<bool>("isOneOfCatSet");
    QTest::addColumn<bool>("isOneOfDogValid");
    QTest::addColumn<bool>("isOneOfDogSet");
    QTest::addColumn<bool>("isValid");
    QTest::addColumn<bool>("isSet");

    SchemasModelsOneOf::Pet pet;
    QTest::newRow("Empty Pet object")
        // pet                         // expectedJson
        << pet                         << QString("{}"_L1)
        // expectedJsonValue
        << QJsonValue(QJsonObject())
        // oneOfCatExpectedJson        // oneOfDogExpectedJson
        << QString("{}"_L1)            << QString("{}"_L1)
        // getOneOfCat                 // getOneOfDog
        << SchemasModelsOneOf::Cat()   << SchemasModelsOneOf::Dog()
        // isOneOfCatValid             // isOneOfCatSet
        << false                       << false
        // isOneOfDogValid             // isOneOfDogSet
        << false                       << false
        // isValid                     // isSet
        << false                       << false;

    SchemasModelsOneOf::Cat kitty;
    kitty.setHunts(false);
    kitty.setAge(7);

    SchemasModelsOneOf::Dog hund;
    hund.setBark(false);
    hund.setBreed("French Bulldog");

    // check setters work fine
    pet.setOneOfCat(kitty);
    QTest::newRow("Set a valid Cat object via setter")
        // pet                         // expectedJson
        << pet                         << kitty.asJson()
        // expectedJsonValue
        << kitty.asJsonValue()
        // oneOfCatExpectedJson                       // oneOfDogExpectedJson
        << QString("{\"age\":7,\"hunts\":false}"_L1)  << QString("{}"_L1)
        // getOneOfCat                                // getOneOfDog
        << kitty                                      << SchemasModelsOneOf::Dog()
        // isOneOfCatValid                            // isOneOfCatSet
        << true                                       << true
        // isOneOfDogValid                            // isOneOfDogSet
        << false                                      << false
        // isValid                                    // isSet
        << true                                       << true;

    // Dog has required field bark, so empty Dog is invalid
    pet.setOneOfDog(hund);
    QTest::newRow("Set a valid Dog object via setter")
        // pet                                        // expectedJson
        << pet                                        << hund.asJson()
        // expectedJsonValue
        << hund.asJsonValue()
        // oneOfCatExpectedJson                       // oneOfDogExpectedJson
        << QString("{}"_L1)                           << QString("{\"bark\":false,\"breed\":\"French Bulldog\"}"_L1)
        // getOneOfCat                                // getOneOfDog
        << SchemasModelsOneOf::Cat()                  << hund
        // isOneOfCatValid                            // isOneOfCatSet
        << false                                      << false
        // isOneOfDogValid                            // isOneOfDogSet
        << true                                       << true
        // isValid                                    // isSet
        << true                                       << true;

    // Set an empty Dog object, the dog has required fields, so an empty
    // dog is INVALID => entire Pet stays invalid
    const SchemasModelsOneOf::Dog emptyDog;
    pet.setOneOfDog(emptyDog);
    QTest::newRow("Set an empty Dog object via setter")
        // pet                                        // expectedJson
        << pet                                        << emptyDog.asJson()
        // expectedJsonValue
        << emptyDog.asJsonValue()
        // oneOfCatExpectedJson                       // oneOfDogExpectedJson
        << QString("{}"_L1)                           << QString("{}"_L1)
        // getOneOfCat                                // getOneOfDog
        << SchemasModelsOneOf::Cat()                  << emptyDog
        // isOneOfCatValid                            // isOneOfCatSet
        << false                                      << false
        // isOneOfDogValid                            // isOneOfDogSet
        << false                                      << false
        // isValid                                    // isSet
        << false                                      << false;

    // Set an empty Cat object, the cat has no required fields, so an empty
    // cat is VALID => entire Pet becomes valid
    const SchemasModelsOneOf::Cat emptyCat;
    pet.setOneOfCat(emptyCat);
    QTest::newRow("Set an empty Cat object via setter")
        // pet                                        // expectedJson
        << pet                                        << emptyCat.asJson()
        // expectedJsonValue
        << emptyCat.asJsonValue()
        // oneOfCatExpectedJson                       // oneOfDogExpectedJson
        << QString("{}"_L1)                           << QString("{}"_L1)
        // getOneOfCat                                // getOneOfDog
        << emptyCat                                   << emptyDog
        // isOneOfCatValid                            // isOneOfCatSet
        << true                                       << true
        // isOneOfDogValid                            // isOneOfDogSet
        << false                                      << false
        // isValid                                    // isSet
        << true                                       << true;

    // fromJsonValue with EMPTY data.
    // Attempting to set an empty Dog via pet.fromJsonValue().
    // Dog defines required fields, so an empty object is INVALID for Dog.
    //
    // However, emptyDog.asJsonValue() produces '{}' because it is an empty object.
    //
    // The Pet::fromJsonValue() logic cannot determine whether '{}' represents
    // a Cat or a Dog. The only available approach is to try deserializing '{}'
    // against each schema and select the one that validates successfully.
    //
    // This leads to ambiguity, since '{}' is a valid Cat instance.
    //
    // To avoid such situations, the schema should define either a discriminator
    // or required fields for each Object in the YAML specification.
    //
    // This ambiguity is intentional here for testing purposes, to highlight
    // the importance of well-defined API schemas. Ensuring correctness of the
    // API design is the responsibility of the API author.
    pet.fromJsonValue(emptyDog.asJsonValue());
    QTest::newRow("Set an empty Dog via ::fromJsonValue(emptyDog.asJsonValue())")
        // pet                                        // expectedJson
        << pet                                        << emptyDog.asJson()
        // expectedJsonValue
        << emptyDog.asJsonValue()
        // oneOfCatExpectedJson                       // oneOfDogExpectedJson
        << QString("{}"_L1)                           << QString("{}"_L1)
        // getOneOfCat                                // getOneOfDog
        << emptyCat                                   << emptyDog
        // !!! NOTE: ambiguity is here, cat is VALID!
        // isOneOfCatValid                            // isOneOfCatSet
        << true                                       << true
        // isOneOfDogValid                            // isOneOfDogSet
        << false                                      << false
        // isValid                                    // isSet
        << true                                       << true;

    // fromJson with EMPTY data.
    // Same expectations like with
    // 'Set an empty Dog via pet.fromJsonValue(emptyDog.asJsonValue())'
    pet.fromJson(emptyDog.asJson());
    QTest::newRow("Set an empty Dog via ::fromJson(emptyDog.asJson())")
        // pet                                        // expectedJson
        << pet                                        << emptyDog.asJson()
        // expectedJsonValue
        << emptyDog.asJsonValue()
        // oneOfCatExpectedJson                       // oneOfDogExpectedJson
        << QString("{}"_L1)                           << QString("{}"_L1)
        // getOneOfCat                                // getOneOfDog
        << emptyCat                                   << emptyDog
        // !!! NOTE: ambiguity is here, cat is VALID!
        // isOneOfCatValid                            // isOneOfCatSet
        << true                                       << true
        // isOneOfDogValid                            // isOneOfDogSet
        << false                                      << false
        // isValid                                    // isSet
        << true                                       << true;

    // fromJson with EMPTY data.
    // Same expectations like with
    // 'Set an empty Dog via pet.fromJsonValue(emptyDog.asJsonValue())'
    pet.fromJson(emptyCat.asJson());
    QTest::newRow("Set an empty Cat via ::fromJson(emptyCat.asJson())")
        // pet                                        // expectedJson
        << pet                                        << emptyCat.asJson()
        // expectedJsonValue
        << emptyCat.asJsonValue()
        // oneOfCatExpectedJson                       // oneOfDogExpectedJson
        << QString("{}"_L1)                           << QString("{}"_L1)
        // getOneOfCat                                // getOneOfDog
        << emptyCat                                   << emptyDog
        // isOneOfCatValid                            // isOneOfCatSet
        << true                                       << true
        // isOneOfDogValid                            // isOneOfDogSet
        << false                                      << false
        // isValid                                    // isSet
        << true                                       << true;

    pet.fromJsonValue(emptyCat.asJsonValue());
    QTest::newRow("Set an empty Cat via ::fromJsonValue(emptyCat.asJsonValue())")
        // pet                                        // expectedJson
        << pet                                        << emptyCat.asJson()
        // expectedJsonValue
        << emptyCat.asJsonValue()
        // oneOfCatExpectedJson                       // oneOfDogExpectedJson
        << QString("{}"_L1)                           << QString("{}"_L1)
        // getOneOfCat                                // getOneOfDog
        << emptyCat                                   << emptyDog
        // isOneOfCatValid                            // isOneOfCatSet
        << true                                       << true
        // isOneOfDogValid                            // isOneOfDogSet
        << false                                      << false
        // isValid                                    // isSet
        << true                                       << true;

    // check fromJson() works fine for VALID cat object
    const QString kittyCat("{\"age\":90,\"hunts\":false}"_L1);
    pet.fromJson(kittyCat);
    QTest::newRow("Set a valid Cat object via ::fromJson()")
        // pet                                        // expectedJson
        << pet                                        << kittyCat
        // expectedJsonValue
        << pet.asJsonValue()
        // oneOfCatExpectedJson                       // oneOfDogExpectedJson
        << kittyCat                                   << QString("{}"_L1)
        // getOneOfCat                                // getOneOfDog
        << SchemasModelsOneOf::Cat(kittyCat)          << emptyDog
        // isOneOfCatValid                            // isOneOfCatSet
        << true                                       << true
        // isOneOfDogValid                            // isOneOfDogSet
        << false                                      << false
        // isValid                                    // isSet
        << true                                       << true;

    // check fromJsonValue() works fine for VALID cat object
    const SchemasModelsOneOf::Cat okCat(kittyCat);
    pet.fromJsonValue(okCat.asJsonValue());
    QTest::newRow("Set a valid Cat object via ::fromJsonValue()")
        // pet                                        // expectedJson
        << pet                                        << kittyCat
        // expectedJsonValue
        << pet.asJsonValue()
        // oneOfCatExpectedJson                       // oneOfDogExpectedJson
        << kittyCat                                   << QString("{}"_L1)
        // getOneOfCat                                // getOneOfDog
        << SchemasModelsOneOf::Cat(kittyCat)          << emptyDog
        // isOneOfCatValid                            // isOneOfCatSet
        << true                                       << true
        // isOneOfDogValid                            // isOneOfDogSet
        << false                                      << false
        // isValid                                    // isSet
        << true                                       << true;

    // check fromJson() works fine for VALID cat object
    const QString doggyDog("{\"bark\":false,\"breed\":\"French Bulldog\"}"_L1);
    pet.fromJson(doggyDog);
    QTest::newRow("Set a valid Dog object via ::fromJson()")
        // pet                                        // expectedJson
        << pet                                        << doggyDog
        // expectedJsonValue
        << pet.asJsonValue()
        // oneOfCatExpectedJson                       // oneOfDogExpectedJson
        << QString("{}"_L1)                           << doggyDog
        // getOneOfCat                                // getOneOfDog
        << emptyCat                                   << SchemasModelsOneOf::Dog(doggyDog)
        // isOneOfCatValid                            // isOneOfCatSet
        << false                                      << false
        // isOneOfDogValid                            // isOneOfDogSet
        << true                                       << true
        // isValid                                    // isSet
        << true                                       << true;

    // check fromJsonValue() works fine for VALID cat object
    const SchemasModelsOneOf::Dog okDog(doggyDog);
    pet.fromJsonValue(okDog.asJsonValue());
    QTest::newRow("Set a valid Dog object via ::fromJsonValue()")
        // pet                                        // expectedJson
        << pet                                        << doggyDog
        // expectedJsonValue
        << pet.asJsonValue()
        // oneOfCatExpectedJson                       // oneOfDogExpectedJson
        << QString("{}"_L1)                           << doggyDog
        // getOneOfCat                                // getOneOfDog
        << emptyCat                                   << okDog
        // isOneOfCatValid                            // isOneOfCatSet
        << false                                      << false
        // isOneOfDogValid                            // isOneOfDogSet
        << true                                       << true
        // isValid                                    // isSet
        << true                                       << true;

    // invalid json!
    pet.fromJson("{invalid json}"_L1);
    QTest::newRow("Set an invalid json via ::fromJson()")
        // pet                                        // expectedJson
        << pet                                        << QString("{}"_L1)
        // expectedJsonValue
        << pet.asJsonValue()
        // oneOfCatExpectedJson                       // oneOfDogExpectedJson
        << QString("{}"_L1)                           << QString("{}"_L1)
        // getOneOfCat                                // getOneOfDog
        << emptyCat                                   << emptyDog
        // isOneOfCatValid                            // isOneOfCatSet
        << false                                      << false
        // isOneOfDogValid                            // isOneOfDogSet
        << false                                      << false
        // isValid                                    // isSet
        << false                                      << false;

    // Test with non-object JSON (array)
    pet.fromJson("[]"_L1);
    QTest::newRow("Set an unexpected array [] via ::fromJson()")
        // pet                                        // expectedJson
        << pet                                        << QString("{}"_L1)
        // expectedJsonValue
        << pet.asJsonValue()
        // oneOfCatExpectedJson                       // oneOfDogExpectedJson
        << QString("{}"_L1)                           << QString("{}"_L1)
        // getOneOfCat                                // getOneOfDog
        << emptyCat                                   << emptyDog
        // isOneOfCatValid                            // isOneOfCatSet
        << false                                      << false
        // isOneOfDogValid                            // isOneOfDogSet
        << false                                      << false
        // isValid                                    // isSet
        << false                                      << false;


    // Test with non-object JSON (string)
    pet.fromJson("\"just a string\""_L1);
    QTest::newRow("Set an unexpected string via ::fromJson()")
        // pet                                        // expectedJson
        << pet                                        << QString("{}"_L1)
        // expectedJsonValue
        << pet.asJsonValue()
        // oneOfCatExpectedJson                       // oneOfDogExpectedJson
        << QString("{}"_L1)                           << QString("{}"_L1)
        // getOneOfCat                                // getOneOfDog
        << emptyCat                                   << emptyDog
        // isOneOfCatValid                            // isOneOfCatSet
        << false                                      << false
        // isOneOfDogValid                            // isOneOfDogSet
        << false                                      << false
        // isValid                                    // isSet
        << false                                      << false;

    // Missing Required Fields (Dog's 'bark' is required)
    // Dog without required 'bark' field - should not be valid
    SchemasModelsOneOf::Dog invalidDog;
    invalidDog.setBreed("Husky-kolbasky");
    pet.setOneOfDog(invalidDog);
    QTest::newRow("Set an invalid Dog via setter")
        // pet                                        // expectedJson
        << pet                                        << QString("{}"_L1)
        // expectedJsonValue
        << pet.asJsonValue()
        // oneOfCatExpectedJson                       // oneOfDogExpectedJson
        << QString("{}"_L1)                           << QString("{\"breed\":\"Husky-kolbasky\"}"_L1)
        // getOneOfCat                                // getOneOfDog
        << emptyCat                                   << invalidDog
        // isOneOfCatValid                            // isOneOfCatSet
        << false                                      << false
        // The status here reflects the status of the model that was set.
        // The model's field was set by setter, so invalidDog.isSet() == true
        // but invalidDog.isValid() == false, because required field is not set.
        // isOneOfDogValid                            // isOneOfDogSet
        << invalidDog.isValid()                       << invalidDog.isSet()
        // isValid                                    // isSet
        << false                                      << true;

    // Try to parse JSON missing required field, the dog is invalid.
    pet.fromJson("{\"breed\":\"Retriever\"}"_L1);
    QTest::newRow("Set an invalid Dog via ::fromJson()")
        // pet                                        // expectedJson
        << pet                                        << QString("{}"_L1)
        // expectedJsonValue
        << pet.asJsonValue()
        // oneOfCatExpectedJson                       // oneOfDogExpectedJson
        << QString("{}"_L1)                           << QString("{\"breed\":\"Retriever\"}"_L1)
        // getOneOfCat                                // getOneOfDog
        << emptyCat                                   << SchemasModelsOneOf::Dog("{\"breed\":\"Retriever\"}"_L1)
        // isOneOfCatValid                            // isOneOfCatSet
        << false                                      << false
        // isOneOfDogValid                            // isOneOfDogSet
        << false                                      << true
        // isValid                                    // isSet
        << false                                      << true;
    // Partial/Incomplete JSON Objects
    // Only one field for Cat (age without hunts),
    // should work since Cat has no required fields
    const QString notFullCat("{\"age\":5}"_L1);
    pet.fromJson(notFullCat);
    QTest::newRow("Set a valid non-full Cat via ::fromJson()")
        // pet                                        // expectedJson
        << pet                                        << notFullCat
        // expectedJsonValue
        << pet.asJsonValue()
        // oneOfCatExpectedJson                       // oneOfDogExpectedJson
        << notFullCat                                 << QString("{}"_L1)
        // getOneOfCat                                // getOneOfDog
        << SchemasModelsOneOf::Cat("{\"age\":5}"_L1)  << emptyDog
        // isOneOfCatValid                            // isOneOfCatSet
        << true                                       << true
        // isOneOfDogValid                            // isOneOfDogSet
        << false                                      << false
        // isValid                                    // isSet
        << true                                       << true;

    // Extra/Unknown Fields in JSON
    // JSON with fields not in schema - should parse known fields and ignore unknown
    pet.fromJson("{\"age\":7,\"hunts\":false,\"name\":\"Fluffy\",\"color\":\"orange\"}"_L1);
    QString expected("{\"age\":7,\"hunts\":false}"_L1);
    QTest::newRow("Extra/Unknown Fields in JSON via ::fromJson()")
        // pet                                        // expectedJson
        << pet                                        << expected
        // expectedJsonValue
        << pet.asJsonValue()
        // oneOfCatExpectedJson                       // oneOfDogExpectedJson
        << expected                                   << QString("{}"_L1)
        // getOneOfCat                                // getOneOfDog
        << SchemasModelsOneOf::Cat(expected)          << emptyDog
        // isOneOfCatValid                            // isOneOfCatSet
        << true                                       << true
        // isOneOfDogValid                            // isOneOfDogSet
        << false                                      << false
        // isValid                                    // isSet
        << true                                       << true;

    // Empty Strings in Fields
    expected = QString("{\"bark\":true,\"breed\":\"\"}"_L1);
    pet.fromJson(expected);
    QTest::newRow("An empty String in Dog breed field via ::fromJson()")
        // pet                                        // expectedJson
        << pet                                        << expected
        // expectedJsonValue
        << pet.asJsonValue()
        // oneOfCatExpectedJson                       // oneOfDogExpectedJson
        << QString("{}"_L1)                           << expected
        // getOneOfCat                                // getOneOfDog
        << emptyCat                                   << SchemasModelsOneOf::Dog(expected)
        // isOneOfCatValid                            // isOneOfCatSet
        << false                                      << false
        // isOneOfDogValid                            // isOneOfDogSet
        << true                                       << true
        // isValid                                    // isSet
        << true                                       << true;

    // Invalid Field Types for Cat(). Wrong type for age (string instead of integer)
    // Note: field of Cat type are not required, it means wrong fields can be omitted
    expected = QString("{\"age\":\"seven\",\"hunts\":false}"_L1);
    pet.fromJson(expected);
    QTest::newRow("Invalid field 'age=seven' in Cat via ::fromJson()")
        // pet                                        // expectedJson
        << pet                                        << QString("{\"hunts\":false}"_L1)
        // expectedJsonValue
        << pet.asJsonValue()
        // oneOfCatExpectedJson                       // oneOfDogExpectedJson
        << QString("{\"hunts\":false}"_L1)            << QString("{}"_L1)
        // getOneOfCat                                // getOneOfDog
        << SchemasModelsOneOf::Cat(expected)          << emptyDog
        // isOneOfCatValid                            // isOneOfCatSet
        << true                                       << true
        // isOneOfDogValid                            // isOneOfDogSet
        << false                                      << false
        // isValid                                    // isSet
        << true                                       << true;

    // Invalid Field Types for Cat(). Wrong type for age (double instead of integer)
    // Note: field of Cat type are not required, it means wrong fields can be omitted
    expected = QString("{\"age\":0.1,\"hunts\":true}"_L1);
    pet.fromJson(expected);
    QTest::newRow("Invalid field 'age=0.1' in Cat via ::fromJson()")
        // pet                                        // expectedJson
        << pet                                        << QString("{\"hunts\":true}"_L1)
        // expectedJsonValue
        << pet.asJsonValue()
        // oneOfCatExpectedJson                       // oneOfDogExpectedJson
        << QString("{\"hunts\":true}"_L1)             << QString("{}"_L1)
        // getOneOfCat                                // getOneOfDog
        << SchemasModelsOneOf::Cat(expected)          << emptyDog
        // isOneOfCatValid                            // isOneOfCatSet
        << true                                       << true
        // isOneOfDogValid                            // isOneOfDogSet
        << false                                      << false
        // isValid                                    // isSet
        << true                                       << true;

    // Invalid Field Types in for Cat(). Wrong type for age (bool instead of integer)
    // Note: field of Cat type are not required, it means wrong fields can be omitted
    expected = QString("{\"age\":true,\"hunts\":true}"_L1);
    pet.fromJson(expected);
    QTest::newRow("Invalid field 'age=true' in Cat via ::fromJson()")
        // pet                                        // expectedJson
        << pet                                        << QString("{\"hunts\":true}"_L1)
        // expectedJsonValue
        << pet.asJsonValue()
        // oneOfCatExpectedJson                       // oneOfDogExpectedJson
        << QString("{\"hunts\":true}"_L1)             << QString("{}"_L1)
        // getOneOfCat                                // getOneOfDog
        << SchemasModelsOneOf::Cat(expected)          << emptyDog
        // isOneOfCatValid                            // isOneOfCatSet
        << true                                       << true
        // isOneOfDogValid                            // isOneOfDogSet
        << false                                      << false
        // isValid                                    // isSet
        << true                                       << true;

    expected = QString("{\"age\":true,\"hunts\":1}"_L1);
    pet.fromJson(expected);
    QTest::newRow("Invalid fields: 'age=true', hunts=1 in Cat via ::fromJson()")
        // pet                                        // expectedJson
        << pet                                        << QString("{}"_L1)
        // expectedJsonValue
        << pet.asJsonValue()
        // oneOfCatExpectedJson                       // oneOfDogExpectedJson
        << QString("{}"_L1)                           << QString("{}"_L1)
        // getOneOfCat                                // getOneOfDog
        << emptyCat                                   << emptyDog
        // isOneOfCatValid                            // isOneOfCatSet
        << false                                      << false
        // isOneOfDogValid                            // isOneOfDogSet
        << false                                      << false
        // isValid                                    // isSet
        << false                                      << false;
}

void OneOfTest::testAlternativeSchemasFunctions()
{
    QFETCH(SchemasModelsOneOf::Pet, pet);
    QFETCH(QString, expectedJson);
    QFETCH(QJsonValue, expectedJsonValue);
    QFETCH(QString, oneOfCatExpectedJson);
    QFETCH(QString, oneOfDogExpectedJson);
    QFETCH(SchemasModelsOneOf::Cat, getOneOfCat);
    QFETCH(SchemasModelsOneOf::Dog, getOneOfDog);
    QFETCH(bool, isOneOfCatValid);
    QFETCH(bool, isOneOfCatSet);
    QFETCH(bool, isOneOfDogValid);
    QFETCH(bool, isOneOfDogSet);
    QFETCH(bool, isValid);
    QFETCH(bool, isSet);

    QCOMPARE(pet.asJson(), expectedJson);
    QCOMPARE(pet.asJsonValue(), expectedJsonValue);
    QCOMPARE(pet.getOneOfCat().asJson(), oneOfCatExpectedJson);
    QCOMPARE(pet.isOneOfCatValid(), isOneOfCatValid);
    QCOMPARE(pet.isOneOfCatSet(), isOneOfCatSet);
    QCOMPARE(pet.getOneOfDog().asJson(), oneOfDogExpectedJson);
    QCOMPARE(pet.getOneOfCat(), getOneOfCat);
    QCOMPARE(pet.getOneOfDog(), getOneOfDog);
    QCOMPARE(pet.isOneOfDogValid(), isOneOfDogValid);
    QCOMPARE(pet.isOneOfDogSet(), isOneOfDogSet);
    QCOMPARE(pet.isValid(), isValid);
    QCOMPARE(pet.isSet(), isSet);
}

void OneOfTest::testAlternativeSchemasOptional_data()
{
    generateAlternativeSchemasTestData();
}

void OneOfTest::testAlternativeSchemasOptional()
{
    QFETCH(SchemasModelsOneOf::Pet, pet);
    QFETCH(SchemasModelsOneOf::Cat, cat);
    QFETCH(SchemasModelsOneOf::Dog, dog);
    QFETCH(OneOfTest::StoreApiMode, mode);
    QFETCH(QString, expectedJson);

    bool done = true;
    SchemasModelsOneOf::StoreApi api;
    LoggingNetworkAccessManager logging(&api);
    QRestAccessManager restManager(&logging, &api);
    api.setRestAccessManager(&restManager);

    switch (mode) {
    case OneOfTest::StoreApiMode::UseCat:
    {
        api.postPetData(QtOpenApiCommon::OptionalParameter<SchemasModelsOneOf::Pet>(cat),
                        this, [&](const QRestReply &reply) {
                            done = reply.isSuccess();
                        });
    } break;
    case OneOfTest::StoreApiMode::UseDog:
    {
        api.postPetData(QtOpenApiCommon::OptionalParameter<SchemasModelsOneOf::Pet>(dog),
                        this, [&](const QRestReply &reply) {
                            done = reply.isSuccess();
                        });
    } break;
    case OneOfTest::StoreApiMode::UsePet:
    {
        api.postPetData(pet, this, [&](const QRestReply &reply) {
            done = reply.isSuccess();
        });
    } break;
    }
    QCOMPARE(logging.m_content, expectedJson);
    QTRY_COMPARE_EQ(done, false);
}

void OneOfTest::generateAlternativeSchemasTestData()
{
    QTest::addColumn<SchemasModelsOneOf::Pet>("pet");
    QTest::addColumn<SchemasModelsOneOf::Cat>("cat");
    QTest::addColumn<SchemasModelsOneOf::Dog>("dog");
    QTest::addColumn<OneOfTest::StoreApiMode>("mode");
    QTest::addColumn<QString>("expectedJson");

    QTest::newRow("empty")
        << SchemasModelsOneOf::Pet{}
        << SchemasModelsOneOf::Cat{}
        << SchemasModelsOneOf::Dog{}
        << OneOfTest::StoreApiMode::UsePet
        << QString("{}"_L1);

    SchemasModelsOneOf::Pet pet;
    SchemasModelsOneOf::Cat kitty;
    kitty.setHunts(true);
    kitty.setAge(7);

    SchemasModelsOneOf::Dog hund;
    hund.setBark(false);
    hund.setBreed("Labrador");

    QTest::newRow("Use Cat object")
        << SchemasModelsOneOf::Pet{}
        << kitty
        << SchemasModelsOneOf::Dog{}
        << OneOfTest::StoreApiMode::UseCat
        << QString("{\"age\":7,\"hunts\":true}"_L1);

    QTest::newRow("Use Dog object")
        << SchemasModelsOneOf::Pet{}
        << SchemasModelsOneOf::Cat{}
        << hund
        << OneOfTest::StoreApiMode::UseDog
        << QString("{\"bark\":false,\"breed\":\"Labrador\"}"_L1);

    // Setting the cat the pet()
    kitty.setAge(9);
    pet.setOneOfCat(kitty);
    QTest::newRow("Set the cat data to a pet object")
        << pet
        << SchemasModelsOneOf::Cat{}
        << SchemasModelsOneOf::Dog{}
        << OneOfTest::StoreApiMode::UsePet
        << QString("{\"age\":9,\"hunts\":true}"_L1);

    hund.setBark(true);
    pet.setOneOfDog(hund);
    QTest::newRow("Set the dog data to a pet object")
        << pet
        << SchemasModelsOneOf::Cat{}
        << SchemasModelsOneOf::Dog{}
        << OneOfTest::StoreApiMode::UsePet
        << QString("{\"bark\":true,\"breed\":\"Labrador\"}"_L1);

    pet.setOneOfDog(SchemasModelsOneOf::Dog());
    QTest::newRow("Reset the pet data to an empty Dog()")
        << pet
        << SchemasModelsOneOf::Cat{}
        << SchemasModelsOneOf::Dog{}
        << OneOfTest::StoreApiMode::UsePet
        << QString("{}"_L1);

    SchemasModelsOneOf::Dog invalidDog;
    invalidDog.setBreed("Unknown");
    QTest::newRow("Construct the dog without setting the `required` field(bark)")
        << SchemasModelsOneOf::Pet{}
        << SchemasModelsOneOf::Cat{}
        << invalidDog
        << OneOfTest::StoreApiMode::UseDog
        // It's empty because the object is not valid, without setting the required field
        << QString("{}"_L1);

    // Construct the cat but skipping some fields.
    // NOTE: Cat type doesn't have `required` fields,
    // so it will not be empty.
    SchemasModelsOneOf::Cat cat;
    cat.setAge(5);
    QTest::newRow("Construct the cat with only age field set")
        << SchemasModelsOneOf::Pet{}
        << cat
        << SchemasModelsOneOf::Dog{}
        << OneOfTest::StoreApiMode::UseCat
        << QString("{\"age\":5}"_L1);
}

void OneOfTest::testAlternativeSchemasRequired_data()
{
    generateAlternativeSchemasTestData();
}

void OneOfTest::testAlternativeSchemasRequired()
{
    QFETCH(SchemasModelsOneOf::Pet, pet);
    QFETCH(SchemasModelsOneOf::Cat, cat);
    QFETCH(SchemasModelsOneOf::Dog, dog);
    QFETCH(OneOfTest::StoreApiMode, mode);
    QFETCH(QString, expectedJson);

    bool done = true;
    SchemasModelsOneOf::StoreApi api;
    LoggingNetworkAccessManager logging(&api);
    QRestAccessManager restManager(&logging, &api);
    api.setRestAccessManager(&restManager);

    switch (mode) {
    case OneOfTest::StoreApiMode::UseCat:
    {
        api.patchPetData(cat, this, [&](const QRestReply &reply) {
            done = reply.isSuccess();
        });
    } break;
    case OneOfTest::StoreApiMode::UseDog:
    {
        api.patchPetData(dog, this, [&](const QRestReply &reply) {
            done = reply.isSuccess();
        });
    } break;
    case OneOfTest::StoreApiMode::UsePet:
    {
        api.patchPetData(pet, this, [&](const QRestReply &reply) {
            done = reply.isSuccess();
        });
    } break;
    }
    QCOMPARE(logging.m_content, expectedJson);
    QTRY_COMPARE_EQ(done, false);
}

QTEST_MAIN(OneOfTest)
#include "tst_oneof.moc"
