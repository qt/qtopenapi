// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qoaicommonglobal.h"

#include "../basicSchemaAlternatives/client/bankapi.h"
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

    enum class BankApiPatchMode {
        UsePatchPaymentRequest,
        UseString,
        UseBool,
        UseDouble
    };

    enum class BankApiPostMode {
        UsePostPaymentRequest,
        UseCreditCard,
        UsePaypal
    };

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
    void testInlineSchemasJsonConversionMethods_data();
    void testInlineSchemasJsonConversionMethods();
    void testInlineSchemasPatch_data();
    void testInlineSchemasPatch();
    void testInlineSchemasPost_data();
    void testInlineSchemasPost();
    void testOneOfPrimitiveJsonConversionMethods_data();
    void testOneOfPrimitiveJsonConversionMethods();
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

void OneOfTest::testInlineSchemasJsonConversionMethods_data()
{
    QTest::addColumn<SchemasModelsOneOf::PostPaymentData_request>("postRequest");
    QTest::addColumn<QString>("expectedJson");
    QTest::addColumn<QJsonValue>("expectedJsonValue");
    QTest::addColumn<QString>("oneOf0ExpectedJson");
    QTest::addColumn<QString>("oneOf1ExpectedJson");
    QTest::addColumn<SchemasModelsOneOf::PostPaymentData_request_oneOf>("getOneOfPostPaymentData_request_oneOf");
    QTest::addColumn<SchemasModelsOneOf::PostPaymentData_request_oneOf_1>("getOneOfPostPaymentData_request_oneOf_1");
    QTest::addColumn<bool>("isOneOfPostPaymentDataRequestOneOfValid");
    QTest::addColumn<bool>("isOneOfPostPaymentDataRequestOneOfSet");
    QTest::addColumn<bool>("isOneOfPostPaymentDataRequestOneOf1Valid");
    QTest::addColumn<bool>("isOneOfPostPaymentDataRequestOneOf1Set");
    QTest::addColumn<bool>("isValid");
    QTest::addColumn<bool>("isSet");

    // PostPaymentData_request has two inline object schemas:
    // oneOf0: PostPaymentData_request_oneOf with cardNumber (string, REQUIRED)
    // and cardAvailability (bool)
    // oneOf1: PostPaymentData_request_oneOf_1 with paypalEmail (string)
    // and paypalAvailability (bool)
    SchemasModelsOneOf::PostPaymentData_request postRequest;
    QTest::newRow("Empty PostPaymentData_request object")
        // postRequest                    // expectedJson
        << postRequest                    << QString("{}"_L1)
        // expectedJsonValue
        << QJsonValue(QJsonObject())
        // oneOf0ExpectedJson             // oneOf1ExpectedJson
        << QString("{}"_L1)               << QString("{}"_L1)
        // getOneOfPostPaymentData_request_oneOf
        << SchemasModelsOneOf::PostPaymentData_request_oneOf()
        // getOneOfPostPaymentData_request_oneOf_1
        << SchemasModelsOneOf::PostPaymentData_request_oneOf_1()
        // isOneOfPostPaymentDataRequestOneOfValid
        << false
        // isOneOfPostPaymentDataRequestOneOfSet
        << false
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << false
        // isOneOfPostPaymentDataRequestOneOf1Set
        << false
        // isValid                        // isSet
        << false                          << false;

    // Create valid credit card (oneOf0) - has required 'cardNumber'
    SchemasModelsOneOf::PostPaymentData_request_oneOf creditCard;
    creditCard.setCardNumber("1234-5678-9012-3456");
    creditCard.setCardAvailability(true);

    // Create valid paypal (oneOf1) - no required fields
    SchemasModelsOneOf::PostPaymentData_request_oneOf_1 paypal;
    paypal.setPaypalEmail("user@example.com");
    paypal.setPaypalAvailability(true);

    // Set a valid credit card via setter
    postRequest.setOneOfPostPaymentData_request_oneOf(creditCard);
    QTest::newRow("Set a valid CreditCard object via setter")
        // postRequest                    // expectedJson
        << postRequest                    << creditCard.asJson()
        // expectedJsonValue
        << creditCard.asJsonValue()
        // oneOf0ExpectedJson             // oneOf1ExpectedJson
        << creditCard.asJson()            << QString("{}"_L1)
        // getOneOfPostPaymentData_request_oneOf
        << creditCard
        // getOneOfPostPaymentData_request_oneOf_1
        << SchemasModelsOneOf::PostPaymentData_request_oneOf_1()
        // isOneOfPostPaymentDataRequestOneOfValid
        << true
        // isOneOfPostPaymentDataRequestOneOfSet
        << true
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << false
        // isOneOfPostPaymentDataRequestOneOf1Set
        << false
        // isValid                        // isSet
        << true                           << true;

    // Set a valid paypal via setter (overrides credit card)
    postRequest.setOneOfPostPaymentData_request_oneOf_1(paypal);
    QTest::newRow("Set a valid Paypal object via setter")
        // postRequest                    // expectedJson
        << postRequest                    << paypal.asJson()
        // expectedJsonValue
        << paypal.asJsonValue()
        // oneOf0ExpectedJson             // oneOf1ExpectedJson
        << QString("{}"_L1)               << paypal.asJson()
        // getOneOfPostPaymentData_request_oneOf
        << SchemasModelsOneOf::PostPaymentData_request_oneOf()
        // getOneOfPostPaymentData_request_oneOf_1
        << paypal
        // isOneOfPostPaymentDataRequestOneOfValid
        << false
        // isOneOfPostPaymentDataRequestOneOfSet
        << false
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << true
        // isOneOfPostPaymentDataRequestOneOf1Set
        << true
        // isValid                        // isSet
        << true                           << true;

    // Set an empty CreditCard object. CreditCard has required 'cardNumber',
    // so an empty credit card is INVALID => entire PostPaymentData_request stays invalid
    const SchemasModelsOneOf::PostPaymentData_request_oneOf emptyCreditCard;
    postRequest.setOneOfPostPaymentData_request_oneOf(emptyCreditCard);
    QTest::newRow("Set an empty CreditCard object via setter")
        // postRequest                    // expectedJson
        << postRequest                    << emptyCreditCard.asJson()
        // expectedJsonValue
        << emptyCreditCard.asJsonValue()
        // oneOf0ExpectedJson             // oneOf1ExpectedJson
        << QString("{}"_L1)               << QString("{}"_L1)
        // getOneOfPostPaymentData_request_oneOf
        << emptyCreditCard
        // getOneOfPostPaymentData_request_oneOf_1
        << SchemasModelsOneOf::PostPaymentData_request_oneOf_1()
        // isOneOfPostPaymentDataRequestOneOfValid
        << false
        // isOneOfPostPaymentDataRequestOneOfSet
        << false
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << false
        // isOneOfPostPaymentDataRequestOneOf1Set
        << false
        // isValid                        // isSet
        << false                          << false;

    // Set an empty Paypal object. Paypal has no required fields,
    // so an empty paypal is VALID => entire PostPaymentData_request becomes valid
    const SchemasModelsOneOf::PostPaymentData_request_oneOf_1 emptyPaypal;
    postRequest.setOneOfPostPaymentData_request_oneOf_1(emptyPaypal);
    QTest::newRow("Set an empty Paypal object via setter")
        // postRequest                    // expectedJson
        << postRequest                    << emptyPaypal.asJson()
        // expectedJsonValue
        << emptyPaypal.asJsonValue()
        // oneOf0ExpectedJson             // oneOf1ExpectedJson
        << QString("{}"_L1)               << QString("{}"_L1)
        // getOneOfPostPaymentData_request_oneOf
        << emptyCreditCard
        // getOneOfPostPaymentData_request_oneOf_1
        << emptyPaypal
        // isOneOfPostPaymentDataRequestOneOfValid
        << false
        // isOneOfPostPaymentDataRequestOneOfSet
        << false
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << true
        // isOneOfPostPaymentDataRequestOneOf1Set
        << true
        // isValid                        // isSet
        << true                           << true;

    // fromJsonValue with EMPTY data.
    // Attempting to set an empty CreditCard via postRequest.fromJsonValue().
    // CreditCard defines required field 'cardNumber', so an empty object is INVALID
    // for CreditCard.
    //
    // However, emptyPaypal.asJsonValue() produces '{}' because it is an empty object.
    //
    // The PostPaymentData_request::fromJsonValue() logic cannot determine whether '{}'
    // represents a CreditCard or Paypal. It tries each schema:
    // - CreditCard: INVALID (missing required cardNumber)
    // - Paypal: VALID (no required fields)
    //
    // So '{}' resolves to Paypal (oneOf1).
    postRequest.fromJsonValue(emptyCreditCard.asJsonValue());
    QTest::newRow("Set an empty CreditCard via ::fromJsonValue(emptyCreditCard.asJsonValue())")
        // postRequest                    // expectedJson
        << postRequest                    << emptyCreditCard.asJson()
        // expectedJsonValue
        << emptyCreditCard.asJsonValue()
        // oneOf0ExpectedJson             // oneOf1ExpectedJson
        << QString("{}"_L1)               << QString("{}"_L1)
        // getOneOfPostPaymentData_request_oneOf
        << emptyCreditCard
        // getOneOfPostPaymentData_request_oneOf_1
        << emptyPaypal
        // !!! NOTE: ambiguity resolved to Paypal (oneOf1) since CreditCard is invalid
        // isOneOfPostPaymentDataRequestOneOfValid
        << false
        // isOneOfPostPaymentDataRequestOneOfSet
        << false
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << true
        // isOneOfPostPaymentDataRequestOneOf1Set
        << true
        // isValid                        // isSet
        << true                           << true;

    // fromJson with EMPTY data. Same expectations as above.
    postRequest.fromJson(emptyCreditCard.asJson());
    QTest::newRow("Set an empty CreditCard via ::fromJson(emptyCreditCard.asJson())")
        // postRequest                    // expectedJson
        << postRequest                    << emptyCreditCard.asJson()
        // expectedJsonValue
        << emptyCreditCard.asJsonValue()
        // oneOf0ExpectedJson             // oneOf1ExpectedJson
        << QString("{}"_L1)               << QString("{}"_L1)
        // getOneOfPostPaymentData_request_oneOf
        << emptyCreditCard
        // getOneOfPostPaymentData_request_oneOf_1
        << emptyPaypal
        // isOneOfPostPaymentDataRequestOneOfValid
        << false
        // isOneOfPostPaymentDataRequestOneOfSet
        << false
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << true
        // isOneOfPostPaymentDataRequestOneOf1Set
        << true
        // isValid                        // isSet
        << true                           << true;

    // ::fromJson with EMPTY Paypal data. Same expectations.
    postRequest.fromJson(emptyPaypal.asJson());
    QTest::newRow("Set an empty Paypal via ::fromJson(emptyPaypal.asJson())")
        // postRequest                    // expectedJson
        << postRequest                    << emptyPaypal.asJson()
        // expectedJsonValue
        << emptyPaypal.asJsonValue()
        // oneOf0ExpectedJson             // oneOf1ExpectedJson
        << QString("{}"_L1)               << QString("{}"_L1)
        // getOneOfPostPaymentData_request_oneOf
        << emptyCreditCard
        // getOneOfPostPaymentData_request_oneOf_1
        << emptyPaypal
        // isOneOfPostPaymentDataRequestOneOfValid
        << false
        // isOneOfPostPaymentDataRequestOneOfSet
        << false
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << true
        // isOneOfPostPaymentDataRequestOneOf1Set
        << true
        // isValid                        // isSet
        << true                           << true;

    postRequest.fromJsonValue(emptyPaypal.asJsonValue());
    QTest::newRow("Set an empty Paypal via ::fromJsonValue(emptyPaypal.asJsonValue())")
        // postRequest                    // expectedJson
        << postRequest                    << emptyPaypal.asJson()
        // expectedJsonValue
        << emptyPaypal.asJsonValue()
        // oneOf0ExpectedJson             // oneOf1ExpectedJson
        << QString("{}"_L1)               << QString("{}"_L1)
        // getOneOfPostPaymentData_request_oneOf
        << emptyCreditCard
        // getOneOfPostPaymentData_request_oneOf_1
        << emptyPaypal
        // isOneOfPostPaymentDataRequestOneOfValid
        << false
        // isOneOfPostPaymentDataRequestOneOfSet
        << false
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << true
        // isOneOfPostPaymentDataRequestOneOf1Set
        << true
        // isValid                        // isSet
        << true                           << true;

    // check ::fromJson() works fine for VALID credit card object
    const QString validCard("{\"cardAvailability\":true,\"cardNumber\":\"1234-5678-9012-3456\"}"_L1);
    postRequest.fromJson(validCard);
    QTest::newRow("Set a valid CreditCard object via ::fromJson()")
        // postRequest                    // expectedJson
        << postRequest                    << validCard
        // expectedJsonValue
        << postRequest.asJsonValue()
        // oneOf0ExpectedJson             // oneOf1ExpectedJson
        << validCard                      << QString("{}"_L1)
        // getOneOfPostPaymentData_request_oneOf
        << SchemasModelsOneOf::PostPaymentData_request_oneOf(validCard)
        // getOneOfPostPaymentData_request_oneOf_1
        << emptyPaypal
        // isOneOfPostPaymentDataRequestOneOfValid
        << true
        // isOneOfPostPaymentDataRequestOneOfSet
        << true
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << false
        // isOneOfPostPaymentDataRequestOneOf1Set
        << false
        // isValid                        // isSet
        << true                           << true;

    // check ::fromJsonValue() works fine for VALID credit card object
    const SchemasModelsOneOf::PostPaymentData_request_oneOf okCard(validCard);
    postRequest.fromJsonValue(okCard.asJsonValue());
    QTest::newRow("Set a valid CreditCard object via ::fromJsonValue()")
        // postRequest                    // expectedJson
        << postRequest                    << validCard
        // expectedJsonValue
        << postRequest.asJsonValue()
        // oneOf0ExpectedJson             // oneOf1ExpectedJson
        << validCard                      << QString("{}"_L1)
        // getOneOfPostPaymentData_request_oneOf
        << okCard
        // getOneOfPostPaymentData_request_oneOf_1
        << emptyPaypal
        // isOneOfPostPaymentDataRequestOneOfValid
        << true
        // isOneOfPostPaymentDataRequestOneOfSet
        << true
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << false
        // isOneOfPostPaymentDataRequestOneOf1Set
        << false
        // isValid                        // isSet
        << true                           << true;

    // check ::fromJson() works fine for VALID paypal object
    // Note:  an empty credit card (emptyCreditCard) is not valid and not set,
    // because it has a required field.
    const QString validPaypal("{\"paypalAvailability\":true,\"paypalEmail\":\"user@example.com\"}"_L1);
    postRequest.fromJson(validPaypal);
    QTest::newRow("Set a valid Paypal object via ::fromJson()")
        // postRequest                    // expectedJson
        << postRequest                    << validPaypal
        // expectedJsonValue
        << postRequest.asJsonValue()
        // oneOf0ExpectedJson             // oneOf1ExpectedJson
        << QString("{}"_L1)               << validPaypal
        // getOneOfPostPaymentData_request_oneOf
        << emptyCreditCard
        // getOneOfPostPaymentData_request_oneOf_1
        << SchemasModelsOneOf::PostPaymentData_request_oneOf_1(validPaypal)
        // isOneOfPostPaymentDataRequestOneOfValid
        << false
        // isOneOfPostPaymentDataRequestOneOfSet
        << false
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << true
        // isOneOfPostPaymentDataRequestOneOf1Set
        << true
        // isValid                        // isSet
        << true                           << true;

    // check ::fromJsonValue() works fine for VALID paypal object
    const SchemasModelsOneOf::PostPaymentData_request_oneOf_1 okPaypal(validPaypal);
    postRequest.fromJsonValue(okPaypal.asJsonValue());
    QTest::newRow("Set a valid Paypal object via ::fromJsonValue()")
        // postRequest                    // expectedJson
        << postRequest                    << validPaypal
        // expectedJsonValue
        << postRequest.asJsonValue()
        // oneOf0ExpectedJson             // oneOf1ExpectedJson
        << QString("{}"_L1)               << validPaypal
        // getOneOfPostPaymentData_request_oneOf
        << emptyCreditCard
        // getOneOfPostPaymentData_request_oneOf_1
        << okPaypal
        // isOneOfPostPaymentDataRequestOneOfValid
        << false
        // isOneOfPostPaymentDataRequestOneOfSet
        << false
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << true
        // isOneOfPostPaymentDataRequestOneOf1Set
        << true
        // isValid                        // isSet
        << true                           << true;

    // invalid json! => the model is RESET to INITIAL state
    postRequest.fromJson("{invalid json}"_L1);
    QTest::newRow("Set an invalid json via ::fromJson()")
        // postRequest                    // expectedJson
        << postRequest                    << QString("{}"_L1)
        // expectedJsonValue
        << postRequest.asJsonValue()
        // oneOf0ExpectedJson             // oneOf1ExpectedJson
        << QString("{}"_L1)               << QString("{}"_L1)
        // getOneOfPostPaymentData_request_oneOf
        << emptyCreditCard
        // getOneOfPostPaymentData_request_oneOf_1
        << emptyPaypal
        // isOneOfPostPaymentDataRequestOneOfValid
        << false
        // isOneOfPostPaymentDataRequestOneOfSet
        << false
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << false
        // isOneOfPostPaymentDataRequestOneOf1Set
        << false
        // isValid                        // isSet
        << false                          << false;

    // Test with non-object JSON (array) => the model is RESET to INITIAL state
    postRequest.fromJson("[]"_L1);
    QTest::newRow("Set an unexpected array [] via ::fromJson()")
        // postRequest                    // expectedJson
        << postRequest                    << QString("{}"_L1)
        // expectedJsonValue
        << postRequest.asJsonValue()
        // oneOf0ExpectedJson             // oneOf1ExpectedJson
        << QString("{}"_L1)               << QString("{}"_L1)
        // getOneOfPostPaymentData_request_oneOf
        << emptyCreditCard
        // getOneOfPostPaymentData_request_oneOf_1
        << emptyPaypal
        // isOneOfPostPaymentDataRequestOneOfValid
        << false
        // isOneOfPostPaymentDataRequestOneOfSet
        << false
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << false
        // isOneOfPostPaymentDataRequestOneOf1Set
        << false
        // isValid                        // isSet
        << false                          << false;

    // Test with non-object JSON (string) => the model is RESET to INITIAL state
    postRequest.fromJson("\"just a string\""_L1);
    QTest::newRow("Set an unexpected string via ::fromJson()")
        // postRequest                    // expectedJson
        << postRequest                    << QString("{}"_L1)
        // expectedJsonValue
        << postRequest.asJsonValue()
        // oneOf0ExpectedJson             // oneOf1ExpectedJson
        << QString("{}"_L1)               << QString("{}"_L1)
        // getOneOfPostPaymentData_request_oneOf
        << emptyCreditCard
        // getOneOfPostPaymentData_request_oneOf_1
        << emptyPaypal
        // isOneOfPostPaymentDataRequestOneOfValid
        << false
        // isOneOfPostPaymentDataRequestOneOfSet
        << false
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << false
        // isOneOfPostPaymentDataRequestOneOf1Set
        << false
        // isValid                        // isSet
        << false                          << false;

    // Missing Required Fields (CreditCard's 'cardNumber' is required)
    // CreditCard without required 'cardNumber' field - should not be valid
    SchemasModelsOneOf::PostPaymentData_request_oneOf invalidCard;
    invalidCard.setCardAvailability(true);
    postRequest.setOneOfPostPaymentData_request_oneOf(invalidCard);
    QTest::newRow("Set an invalid CreditCard via setter")
        // postRequest                    // expectedJson
        << postRequest                    << QString("{}"_L1)
        // expectedJsonValue
        << postRequest.asJsonValue()
        // oneOf0ExpectedJson                            // oneOf1ExpectedJson
        << QString("{\"cardAvailability\":true}"_L1)     << QString("{}"_L1)
        // getOneOfPostPaymentData_request_oneOf
        << invalidCard
        // getOneOfPostPaymentData_request_oneOf_1
        << emptyPaypal
        // The status here reflects the status of the model that was set.
        // The model's field was set by setter, so invalidCard.isSet() == true
        // but invalidCard.isValid() == false, because required field is not set.
        // isOneOfPostPaymentDataRequestOneOfValid
        << invalidCard.isValid()
        // isOneOfPostPaymentDataRequestOneOfSet
        << invalidCard.isSet()
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << false
        // isOneOfPostPaymentDataRequestOneOf1Set
        << false
        // isValid                        // isSet
        << false                          << true;

    // Try to parse JSON missing required field for CreditCard.
    // cardNumber is required but missing => CreditCard is invalid.
    // Paypal has paypalAvailability (bool) but not cardAvailability,
    // so this doesn't match Paypal either since field name differs.
    postRequest.fromJson("{\"cardAvailability\":true}"_L1);
    QTest::newRow("Set an invalid CreditCard via ::fromJson()")
        // postRequest                    // expectedJson
        << postRequest                    << QString("{}"_L1)
        // expectedJsonValue
        << postRequest.asJsonValue()
        // oneOf0ExpectedJson                        // oneOf1ExpectedJson
        << QString("{\"cardAvailability\":true}"_L1) << QString("{}"_L1)
        // getOneOfPostPaymentData_request_oneOf
        << SchemasModelsOneOf::PostPaymentData_request_oneOf("{\"cardAvailability\":true}"_L1)
        // getOneOfPostPaymentData_request_oneOf_1
        << emptyPaypal
        // isOneOfPostPaymentDataRequestOneOfValid
        << false
        // isOneOfPostPaymentDataRequestOneOfSet
        << true
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << false
        // isOneOfPostPaymentDataRequestOneOf1Set
        << false
        // isValid                        // isSet
        << false                          << true;

    // Partial/Incomplete JSON Objects
    // Only cardNumber for CreditCard (without cardAvailability),
    // should work since cardNumber is required and provided
    const QString cardOnly("{\"cardNumber\":\"9999-8888-7777-6666\"}"_L1);
    postRequest.fromJson(cardOnly);
    QTest::newRow("Set a valid non-full CreditCard via ::fromJson()")
        // postRequest                    // expectedJson
        << postRequest                    << cardOnly
        // expectedJsonValue
        << postRequest.asJsonValue()
        // oneOf0ExpectedJson             // oneOf1ExpectedJson
        << cardOnly                       << QString("{}"_L1)
        // getOneOfPostPaymentData_request_oneOf
        << SchemasModelsOneOf::PostPaymentData_request_oneOf(cardOnly)
        // getOneOfPostPaymentData_request_oneOf_1
        << emptyPaypal
        // isOneOfPostPaymentDataRequestOneOfValid
        << true
        // isOneOfPostPaymentDataRequestOneOfSet
        << true
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << false
        // isOneOfPostPaymentDataRequestOneOf1Set
        << false
        // isValid                        // isSet
        << true                           << true;

    // Only paypalEmail for Paypal (without paypalAvailability),
    // should work since Paypal has no required fields
    const QString emailOnly("{\"paypalEmail\":\"test@test.com\"}"_L1);
    postRequest.fromJson(emailOnly);
    QTest::newRow("Set a valid non-full Paypal via ::fromJson()")
        // postRequest                    // expectedJson
        << postRequest                    << emailOnly
        // expectedJsonValue
        << postRequest.asJsonValue()
        // oneOf0ExpectedJson             // oneOf1ExpectedJson
        << QString("{}"_L1)               << emailOnly
        // getOneOfPostPaymentData_request_oneOf
        << emptyCreditCard
        // getOneOfPostPaymentData_request_oneOf_1
        << SchemasModelsOneOf::PostPaymentData_request_oneOf_1(emailOnly)
        // isOneOfPostPaymentDataRequestOneOfValid
        << false
        // isOneOfPostPaymentDataRequestOneOfSet
        << false
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << true
        // isOneOfPostPaymentDataRequestOneOf1Set
        << true
        // isValid                        // isSet
        << true                           << true;

    // Extra/Unknown Fields in JSON
    // JSON with fields not in schema - should parse known fields and ignore unknown
    postRequest.fromJson("{\"cardNumber\":\"4444-3333\",\"cardAvailability\":false,\"unknown\":\"field\",\"extra\":123}"_L1);
    QString expected("{\"cardAvailability\":false,\"cardNumber\":\"4444-3333\"}"_L1);
    QTest::newRow("Extra/Unknown Fields in CreditCard JSON via ::fromJson()")
        // postRequest                    // expectedJson
        << postRequest                    << expected
        // expectedJsonValue
        << postRequest.asJsonValue()
        // oneOf0ExpectedJson             // oneOf1ExpectedJson
        << expected                       << QString("{}"_L1)
        // getOneOfPostPaymentData_request_oneOf
        << SchemasModelsOneOf::PostPaymentData_request_oneOf(expected)
        // getOneOfPostPaymentData_request_oneOf_1
        << emptyPaypal
        // isOneOfPostPaymentDataRequestOneOfValid
        << true
        // isOneOfPostPaymentDataRequestOneOfSet
        << true
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << false
        // isOneOfPostPaymentDataRequestOneOf1Set
        << false
        // isValid                        // isSet
        << true                           << true;

    // Extra/Unknown Fields in Paypal JSON
    postRequest.fromJson("{\"paypalEmail\":\"a@b.com\",\"paypalAvailability\":true,\"foo\":\"bar\"}"_L1);
    expected = QString("{\"paypalAvailability\":true,\"paypalEmail\":\"a@b.com\"}"_L1);
    QTest::newRow("Extra/Unknown Fields in Paypal JSON via ::fromJson()")
        // postRequest                    // expectedJson
        << postRequest                    << expected
        // expectedJsonValue
        << postRequest.asJsonValue()
        // oneOf0ExpectedJson             // oneOf1ExpectedJson
        << QString("{}"_L1)               << expected
        // getOneOfPostPaymentData_request_oneOf
        << emptyCreditCard
        // getOneOfPostPaymentData_request_oneOf_1
        << SchemasModelsOneOf::PostPaymentData_request_oneOf_1(expected)
        // isOneOfPostPaymentDataRequestOneOfValid
        << false
        // isOneOfPostPaymentDataRequestOneOfSet
        << false
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << true
        // isOneOfPostPaymentDataRequestOneOf1Set
        << true
        // isValid                        // isSet
        << true                           << true;

    // Empty String in CreditCard cardNumber field
    // cardNumber is required and present (even though empty string), so it's valid
    expected = QString("{\"cardNumber\":\"\"}"_L1);
    postRequest.fromJson(expected);
    QTest::newRow("An empty String in CreditCard cardNumber field via ::fromJson()")
        // postRequest                    // expectedJson
        << postRequest                    << expected
        // expectedJsonValue
        << postRequest.asJsonValue()
        // oneOf0ExpectedJson             // oneOf1ExpectedJson
        << expected                       << QString("{}"_L1)
        // getOneOfPostPaymentData_request_oneOf
        << SchemasModelsOneOf::PostPaymentData_request_oneOf(expected)
        // getOneOfPostPaymentData_request_oneOf_1
        << emptyPaypal
        // isOneOfPostPaymentDataRequestOneOfValid
        << true
        // isOneOfPostPaymentDataRequestOneOfSet
        << true
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << false
        // isOneOfPostPaymentDataRequestOneOf1Set
        << false
        // isValid                        // isSet
        << true                           << true;

    // Invalid Field Types for CreditCard. Wrong type for cardNumber (integer instead of string)
    // The CreditCard is set, but invalid, because the required field is ommited due to invalid
    // data. The entire model returns {}, the getter returns the data was valid on setting:
    // {\"cardAvailability\":true}
    expected = QString("{\"cardNumber\":12345,\"cardAvailability\":true}"_L1);
    postRequest.fromJson(expected);
    QTest::newRow("Invalid field 'cardNumber=12345' in CreditCard via ::fromJson()")
        // postRequest                    // expectedJson
        << postRequest                    << QString("{}"_L1)
        // expectedJsonValue
        << postRequest.asJsonValue()
        // oneOf0ExpectedJson                          // oneOf1ExpectedJson
        << QString("{\"cardAvailability\":true}"_L1)   << QString("{}"_L1)
        // getOneOfPostPaymentData_request_oneOf
        << SchemasModelsOneOf::PostPaymentData_request_oneOf(expected)
        // getOneOfPostPaymentData_request_oneOf_1
        << emptyPaypal
        // isOneOfPostPaymentDataRequestOneOfValid
        << false
        // isOneOfPostPaymentDataRequestOneOfSet
        << true
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << false
        // isOneOfPostPaymentDataRequestOneOf1Set
        << false
        // isValid                        // isSet
        << false                          << true;

    // Invalid Field Types for CreditCard. Wrong type for cardAvailability (string instead of bool)
    expected = QString("{\"cardNumber\":\"5555\",\"cardAvailability\":\"yes\"}"_L1);
    postRequest.fromJson(expected);
    QTest::newRow("Invalid field 'cardAvailability=yes' in CreditCard via ::fromJson()")
        // postRequest                    // expectedJson
        << postRequest                    << QString("{\"cardNumber\":\"5555\"}"_L1)
        // expectedJsonValue
        << postRequest.asJsonValue()
        // oneOf0ExpectedJson                      // oneOf1ExpectedJson
        << QString("{\"cardNumber\":\"5555\"}"_L1) << QString("{}"_L1)
        // getOneOfPostPaymentData_request_oneOf
        << SchemasModelsOneOf::PostPaymentData_request_oneOf(expected)
        // getOneOfPostPaymentData_request_oneOf_1
        << emptyPaypal
        // isOneOfPostPaymentDataRequestOneOfValid
        << true
        // isOneOfPostPaymentDataRequestOneOfSet
        << true
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << false
        // isOneOfPostPaymentDataRequestOneOf1Set
        << false
        // isValid                        // isSet
        << true                           << true;

    // Invalid Field Types for Paypal. Wrong type for paypalAvailability (integer instead of bool)
    // Note: paypalAvailability is optional, so wrong type is just omitted
    expected = QString("{\"paypalEmail\":\"x@y.com\",\"paypalAvailability\":1}"_L1);
    postRequest.fromJson(expected);
    QTest::newRow("Invalid field 'paypalAvailability=1' in Paypal via ::fromJson()")
        // postRequest                    // expectedJson
        << postRequest                    << QString("{\"paypalEmail\":\"x@y.com\"}"_L1)
        // expectedJsonValue
        << postRequest.asJsonValue()
        // oneOf0ExpectedJson             // oneOf1ExpectedJson
        << QString("{}"_L1)               << QString("{\"paypalEmail\":\"x@y.com\"}"_L1)
        // getOneOfPostPaymentData_request_oneOf
        << emptyCreditCard
        // getOneOfPostPaymentData_request_oneOf_1
        << SchemasModelsOneOf::PostPaymentData_request_oneOf_1(expected)
        // isOneOfPostPaymentDataRequestOneOfValid
        << false
        // isOneOfPostPaymentDataRequestOneOfSet
        << false
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << true
        // isOneOfPostPaymentDataRequestOneOf1Set
        << true
        // isValid                        // isSet
        << true                           << true;

    // Invalid Field Types for Paypal. Wrong type for paypalEmail (bool instead of string)
    // Note: paypalEmail is optional, so wrong type is just omitted
    expected = QString("{\"paypalEmail\":true,\"paypalAvailability\":false}"_L1);
    postRequest.fromJson(expected);
    QTest::newRow("Invalid field 'paypalEmail=true' in Paypal via ::fromJson()")
        // postRequest                    // expectedJson
        << postRequest                    << QString("{\"paypalAvailability\":false}"_L1)
        // expectedJsonValue
        << postRequest.asJsonValue()
        // oneOf0ExpectedJson             // oneOf1ExpectedJson
        << QString("{}"_L1)               << QString("{\"paypalAvailability\":false}"_L1)
        // getOneOfPostPaymentData_request_oneOf
        << emptyCreditCard
        // getOneOfPostPaymentData_request_oneOf_1
        << SchemasModelsOneOf::PostPaymentData_request_oneOf_1(expected)
        // isOneOfPostPaymentDataRequestOneOfValid
        << false
        // isOneOfPostPaymentDataRequestOneOfSet
        << false
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << true
        // isOneOfPostPaymentDataRequestOneOf1Set
        << true
        // isValid                        // isSet
        << true                           << true;

    // All fields have wrong types - both oneOfs fail to parse valid data
    expected = QString("{\"cardNumber\":true,\"cardAvailability\":\"yes\"}"_L1);
    postRequest.fromJson(expected);
    QTest::newRow("All invalid fields in CreditCard via ::fromJson()")
        // postRequest                    // expectedJson
        << postRequest                    << QString("{}"_L1)
        // expectedJsonValue
        << postRequest.asJsonValue()
        // oneOf0ExpectedJson             // oneOf1ExpectedJson
        << QString("{}"_L1)               << QString("{}"_L1)
        // getOneOfPostPaymentData_request_oneOf
        << emptyCreditCard
        // getOneOfPostPaymentData_request_oneOf_1
        << emptyPaypal
        // isOneOfPostPaymentDataRequestOneOfValid
        << false
        // isOneOfPostPaymentDataRequestOneOfSet
        << false
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << false
        // isOneOfPostPaymentDataRequestOneOf1Set
        << false
        // isValid                        // isSet
        << false                          << false;

    // CreditCard with only required field set via setter
    SchemasModelsOneOf::PostPaymentData_request_oneOf cardOnly2;
    cardOnly2.setCardNumber("0000-1111-2222-3333");
    postRequest.setOneOfPostPaymentData_request_oneOf(cardOnly2);
    QTest::newRow("Set CreditCard with only cardNumber via setter")
        // postRequest                    // expectedJson
        << postRequest                    << cardOnly2.asJson()
        // expectedJsonValue
        << cardOnly2.asJsonValue()
        // oneOf0ExpectedJson             // oneOf1ExpectedJson
        << cardOnly2.asJson()             << QString("{}"_L1)
        // getOneOfPostPaymentData_request_oneOf
        << cardOnly2
        // getOneOfPostPaymentData_request_oneOf_1
        << emptyPaypal
        // isOneOfPostPaymentDataRequestOneOfValid
        << true
        // isOneOfPostPaymentDataRequestOneOfSet
        << true
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << false
        // isOneOfPostPaymentDataRequestOneOf1Set
        << false
        // isValid                        // isSet
        << true                           << true;

    // Paypal with only paypalAvailability set via setter
    SchemasModelsOneOf::PostPaymentData_request_oneOf_1 availOnly;
    availOnly.setPaypalAvailability(false);
    postRequest.setOneOfPostPaymentData_request_oneOf_1(availOnly);
    QTest::newRow("Set Paypal with only paypalAvailability via setter")
        // postRequest                    // expectedJson
        << postRequest                    << availOnly.asJson()
        // expectedJsonValue
        << availOnly.asJsonValue()
        // oneOf0ExpectedJson             // oneOf1ExpectedJson
        << QString("{}"_L1)               << availOnly.asJson()
        // getOneOfPostPaymentData_request_oneOf
        << emptyCreditCard
        // getOneOfPostPaymentData_request_oneOf_1
        << availOnly
        // isOneOfPostPaymentDataRequestOneOfValid
        << false
        // isOneOfPostPaymentDataRequestOneOfSet
        << false
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << true
        // isOneOfPostPaymentDataRequestOneOf1Set
        << true
        // isValid                        // isSet
        << true                           << true;

    // fromJsonValue with valid CreditCard data
    postRequest.fromJsonValue(creditCard.asJsonValue());
    QTest::newRow("Set a valid CreditCard via ::fromJsonValue(creditCard.asJsonValue())")
        // postRequest                    // expectedJson
        << postRequest                    << creditCard.asJson()
        // expectedJsonValue
        << postRequest.asJsonValue()
        // oneOf0ExpectedJson             // oneOf1ExpectedJson
        << creditCard.asJson()            << QString("{}"_L1)
        // getOneOfPostPaymentData_request_oneOf
        << creditCard
        // getOneOfPostPaymentData_request_oneOf_1
        << emptyPaypal
        // isOneOfPostPaymentDataRequestOneOfValid
        << true
        // isOneOfPostPaymentDataRequestOneOfSet
        << true
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << false
        // isOneOfPostPaymentDataRequestOneOf1Set
        << false
        // isValid                        // isSet
        << true                           << true;

    // fromJsonValue with valid Paypal data
    postRequest.fromJsonValue(paypal.asJsonValue());
    QTest::newRow("Set a valid Paypal via ::fromJsonValue(paypal.asJsonValue())")
        // postRequest                    // expectedJson
        << postRequest                    << paypal.asJson()
        // expectedJsonValue
        << postRequest.asJsonValue()
        // oneOf0ExpectedJson             // oneOf1ExpectedJson
        << QString("{}"_L1)               << paypal.asJson()
        // getOneOfPostPaymentData_request_oneOf
        << emptyCreditCard
        // getOneOfPostPaymentData_request_oneOf_1
        << paypal
        // isOneOfPostPaymentDataRequestOneOfValid
        << false
        // isOneOfPostPaymentDataRequestOneOfSet
        << false
        // isOneOfPostPaymentDataRequestOneOf1Valid
        << true
        // isOneOfPostPaymentDataRequestOneOf1Set
        << true
        // isValid                        // isSet
        << true                           << true;
}

