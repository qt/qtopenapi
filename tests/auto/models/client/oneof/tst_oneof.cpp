// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qoaicommonglobal.h"

#include "../basicSchemaAlternatives/client/farmapi.h"
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

    enum class FarmApiMode {
        UseBunny,
        UseDog,
        UseDuck,
        UsePostPet
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
    void testPolymorphedRequestBody_data();
    void testPolymorphedRequestBody();
    void testPostFarmPetRequestJsonConversionMethods_data();
    void testPostFarmPetRequestJsonConversionMethods();
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

void OneOfTest::testPolymorphedRequestBody_data()
{
    QTest::addColumn<SchemasModelsOneOf::PostFarmPet_request>("pet");
    QTest::addColumn<SchemasModelsOneOf::Dog>("dog");
    QTest::addColumn<SchemasModelsOneOf::Duck>("duck");
    QTest::addColumn<SchemasModelsOneOf::Bunny>("bunny");
    QTest::addColumn<OneOfTest::FarmApiMode>("mode");
    QTest::addColumn<QString>("expectedJson");

    // Empty
    QTest::newRow("empty duck, bunny, dog and farm pet")
        << SchemasModelsOneOf::PostFarmPet_request{}
        << SchemasModelsOneOf::Dog{}
        << SchemasModelsOneOf::Duck{}
        << SchemasModelsOneOf::Bunny{}
        << OneOfTest::FarmApiMode::UsePostPet
        << QString("{}"_L1);

    SchemasModelsOneOf::PostFarmPet_request model;
    SchemasModelsOneOf::Duck duck;
    duck.setHunts(true);
    duck.setAge(9);

    SchemasModelsOneOf::Bunny bunny;
    bunny.setBreedType("Wild");
    bunny.setWeight(5);

    QTest::newRow("Set Bunny object")
        << SchemasModelsOneOf::PostFarmPet_request{}
        << SchemasModelsOneOf::Dog{}
        << SchemasModelsOneOf::Duck{}
        << bunny
        << OneOfTest::FarmApiMode::UseBunny
        << QString("{\"breed-type\":\"Wild\",\"weight\":5}"_L1);

    QTest::newRow("Set Duck object")
        << SchemasModelsOneOf::PostFarmPet_request{}
        << SchemasModelsOneOf::Dog{}
        << duck
        << SchemasModelsOneOf::Bunny{}
        << OneOfTest::FarmApiMode::UseDuck
        << QString("{\"age\":9,\"hunts\":true}"_L1);

    model.setOneOfDuck(duck);
    QTest::newRow("Set PostFarmPet_request object with duck data")
        << model
        << SchemasModelsOneOf::Dog{}
        << SchemasModelsOneOf::Duck{}
        << SchemasModelsOneOf::Bunny{}
        << OneOfTest::FarmApiMode::UsePostPet
        << QString("{\"age\":9,\"hunts\":true}"_L1);

    model.setOneOfBunny(bunny);
    QTest::newRow("Set PostFarmPet_request object with bunny data")
        << model
        << SchemasModelsOneOf::Dog{}
        << SchemasModelsOneOf::Duck{}
        << SchemasModelsOneOf::Bunny{}
        << OneOfTest::FarmApiMode::UsePostPet
        << QString("{\"breed-type\":\"Wild\",\"weight\":5}"_L1);

    model.setOneOfBunny(SchemasModelsOneOf::Bunny{});
    QTest::newRow("Reset PostFarmPet_request object to empty")
        << model
        << SchemasModelsOneOf::Dog{}
        << SchemasModelsOneOf::Duck{}
        << SchemasModelsOneOf::Bunny{}
        << OneOfTest::FarmApiMode::UsePostPet
        << QString("{}"_L1);

    SchemasModelsOneOf::Dog dog;
    dog.setBreed("Dingo");
    // Should fail because bark is not set, but bark is required
    QTest::newRow("Set incomplete Dog() object")
        << SchemasModelsOneOf::PostFarmPet_request{}
        << dog
        << SchemasModelsOneOf::Duck{}
        << SchemasModelsOneOf::Bunny{}
        << OneOfTest::FarmApiMode::UseDog
        << QString("{}"_L1);

    dog.setBark(true);
    QTest::newRow("Set full Dog() object")
        << SchemasModelsOneOf::PostFarmPet_request{}
        << dog
        << SchemasModelsOneOf::Duck{}
        << SchemasModelsOneOf::Bunny{}
        << OneOfTest::FarmApiMode::UseDog
        << QString("{\"bark\":true,\"breed\":\"Dingo\"}"_L1);
}

void OneOfTest::testPolymorphedRequestBody()
{
    QFETCH(SchemasModelsOneOf::PostFarmPet_request, pet);
    QFETCH(SchemasModelsOneOf::Dog, dog);
    QFETCH(SchemasModelsOneOf::Duck, duck);
    QFETCH(SchemasModelsOneOf::Bunny, bunny);
    QFETCH(OneOfTest::FarmApiMode, mode);
    QFETCH(QString, expectedJson);

    bool done = true;
    SchemasModelsOneOf::FarmApi farmApi;
    LoggingNetworkAccessManager logging(&farmApi);
    QRestAccessManager restManager(&logging, &farmApi);
    farmApi.setRestAccessManager(&restManager);

    switch (mode) {
    case OneOfTest::FarmApiMode::UsePostPet:
    {
        farmApi.postFarmPet(pet, this, [&](const QRestReply &reply) {
            done = reply.isSuccess();
        });
    } break;
    case OneOfTest::FarmApiMode::UseDuck:
    {
        farmApi.postFarmPet(duck, this, [&](const QRestReply &reply) {
            done = reply.isSuccess();
        });
    } break;
    case OneOfTest::FarmApiMode::UseBunny:
    {
        farmApi.postFarmPet(bunny, this, [&](const QRestReply &reply) {
            done = reply.isSuccess();
        });
    } break;
    case OneOfTest::FarmApiMode::UseDog:
    {
        farmApi.postFarmPet(dog, this, [&](const QRestReply &reply) {
            done = reply.isSuccess();
        });
    } break;
    }
    QCOMPARE(logging.m_content, expectedJson);
    QTRY_COMPARE_EQ(done, false);
}

void OneOfTest::testPostFarmPetRequestJsonConversionMethods_data()
{
    QTest::addColumn<SchemasModelsOneOf::PostFarmPet_request>("postFarmPetRequest");
    QTest::addColumn<QString>("expectedJson");
    QTest::addColumn<QJsonValue>("expectedJsonValue");
    QTest::addColumn<QString>("oneOfBunnyExpectedJson");
    QTest::addColumn<QString>("oneOfDuckExpectedJson");
    QTest::addColumn<QString>("oneOfDogExpectedJson");
    QTest::addColumn<SchemasModelsOneOf::Bunny>("getOneOfBunny");
    QTest::addColumn<SchemasModelsOneOf::Duck>("getOneOfDuck");
    QTest::addColumn<SchemasModelsOneOf::Dog>("getOneOfDog");
    QTest::addColumn<bool>("isOneOfBunnyValid");
    QTest::addColumn<bool>("isOneOfBunnySet");
    QTest::addColumn<bool>("isOneOfDuckValid");
    QTest::addColumn<bool>("isOneOfDuckSet");
    QTest::addColumn<bool>("isOneOfDogValid");
    QTest::addColumn<bool>("isOneOfDogSet");
    QTest::addColumn<bool>("isValid");
    QTest::addColumn<bool>("isSet");

    SchemasModelsOneOf::PostFarmPet_request req;
    QTest::newRow("Empty PostFarmPet_request object")
        // postFarmPetRequest             // expectedJson
        << req                            << QString("{}"_L1)
        // expectedJsonValue
        << QJsonValue(QJsonObject())
        // oneOfBunnyExpectedJson         // oneOfDuckExpectedJson
        << QString("{}"_L1)               << QString("{}"_L1)
        // oneOfDogExpectedJson
        << QString("{}"_L1)
        // getOneOfBunny                  // getOneOfDuck
        << SchemasModelsOneOf::Bunny()    << SchemasModelsOneOf::Duck()
        // getOneOfDog
        << SchemasModelsOneOf::Dog()
        // isOneOfBunnyValid              // isOneOfBunnySet
        << false                          << false
        // isOneOfDuckValid               // isOneOfDuckSet
        << false                          << false
        // isOneOfDogValid                // isOneOfDogSet
        << false                          << false
        // isValid                        // isSet
        << false                          << false;

    // no required fields
    SchemasModelsOneOf::Bunny bunny;
    bunny.setWeight(5);
    bunny.setBreedType("Himalayan");

    // no required fields
    SchemasModelsOneOf::Duck duck;
    duck.setHunts(false);
    duck.setAge(7);

    // has required 'bark' - bool
    SchemasModelsOneOf::Dog hund;
    hund.setBark(false);
    hund.setBreed("French Bulldog");

    // check setters work fine with valid objects
    req.setOneOfBunny(bunny);
    QTest::newRow("Set a valid Bunny object via setter")
        // postFarmPetRequest             // expectedJson
        << req                            << bunny.asJson()
        // expectedJsonValue
        << bunny.asJsonValue()
        // oneOfBunnyExpectedJson
        << bunny.asJson()
        // oneOfDuckExpectedJson             // oneOfDogExpectedJson
        << QString("{}"_L1)               << QString("{}"_L1)
        // getOneOfBunny                  // getOneOfDuck
        << bunny                          << SchemasModelsOneOf::Duck()
        // getOneOfDog
        << SchemasModelsOneOf::Dog()
        // isOneOfBunnyValid              // isOneOfBunnySet
        << true                           << true
        // isOneOfDuckValid               // isOneOfDuckSet
        << false                          << false
        // isOneOfDogValid                // isOneOfDogSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    req.setOneOfDuck(duck);
    QTest::newRow("Set a valid Duck object via setter")
        // postFarmPetRequest             // expectedJson
        << req                            << duck.asJson()
        // expectedJsonValue
        << duck.asJsonValue()
        // oneOfBunnyExpectedJson         // oneOfDuckExpectedJson
        << QString("{}"_L1)               << duck.asJson()
        // oneOfDogExpectedJson
        << QString("{}"_L1)
        // getOneOfBunny                  // getOneOfDuck
        << SchemasModelsOneOf::Bunny()    << duck
        // getOneOfDog
        << SchemasModelsOneOf::Dog()
        // isOneOfBunnyValid              // isOneOfBunnySet
        << false                          << false
        // isOneOfDuckValid               // isOneOfDuckSet
        << true                           << true
        // isOneOfDogValid                // isOneOfDogSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    req.setOneOfDog(hund);
    QTest::newRow("Set a valid Dog object via setter")
        // postFarmPetRequest             // expectedJson
        << req                            << hund.asJson()
        // expectedJsonValue
        << hund.asJsonValue()
        // oneOfBunnyExpectedJson         // oneOfDuckExpectedJson
        << QString("{}"_L1)               << QString("{}"_L1)
        // oneOfDogExpectedJson
        << hund.asJson()
        // getOneOfBunny                  // getOneOfDuck
        << SchemasModelsOneOf::Bunny()    << SchemasModelsOneOf::Duck()
        // getOneOfDog
        << hund
        // isOneOfBunnyValid              // isOneOfBunnySet
        << false                          << false
        // isOneOfDuckValid               // isOneOfDuckSet
        << false                          << false
        // isOneOfDogValid                // isOneOfDogSet
        << true                           << true
        // isValid                        // isSet
        << true                           << true;

    // Set an empty Dog object, the dog has required fields, so an empty
    // dog is INVALID => entire PostFarmPet_request stays invalid
    const SchemasModelsOneOf::Dog emptyDog;
    req.setOneOfDog(emptyDog);
    QTest::newRow("Set an empty Dog object via setter")
        // postFarmPetRequest             // expectedJson
        << req                            << emptyDog.asJson()
        // expectedJsonValue
        << emptyDog.asJsonValue()
        // oneOfBunnyExpectedJson         // oneOfDuckExpectedJson
        << QString("{}"_L1)               << QString("{}"_L1)
        // oneOfDogExpectedJson
        << QString("{}"_L1)
        // getOneOfBunny                  // getOneOfDuck
        << SchemasModelsOneOf::Bunny()    << SchemasModelsOneOf::Duck()
        // getOneOfDog
        << emptyDog
        // isOneOfBunnyValid              // isOneOfBunnySet
        << false                          << false
        // isOneOfDuckValid               // isOneOfDuckSet
        << false                          << false
        // isOneOfDogValid                // isOneOfDogSet
        << false                          << false
        // isValid                        // isSet
        << false                          << false;

    // Set an empty Bunny object, the bunny has no required fields, so an empty
    // bunny is VALID => entire PostFarmPet_request becomes valid
    const SchemasModelsOneOf::Bunny emptyBunny;
    req.setOneOfBunny(emptyBunny);
    QTest::newRow("Set an empty Bunny object via setter")
        // postFarmPetRequest             // expectedJson
        << req                            << emptyBunny.asJson()
        // expectedJsonValue
        << emptyBunny.asJsonValue()
        // oneOfBunnyExpectedJson         // oneOfDuckExpectedJson
        << QString("{}"_L1)               << QString("{}"_L1)
        // oneOfDogExpectedJson
        << QString("{}"_L1)
        // getOneOfBunny                  // getOneOfDuck
        << emptyBunny                     << SchemasModelsOneOf::Duck()
        // getOneOfDog
        << SchemasModelsOneOf::Dog()
        // isOneOfBunnyValid              // isOneOfBunnySet
        << true                           << true
        // isOneOfDuckValid               // isOneOfDuckSet
        << false                          << false
        // isOneOfDogValid                // isOneOfDogSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    // Set an empty Duck object, the duck has no required fields, so an empty
    // duck is VALID => entire PostFarmPet_request becomes valid
    const SchemasModelsOneOf::Duck emptyDuck;
    req.setOneOfDuck(emptyDuck);
    QTest::newRow("Set an empty Duck object via setter")
        // postFarmPetRequest             // expectedJson
        << req                            << emptyDuck.asJson()
        // expectedJsonValue
        << emptyDuck.asJsonValue()
        // oneOfBunnyExpectedJson         // oneOfDuckExpectedJson
        << QString("{}"_L1)               << QString("{}"_L1)
        // oneOfDogExpectedJson
        << QString("{}"_L1)
        // getOneOfBunny                  // getOneOfDuck
        << emptyBunny                     << emptyDuck
        // getOneOfDog
        << SchemasModelsOneOf::Dog()
        // isOneOfBunnyValid              // isOneOfBunnySet
        << false                          << false
        // isOneOfDuckValid               // isOneOfDuckSet
        << true                           << true
        // isOneOfDogValid                // isOneOfDogSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    // fromJsonValue with EMPTY data.
    // Attempting to set an empty Dog via req.fromJsonValue().
    // Dog defines required field 'bark', so an empty object is INVALID for Dog.
    //
    // However, emptyDog.asJsonValue() produces '{}' because it is an empty object.
    //
    // The PostFarmPet_request::fromJsonValue() logic cannot determine whether '{}'
    // represents a Bunny, Duck, or Dog. The only available approach is to try
    // deserializing '{}' against each schema and select the one that validates
    // successfully.
    //
    // This leads to ambiguity, since '{}' is valid for both Bunny and Duck
    // (neither has required fields). Bunny is first, so it wins.
    //
    // To avoid such situations, the schema should define either a discriminator
    // or required fields for each Object in the YAML specification.
    //
    // This ambiguity is intentional here for testing purposes, to highlight
    // the importance of well-defined API schemas. Ensuring correctness of the
    // API design is the responsibility of the API author.
    req.fromJsonValue(emptyDog.asJsonValue());
    QTest::newRow("Set an empty Dog via ::fromJsonValue(emptyDog.asJsonValue())")
        // postFarmPetRequest             // expectedJson
        << req                            << emptyDog.asJson()
        // expectedJsonValue
        << emptyDog.asJsonValue()
        // oneOfBunnyExpectedJson         // oneOfDuckExpectedJson
        << QString("{}"_L1)               << QString("{}"_L1)
        // oneOfDogExpectedJson
        << QString("{}"_L1)
        // getOneOfBunny                  // getOneOfDuck
        << emptyBunny                     << emptyDuck
        // getOneOfDog
        << emptyDog
        // !!! NOTE: ambiguity between Bunny and Duck resolved to Bunny
        // (because it's first in the list)
        // isOneOfBunnyValid              // isOneOfBunnySet
        << true                           << true
        // isOneOfDuckValid               // isOneOfDuckSet
        << false                          << false
        // isOneOfDogValid                // isOneOfDogSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    // fromJson with EMPTY data. Same expectations as with
    // 'Set an empty Dog via req.fromJsonValue(emptyDog.asJsonValue())'
    req.fromJson(emptyDog.asJson());
    QTest::newRow("Set an empty Dog via ::fromJson(emptyDog.asJson())")
        // postFarmPetRequest             // expectedJson
        << req                            << emptyDog.asJson()
        // expectedJsonValue
        << emptyDog.asJsonValue()
        // oneOfBunnyExpectedJson         // oneOfDuckExpectedJson
        << QString("{}"_L1)               << QString("{}"_L1)
        // oneOfDogExpectedJson
        << QString("{}"_L1)
        // getOneOfBunny                  // getOneOfDuck
        << emptyBunny                     << emptyDuck
        // getOneOfDog
        << emptyDog
        // !!! NOTE: ambiguity between Bunny and Duck, Bunny wins
        // isOneOfBunnyValid              // isOneOfBunnySet
        << true                           << true
        // isOneOfDuckValid               // isOneOfDuckSet
        << false                          << false
        // isOneOfDogValid                // isOneOfDogSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    // ::fromJson with EMPTY Bunny data. Same expectations as above.
    req.fromJson(emptyBunny.asJson());
    QTest::newRow("Set an empty Bunny via ::fromJson(emptyBunny.asJson())")
        // postFarmPetRequest             // expectedJson
        << req                            << emptyBunny.asJson()
        // expectedJsonValue
        << emptyBunny.asJsonValue()
        // oneOfBunnyExpectedJson         // oneOfDuckExpectedJson
        << QString("{}"_L1)               << QString("{}"_L1)
        // oneOfDogExpectedJson
        << QString("{}"_L1)
        // getOneOfBunny                  // getOneOfDuck
        << emptyBunny                     << emptyDuck
        // getOneOfDog
        << emptyDog
        // isOneOfBunnyValid              // isOneOfBunnySet
        << true                           << true
        // isOneOfDuckValid               // isOneOfDuckSet
        << false                          << false
        // isOneOfDogValid                // isOneOfDogSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    req.fromJsonValue(emptyBunny.asJsonValue());
    QTest::newRow("Set an empty Bunny via ::fromJsonValue(emptyBunny.asJsonValue())")
        // postFarmPetRequest             // expectedJson
        << req                            << emptyBunny.asJson()
        // expectedJsonValue
        << emptyBunny.asJsonValue()
        // oneOfBunnyExpectedJson         // oneOfDuckExpectedJson
        << QString("{}"_L1)               << QString("{}"_L1)
        // oneOfDogExpectedJson
        << QString("{}"_L1)
        // getOneOfBunny                  // getOneOfDuck
        << emptyBunny                     << emptyDuck
        // getOneOfDog
        << emptyDog
        // isOneOfBunnyValid              // isOneOfBunnySet
        << true                           << true
        // isOneOfDuckValid               // isOneOfDuckSet
        << false                          << false
        // isOneOfDogValid                // isOneOfDogSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    // check ::fromJson() works fine for VALID bunny object
    // Using weight-only JSON to avoid 'breed-type' field ambiguity with Dog
    const QString bunnyWeight("{\"weight\":5}"_L1);
    req.fromJson(bunnyWeight);
    QTest::newRow("Set a valid Bunny object via ::fromJson()")
        // postFarmPetRequest             // expectedJson
        << req                            << bunnyWeight
        // expectedJsonValue
        << req.asJsonValue()
        // oneOfBunnyExpectedJson         // oneOfDuckExpectedJson
        << bunnyWeight                    << QString("{}"_L1)
        // oneOfDogExpectedJson
        << QString("{}"_L1)
        // getOneOfBunny                         // getOneOfDuck
        << SchemasModelsOneOf::Bunny(bunnyWeight) << emptyDuck
        // getOneOfDog
        << emptyDog
        // isOneOfBunnyValid              // isOneOfBunnySet
        << true                           << true
        // isOneOfDuckValid               // isOneOfDuckSet
        << false                          << false
        // isOneOfDogValid                // isOneOfDogSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    // check fromJsonValue() works fine for VALID bunny object
    const SchemasModelsOneOf::Bunny okBunny(bunnyWeight);
    req.fromJsonValue(okBunny.asJsonValue());
    QTest::newRow("Set a valid Bunny object via ::fromJsonValue()")
        // postFarmPetRequest             // expectedJson
        << req                            << bunnyWeight
        // expectedJsonValue
        << req.asJsonValue()
        // oneOfBunnyExpectedJson         // oneOfDuckExpectedJson
        << bunnyWeight                    << QString("{}"_L1)
        // oneOfDogExpectedJson
        << QString("{}"_L1)
        // getOneOfBunny                         // getOneOfDuck
        << SchemasModelsOneOf::Bunny(bunnyWeight) << emptyDuck
        // getOneOfDog
        << emptyDog
        // isOneOfBunnyValid              // isOneOfBunnySet
        << true                           << true
        // isOneOfDuckValid               // isOneOfDuckSet
        << false                          << false
        // isOneOfDogValid                // isOneOfDogSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    // check fromJson() works fine for VALID duck object
    const QString duckyDuck("{\"age\":7,\"hunts\":false}"_L1);
    req.fromJson(duckyDuck);
    QTest::newRow("Set a valid Duck object via ::fromJson()")
        // postFarmPetRequest             // expectedJson
        << req                            << duckyDuck
        // expectedJsonValue
        << req.asJsonValue()
        // oneOfBunnyExpectedJson         // oneOfDuckExpectedJson
        << QString("{}"_L1)               << duckyDuck
        // oneOfDogExpectedJson
        << QString("{}"_L1)
        // getOneOfBunny                  // getOneOfDuck
        << emptyBunny                     << SchemasModelsOneOf::Duck(duckyDuck)
        // getOneOfDog
        << emptyDog
        // isOneOfBunnyValid              // isOneOfBunnySet
        << false                          << false
        // isOneOfDuckValid               // isOneOfDuckSet
        << true                           << true
        // isOneOfDogValid                // isOneOfDogSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    // check ::fromJsonValue() works fine for VALID duck object
    const SchemasModelsOneOf::Duck okDuck(duckyDuck);
    req.fromJsonValue(okDuck.asJsonValue());
    QTest::newRow("Set a valid Duck object via ::fromJsonValue()")
        // postFarmPetRequest             // expectedJson
        << req                            << duckyDuck
        // expectedJsonValue
        << req.asJsonValue()
        // oneOfBunnyExpectedJson         // oneOfDuckExpectedJson
        << QString("{}"_L1)               << duckyDuck
        // oneOfDogExpectedJson
        << QString("{}"_L1)
        // getOneOfBunny                  // getOneOfDuck
        << emptyBunny                     << SchemasModelsOneOf::Duck(duckyDuck)
        // getOneOfDog
        << emptyDog
        // isOneOfBunnyValid              // isOneOfBunnySet
        << false                          << false
        // isOneOfDuckValid               // isOneOfDuckSet
        << true                           << true
        // isOneOfDogValid                // isOneOfDogSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    // check fromJson() works fine for VALID dog object
    const QString doggyDog("{\"bark\":false}"_L1);
    req.fromJson(doggyDog);
    QTest::newRow("Set a valid Dog object via ::fromJson()")
        // postFarmPetRequest             // expectedJson
        << req                            << doggyDog
        // expectedJsonValue
        << req.asJsonValue()
        // oneOfBunnyExpectedJson         // oneOfDuckExpectedJson
        << QString("{}"_L1)               << QString("{}"_L1)
        // oneOfDogExpectedJson
        << doggyDog
        // getOneOfBunny                  // getOneOfDuck
        << emptyBunny                     << emptyDuck
        // getOneOfDog
        << SchemasModelsOneOf::Dog(doggyDog)
        // isOneOfBunnyValid              // isOneOfBunnySet
        << false                          << false
        // isOneOfDuckValid               // isOneOfDuckSet
        << false                          << false
        // isOneOfDogValid                // isOneOfDogSet
        << true                           << true
        // isValid                       // isSet
        << true                           << true;

    // check fromJsonValue() works fine for VALID dog object
    const SchemasModelsOneOf::Dog okDog(doggyDog);
    req.fromJsonValue(okDog.asJsonValue());
    QTest::newRow("Set a valid Dog object via ::fromJsonValue()")
        // postFarmPetRequest             // expectedJson
        << req                            << doggyDog
        // expectedJsonValue
        << req.asJsonValue()
        // oneOfBunnyExpectedJson         // oneOfDuckExpectedJson
        << QString("{}"_L1)               << QString("{}"_L1)
        // oneOfDogExpectedJson
        << doggyDog
        // getOneOfBunny                  // getOneOfDuck
        << emptyBunny                     << emptyDuck
        // getOneOfDog
        << SchemasModelsOneOf::Dog(doggyDog)
        // isOneOfBunnyValid              // isOneOfBunnySet
        << false                          << false
        // isOneOfDuckValid               // isOneOfDuckSet
        << false                          << false
        // isOneOfDogValid                // isOneOfDogSet
        << true                           << true
        // isValid                        // isSet
        << true                           << true;

    // invalid json! => the model is RESET to INITIAL state
    req.fromJson("{invalid json}"_L1);
    QTest::newRow("Set an invalid json via ::fromJson()")
        // postFarmPetRequest             // expectedJson
        << req                            << QString("{}"_L1)
        // expectedJsonValue
        << req.asJsonValue()
        // oneOfBunnyExpectedJson         // oneOfDuckExpectedJson
        << QString("{}"_L1)               << QString("{}"_L1)
        // oneOfDogExpectedJson
        << QString("{}"_L1)
        // getOneOfBunny                  // getOneOfDuck
        << emptyBunny                     << emptyDuck
        // getOneOfDog
        << emptyDog
        // isOneOfBunnyValid              // isOneOfBunnySet
        << false                          << false
        // isOneOfDuckValid               // isOneOfDuckSet
        << false                          << false
        // isOneOfDogValid                // isOneOfDogSet
        << false                          << false
        // isValid                        // isSet
        << false                          << false;

    // Test with non-object JSON (array) => the model is RESET to INITIAL state
    req.fromJson("[]"_L1);
    QTest::newRow("Set an unexpected array [] via ::fromJson()")
        // postFarmPetRequest             // expectedJson
        << req                            << QString("{}"_L1)
        // expectedJsonValue
        << req.asJsonValue()
        // oneOfBunnyExpectedJson         // oneOfDuckExpectedJson
        << QString("{}"_L1)               << QString("{}"_L1)
        // oneOfDogExpectedJson
        << QString("{}"_L1)
        // getOneOfBunny                  // getOneOfDuck
        << emptyBunny                     << emptyDuck
        // getOneOfDog
        << emptyDog
        // isOneOfBunnyValid              // isOneOfBunnySet
        << false                          << false
        // isOneOfDuckValid               // isOneOfDuckSet
        << false                          << false
        // isOneOfDogValid                // isOneOfDogSet
        << false                          << false
        // isValid                        // isSet
        << false                          << false;

    // Test with non-object JSON (string) => the model is RESET to INITIAL state
    req.fromJson("\"just a string\""_L1);
    QTest::newRow("Set an unexpected string via ::fromJson()")
        // postFarmPetRequest             // expectedJson
        << req                            << QString("{}"_L1)
        // expectedJsonValue
        << req.asJsonValue()
        // oneOfBunnyExpectedJson         // oneOfDuckExpectedJson
        << QString("{}"_L1)               << QString("{}"_L1)
        // oneOfDogExpectedJson
        << QString("{}"_L1)
        // getOneOfBunny                  // getOneOfDuck
        << emptyBunny                     << emptyDuck
        // getOneOfDog
        << emptyDog
        // isOneOfBunnyValid              // isOneOfBunnySet
        << false                          << false
        // isOneOfDuckValid               // isOneOfDuckSet
        << false                          << false
        // isOneOfDogValid                // isOneOfDogSet
        << false                          << false
        // isValid                        // isSet
        << false                          << false;

    // Missing Required Fields (Dog's 'bark' is required)
    // Dog without required 'bark' field - should not be valid
    SchemasModelsOneOf::Dog invalidDog;
    invalidDog.setBreed("Husky-kolbasky");
    req.setOneOfDog(invalidDog);
    QTest::newRow("Set an invalid Dog via setter")
        // postFarmPetRequest             // expectedJson
        << req                            << QString("{}"_L1)
        // expectedJsonValue
        << req.asJsonValue()
        // oneOfBunnyExpectedJson         // oneOfDuckExpectedJson
        << QString("{}"_L1)               << QString("{}"_L1)
        // oneOfDogExpectedJson
        << invalidDog.asJson()
        // getOneOfBunny                  // getOneOfDuck
        << emptyBunny                     << emptyDuck
        // getOneOfDog
        << invalidDog
        // isOneOfBunnyValid              // isOneOfBunnySet
        << false                          << false
        // isOneOfDuckValid               // isOneOfDuckSet
        << false                          << false
        // The status here reflects the status of the model that was set.
        // The model's field was set by setter, so invalidDog.isSet() == true
        // but invalidDog.isValid() == false, because required field is not set.
        // isOneOfDogValid                // isOneOfDogSet
        << invalidDog.isValid()           << invalidDog.isSet()
        // isValid                        // isSet
        << false                          << true;

    // Try to parse JSON with only 'breed' field
    // Dog also has 'breed', but bark is missing => Dog is invalid.
    const QString testValue("{\"breed\":\"Retriever\"}"_L1);
    req.fromJson(testValue);
    QTest::newRow("Set a breed-only Dog JSON via ::fromJson()")
        // postFarmPetRequest             // expectedJson
        << req                            << QString("{}"_L1)
        // expectedJsonValue
        << req.asJsonValue()
        // oneOfBunnyExpectedJson         // oneOfDuckExpectedJson
        << QString("{}"_L1)               << QString("{}"_L1)
        // oneOfDogExpectedJson
        << testValue
        // getOneOfBunny                  // getOneOfDuck
        << emptyBunny                     << emptyDuck
        // getOneOfDog
        << SchemasModelsOneOf::Dog(testValue)
        // isOneOfBunnyValid              // isOneOfBunnySet
        << false                          << false
        // isOneOfDuckValid               // isOneOfDuckSet
        << false                          << false
        // isOneOfDogValid                // isOneOfDogSet
        << false                          << true
        // isValid                        // isSet
        << false                          << true;

    // Partial/Incomplete JSON Objects
    // Only weight field for Bunny (without breed-type),
    // should work since Bunny has no required fields
    const QString notFullBunny("{\"weight\":3}"_L1);
    req.fromJson(notFullBunny);
    QTest::newRow("Set a valid non-full Bunny via ::fromJson()")
        // postFarmPetRequest             // expectedJson
        << req                            << notFullBunny
        // expectedJsonValue
        << req.asJsonValue()
        // oneOfBunnyExpectedJson         // oneOfDuckExpectedJson
        << notFullBunny                   << QString("{}"_L1)
        // oneOfDogExpectedJson
        << QString("{}"_L1)
        // getOneOfBunny                          // getOneOfDuck
        << SchemasModelsOneOf::Bunny(notFullBunny) << emptyDuck
        // getOneOfDog
        << emptyDog
        // isOneOfBunnyValid              // isOneOfBunnySet
        << true                           << true
        // isOneOfDuckValid               // isOneOfDuckSet
        << false                          << false
        // isOneOfDogValid                // isOneOfDogSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    // Extra/Unknown Fields in JSON
    // JSON with fields not in schema - should parse known fields and ignore unknown
    // Using Duck fields + unknown fields to avoid breed ambiguity with Dog
    req.fromJson("{\"age\":7,\"hunts\":false,\"name\":\"Donald\",\"color\":\"white\"}"_L1);
    QString expected("{\"age\":7,\"hunts\":false}"_L1);
    QTest::newRow("Extra/Unknown Fields in JSON via ::fromJson()")
        // postFarmPetRequest             // expectedJson
        << req                            << expected
        // expectedJsonValue
        << req.asJsonValue()
        // oneOfBunnyExpectedJson         // oneOfDuckExpectedJson
        << QString("{}"_L1)               << expected
        // oneOfDogExpectedJson
        << QString("{}"_L1)
        // getOneOfBunny                  // getOneOfDuck
        << emptyBunny                     << SchemasModelsOneOf::Duck(expected)
        // getOneOfDog
        << emptyDog
        // isOneOfBunnyValid              // isOneOfBunnySet
        << false                          << false
        // isOneOfDuckValid               // isOneOfDuckSet
        << true                           << true
        // isOneOfDogValid                // isOneOfDogSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    // Invalid Field Types for Duck(). Wrong type for age (string instead of integer)
    // Note: fields of Duck type are not required, it means wrong fields can be omitted
    expected = QString("{\"age\":\"seven\",\"hunts\":false}"_L1);
    req.fromJson(expected);
    QTest::newRow("Invalid field 'age=seven' in Duck via ::fromJson()")
        // postFarmPetRequest             // expectedJson
        << req                            << QString("{\"hunts\":false}"_L1)
        // expectedJsonValue
        << req.asJsonValue()
        // oneOfBunnyExpectedJson         // oneOfDuckExpectedJson
        << QString("{}"_L1)               << QString("{\"hunts\":false}"_L1)
        // oneOfDogExpectedJson
        << QString("{}"_L1)
        // getOneOfBunny                  // getOneOfDuck
        << emptyBunny                     << SchemasModelsOneOf::Duck(expected)
        // getOneOfDog
        << emptyDog
        // isOneOfBunnyValid              // isOneOfBunnySet
        << false                          << false
        // isOneOfDuckValid               // isOneOfDuckSet
        << true                           << true
        // isOneOfDogValid                // isOneOfDogSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    // Invalid Field Types for Duck(). Wrong type for age (double instead of integer)
    // Note: fields of Duck type are not required, it means wrong fields can be omitted
    expected = QString("{\"age\":0.1,\"hunts\":true}"_L1);
    req.fromJson(expected);
    QTest::newRow("Invalid field 'age=0.1' in Duck via ::fromJson()")
        // postFarmPetRequest             // expectedJson
        << req                            << QString("{\"hunts\":true}"_L1)
        // expectedJsonValue
        << req.asJsonValue()
        // oneOfBunnyExpectedJson         // oneOfDuckExpectedJson
        << QString("{}"_L1)               << QString("{\"hunts\":true}"_L1)
        // oneOfDogExpectedJson
        << QString("{}"_L1)
        // getOneOfBunny                  // getOneOfDuck
        << emptyBunny                     << SchemasModelsOneOf::Duck(expected)
        // getOneOfDog
        << emptyDog
        // isOneOfBunnyValid              // isOneOfBunnySet
        << false                          << false
        // isOneOfDuckValid               // isOneOfDuckSet
        << true                           << true
        // isOneOfDogValid                // isOneOfDogSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    // Invalid Field Types for Duck(). Wrong type for age (bool instead of integer)
    // Note: fields of Duck type are not required, it means wrong fields can be omitted
    expected = QString("{\"age\":true,\"hunts\":true}"_L1);
    req.fromJson(expected);
    QTest::newRow("Invalid field 'age=true' in Duck via ::fromJson()")
        // postFarmPetRequest             // expectedJson
        << req                            << QString("{\"hunts\":true}"_L1)
        // expectedJsonValue
        << req.asJsonValue()
        // oneOfBunnyExpectedJson         // oneOfDuckExpectedJson
        << QString("{}"_L1)               << QString("{\"hunts\":true}"_L1)
        // oneOfDogExpectedJson
        << QString("{}"_L1)
        // getOneOfBunny                  // getOneOfDuck
        << emptyBunny                     << SchemasModelsOneOf::Duck(expected)
        // getOneOfDog
        << emptyDog
        // isOneOfBunnyValid              // isOneOfBunnySet
        << false                          << false
        // isOneOfDuckValid               // isOneOfDuckSet
        << true                           << true
        // isOneOfDogValid                // isOneOfDogSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    expected = QString("{\"age\":true,\"hunts\":1}"_L1);
    req.fromJson(expected);
    QTest::newRow("Invalid fields: 'age=true', hunts=1 in Duck via ::fromJson()")
        // postFarmPetRequest             // expectedJson
        << req                            << QString("{}"_L1)
        // expectedJsonValue
        << req.asJsonValue()
        // oneOfBunnyExpectedJson         // oneOfDuckExpectedJson
        << QString("{}"_L1)               << QString("{}"_L1)
        // oneOfDogExpectedJson
        << QString("{}"_L1)
        // getOneOfBunny                  // getOneOfDuck
        << emptyBunny                     << emptyDuck
        // getOneOfDog
        << emptyDog
        // isOneOfBunnyValid              // isOneOfBunnySet
        << false                          << false
        // isOneOfDuckValid               // isOneOfDuckSet
        << false                          << false
        // isOneOfDogValid                // isOneOfDogSet
        << false                          << false
        // isValid                        // isSet
        << false                          << false;
}

void OneOfTest::testPostFarmPetRequestJsonConversionMethods()
{
    QFETCH(SchemasModelsOneOf::PostFarmPet_request, postFarmPetRequest);
    QFETCH(QString, expectedJson);
    QFETCH(QJsonValue, expectedJsonValue);
    QFETCH(QString, oneOfBunnyExpectedJson);
    QFETCH(QString, oneOfDuckExpectedJson);
    QFETCH(QString, oneOfDogExpectedJson);
    QFETCH(SchemasModelsOneOf::Bunny, getOneOfBunny);
    QFETCH(SchemasModelsOneOf::Duck, getOneOfDuck);
    QFETCH(SchemasModelsOneOf::Dog, getOneOfDog);
    QFETCH(bool, isOneOfBunnyValid);
    QFETCH(bool, isOneOfBunnySet);
    QFETCH(bool, isOneOfDuckValid);
    QFETCH(bool, isOneOfDuckSet);
    QFETCH(bool, isOneOfDogValid);
    QFETCH(bool, isOneOfDogSet);
    QFETCH(bool, isValid);
    QFETCH(bool, isSet);

    QCOMPARE(postFarmPetRequest.asJson(), expectedJson);
    QCOMPARE(postFarmPetRequest.asJsonValue(), expectedJsonValue);
    QCOMPARE(postFarmPetRequest.getOneOfBunny().asJson(), oneOfBunnyExpectedJson);
    QCOMPARE(postFarmPetRequest.isOneOfBunnyValid(), isOneOfBunnyValid);
    QCOMPARE(postFarmPetRequest.isOneOfBunnySet(), isOneOfBunnySet);
    QCOMPARE(postFarmPetRequest.getOneOfDuck().asJson(), oneOfDuckExpectedJson);
    QCOMPARE(postFarmPetRequest.isOneOfDuckValid(), isOneOfDuckValid);
    QCOMPARE(postFarmPetRequest.isOneOfDuckSet(), isOneOfDuckSet);
    QCOMPARE(postFarmPetRequest.getOneOfDog().asJson(), oneOfDogExpectedJson);
    QCOMPARE(postFarmPetRequest.getOneOfBunny(), getOneOfBunny);
    QCOMPARE(postFarmPetRequest.getOneOfDuck(), getOneOfDuck);
    QCOMPARE(postFarmPetRequest.getOneOfDog(), getOneOfDog);
    QCOMPARE(postFarmPetRequest.isOneOfDogValid(), isOneOfDogValid);
    QCOMPARE(postFarmPetRequest.isOneOfDogSet(), isOneOfDogSet);
    QCOMPARE(postFarmPetRequest.isValid(), isValid);
    QCOMPARE(postFarmPetRequest.isSet(), isSet);
}

QTEST_MAIN(OneOfTest)
#include "tst_oneof.moc"