void OneOfTest::testInlineSchemasJsonConversionMethods()
{
    QFETCH(SchemasModelsOneOf::PostPaymentData_request, postRequest);
    QFETCH(QString, expectedJson);
    QFETCH(QJsonValue, expectedJsonValue);
    QFETCH(QString, oneOf0ExpectedJson);
    QFETCH(QString, oneOf1ExpectedJson);
    QFETCH(SchemasModelsOneOf::PostPaymentData_request_oneOf,
           getOneOfPostPaymentData_request_oneOf);
    QFETCH(SchemasModelsOneOf::PostPaymentData_request_oneOf_1,
           getOneOfPostPaymentData_request_oneOf_1);
    QFETCH(bool, isOneOfPostPaymentDataRequestOneOfValid);
    QFETCH(bool, isOneOfPostPaymentDataRequestOneOfSet);
    QFETCH(bool, isOneOfPostPaymentDataRequestOneOf1Valid);
    QFETCH(bool, isOneOfPostPaymentDataRequestOneOf1Set);
    QFETCH(bool, isValid);
    QFETCH(bool, isSet);

    QCOMPARE(postRequest.asJson(), expectedJson);
    QCOMPARE(postRequest.asJsonValue(), expectedJsonValue);
    QCOMPARE(postRequest.getOneOfPostPaymentData_request_oneOf().asJson(),
             oneOf0ExpectedJson);
    QCOMPARE(postRequest.isOneOfPostPaymentDataRequestOneOfValid(),
             isOneOfPostPaymentDataRequestOneOfValid);
    QCOMPARE(postRequest.isOneOfPostPaymentDataRequestOneOfSet(),
             isOneOfPostPaymentDataRequestOneOfSet);
    QCOMPARE(postRequest.getOneOfPostPaymentData_request_oneOf_1().asJson(),
             oneOf1ExpectedJson);
    QCOMPARE(postRequest.isOneOfPostPaymentDataRequestOneOf1Valid(),
             isOneOfPostPaymentDataRequestOneOf1Valid);
    QCOMPARE(postRequest.isOneOfPostPaymentDataRequestOneOf1Set(),
             isOneOfPostPaymentDataRequestOneOf1Set);
    QCOMPARE(postRequest.getOneOfPostPaymentData_request_oneOf(),
             getOneOfPostPaymentData_request_oneOf);
    QCOMPARE(postRequest.getOneOfPostPaymentData_request_oneOf_1(),
             getOneOfPostPaymentData_request_oneOf_1);
    QCOMPARE(postRequest.isValid(), isValid);
    QCOMPARE(postRequest.isSet(), isSet);
}

void OneOfTest::testInlineSchemasPatch_data()
{
    QTest::addColumn<SchemasModelsOneOf::PatchPaymentData_request>("patchPaymentRequest");
    QTest::addColumn<QString>("stringValue");
    QTest::addColumn<bool>("boolValue");
    QTest::addColumn<double>("doubleValue");
    QTest::addColumn<OneOfTest::BankApiPatchMode>("mode");
    QTest::addColumn<QString>("expectedJson");

    // Empty patchPaymentRequest
    QTest::newRow("empty PatchPaymentData_request")
        << SchemasModelsOneOf::PatchPaymentData_request{}
        << QString()
        << false
        << 0.0
        << OneOfTest::BankApiPatchMode::UsePatchPaymentRequest
        << QString("{}"_L1);

    // Test patchPaymentData with string value
    QTest::newRow("string value")
        << SchemasModelsOneOf::PatchPaymentData_request{}
        << QString("test-string-value"_L1)
        << false
        << 0.0
        << OneOfTest::BankApiPatchMode::UseString
        << QString("\"test-string-value\""_L1);

    // Test patchPaymentData with bool value
    QTest::newRow("bool value true")
        << SchemasModelsOneOf::PatchPaymentData_request{}
        << QString()
        << true
        << 0.0
        << OneOfTest::BankApiPatchMode::UseBool
        << QString("true"_L1);

    QTest::newRow("bool value false")
        << SchemasModelsOneOf::PatchPaymentData_request{}
        << QString()
        << false
        << 0.0
        << OneOfTest::BankApiPatchMode::UseBool
        << QString("false"_L1);

    // Test patchPaymentData with double value
    QTest::newRow("double value")
        << SchemasModelsOneOf::PatchPaymentData_request{}
        << QString()
        << false
        << 42.5
        << OneOfTest::BankApiPatchMode::UseDouble
        << QString("42.5"_L1);

    // Test PatchPaymentData_request with string set
    SchemasModelsOneOf::PatchPaymentData_request patchRequest;
    patchRequest.setOneOfQString("request-string"_L1);

    QTest::newRow("PatchPaymentData_request with string")
        << patchRequest
        << QString()
        << false
        << 0.0
        << OneOfTest::BankApiPatchMode::UsePatchPaymentRequest
        << QString("\"request-string\""_L1);

    // Test PatchPaymentData_request with bool set
    patchRequest.setOneOfBool(true);

    QTest::newRow("PatchPaymentData_request with bool")
        << patchRequest
        << QString()
        << false
        << 0.0
        << OneOfTest::BankApiPatchMode::UsePatchPaymentRequest
        << QString("true"_L1);

    // Test PatchPaymentData_request with double set
    patchRequest.setOneOfDouble(99.99);

    QTest::newRow("PatchPaymentData_request with double")
        << patchRequest
        << QString()
        << false
        << 0.0
        << OneOfTest::BankApiPatchMode::UsePatchPaymentRequest
        << QString("99.99"_L1);
}

void OneOfTest::testInlineSchemasPatch()
{
    QFETCH(SchemasModelsOneOf::PatchPaymentData_request, patchPaymentRequest);
    QFETCH(QString, stringValue);
    QFETCH(bool, boolValue);
    QFETCH(double, doubleValue);
    QFETCH(OneOfTest::BankApiPatchMode, mode);
    QFETCH(QString, expectedJson);

    bool done = true;
    SchemasModelsOneOf::BankApi bankApi;
    LoggingNetworkAccessManager logging(&bankApi);
    QRestAccessManager restManager(&logging, &bankApi);
    bankApi.setRestAccessManager(&restManager);

    switch (mode) {
    case OneOfTest::BankApiPatchMode::UsePatchPaymentRequest:
    {
        bankApi.patchPaymentData(patchPaymentRequest, this, [&](const QRestReply &reply) {
            done = reply.isSuccess();
        });
    } break;
    case OneOfTest::BankApiPatchMode::UseString:
    {
        bankApi.patchPaymentData(stringValue, this, [&](const QRestReply &reply) {
            done = reply.isSuccess();
        });
    } break;
    case OneOfTest::BankApiPatchMode::UseBool:
    {
        bankApi.patchPaymentData(boolValue, this, [&](const QRestReply &reply) {
            done = reply.isSuccess();
        });
    } break;
    case OneOfTest::BankApiPatchMode::UseDouble:
    {
        bankApi.patchPaymentData(doubleValue, this, [&](const QRestReply &reply) {
            done = reply.isSuccess();
        });
    } break;
    }
    QCOMPARE(logging.m_content, expectedJson);
    QTRY_COMPARE_EQ(done, false);
}

void OneOfTest::testInlineSchemasPost_data()
{
    QTest::addColumn<SchemasModelsOneOf::PostPaymentData_request>("postPaymentRequest");
    QTest::addColumn<SchemasModelsOneOf::PostPaymentData_request_oneOf>("creditCard");
    QTest::addColumn<SchemasModelsOneOf::PostPaymentData_request_oneOf_1>("paypal");
    QTest::addColumn<OneOfTest::BankApiPostMode>("mode");
    QTest::addColumn<QString>("expectedJson");

    // Empty postPaymentRequest
    QTest::newRow("empty PostPaymentData_request")
        << SchemasModelsOneOf::PostPaymentData_request{}
        << SchemasModelsOneOf::PostPaymentData_request_oneOf{}
        << SchemasModelsOneOf::PostPaymentData_request_oneOf_1{}
        << OneOfTest::BankApiPostMode::UsePostPaymentRequest
        << QString("{}"_L1);

    // Test Credit Card payment with required field
    SchemasModelsOneOf::PostPaymentData_request_oneOf creditCard;
    creditCard.setCardNumber("1234-5678-9012-3456");
    creditCard.setCardAvailability(true);

    QTest::newRow("credit card with required field")
        << SchemasModelsOneOf::PostPaymentData_request{}
        << creditCard
        << SchemasModelsOneOf::PostPaymentData_request_oneOf_1{}
        << OneOfTest::BankApiPostMode::UseCreditCard
        << QString("{\"cardAvailability\":true,\"cardNumber\":\"1234-5678-9012-3456\"}"_L1);

    // Test PayPal payment
    SchemasModelsOneOf::PostPaymentData_request_oneOf_1 paypal;
    paypal.setPaypalEmail("user@example.com");
    paypal.setPaypalAvailability(true);

    QTest::newRow("paypal payment")
        << SchemasModelsOneOf::PostPaymentData_request{}
        << SchemasModelsOneOf::PostPaymentData_request_oneOf{}
        << paypal
        << OneOfTest::BankApiPostMode::UsePaypal
        << QString("{\"paypalAvailability\":true,\"paypalEmail\":\"user@example.com\"}"_L1);

    // Test PostPaymentData_request with credit card set
    SchemasModelsOneOf::PostPaymentData_request postRequest;
    postRequest.setOneOfPostPaymentData_request_oneOf(creditCard);

    QTest::newRow("PostPaymentData_request with credit card")
        << postRequest
        << SchemasModelsOneOf::PostPaymentData_request_oneOf{}
        << SchemasModelsOneOf::PostPaymentData_request_oneOf_1{}
        << OneOfTest::BankApiPostMode::UsePostPaymentRequest
        << QString("{\"cardAvailability\":true,\"cardNumber\":\"1234-5678-9012-3456\"}"_L1);

    // Test PostPaymentData_request with PayPal set
    postRequest.setOneOfPostPaymentData_request_oneOf_1(paypal);

    QTest::newRow("PostPaymentData_request with paypal")
        << postRequest
        << SchemasModelsOneOf::PostPaymentData_request_oneOf{}
        << SchemasModelsOneOf::PostPaymentData_request_oneOf_1{}
        << OneOfTest::BankApiPostMode::UsePostPaymentRequest
        << QString("{\"paypalAvailability\":true,\"paypalEmail\":\"user@example.com\"}"_L1);

    // Test credit card without required field (cardNumber is required)
    SchemasModelsOneOf::PostPaymentData_request_oneOf invalidCreditCard;
    invalidCreditCard.setCardAvailability(true);

    QTest::newRow("invalid credit card missing required field")
        << SchemasModelsOneOf::PostPaymentData_request{}
        << invalidCreditCard
        << SchemasModelsOneOf::PostPaymentData_request_oneOf_1{}
        << OneOfTest::BankApiPostMode::UseCreditCard
        << QString("{}"_L1);

    // Test PayPal with only email (paypalAvailability is optional)
    SchemasModelsOneOf::PostPaymentData_request_oneOf_1 partialPaypal;
    partialPaypal.setPaypalEmail("partial@example.com");

    QTest::newRow("paypal with only email")
        << SchemasModelsOneOf::PostPaymentData_request{}
        << SchemasModelsOneOf::PostPaymentData_request_oneOf{}
        << partialPaypal
        << OneOfTest::BankApiPostMode::UsePaypal
        << QString("{\"paypalEmail\":\"partial@example.com\"}"_L1);
}

void OneOfTest::testInlineSchemasPost()
{
    QFETCH(SchemasModelsOneOf::PostPaymentData_request, postPaymentRequest);
    QFETCH(SchemasModelsOneOf::PostPaymentData_request_oneOf, creditCard);
    QFETCH(SchemasModelsOneOf::PostPaymentData_request_oneOf_1, paypal);
    QFETCH(OneOfTest::BankApiPostMode, mode);
    QFETCH(QString, expectedJson);

    bool done = true;
    SchemasModelsOneOf::BankApi bankApi;
    LoggingNetworkAccessManager logging(&bankApi);
    QRestAccessManager restManager(&logging, &bankApi);
    bankApi.setRestAccessManager(&restManager);

    switch (mode) {
    case OneOfTest::BankApiPostMode::UsePostPaymentRequest:
    {
        bankApi.postPaymentData(postPaymentRequest, this, [&](const QRestReply &reply) {
            done = reply.isSuccess();
        });
    } break;
    case OneOfTest::BankApiPostMode::UseCreditCard:
    {
        bankApi.postPaymentData(creditCard, this, [&](const QRestReply &reply) {
            done = reply.isSuccess();
        });
    } break;
    case OneOfTest::BankApiPostMode::UsePaypal:
    {
        bankApi.postPaymentData(paypal, this, [&](const QRestReply &reply) {
            done = reply.isSuccess();
        });
    } break;
    }
    QCOMPARE(logging.m_content, expectedJson);
    QTRY_COMPARE_EQ(done, false);
}

void OneOfTest::testOneOfPrimitiveJsonConversionMethods_data()
{
    QTest::addColumn<SchemasModelsOneOf::PatchPaymentData_request>("patchRequest");
    QTest::addColumn<QString>("expectedJson");
    QTest::addColumn<QJsonValue>("expectedJsonValue");
    QTest::addColumn<QString>("getOneOfQString");
    QTest::addColumn<bool>("getOneOfBool");
    QTest::addColumn<double>("getOneOfDouble");
    QTest::addColumn<bool>("isOneOfQStringValid");
    QTest::addColumn<bool>("isOneOfQStringSet");
    QTest::addColumn<bool>("isOneOfBoolValid");
    QTest::addColumn<bool>("isOneOfBoolSet");
    QTest::addColumn<bool>("isOneOfDoubleValid");
    QTest::addColumn<bool>("isOneOfDoubleSet");
    QTest::addColumn<bool>("isValid");
    QTest::addColumn<bool>("isSet");

    // PatchPaymentData_request has three primitive oneOf alternatives:
    // oneOf0: string
    // oneOf1: boolean
    // oneOf2: number (double)
    // All primitives have no "required fields" concept, so they are always "valid".
    SchemasModelsOneOf::PatchPaymentData_request patchRequest;
    QTest::newRow("Empty PatchPaymentData_request object")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("{}"_L1)
        // expectedJsonValue
        << QJsonValue(QJsonObject())
        // getOneOfQString                // getOneOfBool
        << QString()                      << false
        // getOneOfDouble
        << 0.0
        // isOneOfQStringValid            // isOneOfQStringSet
        << false                          << false
        // isOneOfBoolValid               // isOneOfBoolSet
        << false                          << false
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << false                          << false
        // isValid                        // isSet
        << false                          << false;

    // Set a string value via setter
    const QString hello("hello-world"_L1);
    patchRequest.setOneOfQString(hello);
    QTest::newRow("Set a valid string via setter")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("\"hello-world\""_L1)
        // expectedJsonValue
        << QJsonValue(hello)
        // getOneOfQString                // getOneOfBool
        << hello                          << false
        // getOneOfDouble
        << 0.0
        // isOneOfQStringValid            // isOneOfQStringSet
        << true                           << true
        // isOneOfBoolValid               // isOneOfBoolSet
        << false                          << false
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    // Set a bool value via setter (overrides string)
    patchRequest.setOneOfBool(true);
    QTest::newRow("Set a valid bool 'true' via setter")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("true"_L1)
        // expectedJsonValue
        << QJsonValue(true)
        // getOneOfQString                // getOneOfBool
        << QString()                      << true
        // getOneOfDouble
        << 0.0
        // isOneOfQStringValid            // isOneOfQStringSet
        << false                          << false
        // isOneOfBoolValid               // isOneOfBoolSet
        << true                           << true
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    // Set a bool 'false' value via setter
    patchRequest.setOneOfBool(false);
    QTest::newRow("Set a valid bool 'false' via setter")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("false"_L1)
        // expectedJsonValue
        << QJsonValue(false)
        // getOneOfQString                // getOneOfBool
        << QString()                      << false
        // getOneOfDouble
        << 0.0
        // isOneOfQStringValid            // isOneOfQStringSet
        << false                          << false
        // isOneOfBoolValid               // isOneOfBoolSet
        << true                           << true
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    // Set a double value via setter (overrides bool)
    patchRequest.setOneOfDouble(42.5);
    QTest::newRow("Set a valid double '42.5' via setter")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("42.5"_L1)
        // expectedJsonValue
        << QJsonValue(42.5)
        // getOneOfQString                // getOneOfBool
        << QString()                      << false
        // getOneOfDouble
        << 42.5
        // isOneOfQStringValid            // isOneOfQStringSet
        << false                          << false
        // isOneOfBoolValid               // isOneOfBoolSet
        << false                          << false
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << true                           << true
        // isValid                        // isSet
        << true                           << true;

    // Set a negative double value via setter
    patchRequest.setOneOfDouble(-99.99);
    QTest::newRow("Set a valid double '-99.99' via setter")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("-99.99"_L1)
        // expectedJsonValue
        << QJsonValue(-99.99)
        // getOneOfQString                // getOneOfBool
        << QString()                      << false
        // getOneOfDouble
        << -99.99
        // isOneOfQStringValid            // isOneOfQStringSet
        << false                          << false
        // isOneOfBoolValid               // isOneOfBoolSet
        << false                          << false
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << true                           << true
        // isValid                        // isSet
        << true                           << true;

    // Set a zero double value via setter
    patchRequest.setOneOfDouble(0.0);
    QTest::newRow("Set a valid double '0.0' via setter")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("0"_L1)
        // expectedJsonValue
        << QJsonValue(0.0)
        // getOneOfQString                // getOneOfBool
        << QString()                      << false
        // getOneOfDouble
        << 0.0
        // isOneOfQStringValid            // isOneOfQStringSet
        << false                          << false
        // isOneOfBoolValid               // isOneOfBoolSet
        << false                          << false
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << true                           << true
        // isValid                        // isSet
        << true                           << true;

    // Set an empty string via setter
    patchRequest.setOneOfQString("");
    QTest::newRow("Set an empty string via setter")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("\"\""_L1)
        // expectedJsonValue
        << QJsonValue(""_L1)
        // getOneOfQString                // getOneOfBool
        << QString(""_L1)                 << false
        // getOneOfDouble
        << 0.0
        // isOneOfQStringValid            // isOneOfQStringSet
        << true                           << true
        // isOneOfBoolValid               // isOneOfBoolSet
        << false                          << false
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    // fromJson with a string value
    patchRequest.fromJson("\"payment-token-abc\""_L1);
    QTest::newRow("Set a valid string via ::fromJson()")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("\"payment-token-abc\""_L1)
        // expectedJsonValue
        << QJsonValue("payment-token-abc"_L1)
        // getOneOfQString                // getOneOfBool
        << QString("payment-token-abc"_L1) << false
        // getOneOfDouble
        << 0.0
        // isOneOfQStringValid            // isOneOfQStringSet
        << true                           << true
        // isOneOfBoolValid               // isOneOfBoolSet
        << false                          << false
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    // fromJson with a bool 'true' value
    patchRequest.fromJson("true"_L1);
    QTest::newRow("Set a valid bool 'true' via ::fromJson()")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("true"_L1)
        // expectedJsonValue
        << QJsonValue(true)
        // getOneOfQString                // getOneOfBool
        << QString()                      << true
        // getOneOfDouble
        << 0.0
        // isOneOfQStringValid            // isOneOfQStringSet
        << false                          << false
        // isOneOfBoolValid               // isOneOfBoolSet
        << true                           << true
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    // fromJson with a bool 'false' value
    patchRequest.fromJson("false"_L1);
    QTest::newRow("Set a valid bool 'false' via ::fromJson()")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("false"_L1)
        // expectedJsonValue
        << QJsonValue(false)
        // getOneOfQString                // getOneOfBool
        << QString()                      << false
        // getOneOfDouble
        << 0.0
        // isOneOfQStringValid            // isOneOfQStringSet
        << false                          << false
        // isOneOfBoolValid               // isOneOfBoolSet
        << true                           << true
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    // fromJson with a double value
    patchRequest.fromJson("3.14159"_L1);
    QTest::newRow("Set a valid double '3.14159' via ::fromJson()")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("3.14159"_L1)
        // expectedJsonValue
        << QJsonValue(3.14159)
        // getOneOfQString                // getOneOfBool
        << QString()                      << false
        // getOneOfDouble
        << 3.14159
        // isOneOfQStringValid            // isOneOfQStringSet
        << false                          << false
        // isOneOfBoolValid               // isOneOfBoolSet
        << false                          << false
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << true                           << true
        // isValid                        // isSet
        << true                           << true;

    // fromJson with a negative double value
    patchRequest.fromJson("-100.5"_L1);
    QTest::newRow("Set a valid double '-100.5' via ::fromJson()")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("-100.5"_L1)
        // expectedJsonValue
        << QJsonValue(-100.5)
        // getOneOfQString                // getOneOfBool
        << QString()                      << false
        // getOneOfDouble
        << -100.5
        // isOneOfQStringValid            // isOneOfQStringSet
        << false                          << false
        // isOneOfBoolValid               // isOneOfBoolSet
        << false                          << false
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << true                           << true
        // isValid                        // isSet
        << true                           << true;

    // fromJson with an integer value (should match double/number)
    patchRequest.fromJson("42"_L1);
    QTest::newRow("Set an integer '42' via ::fromJson() (matches double)")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("42"_L1)
        // expectedJsonValue
        << QJsonValue(42.0)
        // getOneOfQString                // getOneOfBool
        << QString()                      << false
        // getOneOfDouble
        << 42.0
        // isOneOfQStringValid            // isOneOfQStringSet
        << false                          << false
        // isOneOfBoolValid               // isOneOfBoolSet
        << false                          << false
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << true                           << true
        // isValid                        // isSet
        << true                           << true;

    // fromJsonValue with a string value
    patchRequest.fromJsonValue(QJsonValue("json-value-string"_L1));
    QTest::newRow("Set a valid string via ::fromJsonValue()")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("\"json-value-string\""_L1)
        // expectedJsonValue
        << QJsonValue("json-value-string"_L1)
        // getOneOfQString                // getOneOfBool
        << QString("json-value-string"_L1) << false
        // getOneOfDouble
        << 0.0
        // isOneOfQStringValid            // isOneOfQStringSet
        << true                           << true
        // isOneOfBoolValid               // isOneOfBoolSet
        << false                          << false
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    // fromJsonValue with a bool value
    patchRequest.fromJsonValue(QJsonValue(true));
    QTest::newRow("Set a valid bool 'true' via ::fromJsonValue()")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("true"_L1)
        // expectedJsonValue
        << QJsonValue(true)
        // getOneOfQString                // getOneOfBool
        << QString()                      << true
        // getOneOfDouble
        << 0.0
        // isOneOfQStringValid            // isOneOfQStringSet
        << false                          << false
        // isOneOfBoolValid               // isOneOfBoolSet
        << true                           << true
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    // fromJsonValue with a double value
    patchRequest.fromJsonValue(QJsonValue(99.99));
    QTest::newRow("Set a valid double '99.99' via ::fromJsonValue()")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("99.99"_L1)
        // expectedJsonValue
        << QJsonValue(99.99)
        // getOneOfQString                // getOneOfBool
        << QString()                      << false
        // getOneOfDouble
        << 99.99
        // isOneOfQStringValid            // isOneOfQStringSet
        << false                          << false
        // isOneOfBoolValid               // isOneOfBoolSet
        << false                          << false
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << true                           << true
        // isValid                        // isSet
        << true                           << true;

    // invalid json! => the model is RESET to INITIAL state
    patchRequest.fromJson("{invalid json}"_L1);
    QTest::newRow("Set an invalid json via ::fromJson()")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("{}"_L1)
        // expectedJsonValue
        << QJsonValue(QJsonObject())
        // getOneOfQString                // getOneOfBool
        << QString()                      << false
        // getOneOfDouble
        << 0.0
        // isOneOfQStringValid            // isOneOfQStringSet
        << false                          << false
        // isOneOfBoolValid               // isOneOfBoolSet
        << false                          << false
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << false                          << false
        // isValid                        // isSet
        << false                          << false;

    // Test with non-primitive JSON (object) => doesn't match any primitive type
    patchRequest.fromJson("{\"key\":\"value\"}"_L1);
    QTest::newRow("Set an unexpected object via ::fromJson()")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("{}"_L1)
        // expectedJsonValue
        << QJsonValue(QJsonObject())
        // getOneOfQString                // getOneOfBool
        << QString()                      << false
        // getOneOfDouble
        << 0.0
        // isOneOfQStringValid            // isOneOfQStringSet
        << false                          << false
        // isOneOfBoolValid               // isOneOfBoolSet
        << false                          << false
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << false                          << false
        // isValid                        // isSet
        << false                          << false;

    // Test with non-primitive JSON (array) => doesn't match any primitive type
    patchRequest.fromJson("[1, 2, 3]"_L1);
    QTest::newRow("PatchPaymentData_request: set an unexpected array via ::fromJson()")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("{}"_L1)
        // expectedJsonValue
        << QJsonValue(QJsonObject())
        // getOneOfQString                // getOneOfBool
        << QString()                      << false
        // getOneOfDouble
        << 0.0
        // isOneOfQStringValid            // isOneOfQStringSet
        << false                          << false
        // isOneOfBoolValid               // isOneOfBoolSet
        << false                          << false
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << false                          << false
        // isValid                        // isSet
        << false                          << false;

    // Test with null JSON value => doesn't match string/bool/double
    patchRequest.fromJson("null"_L1);
    QTest::newRow("Set a null value via ::fromJson()")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("{}"_L1)
        // expectedJsonValue
        << QJsonValue(QJsonObject())
        // getOneOfQString                // getOneOfBool
        << QString()                      << false
        // getOneOfDouble
        << 0.0
        // isOneOfQStringValid            // isOneOfQStringSet
        << false                          << false
        // isOneOfBoolValid               // isOneOfBoolSet
        << false                          << false
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << false                          << false
        // isValid                        // isSet
        << false                          << false;

    // fromJsonValue with null QJsonValue => doesn't match primitives
    patchRequest.fromJsonValue(QJsonValue());
    QTest::newRow("Set a null QJsonValue via ::fromJsonValue()")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("{}"_L1)
        // expectedJsonValue
        << QJsonValue(QJsonObject())
        // getOneOfQString                // getOneOfBool
        << QString()                      << false
        // getOneOfDouble
        << 0.0
        // isOneOfQStringValid            // isOneOfQStringSet
        << false                          << false
        // isOneOfBoolValid               // isOneOfBoolSet
        << false                          << false
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << false                          << false
        // isValid                        // isSet
        << false                          << false;

    // fromJsonValue with QJsonObject => doesn't match primitives
    patchRequest.fromJsonValue(QJsonValue(QJsonObject()));
    QTest::newRow("Set an empty QJsonObject via ::fromJsonValue()")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("{}"_L1)
        // expectedJsonValue
        << QJsonValue(QJsonObject())
        // getOneOfQString                // getOneOfBool
        << QString()                      << false
        // getOneOfDouble
        << 0.0
        // isOneOfQStringValid            // isOneOfQStringSet
        << false                          << false
        // isOneOfBoolValid               // isOneOfBoolSet
        << false                          << false
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << false                          << false
        // isValid                        // isSet
        << false                          << false;

    // fromJson with an empty string JSON => should match string (oneOf0)
    patchRequest.fromJson("\"\""_L1);
    QTest::newRow("Set an empty string via ::fromJson()")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("\"\""_L1)
        // expectedJsonValue
        << QJsonValue(""_L1)
        // getOneOfQString                // getOneOfBool
        << QString(""_L1)                 << false
        // getOneOfDouble
        << 0.0
        // isOneOfQStringValid            // isOneOfQStringSet
        << true                           << true
        // isOneOfBoolValid               // isOneOfBoolSet
        << false                          << false
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    // fromJson with a very large double
    patchRequest.fromJson("1.7976931348623157e+308"_L1);
    QTest::newRow("Set a very large double via ::fromJson()")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("1.7976931348623157e+308"_L1)
        // expectedJsonValue
        << QJsonValue(1.7976931348623157e+308)
        // getOneOfQString                // getOneOfBool
        << QString()                      << false
        // getOneOfDouble
        << 1.7976931348623157e+308
        // isOneOfQStringValid            // isOneOfQStringSet
        << false                          << false
        // isOneOfBoolValid               // isOneOfBoolSet
        << false                          << false
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << true                           << true
        // isValid                        // isSet
        << true                           << true;

    // fromJson with a string that looks like a number (but is quoted => string)
    patchRequest.fromJson("\"42.5\""_L1);
    QTest::newRow("Set a string '42.5' (quoted) via ::fromJson()")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("\"42.5\""_L1)
        // expectedJsonValue
        << QJsonValue("42.5"_L1)
        // getOneOfQString                // getOneOfBool
        << QString("42.5"_L1)             << false
        // getOneOfDouble
        << 0.0
        // isOneOfQStringValid            // isOneOfQStringSet
        << true                           << true
        // isOneOfBoolValid               // isOneOfBoolSet
        << false                          << false
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    // fromJson with a string that looks like a bool (but is quoted => string)
    patchRequest.fromJson("\"true\""_L1);
    QTest::newRow("Set a string 'true' (quoted) via ::fromJson()")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("\"true\""_L1)
        // expectedJsonValue
        << QJsonValue("true"_L1)
        // getOneOfQString                // getOneOfBool
        << QString("true"_L1)             << false
        // getOneOfDouble
        << 0.0
        // isOneOfQStringValid            // isOneOfQStringSet
        << true                           << true
        // isOneOfBoolValid               // isOneOfBoolSet
        << false                          << false
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    // fromJson with zero (integer form) => matches double
    patchRequest.fromJson("0"_L1);
    QTest::newRow("Set zero '0' via ::fromJson() (matches double)")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("0"_L1)
        // expectedJsonValue
        << QJsonValue(0.0)
        // getOneOfQString                // getOneOfBool
        << QString()                      << false
        // getOneOfDouble
        << 0.0
        // isOneOfQStringValid            // isOneOfQStringSet
        << false                          << false
        // isOneOfBoolValid               // isOneOfBoolSet
        << false                          << false
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << true                           << true
        // isValid                        // isSet
        << true                           << true;

    // Set string with special characters via setter
    patchRequest.setOneOfQString("hello \"world\" \n\ttab");
    QTest::newRow("Set a string with special characters via setter")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("\"hello \\\"world\\\" \\n\\ttab\""_L1)
        // expectedJsonValue
        << QJsonValue("hello \"world\" \n\ttab"_L1)
        // getOneOfQString                // getOneOfBool
        << QString("hello \"world\" \n\ttab"_L1) << false
        // getOneOfDouble
        << 0.0
        // isOneOfQStringValid            // isOneOfQStringSet
        << true                           << true
        // isOneOfBoolValid               // isOneOfBoolSet
        << false                          << false
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    // Set double via setter then override with string via setter
    patchRequest.setOneOfDouble(123.456);
    patchRequest.setOneOfQString("override");
    QTest::newRow("Override double with string via setter")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("\"override\""_L1)
        // expectedJsonValue
        << QJsonValue("override"_L1)
        // getOneOfQString                // getOneOfBool
        << QString("override"_L1)         << false
        // getOneOfDouble
        << 0.0
        // isOneOfQStringValid            // isOneOfQStringSet
        << true                           << true
        // isOneOfBoolValid               // isOneOfBoolSet
        << false                          << false
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    // Set string via setter then override with bool via setter
    patchRequest.setOneOfQString("will be overridden");
    patchRequest.setOneOfBool(false);
    QTest::newRow("Override string with bool 'false' via setter")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("false"_L1)
        // expectedJsonValue
        << QJsonValue(false)
        // getOneOfQString                // getOneOfBool
        << QString()                      << false
        // getOneOfDouble
        << 0.0
        // isOneOfQStringValid            // isOneOfQStringSet
        << false                          << false
        // isOneOfBoolValid               // isOneOfBoolSet
        << true                           << true
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << false                          << false
        // isValid                        // isSet
        << true                           << true;

    // Set bool via setter then override with double via setter
    patchRequest.setOneOfBool(true);
    patchRequest.setOneOfDouble(0.001);
    QTest::newRow("Override bool with double '0.001' via setter")
        // patchRequest                   // expectedJson
        << patchRequest                   << QString("0.001"_L1)
        // expectedJsonValue
        << QJsonValue(0.001)
        // getOneOfQString                // getOneOfBool
        << QString()                      << false
        // getOneOfDouble
        << 0.001
        // isOneOfQStringValid            // isOneOfQStringSet
        << false                          << false
        // isOneOfBoolValid               // isOneOfBoolSet
        << false                          << false
        // isOneOfDoubleValid             // isOneOfDoubleSet
        << true                           << true
        // isValid                        // isSet
        << true                           << true;
}

void OneOfTest::testOneOfPrimitiveJsonConversionMethods()
{
    QFETCH(SchemasModelsOneOf::PatchPaymentData_request, patchRequest);
    QFETCH(QString, expectedJson);
    QFETCH(QJsonValue, expectedJsonValue);
    QFETCH(QString, getOneOfQString);
    QFETCH(bool, getOneOfBool);
    QFETCH(double, getOneOfDouble);
    QFETCH(bool, isOneOfQStringValid);
    QFETCH(bool, isOneOfQStringSet);
    QFETCH(bool, isOneOfBoolValid);
    QFETCH(bool, isOneOfBoolSet);
    QFETCH(bool, isOneOfDoubleValid);
    QFETCH(bool, isOneOfDoubleSet);
    QFETCH(bool, isValid);
    QFETCH(bool, isSet);

    QCOMPARE(patchRequest.asJson(), expectedJson);
    QCOMPARE(patchRequest.asJsonValue(), expectedJsonValue);
    QCOMPARE(patchRequest.getOneOfQString(), getOneOfQString);
    QCOMPARE(patchRequest.isOneOfQStringValid(), isOneOfQStringValid);
    QCOMPARE(patchRequest.isOneOfQStringSet(), isOneOfQStringSet);
    QCOMPARE(patchRequest.getOneOfBool(), getOneOfBool);
    QCOMPARE(patchRequest.isOneOfBoolValid(), isOneOfBoolValid);
    QCOMPARE(patchRequest.isOneOfBoolSet(), isOneOfBoolSet);
    QCOMPARE(patchRequest.getOneOfDouble(), getOneOfDouble);
    QCOMPARE(patchRequest.isOneOfDoubleValid(), isOneOfDoubleValid);
    QCOMPARE(patchRequest.isOneOfDoubleSet(), isOneOfDoubleSet);
    QCOMPARE(patchRequest.isValid(), isValid);
    QCOMPARE(patchRequest.isSet(), isSet);
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
