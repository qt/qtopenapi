// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qoaicommonglobal.h"

#include "../basicSchemas/client/bunny.h"
#include "../basicSchemas/client/cat.h"
#include "../basicSchemas/client/dog.h"
#include "../basicSchemas/client/duck.h"
#include "../basicSchemas/client/duck_family.h"
#include "../basicSchemas/client/postaccount_request.h"
#include "../basicSchemas/client/postaccount_request_otheraccdata_inner.h"
#include "../basicSchemas/client/dataapi.h"

#include <QtCore/qobject.h>
#include <QtTest/qtest.h>

#include <limits>

using namespace Qt::StringLiterals;
using namespace QtOpenApiCommon;

class StandardModelsTest : public QObject {
    Q_OBJECT

private Q_SLOTS:
    void testCatJsonConversionMethods_data();
    void testCatJsonConversionMethods();
    void testDogJsonConversionMethods_data();
    void testDogJsonConversionMethods();
    void testBunnyJsonConversionMethods_data();
    void testBunnyJsonConversionMethods();
    void testDuckFamilyJsonConversionMethods_data();
    void testDuckFamilyJsonConversionMethods();
    void testDuckJsonConversionMethods_data();
    void testDuckJsonConversionMethods();
    void testPostAccountRequestOtherAccDataInnerJsonConversionMethods_data();
    void testPostAccountRequestOtherAccDataInnerJsonConversionMethods();
    void testPostAccountRequestJsonConversionMethods_data();
    void testPostAccountRequestJsonConversionMethods();
};

void StandardModelsTest::testCatJsonConversionMethods_data()
{
    QTest::addColumn<StandardSchemasModels::Cat>("cat");
    QTest::addColumn<QString>("expectedJson");
    QTest::addColumn<bool>("isHuntsSet");
    QTest::addColumn<bool>("isHuntsValid");
    QTest::addColumn<bool>("isAgeSet");
    QTest::addColumn<bool>("isAgeValid");
    QTest::addColumn<bool>("modelIsValid");
    QTest::addColumn<bool>("modelIsSet");

    // Cat model doesn't have required fields.
    // If no fields are required, then an empty model is a valid case.
    QTest::newRow("empty") << StandardSchemasModels::Cat{} << QString("{}"_L1)
                           // isHuntsSet   isHuntsValid
                           << false        << false
                           // isAgeSet     isAgeValid
                           << false        << false
                           // modelIsValid modelIsSet
                           << true         << false;

    StandardSchemasModels::Cat kitty;
    kitty.setHunts(false);
    kitty.setAge(7);
    QTest::newRow("hunts=false; age=7") << kitty << QString("{\"age\":7,\"hunts\":false}"_L1)
                                       // isHuntsSet   isHuntsValid
                                       << true         << true
                                       // isAgeSet     isAgeValid
                                       << true         << true
                                       // modelIsValid modelIsSet
                                       << true         << true;

    // Only the 'age' field set; 'hunts' absent from JSON output.
    // Note: if fields are not required, they are optional.
    StandardSchemasModels::Cat ageOnly;
    ageOnly.setAge(3);
    QTest::newRow("age=3; hunts not set") << ageOnly << QString("{\"age\":3}"_L1)
                                         // isHuntsSet   isHuntsValid
                                         << false        << false
                                         // isAgeSet     isAgeValid
                                         << true         << true
                                         // modelIsValid modelIsSet
                                         << true         << true;

    // Only the optional 'hunts' field set; 'age' absent from JSON output.
    StandardSchemasModels::Cat huntsOnly;
    huntsOnly.setHunts(true);
    QTest::newRow("hunts=true; age not set") << huntsOnly << QString("{\"hunts\":true}"_L1)
                                            // isHuntsSet   isHuntsValid
                                            << true         << true
                                            // isAgeSet     isAgeValid
                                            << false        << false
                                            // modelIsValid modelIsSet
                                            << true         << true;

    // Model with a wrong type for 'age' (string instead of integer)
    // NOTE: 'age' should be omitted, because we're trying to set incorrect data type!
    // The test case for QTBUG-145410
    // The model stays valid because Cat has no required fields.
    StandardSchemasModels::Cat stringAgeType("{\"age\":\"seven\",\"hunts\":true}"_L1);
    QTest::newRow("hunts=true; age (string) dropped")
            << stringAgeType << QString("{\"hunts\":true}"_L1)
            // isHuntsSet   isHuntsValid
            << true         << true
            // isAgeSet     isAgeValid
            << false        << false
            // modelIsValid modelIsSet
            << true         << true;

    // Let's test double!
    // NOTE: 'age' should be omitted, because we're trying to set incorrect data type!
    // The test case for QTBUG-145410
    // The model stays valid because Cat has no required fields.
    StandardSchemasModels::Cat doubleAgeType("{\"age\":3.9,\"hunts\":true}"_L1);
    QTest::newRow("hunts=true; age (double) dropped")
            << doubleAgeType << QString("{\"hunts\":true}"_L1)
            // isHuntsSet   isHuntsValid
            << true         << true
            // isAgeSet     isAgeValid
            << false        << false
            // modelIsValid modelIsSet
            << true         << true;

    // Let's test a double that can be defined as integer mathematically!
    // See https://spec.openapis.org/oas/v3.1.1.html#data-types
    // Quote: "JSON Schema defines integers mathematically. This means that
    // both 1 and 1.0 are equivalent, and are both considered to be integers."
    // NOTE: 'age' should NOT be omitted, because we're trying to set correct data!
    // The test case for QTBUG-145410
    // The model stays valid because Cat has no required fields.
    StandardSchemasModels::Cat double2AgeType("{\"age\":3.0,\"hunts\":true}"_L1);
    QTest::newRow("hunts=true; age=3")
            << double2AgeType << QString("{\"age\":3,\"hunts\":true}"_L1)
            // isHuntsSet   isHuntsValid
            << true         << true
            // isAgeSet     isAgeValid
            << true         << true
            // modelIsValid modelIsSet
            << true         << true;


    // Let's test bool!
    // NOTE: 'age' should be omitted, because we're trying to set incorrect data type!
    // The test case for QTBUG-145410
    // The model stays valid because Cat has no required fields.
    StandardSchemasModels::Cat boolAgeType("{\"age\":true,\"hunts\":true}"_L1);
    QTest::newRow("hunts=true; age (bool) dropped")
            << boolAgeType << QString("{\"hunts\":true}"_L1)
            // isHuntsSet   isHuntsValid
            << true         << true
            // isAgeSet     isAgeValid
            << false        << false
            // modelIsValid modelIsSet
            << true         << true;

    // age is empty. The entire json is dropped, because it's invalid input data.
    // The model stays valid because Cat has no required fields.
    StandardSchemasModels::Cat brokenJsonType("{\"age\": ,\"hunts\":true}"_L1);
    QTest::newRow("Invalid json => age (empty) dropped") << brokenJsonType << QString("{}"_L1)
                                             // isHuntsSet   isHuntsValid
                                             << false        << false
                                             // isAgeSet     isAgeValid
                                             << false        << false
                                             // modelIsValid modelIsSet
                                             << true         << false;

    // Model with a wrong type for 'hunts' (integer instead of boolean):
    // 'hunts' is dropped, only 'age' survives.
    StandardSchemasModels::Cat wrongHuntsType("{\"age\":5,\"hunts\":42}"_L1);
    QTest::newRow("hunts dropped; age=5;") << wrongHuntsType << QString("{\"age\":5}"_L1)
                                           // isHuntsSet   isHuntsValid
                                           << false        << false
                                           // isAgeSet     isAgeValid
                                           << true         << true
                                           // modelIsValid modelIsSet
                                           << true         << true;

    const QString testVal = QString("{\"age\":9007199254740993,\"hunts\":true}"_L1);
    StandardSchemasModels::Cat bigValue(testVal);
    QTest::newRow("hunts=true; age=9007199254740993;")
            << bigValue << testVal
            // isHuntsSet   isHuntsValid
            << true         << true
            // isAgeSet     isAgeValid
            << true         << true
            // modelIsValid modelIsSet
            << true         << true;

    // <long long >::max() (9223372036854775807): maximum value for a signed 64-bit integer.
    const QString llongMaxVal =
            QString("{\"age\":%1,\"hunts\":true}"_L1).arg(std::numeric_limits<long long>::max());
    StandardSchemasModels::Cat llongMaxCat(llongMaxVal);
    QTest::newRow("hunts=true; age=<long long >::max();")
            << llongMaxCat << llongMaxVal
            // isHuntsSet   isHuntsValid
            << true         << true
            // isAgeSet     isAgeValid
            << true         << true
            // modelIsValid modelIsSet
            << true         << true;

    // <long long >::min() (-9223372036854775808): minimum value for a signed 64-bit integer.
    const QString llongMinVal =
            QString("{\"age\":%1,\"hunts\":true}"_L1).arg(std::numeric_limits<long long>::min());
    StandardSchemasModels::Cat llongMinCat(llongMinVal);
    QTest::newRow("hunts=true; age=LLONG_MIN;")
            << llongMinCat << llongMinVal
            // isHuntsSet   isHuntsValid
            << true         << true
            // isAgeSet     isAgeValid
            << true         << true
            // modelIsValid modelIsSet
            << true         << true;

    // ULLONG_MAX (18446744073709551615): maximum value for an unsigned 64-bit integer.
    // This exceeds the int64 range, so 'age' cannot be represented and must be dropped.
    StandardSchemasModels::Cat ullongMaxCat(
            QString("{\"age\":18446744073709551615,\"hunts\":true}"_L1));
    QTest::newRow("hunts=true; age=ULLONG_MAX (overflow, dropped);")
            << ullongMaxCat << QString("{\"hunts\":true}"_L1)
            // isHuntsSet   isHuntsValid
            << true         << true
            // isAgeSet     isAgeValid
            << false        << false
            // modelIsValid modelIsSet
            << true         << true;

    // Model with unknown fields: known fields are parsed,
    // unknown ones silently ignored.
    StandardSchemasModels::Cat unknownFields(
        "{\"age\":2,\"name\":\"Whiskers\",\"hunts\":false,\"color\":\"orange\"}"_L1);
    QTest::newRow("Additional unknown fields ignored")
        << unknownFields << QString("{\"age\":2,\"hunts\":false}"_L1)
        // isHuntsSet   isHuntsValid
        << true         << true
        // isAgeSet     isAgeValid
        << true         << true
        // modelIsValid modelIsSet
        << true         << true;

    StandardSchemasModels::Cat allUnknownFields;
    unknownFields.fromJson(
        "\"name\":\"Whiskers\",\"color\":\"orange\"}"_L1);
    QTest::newRow("All unknown fields dropped") << allUnknownFields << QString("{}"_L1)
                                                // isHuntsSet   isHuntsValid
                                                << false         << false
                                                // isAgeSet     isAgeValid
                                                << false         << false
                                                // modelIsValid modelIsSet
                                                << true         << false;

    // Model with invalid JSON:
    // so all fields stay at their initial invalid/unset state.
    // Cat is still valid (no required fields), but nothing is set.
    StandardSchemasModels::Cat fromInvalidJson("{invalid json}"_L1);
    QTest::newRow("invalid JSON") << fromInvalidJson << QString("{}"_L1)
                                  // isHuntsSet   isHuntsValid
                                  << false        << false
                                  // isAgeSet     isAgeValid
                                  << false        << false
                                  // modelIsValid modelIsSet
                                  << true         << false;
}

void StandardModelsTest::testCatJsonConversionMethods()
{
    QFETCH(StandardSchemasModels::Cat, cat);
    QFETCH(QString, expectedJson);
    QFETCH(bool, isHuntsSet);
    QFETCH(bool, isHuntsValid);
    QFETCH(bool, isAgeSet);
    QFETCH(bool, isAgeValid);
    QFETCH(bool, modelIsValid);
    QFETCH(bool, modelIsSet);

    // Expected model from _data()
    const QJsonValue testValue = QJsonValue::fromJson(QByteArrayView(expectedJson.toUtf8()));
    QCOMPARE(cat.asJson(), expectedJson);
    QCOMPARE(cat.asJsonObject(), testValue.toObject());
    QCOMPARE(cat.isHuntsSet(), isHuntsSet);
    QCOMPARE(cat.isHuntsValid(), isHuntsValid);
    QCOMPARE(cat.isAgeSet(), isAgeSet);
    QCOMPARE(cat.isAgeValid(), isAgeValid);
    QCOMPARE(cat.isValid(), modelIsValid);
    QCOMPARE(cat.isSet(), modelIsSet);

    // Model::fromJson()
    StandardSchemasModels::Cat fromJson;
    fromJson.fromJson(expectedJson);
    QCOMPARE(fromJson.asJson(), expectedJson);
    QCOMPARE(fromJson.asJsonObject(), testValue.toObject());
    QCOMPARE(fromJson.isHuntsSet(), isHuntsSet);
    QCOMPARE(fromJson.isHuntsValid(), isHuntsValid);
    QCOMPARE(fromJson.isAgeSet(), isAgeSet);
    QCOMPARE(fromJson.isAgeValid(), isAgeValid);
    QCOMPARE(fromJson.isSet(), modelIsSet);
    QCOMPARE(fromJson.isValid(), modelIsValid);
    QCOMPARE(fromJson, cat);

    // Model::fromJsonObject()
    StandardSchemasModels::Cat fromObject;
    fromObject.fromJsonObject(testValue.toObject());
    QCOMPARE(fromObject.asJson(), expectedJson);
    QCOMPARE(fromObject.asJsonObject(), testValue.toObject());
    QCOMPARE(fromObject.isHuntsSet(), isHuntsSet);
    QCOMPARE(fromObject.isHuntsValid(), isHuntsValid);
    QCOMPARE(fromObject.isAgeSet(), isAgeSet);
    QCOMPARE(fromObject.isAgeValid(), isAgeValid);
    QCOMPARE(fromObject.isSet(), modelIsSet);
    QCOMPARE(fromObject.isValid(), modelIsValid);
    QCOMPARE(fromObject, cat);
}

void StandardModelsTest::testDogJsonConversionMethods_data()
{
    QTest::addColumn<StandardSchemasModels::Dog>("dog");
    QTest::addColumn<QString>("expectedJson");
    QTest::addColumn<bool>("isBarkSet");
    QTest::addColumn<bool>("isBarkValid");
    QTest::addColumn<bool>("isBreedSet");
    QTest::addColumn<bool>("isBreedValid");
    QTest::addColumn<bool>("modelIsValid");
    QTest::addColumn<bool>("modelIsSet");

    // Dog model has 'bark' as a required field!
    // An empty Dog is therefore invalid.
    QTest::newRow("empty") << StandardSchemasModels::Dog{} << QString("{}"_L1)
                           // isBarkSet    isBarkValid
                           << false        << false
                           // isBreedSet   isBreedValid
                           << false        << false
                           // modelIsValid modelIsSet
                           << false        << false;

    StandardSchemasModels::Dog hund;
    hund.setBark(false);
    hund.setBreed("French Bulldog");
    QTest::newRow("bark=false; breed=French Bulldog")
        << hund << QString("{\"bark\":false,\"breed\":\"French Bulldog\"}"_L1)
        // isBarkSet    isBarkValid
        << true         << true
        // isBreedSet   isBreedValid
        << true         << true
        // modelIsValid modelIsSet
        << true         << true;

    // Only the required 'bark' field set; 'breed' absent from JSON output.
    StandardSchemasModels::Dog barkOnly;
    barkOnly.setBark(true);
    QTest::newRow("bark=true; breed not set") << barkOnly << QString("{\"bark\":true}"_L1)
                                             // isBarkSet    isBarkValid
                                             << true         << true
                                             // isBreedSet   isBreedValid
                                             << false        << false
                                             // modelIsValid modelIsSet
                                             << true         << true;

    // Model with a wrong type for 'bark' (string instead of boolean):
    // fromJsonValue() rejects the value, so 'bark' is dropped and the required
    // field is missing, so the model becomes invalid.
    StandardSchemasModels::Dog wrongBarkType("{\"bark\":\"yes\",\"breed\":\"Dingo\"}"_L1);
    QTest::newRow("breed=Dingo; bark=wrong type => invalid model")
        << wrongBarkType << QString("{\"breed\":\"Dingo\"}"_L1)
        // isBarkSet    isBarkValid
        << false        << false
        // isBreedSet   isBreedValid
        << true         << true
        // modelIsValid modelIsSet
        << false        << true; // 'breed' was parsed successfully, so isSet() is true

    // Model with a wrong type for 'breed' (integer instead of string):
    // 'breed' is dropped
    // 'bark' (required) is present, so the model stays valid.
    // The test case for QTBUG-145423
    StandardSchemasModels::Dog wrongBreedType("{\"bark\":true,\"breed\":42}"_L1);
    QTest::newRow("bark=true; breed=wrong (int) type => dropped")
        << wrongBreedType << QString("{\"bark\":true}"_L1)
        // isBarkSet    isBarkValid
        << true         << true
        // isBreedSet   isBreedValid
        << false        << false
        // modelIsValid modelIsSet
        << true         << true;

    // Model with a wrong type for 'breed' (bool instead of string):
    // 'breed' is dropped
    // 'bark' (required) is present, so the model stays valid.
    // The test case for QTBUG-145423
    StandardSchemasModels::Dog wrongIntBreedType("{\"bark\":true,\"breed\":true}"_L1);
    QTest::newRow("bark=true; breed=wrong (bool) type => dropped")
            << wrongIntBreedType << QString("{\"bark\":true}"_L1)
            // isBarkSet    isBarkValid
            << true         << true
            // isBreedSet   isBreedValid
            << false        << false
            // modelIsValid modelIsSet
            << true         << true;

    // Model with a wrong type for 'breed' (double instead of string):
    // 'breed' is dropped
    // 'bark' (required) is present, so the model stays valid.
    // The test case for QTBUG-145423
    StandardSchemasModels::Dog wrongDoubleBreedType("{\"bark\":true,\"breed\":42.99}"_L1);
    QTest::newRow("bark=true; breed=wrong (double) type => dropped")
            << wrongDoubleBreedType << QString("{\"bark\":true}"_L1)
            // isBarkSet    isBarkValid
            << true         << true
            // isBreedSet   isBreedValid
            << false        << false
            // modelIsValid modelIsSet
            << true         << true;

    // Model with a broken 'breed' (empty instead of string):
    // everything is dropped, so the model is invalid (required field is not set).
    StandardSchemasModels::Dog brokenBreedType("{\"bark\":true,\"breed\":}"_L1);
    QTest::newRow("bark=true; breed=broken field => dropped all")
            << brokenBreedType << QString("{}"_L1)
            // isBarkSet    isBarkValid
            << false        << false
            // isBreedSet   isBreedValid
            << false        << false
            // modelIsValid modelIsSet
            << false        << false;

    // Model with a string type that holds a stringified integer for 'breed':
    // 'breed' is valid
    // 'bark' (required) is present, so the model stays valid.
    StandardSchemasModels::Dog stringifiedIntBreedType("{\"bark\":true,\"breed\":\"42\"}"_L1);
    QTest::newRow("bark=true; breed=stringified integer")
        << stringifiedIntBreedType << QString("{\"bark\":true,\"breed\":\"42\"}"_L1)
        // isBarkSet    isBarkValid
        << true         << true
        // isBreedSet   isBreedValid
        << true         << true
        // modelIsValid modelIsSet
        << true         << true;

    // Model with unknown fields: known fields are parsed, unknown ones silently ignored.
    StandardSchemasModels::Dog unknownFields(
        "{\"bark\":false,\"name\":\"Rex\",\"age\":3,\"breed\":\"Husky\"}"_L1);
    QTest::newRow("unknown fields dropped")
        << unknownFields << QString("{\"bark\":false,\"breed\":\"Husky\"}"_L1)
        // isBarkSet    isBarkValid
        << true         << true
        // isBreedSet   isBreedValid
        << true         << true
        // modelIsValid modelIsSet
        << true         << true;

    // Model with invalid JSON
    // so all fields stay at their initial invalid/unset state.
    // 'bark' is required but not valid, so the model is invalid.
    StandardSchemasModels::Dog fromInvalidJson("{invalid json}"_L1);
    QTest::newRow("invalid JSON") << fromInvalidJson << QString("{}"_L1)
                                  // isBarkSet    isBarkValid
                                  << false        << false
                                  // isBreedSet   isBreedValid
                                  << false        << false
                                  // modelIsValid modelIsSet
                                  << false        << false;
}

void StandardModelsTest::testDogJsonConversionMethods()
{
    QFETCH(StandardSchemasModels::Dog, dog);
    QFETCH(QString, expectedJson);
    QFETCH(bool, isBarkSet);
    QFETCH(bool, isBarkValid);
    QFETCH(bool, isBreedSet);
    QFETCH(bool, isBreedValid);
    QFETCH(bool, modelIsValid);
    QFETCH(bool, modelIsSet);

    const QJsonValue testValue
        = QJsonValue::fromJson(QByteArrayView(expectedJson.toUtf8()));
    QCOMPARE(dog.asJson(), expectedJson);
    QCOMPARE(dog.asJsonObject(), testValue.toObject());
    QCOMPARE(dog.isBarkSet(), isBarkSet);
    QCOMPARE(dog.isBarkValid(), isBarkValid);
    QCOMPARE(dog.isBreedSet(), isBreedSet);
    QCOMPARE(dog.isBreedValid(), isBreedValid);
    QCOMPARE(dog.isValid(), modelIsValid);
    QCOMPARE(dog.isSet(), modelIsSet);

    // Model::fromJson()
    StandardSchemasModels::Dog fromJson;
    fromJson.fromJson(expectedJson);
    QCOMPARE(fromJson.asJson(), expectedJson);
    QCOMPARE(fromJson.asJsonObject(), testValue.toObject());
    QCOMPARE(fromJson.isBarkSet(), isBarkSet);
    QCOMPARE(fromJson.isBarkValid(), isBarkValid);
    QCOMPARE(fromJson.isBreedSet(), isBreedSet);
    QCOMPARE(fromJson.isBreedValid(), isBreedValid);
    QCOMPARE(fromJson.isSet(), modelIsSet);
    QCOMPARE(fromJson.isValid(), modelIsValid);
    QCOMPARE(fromJson, dog);

    // Model::fromJsonObject()
    StandardSchemasModels::Dog fromObject;
    fromObject.fromJsonObject(testValue.toObject());
    QCOMPARE(fromObject.asJson(), expectedJson);
    QCOMPARE(fromObject.asJsonObject(), testValue.toObject());
    QCOMPARE(fromObject.isBarkSet(), isBarkSet);
    QCOMPARE(fromObject.isBarkValid(), isBarkValid);
    QCOMPARE(fromObject.isBreedSet(), isBreedSet);
    QCOMPARE(fromObject.isBreedValid(), isBreedValid);
    QCOMPARE(fromObject.isSet(), modelIsSet);
    QCOMPARE(fromObject.isValid(), modelIsValid);
    QCOMPARE(fromObject, dog);
}

void StandardModelsTest::testBunnyJsonConversionMethods_data()
{
    QTest::addColumn<StandardSchemasModels::Bunny>("bunny");
    QTest::addColumn<QString>("expectedJson");
    QTest::addColumn<bool>("isWeightSet");
    QTest::addColumn<bool>("isWeightValid");
    QTest::addColumn<bool>("isExportingCountriesSet");
    QTest::addColumn<bool>("isExportingCountriesValid");
    QTest::addColumn<bool>("modelIsValid");
    QTest::addColumn<bool>("modelIsSet");

    // Bunny requires 'exporting-countries'. An empty Bunny is therefore invalid.
    QTest::newRow("empty") << StandardSchemasModels::Bunny{} << QString("{}"_L1)
                           // isWeightSet             isWeightValid
                           << false                   << false
                           // isExportingCountriesSet isExportingCountriesValid
                           << false                   << false
                           // modelIsValid            modelIsSet
                           << false                   << false;

    // Both fields set, model is valid.
    StandardSchemasModels::Bunny fullBunny;
    fullBunny.setWeight(2.5);
    fullBunny.setExportingCountries({u"Germany"_s, u"France"_s});
    QTest::newRow("weight=2.5; exportingCountries=[Germany,France]")
        << fullBunny
        << QString("{\"exporting-countries\":[\"Germany\",\"France\"],\"weight\":2.5}"_L1)
        // isWeightSet             isWeightValid
        << true                    << true
        // isExportingCountriesSet isExportingCountriesValid
        << true                    << true
        // modelIsValid            modelIsSet
        << true                    << true;

    // Only the required 'exporting-countries' field, model is valid.
    StandardSchemasModels::Bunny countriesOnly;
    countriesOnly.setExportingCountries({u"Poland"_s});
    QTest::newRow("exportingCountries=[Poland]; weight not set")
        << countriesOnly << QString("{\"exporting-countries\":[\"Poland\"]}"_L1)
        // isWeightSet             isWeightValid
        << false                   << false
        // isExportingCountriesSet isExportingCountriesValid
        << true                    << true
        // modelIsValid            modelIsSet
        << true                    << true;

    // Only the optional 'weight' field set, but
    // required 'exporting-countries' absent, so the model is invalid.
    StandardSchemasModels::Bunny weightOnly;
    weightOnly.setWeight(1.2);
    QTest::newRow("weight=1.2; exportingCountries not set")
        << weightOnly << QString("{\"weight\":1.2}"_L1)
        // isWeightSet             isWeightValid
        << true                    << true
        // isExportingCountriesSet isExportingCountriesValid
        << false                   << false
        // modelIsValid            modelIsSet
        << false                   << true;

    // Model with unknown fields: known fields are parsed, unknown ones silently ignored.
    // Model has "additionalProperties: true", it means all unknown fields can be added
    // to the resulting object as additional properties.
    // But it is not supported yet. Should be done by: QTBUG-143257
    const QString testString = QString("{\"exporting-countries\":[\"Japan\"],"
                                        "\"weight\":3.0,\"color\":\"white\"}"_L1);
    StandardSchemasModels::Bunny unknownFields(testString);
    QTest::newRow("Model preserves unknown fields")
        << unknownFields << testString
        // isWeightSet             isWeightValid
        << true                    << true
        // isExportingCountriesSet isExportingCountriesValid
        << true                    << true
        // modelIsValid            modelIsSet
        << true                    << true;

    // Model with an empty array for the required field 'exporting-countries':
    // Per OAS 3.1, an empty array is a valid value and satisfies the required
    // constraint (the key is present). So isExportingCountriesSet and
    // isExportingCountriesValid are both true, and isValid() is true.
    // However, the asJsonObject() omitted the empty arrays with size() > 0,
    // so the empty arrays were silently dropped on serialization.
    // Which is a bug and should be fixed.
    // Note: Sending "exporting-countries": [] clearly signals that the bunny
    // has no exporting countries.
    // Omitting could be ambiguously interpreted as “unknown” or “not specified”
    // depending on server logic.
    // The test case for QTBUG-145412
    StandardSchemasModels::Bunny emptyArray("{\"exporting-countries\":[]}"_L1);
    QTest::newRow("exporting-countries=[]")
        << emptyArray << QString("{\"exporting-countries\":[]}"_L1)
        // isWeightSet             isWeightValid
        << false                   << false
        // isExportingCountriesSet isExportingCountriesValid
        << true                    << true
        // modelIsValid            modelIsSet
        << true                    << true;

    // "exporting-countries" is Array of strings. Non-string value is interpreted as an error
    // The entire array omitted.
    // The test case for QTBUG-145413
    StandardSchemasModels::Bunny nonEmptyErrorArray("{\"exporting-countries\":[42]}"_L1);
    QTest::newRow("exporting-countries=[42]")
        << nonEmptyErrorArray << QString("{}"_L1)
        // isWeightSet             isWeightValid
        << false                   << false
        // isExportingCountriesSet isExportingCountriesValid
        << false                   << false
        // modelIsValid            modelIsSet
        << false                   << false;

    // "exporting-countries" is Array of strings. We can set the string value == "42".
    // The "exporting-countries" array is valid.
    // The model is valid.
    StandardSchemasModels::Bunny nonEmptyOkArray("{\"exporting-countries\":[\"42\"]}"_L1);
    QTest::newRow("exporting-countries=[\"42\"]")
        << nonEmptyOkArray << QString("{\"exporting-countries\":[\"42\"]}"_L1)
        // isWeightSet             isWeightValid
        << false                   << false
        // isExportingCountriesSet isExportingCountriesValid
        << true                    << true
        // modelIsValid            modelIsSet
        << true                    << true;

    StandardSchemasModels::Bunny errorArray("{\"exporting-countries\":[42, \"true\"]}"_L1);
    QTest::newRow("exporting-countries=[42, \"true\"]")
        << errorArray << QString("{}"_L1)
        // isWeightSet             isWeightValid
        << false                   << false
        // isExportingCountriesSet isExportingCountriesValid
        << false                   << false
        // modelIsValid            modelIsSet
        << false                   << false;

    StandardSchemasModels::Bunny nonEmptyArray("{\"exporting-countries\":[\"42\", \"true\"]}"_L1);
    QTest::newRow("exporting-countries=[\"42\",\"true\"]")
        << nonEmptyArray << QString("{\"exporting-countries\":[\"42\",\"true\"]}"_L1)
        // isWeightSet             isWeightValid
        << false                   << false
        // isExportingCountriesSet isExportingCountriesValid
        << true                    << true
        // modelIsValid            modelIsSet
        << true                    << true;

    // Model with invalid JSON: all fields stay unset/invalid.
    // 'exporting-countries' is required but missing, so the model is invalid.
    StandardSchemasModels::Bunny fromInvalidJson("{invalid json}"_L1);
    QTest::newRow("invalid JSON")
        << fromInvalidJson << QString("{}"_L1)
        // isWeightSet             isWeightValid
        << false                   << false
        // isExportingCountriesSet isExportingCountriesValid
        << false                   << false
        // modelIsValid            modelIsSet
        << false                   << false;
}

void StandardModelsTest::testBunnyJsonConversionMethods()
{
    QFETCH(StandardSchemasModels::Bunny, bunny);
    QFETCH(QString, expectedJson);
    QFETCH(bool, isWeightSet);
    QFETCH(bool, isWeightValid);
    QFETCH(bool, isExportingCountriesSet);
    QFETCH(bool, isExportingCountriesValid);
    QFETCH(bool, modelIsValid);
    QFETCH(bool, modelIsSet);

    QEXPECT_FAIL("Model preserves unknown fields",
                 "Known issue, should be fixed: QTBUG-143257", Abort);

    const QJsonValue testValue = QJsonValue::fromJson(QByteArrayView(expectedJson.toUtf8()));
    QCOMPARE(bunny.asJson(), expectedJson);
    QCOMPARE(bunny.asJsonObject(), testValue.toObject());
    QCOMPARE(bunny.isWeightSet(), isWeightSet);
    QCOMPARE(bunny.isWeightValid(), isWeightValid);
    QCOMPARE(bunny.isExportingCountriesSet(), isExportingCountriesSet);
    QCOMPARE(bunny.isExportingCountriesValid(), isExportingCountriesValid);
    QCOMPARE(bunny.isValid(), modelIsValid);
    QCOMPARE(bunny.isSet(), modelIsSet);

    // Model::fromJson()
    StandardSchemasModels::Bunny fromJson;
    fromJson.fromJson(expectedJson);
    QCOMPARE(fromJson.asJson(), expectedJson);
    QCOMPARE(fromJson.asJsonObject(), testValue.toObject());
    QCOMPARE(fromJson.isWeightSet(), isWeightSet);
    QCOMPARE(fromJson.isWeightValid(), isWeightValid);
    QCOMPARE(fromJson.isExportingCountriesSet(), isExportingCountriesSet);
    QCOMPARE(fromJson.isExportingCountriesValid(), isExportingCountriesValid);
    QCOMPARE(fromJson.isSet(), modelIsSet);
    QCOMPARE(fromJson.isValid(), modelIsValid);
    QCOMPARE(fromJson, bunny);

    // Model::fromJsonObject()
    StandardSchemasModels::Bunny fromObject;
    fromObject.fromJsonObject(testValue.toObject());
    QCOMPARE(fromObject.asJson(), expectedJson);
    QCOMPARE(fromObject.asJsonObject(), testValue.toObject());
    QCOMPARE(fromObject.isWeightSet(), isWeightSet);
    QCOMPARE(fromObject.isWeightValid(), isWeightValid);
    QCOMPARE(fromObject.isExportingCountriesSet(), isExportingCountriesSet);
    QCOMPARE(fromObject.isExportingCountriesValid(), isExportingCountriesValid);
    QCOMPARE(fromObject.isSet(), modelIsSet);
    QCOMPARE(fromObject.isValid(), modelIsValid);
    QCOMPARE(fromObject, bunny);
}

void StandardModelsTest::testDuckFamilyJsonConversionMethods_data()
{
    QTest::addColumn<StandardSchemasModels::Duck_family>("duckFamily");
    QTest::addColumn<QString>("expectedJson");
    QTest::addColumn<bool>("isCountryOfOriginSet");
    QTest::addColumn<bool>("isCountryOfOriginValid");
    QTest::addColumn<bool>("isCountSet");
    QTest::addColumn<bool>("isCountValid");
    QTest::addColumn<bool>("modelIsValid");
    QTest::addColumn<bool>("modelIsSet");

    // Duck_family has no required fields; an empty model is valid.
    QTest::newRow("empty") << StandardSchemasModels::Duck_family{} << QString("{}"_L1)
                           // isCountryOfOriginSet isCountryOfOriginValid
                           << false                << false
                           // isCountSet           isCountValid
                           << false                << false
                           // modelIsValid         modelIsSet
                           << true                 << false;

    // TBD have a look at floats parsing
    // JSON does not “round” floats automatically
    // Both fields set.
    StandardSchemasModels::Duck_family full;
    full.setCountryOfOrigin(u"Sweden"_s);
    full.setCount(4.1f);
    QTest::newRow("countryOfOrigin=Sweden; count=4.1")
        << full << QString("{\"count\":4.099999904632568,\"countryOfOrigin\":\"Sweden\"}"_L1)
        // isCountryOfOriginSet isCountryOfOriginValid
        << true                 << true
        // isCountSet           isCountValid
        << true                 << true
        // modelIsValid         modelIsSet
        << true                 << true;

    full.setCountryOfOrigin(u"Sweden"_s);
    full.setCount(4.0f);
    QTest::newRow("countryOfOrigin=Sweden; count=4")
        << full << QString("{\"count\":4,\"countryOfOrigin\":\"Sweden\"}"_L1)
        // isCountryOfOriginSet isCountryOfOriginValid
        << true                 << true
        // isCountSet           isCountValid
        << true                 << true
        // modelIsValid         modelIsSet
        << true                 << true;


    // Only 'countryOfOrigin' set.
    StandardSchemasModels::Duck_family countryOnly;
    countryOnly.setCountryOfOrigin(u"Canada"_s);
    QTest::newRow("countryOfOrigin=Canada; count not set")
        << countryOnly << QString("{\"countryOfOrigin\":\"Canada\"}"_L1)
        // isCountryOfOriginSet isCountryOfOriginValid
        << true                 << true
        // isCountSet           isCountValid
        << false                << false
        // modelIsValid         modelIsSet
        << true                 << true;

    // Only 'count' set.
    StandardSchemasModels::Duck_family countOnly;
    countOnly.setCount(3.5f);
    QTest::newRow("count=3.5; countryOfOrigin not set")
        << countOnly << QString("{\"count\":3.5}"_L1)
        // isCountryOfOriginSet isCountryOfOriginValid
        << false                << false
        // isCountSet           isCountValid
        << true                 << true
        // modelIsValid         modelIsSet
        << true                 << true;

    // Model with a wrong type for 'count' (string instead of float):
    // fromJsonValue() rejects the value, so 'count' is dropped.
    StandardSchemasModels::Duck_family wrongCountType(
        "{\"countryOfOrigin\":\"Brazil\",\"count\":\"many\"}"_L1);
    QTest::newRow("count=wrong type; dropped")
        << wrongCountType << QString("{\"countryOfOrigin\":\"Brazil\"}"_L1)
        // isCountryOfOriginSet isCountryOfOriginValid
        << true                 << true
        // isCountSet           isCountValid
        << false                << false
        // modelIsValid         modelIsSet
        << true                 << true;

    // fromJson() with invalid JSON: all fields stay unset.
    StandardSchemasModels::Duck_family fromInvalidJson("{invalid json}"_L1);
    QTest::newRow("invalid JSON")
        << fromInvalidJson << QString("{}"_L1)
        // isCountryOfOriginSet isCountryOfOriginValid
        << false                << false
        // isCountSet           isCountValid
        << false                << false
        // modelIsValid         modelIsSet
        << true                 << false;
}

void StandardModelsTest::testDuckFamilyJsonConversionMethods()
{
    QFETCH(StandardSchemasModels::Duck_family, duckFamily);
    QFETCH(QString, expectedJson);
    QFETCH(bool, isCountryOfOriginSet);
    QFETCH(bool, isCountryOfOriginValid);
    QFETCH(bool, isCountSet);
    QFETCH(bool, isCountValid);
    QFETCH(bool, modelIsValid);
    QFETCH(bool, modelIsSet);

    const QJsonValue testValue = QJsonValue::fromJson(QByteArrayView(expectedJson.toUtf8()));
    QCOMPARE(duckFamily.asJson(), expectedJson);
    QCOMPARE(duckFamily.asJsonObject(), testValue.toObject());
    QCOMPARE(duckFamily.isCountryOfOriginSet(), isCountryOfOriginSet);
    QCOMPARE(duckFamily.isCountryOfOriginValid(), isCountryOfOriginValid);
    QCOMPARE(duckFamily.isCountSet(), isCountSet);
    QCOMPARE(duckFamily.isCountValid(), isCountValid);
    QCOMPARE(duckFamily.isValid(), modelIsValid);
    QCOMPARE(duckFamily.isSet(), modelIsSet);

    // Model::fromJson()
    StandardSchemasModels::Duck_family fromJson;
    fromJson.fromJson(expectedJson);
    QCOMPARE(fromJson.asJson(), expectedJson);
    QCOMPARE(fromJson.asJsonObject(), testValue.toObject());
    QCOMPARE(fromJson.isCountryOfOriginSet(), isCountryOfOriginSet);
    QCOMPARE(fromJson.isCountryOfOriginValid(), isCountryOfOriginValid);
    QCOMPARE(fromJson.isCountSet(), isCountSet);
    QCOMPARE(fromJson.isCountValid(), isCountValid);
    QCOMPARE(fromJson.isSet(), modelIsSet);
    QCOMPARE(fromJson.isValid(), modelIsValid);
    QCOMPARE(fromJson, duckFamily);

    // Model::fromJsonObject()
    StandardSchemasModels::Duck_family fromObject;
    fromObject.fromJsonObject(testValue.toObject());
    QCOMPARE(fromObject.asJson(), expectedJson);
    QCOMPARE(fromObject.asJsonObject(), testValue.toObject());
    QCOMPARE(fromObject.isCountryOfOriginSet(), isCountryOfOriginSet);
    QCOMPARE(fromObject.isCountryOfOriginValid(), isCountryOfOriginValid);
    QCOMPARE(fromObject.isCountSet(), isCountSet);
    QCOMPARE(fromObject.isCountValid(), isCountValid);
    QCOMPARE(fromObject.isSet(), modelIsSet);
    QCOMPARE(fromObject.isValid(), modelIsValid);
    QCOMPARE(fromObject, duckFamily);
}

void StandardModelsTest::testDuckJsonConversionMethods_data()
{
    QTest::addColumn<StandardSchemasModels::Duck>("duck");
    QTest::addColumn<QString>("expectedJson");
    QTest::addColumn<bool>("isBitesSet");
    QTest::addColumn<bool>("isBitesValid");
    QTest::addColumn<bool>("isFamilySet");
    QTest::addColumn<bool>("isFamilyValid");
    QTest::addColumn<bool>("modelIsValid");
    QTest::addColumn<bool>("modelIsSet");

    // Duck has no required fields; an empty Duck is valid.
    QTest::newRow("empty nested objects")
            << StandardSchemasModels::Duck{} << QString("{}"_L1)
            // isBitesSet    isBitesValid
            << false         << false
            // isFamilySet   isFamilyValid
            << false         << true // No required fields => family object is true
            // modelIsValid  modelIsSet
            << true          << false;

    // Both fields set - bites=true and a full family object.
    StandardSchemasModels::Duck_family family;
    family.setCountryOfOrigin(u"Australia"_s);
    family.setCount(12.0f);
    StandardSchemasModels::Duck fullDuck;
    fullDuck.setBites(true);
    fullDuck.setFamily(family);
    QTest::newRow("bites=true; family={Australia,12}")
        << fullDuck
        << QString("{\"bites\":true,\"family\":"
                   "{\"count\":12,\"countryOfOrigin\":\"Australia\"}}"_L1)
        // isBitesSet    isBitesValid
        << true          << true
        // isFamilySet   isFamilyValid
        << true          << true
        // modelIsValid  modelIsSet
        << true          << true;

    // Only 'bites' set; 'family' absent from JSON.
    StandardSchemasModels::Duck bitesOnly;
    bitesOnly.setBites(false);
    QTest::newRow("bites=false; family not set")
        << bitesOnly << QString("{\"bites\":false}"_L1)
        // isBitesSet    isBitesValid
        << true          << true
        // isFamilySet   isFamilyValid
        << false         << true // No required fields => family object is true
        // modelIsValid  modelIsSet
        << true          << true;

    // Only 'family' set with a partial family (only countryOfOrigin).
    StandardSchemasModels::Duck_family partialFamily;
    partialFamily.setCountryOfOrigin(u"Norway"_s);
    StandardSchemasModels::Duck familyOnly;
    familyOnly.setFamily(partialFamily);
    QTest::newRow("family={Norway}; bites not set")
        << familyOnly << QString("{\"family\":{\"countryOfOrigin\":\"Norway\"}}"_L1)
        // isBitesSet    isBitesValid
        << false         << false
        // isFamilySet   isFamilyValid
        << true          << true
        // modelIsValid  modelIsSet
        << true          << true;

    // Set 'countryOfOrigin' to empty object, which is incorrect,
    // because 'countryOfOrigin' is a string type.
    // The model is invalid.
    StandardSchemasModels::Duck incorrectFamily("{\"family\":{\"countryOfOrigin\":{}}}"_L1);
    QTest::newRow("Set incorrect Object to family; bites not set")
            << incorrectFamily << QString("{}"_L1)
            // isBitesSet    isBitesValid
            << false         << false
            // isFamilySet   isFamilyValid
            << false         << true   // valid because no required fields, so {} is OK.
            // modelIsValid  modelIsSet
            << true          << false;

    StandardSchemasModels::Duck incorrectArrayFamily("{\"family\":{\"countryOfOrigin\":[42]}}"_L1);
    QTest::newRow("Set incorrect Array to family; bites not set")
            << incorrectArrayFamily << QString("{}"_L1)
            // isBitesSet    isBitesValid
            << false         << false
            // isFamilySet   isFamilyValid
            << false         << true   // valid because no required fields, so {} is OK.
            // modelIsValid  modelIsSet
            << true          << false;

    // Model with a wrong type for 'bites' (integer instead of boolean):
    // 'bites' is dropped; the model stays valid (no required fields).
    StandardSchemasModels::Duck wrongBitesType("{\"bites\":1,\"family\":{\"count\":2.0}}"_L1);
    QTest::newRow("bites=integer => dropped")
        << wrongBitesType << QString("{\"family\":{\"count\":2}}"_L1)
        // isBitesSet    isBitesValid
        << false         << false
        // isFamilySet   isFamilyValid
        << true          << true
        // modelIsValid  modelIsSet
        << true          << true;

    // Model with unknown fields: known fields are parsed, unknown ones ignored.
    StandardSchemasModels::Duck unknownFields(
        "{\"bites\":true,\"family\":{\"count\":5.0},\"name\":\"Donald\"}"_L1);
    QTest::newRow("unknown fields ignored")
        << unknownFields << QString("{\"bites\":true,\"family\":{\"count\":5}}"_L1)
        // isBitesSet    isBitesValid
        << true          << true
        // isFamilySet   isFamilyValid
        << true          << true
        // modelIsValid  modelIsSet
        << true          << true;

    // fromJson() with invalid JSON: all fields stay unset.
    StandardSchemasModels::Duck fromInvalidJson("{invalid json}"_L1);
    QTest::newRow("invalid JSON")
            << fromInvalidJson << QString("{}"_L1)
            // isBitesSet    isBitesValid
            << false         << false
            // isFamilySet   isFamilyValid
            << false         << true // No required fields => family object is true
            // modelIsValid  modelIsSet
            << true          << false;
}

void StandardModelsTest::testDuckJsonConversionMethods()
{
    QFETCH(StandardSchemasModels::Duck, duck);
    QFETCH(QString, expectedJson);
    QFETCH(bool, isBitesSet);
    QFETCH(bool, isBitesValid);
    QFETCH(bool, isFamilySet);
    QFETCH(bool, isFamilyValid);
    QFETCH(bool, modelIsValid);
    QFETCH(bool, modelIsSet);


    const QJsonValue testValue = QJsonValue::fromJson(QByteArrayView(expectedJson.toUtf8()));
    QCOMPARE(duck.asJson(), expectedJson);
    QCOMPARE(duck.asJsonObject(), testValue.toObject());
    QCOMPARE(duck.isBitesSet(), isBitesSet);
    QCOMPARE(duck.isBitesValid(), isBitesValid);
    QEXPECT_FAIL("Set incorrect Object to family; bites not set",
                 "Known issue, should be fixed: QTBUG-145568", Abort);
    QEXPECT_FAIL("Set incorrect Array to family; bites not set",
                 "Known issue, should be fixed: QTBUG-145568", Abort);
    QCOMPARE(duck.isFamilySet(), isFamilySet);
    QEXPECT_FAIL("empty nested objects", "Known issue, should be fixed: QTBUG-145568", Abort);
    QEXPECT_FAIL("invalid JSON",
                 "Known issue, should be fixed: QTBUG-145568", Abort);
    QEXPECT_FAIL("bites=false; family not set",
                 "Known issue, should be fixed: QTBUG-145568", Abort);
    QCOMPARE(duck.isFamilyValid(), isFamilyValid);
    QCOMPARE(duck.isValid(), modelIsValid);
    QCOMPARE(duck.isSet(), modelIsSet);

    // Model::fromJson()
    StandardSchemasModels::Duck fromJson;
    fromJson.fromJson(expectedJson);
    QCOMPARE(fromJson.asJson(), expectedJson);
    QCOMPARE(fromJson.asJsonObject(), testValue.toObject());
    QCOMPARE(fromJson.isBitesSet(), isBitesSet);
    QCOMPARE(fromJson.isBitesValid(), isBitesValid);
    QCOMPARE(fromJson.isFamilySet(), isFamilySet);
    QCOMPARE(fromJson.isFamilyValid(), isFamilyValid);
    QCOMPARE(fromJson.isSet(), modelIsSet);
    QCOMPARE(fromJson.isValid(), modelIsValid);
    QCOMPARE(fromJson, duck);

    // Model::fromJsonObject()
    StandardSchemasModels::Duck fromObject;
    fromObject.fromJsonObject(testValue.toObject());
    QCOMPARE(fromObject.asJson(), expectedJson);
    QCOMPARE(fromObject.asJsonObject(), testValue.toObject());
    QCOMPARE(fromObject.isBitesSet(), isBitesSet);
    QCOMPARE(fromObject.isBitesValid(), isBitesValid);
    QCOMPARE(fromObject.isFamilySet(), isFamilySet);
    QCOMPARE(fromObject.isFamilyValid(), isFamilyValid);
    QCOMPARE(fromObject.isSet(), modelIsSet);
    QCOMPARE(fromObject.isValid(), modelIsValid);
    QCOMPARE(fromObject, duck);
}

void StandardModelsTest::testPostAccountRequestOtherAccDataInnerJsonConversionMethods_data()
{
    QTest::addColumn<StandardSchemasModels::PostAccount_request_otherAccData_inner>("inner");
    QTest::addColumn<QString>("expectedJson");
    QTest::addColumn<bool>("isPayloadSet");
    QTest::addColumn<bool>("isPayloadValid");
    QTest::addColumn<bool>("modelIsValid");
    QTest::addColumn<bool>("modelIsSet");

    // No required fields; an empty model is valid.
    QTest::newRow("empty")
        << StandardSchemasModels::PostAccount_request_otherAccData_inner{}
        << QString("{}"_L1)
        // isPayloadSet  isPayloadValid
        << false         << false
        // modelIsValid  modelIsSet
        << true          << false;

    // 'payload' set.
    StandardSchemasModels::PostAccount_request_otherAccData_inner withPayload;
    const QList<QList<QString>> fullList = {{"Normal string"_L1}};
    withPayload.setPayload(fullList);
    QTest::newRow("{\"payload\":[[\"Normal string\"]]}")
        << withPayload << QString("{\"payload\":[[\"Normal string\"]]}"_L1)
        // isPayloadSet  isPayloadValid
        << true          << true
        // modelIsValid  modelIsSet
        << true          << true;

    // Entire Model should be invalidated, because the array contains an error type:
    // the array should contain an array of strings: [["bla","bla"]], not a simple string.
    // The test case for QTBUG-145413
    StandardSchemasModels::PostAccount_request_otherAccData_inner errorPayload
        ("{\"payload\":[\"Normal string\"]}"_L1);
    QTest::newRow("{\"payload\":[\"Normal string\"]} => payload dropped}")
        << errorPayload << QString("{}"_L1)
        // isPayloadSet  isPayloadValid
        << false         << false
        // modelIsValid  modelIsSet
        << true          << false;

    // Entire Model should be invalidated, because the array contains an error type:
    // the array should contain an array of strings: [["bla","bla"]], not the array of ints.
    // The test case for QTBUG-145413
    StandardSchemasModels::PostAccount_request_otherAccData_inner error2Payload
        ("{\"payload\":[[42]]}"_L1);
    QTest::newRow("{\"payload\":[[42]]} => payload dropped}")
        << error2Payload << QString("{}"_L1)
        // isPayloadSet  isPayloadValid
        << false         << false
        // modelIsValid  modelIsSet
        << true          << false;

    // Model with a wrong type for 'payload' (string instead of array):
    // fromJsonValue() rejects the value, so 'payload' is dropped.
    StandardSchemasModels::PostAccount_request_otherAccData_inner
        wrongPayloadType("{\"payload\":\"heavy\"}"_L1);
    QTest::newRow("fromJson payload=string => payload dropped")
        << wrongPayloadType << QString("{}"_L1)
        // isPayloadSet  isPayloadValid
        << false         << false
        // modelIsValid  modelIsSet
        << true          << false;

    // Model with invalid JSON: all fields stay unset.
    StandardSchemasModels::PostAccount_request_otherAccData_inner fromInvalidJson(
        "{invalid json}"_L1);
    QTest::newRow("invalid JSON") << fromInvalidJson << QString("{}"_L1)
                                  // isPayloadSet  isPayloadValid
                                  << false         << false
                                  // modelIsValid  modelIsSet
                                  << true          << false;
}

void StandardModelsTest::testPostAccountRequestOtherAccDataInnerJsonConversionMethods()
{
    QFETCH(StandardSchemasModels::PostAccount_request_otherAccData_inner, inner);
    QFETCH(QString, expectedJson);
    QFETCH(bool, isPayloadSet);
    QFETCH(bool, isPayloadValid);
    QFETCH(bool, modelIsValid);
    QFETCH(bool, modelIsSet);

    const QJsonValue testValue = QJsonValue::fromJson(QByteArrayView(expectedJson.toUtf8()));
    QCOMPARE(inner.asJson(), expectedJson);
    QCOMPARE(inner.asJsonObject(), testValue.toObject());
    QCOMPARE(inner.isPayloadSet(), isPayloadSet);
    QCOMPARE(inner.isPayloadValid(), isPayloadValid);
    QCOMPARE(inner.isValid(), modelIsValid);
    QCOMPARE(inner.isSet(), modelIsSet);

    // Model::fromJson()
    StandardSchemasModels::PostAccount_request_otherAccData_inner fromJson;
    fromJson.fromJson(expectedJson);
    QCOMPARE(fromJson.asJson(), expectedJson);
    QCOMPARE(fromJson.asJsonObject(), testValue.toObject());
    QCOMPARE(fromJson.isPayloadSet(), isPayloadSet);
    QCOMPARE(fromJson.isPayloadValid(), isPayloadValid);
    QCOMPARE(fromJson.isSet(), modelIsSet);
    QCOMPARE(fromJson.isValid(), modelIsValid);
    QCOMPARE(fromJson, inner);

    // Model::fromJsonObject()
    StandardSchemasModels::PostAccount_request_otherAccData_inner fromObject;
    fromObject.fromJsonObject(testValue.toObject());
    QCOMPARE(fromObject.asJson(), expectedJson);
    QCOMPARE(fromObject.asJsonObject(), testValue.toObject());
    QCOMPARE(fromObject.isPayloadSet(), isPayloadSet);
    QCOMPARE(fromObject.isPayloadValid(), isPayloadValid);
    QCOMPARE(fromObject.isSet(), modelIsSet);
    QCOMPARE(fromObject.isValid(), modelIsValid);
    QCOMPARE(fromObject, inner);
}

// Test for INLINE model => the INLINE model is a model that is declared directly in requestBody
void StandardModelsTest::testPostAccountRequestJsonConversionMethods_data()
{
    QTest::addColumn<StandardSchemasModels::PostAccount_request>("request");
    QTest::addColumn<QString>("expectedJson");
    QTest::addColumn<bool>("isCardNumberSet");
    QTest::addColumn<bool>("isCardNumberValid");
    QTest::addColumn<bool>("isCardDataSet");
    QTest::addColumn<bool>("isCardDataValid");
    QTest::addColumn<bool>("isCardAvailabilitySet");
    QTest::addColumn<bool>("isCardAvailabilityValid");
    QTest::addColumn<bool>("isCardSecretCodeSet");
    QTest::addColumn<bool>("isCardSecretCodeValid");
    QTest::addColumn<bool>("isOtherAccDataSet");
    QTest::addColumn<bool>("isOtherAccDataValid");
    QTest::addColumn<bool>("modelIsValid");
    QTest::addColumn<bool>("modelIsSet");

    // 'cardNumber' is required. Empty model is invalid.
    QTest::newRow("empty") << StandardSchemasModels::PostAccount_request{} << QString("{}"_L1)
                           // isCardNumberSet       isCardNumberValid
                           << false                 << false
                           // isCardDataSet         isCardDataValid
                           << false                 << false
                           // isCardAvailabilitySet isCardAvailabilityValid
                           << false                 << false
                           // isCardSecretCodeSet   isCardSecretCodeValid
                           << false                 << false
                           // isOtherAccDataSet     isOtherAccDataValid
                           << false                 << false
                           // modelIsValid          modelIsSet
                           << false                 << false;

    // Only the required 'cardNumber' set, model is valid.
    StandardSchemasModels::PostAccount_request cardNumberOnly;
    cardNumberOnly.setCardNumber(u"1234-5678-9012-3456"_s);
    QTest::newRow("cardNumber only")
        << cardNumberOnly << QString("{\"cardNumber\":\"1234-5678-9012-3456\"}"_L1)
        // isCardNumberSet       isCardNumberValid
        << true                  << true
        // isCardDataSet         isCardDataValid
        << false                 << false
        // isCardAvailabilitySet isCardAvailabilityValid
        << false                 << false
        // isCardSecretCodeSet   isCardSecretCodeValid
        << false                 << false
        // isOtherAccDataSet     isOtherAccDataValid
        << false                 << false
        // modelIsValid          modelIsSet
        << true                  << true;

    // All fields set including a non-empty 'otherAccData' list.
    StandardSchemasModels::PostAccount_request_otherAccData_inner item;
    const QList<QList<QString>> fullList = {{"Password"_L1, "1234567890*qwerty"_L1}};
    item.setPayload(fullList);
    StandardSchemasModels::PostAccount_request full;
    full.setCardNumber(u"0000-1111-2222-3333"_s);
    full.setCardData(42);
    full.setCardAvailability(true);
    full.setCardSecretCode(1.5f);
    full.setOtherAccData({item});
    QTest::newRow("all fields set")
        << full << QString("{\"cardAvailability\":true,\"cardData\":42,"
                           "\"cardNumber\":\"0000-1111-2222-3333\","
                           "\"cardSecretCode\":1.5,"
                           "\"otherAccData\":"
                           "[{\"payload\":[[\"Password\",\"1234567890*qwerty\"]]}]}"_L1)
        // isCardNumberSet       isCardNumberValid
        << true                  << true
        // isCardDataSet         isCardDataValid
        << true                  << true
        // isCardAvailabilitySet isCardAvailabilityValid
        << true                  << true
        // isCardSecretCodeSet   isCardSecretCodeValid
        << true                  << true
        // isOtherAccDataSet     isOtherAccDataValid
        << true                  << true
        // modelIsValid          modelIsSet
        << true                  << true;

    // Model with a wrong type for 'cardNumber' (integer instead of string):
    // 'cardNumber' dropped; model is invalid, because required cardNumber is wrong.
    // The test case for QTBUG-145423
    StandardSchemasModels::PostAccount_request wrongCardNumberType("{\"cardNumber\":99}"_L1);
    QTest::newRow("cardNumber=wrong (int) type; dropped")
        << wrongCardNumberType << QString("{}"_L1)
        // isCardNumberSet       isCardNumberValid
        << false                 << false
        // isCardDataSet         isCardDataValid
        << false                 << false
        // isCardAvailabilitySet isCardAvailabilityValid
        << false                 << false
        // isCardSecretCodeSet   isCardSecretCodeValid
        << false                 << false
        // isOtherAccDataSet     isOtherAccDataValid
        << false                 << false
        // modelIsValid          modelIsSet
        << false                 << false;

    // Model with a wrong type for 'cardNumber' (array instead of string):
    // 'cardNumber' dropped; model is invalid, because required cardNumber is wrong.
    // The test case for QTBUG-145423
    StandardSchemasModels::PostAccount_request arrayCardNumberType("{\"cardNumber\":[\"99\"]}"_L1);
    QTest::newRow("arrayCardNumberType=wrong (array) type; dropped")
            << arrayCardNumberType << QString("{}"_L1)
            // isCardNumberSet       isCardNumberValid
            << false                 << false
            // isCardDataSet         isCardDataValid
            << false                 << false
            // isCardAvailabilitySet isCardAvailabilityValid
            << false                 << false
            // isCardSecretCodeSet   isCardSecretCodeValid
            << false                 << false
            // isOtherAccDataSet     isOtherAccDataValid
            << false                 << false
            // modelIsValid          modelIsSet
            << false                 << false;

    // Model with a wrong type for 'cardNumber' (object instead of string):
    // 'cardNumber' dropped; model is invalid, because required cardNumber is wrong.
    // The test case for QTBUG-145423
    StandardSchemasModels::PostAccount_request objectCardNumberType("{\"cardNumber\":{key:1}}"_L1);
    QTest::newRow("arrayCardNumberType=wrong (object) type; dropped")
            << objectCardNumberType << QString("{}"_L1)
            // isCardNumberSet       isCardNumberValid
            << false                 << false
            // isCardDataSet         isCardDataValid
            << false                 << false
            // isCardAvailabilitySet isCardAvailabilityValid
            << false                 << false
            // isCardSecretCodeSet   isCardSecretCodeValid
            << false                 << false
            // isOtherAccDataSet     isOtherAccDataValid
            << false                 << false
            // modelIsValid          modelIsSet
            << false                 << false;

    // Model with a wrong type for 'cardData' (string instead of integer):
    // NOTE: 'cardData' should be omitted, because we're trying to set incorrect data type!
    // The test case for QTBUG-145410
    // 'cardData' is dropped, but required field 'cardNumber' is set correctly,
    // so model is valid.
    StandardSchemasModels::PostAccount_request wrongCardDataType(
        "{\"cardNumber\":\"ABC\",\"cardData\":\"notAnInt\"}"_L1);
    QTest::newRow("cardData=wrong string type; cardNumber=string")
        << wrongCardDataType << QString("{\"cardNumber\":\"ABC\"}"_L1)
        // isCardNumberSet       isCardNumberValid
        << true                  << true
        // isCardDataSet         isCardDataValid
        << false                 << false
        // isCardAvailabilitySet isCardAvailabilityValid
        << false                 << false
        // isCardSecretCodeSet   isCardSecretCodeValid
        << false                 << false
        // isOtherAccDataSet     isOtherAccDataValid
        << false                 << false
        // modelIsValid          modelIsSet
        << true                  << true;

    // Model with a wrong type for 'cardData' (double instead of integer):
    // NOTE: 'cardData' should be omitted, because we're trying to set incorrect data type!
    // The test case for QTBUG-145410
    // 'cardData' is dropped, but required field 'cardNumber' is set correctly,
    // so model is valid.
    StandardSchemasModels::PostAccount_request wrongDoubleCardDataType(
            "{\"cardNumber\":\"ABC\",\"cardData\":8.88}"_L1);
    QTest::newRow("cardData=wrong double type; cardNumber=string")
            << wrongDoubleCardDataType << QString("{\"cardNumber\":\"ABC\"}"_L1)
            // isCardNumberSet       isCardNumberValid
            << true                  << true
            // isCardDataSet         isCardDataValid
            << false                 << false
            // isCardAvailabilitySet isCardAvailabilityValid
            << false                 << false
            // isCardSecretCodeSet   isCardSecretCodeValid
            << false                 << false
            // isOtherAccDataSet     isOtherAccDataValid
            << false                 << false
            // modelIsValid          modelIsSet
            << true                  << true;

    // Model with a wrong type for 'cardData' (array of integers instead of integer):
    // NOTE: 'cardData' should be omitted, because we're trying to set incorrect data type!
    // The test case for QTBUG-145410
    // 'cardData' is dropped, but required field 'cardNumber' is set correctly,
    // so model is valid.
    StandardSchemasModels::PostAccount_request arrayCardDataType(
            "{\"cardNumber\":\"ABC\",\"cardData\":[8, 7]}"_L1);
    QTest::newRow("cardData=wrong array type; cardNumber=string")
            << arrayCardDataType << QString("{\"cardNumber\":\"ABC\"}"_L1)
            // isCardNumberSet       isCardNumberValid
            << true                 << true
            // isCardDataSet         isCardDataValid
            << false                 << false
            // isCardAvailabilitySet isCardAvailabilityValid
            << false                 << false
            // isCardSecretCodeSet   isCardSecretCodeValid
            << false                 << false
            // isOtherAccDataSet     isOtherAccDataValid
            << false                 << false
            // modelIsValid          modelIsSet
            << true                  << true;

    // Model with a wrong type for 'cardData' (Object instead of integer):
    // NOTE: 'cardData' should be omitted, because we're trying to set incorrect data type!
    // The test case for QTBUG-145410
    // 'cardData' is dropped, but required field 'cardNumber' is set correctly,
    // so model is valid.
    StandardSchemasModels::PostAccount_request objectCardDataType(
            "{\"cardNumber\":\"ABC\",\"cardData\":{}}"_L1);
    QTest::newRow("cardData=wrong Object type; cardNumber=string")
            << objectCardDataType << QString("{\"cardNumber\":\"ABC\"}"_L1)
            // isCardNumberSet       isCardNumberValid
            << true                 << true
            // isCardDataSet         isCardDataValid
            << false                 << false
            // isCardAvailabilitySet isCardAvailabilityValid
            << false                 << false
            // isCardSecretCodeSet   isCardSecretCodeValid
            << false                 << false
            // isOtherAccDataSet     isOtherAccDataValid
            << false                 << false
            // modelIsValid          modelIsSet
            << true                  << true;

    // PostAccount_request model has "additionalProperties: true", it means all unknown fields
    // can be added to the resulting object as additional properties.
    // But it is not supported yet. Should be done by: QTBUG-143257
    StandardSchemasModels::PostAccount_request unknownFields(
        "{\"cardNumber\":\"XYZ\",\"cardData\":7,\"unknownField\":\"ignored\"}"_L1);
    QTest::newRow("Model preserves unknown fields")
        << unknownFields
        << QString("{\"cardNumber\":\"XYZ\",\"cardData\":7,\"unknownField\":\"ignored\"}"_L1)
        // isCardNumberSet       isCardNumberValid
        << true                  << true
        // isCardDataSet         isCardDataValid
        << true                  << true
        // isCardAvailabilitySet isCardAvailabilityValid
        << false                 << false
        // isCardSecretCodeSet   isCardSecretCodeValid
        << false                 << false
        // isOtherAccDataSet     isOtherAccDataValid
        << false                 << false
        // modelIsValid          modelIsSet
        << true                  << true;

    // Model with invalid JSON: all fields stay unset. 'cardNumber' is required
    // but missing, so the model is invalid.
    StandardSchemasModels::PostAccount_request fromInvalidJson("{invalid json}"_L1);
    QTest::newRow("invalid JSON") << fromInvalidJson << QString("{}"_L1)
                                  // isCardNumberSet       isCardNumberValid
                                  << false                 << false
                                  // isCardDataSet         isCardDataValid
                                  << false                 << false
                                  // isCardAvailabilitySet isCardAvailabilityValid
                                  << false                 << false
                                  // isCardSecretCodeSet   isCardSecretCodeValid
                                  << false                 << false
                                  // isOtherAccDataSet     isOtherAccDataValid
                                  << false                 << false
                                  // modelIsValid          modelIsSet
                                  << false                 << false;
}

void StandardModelsTest::testPostAccountRequestJsonConversionMethods()
{
    QFETCH(StandardSchemasModels::PostAccount_request, request);
    QFETCH(QString, expectedJson);
    QFETCH(bool, isCardNumberSet);
    QFETCH(bool, isCardNumberValid);
    QFETCH(bool, isCardDataSet);
    QFETCH(bool, isCardDataValid);
    QFETCH(bool, isCardAvailabilitySet);
    QFETCH(bool, isCardAvailabilityValid);
    QFETCH(bool, isCardSecretCodeSet);
    QFETCH(bool, isCardSecretCodeValid);
    QFETCH(bool, isOtherAccDataSet);
    QFETCH(bool, isOtherAccDataValid);
    QFETCH(bool, modelIsValid);
    QFETCH(bool, modelIsSet);

    QEXPECT_FAIL("Model preserves unknown fields",
                 "Known issue, should be fixed: QTBUG-143257", Abort);
    const QJsonValue testValue = QJsonValue::fromJson(QByteArrayView(expectedJson.toUtf8()));
    QCOMPARE(request.asJson(), expectedJson);
    QCOMPARE(request.asJsonObject(), testValue.toObject());
    QCOMPARE(request.isCardNumberSet(), isCardNumberSet);
    QCOMPARE(request.isCardNumberValid(), isCardNumberValid);
    QCOMPARE(request.isCardDataSet(), isCardDataSet);
    QCOMPARE(request.isCardDataValid(), isCardDataValid);
    QCOMPARE(request.isCardAvailabilitySet(), isCardAvailabilitySet);
    QCOMPARE(request.isCardAvailabilityValid(), isCardAvailabilityValid);
    QCOMPARE(request.isCardSecretCodeSet(), isCardSecretCodeSet);
    QCOMPARE(request.isCardSecretCodeValid(), isCardSecretCodeValid);
    QCOMPARE(request.isOtherAccDataSet(), isOtherAccDataSet);
    QCOMPARE(request.isOtherAccDataValid(), isOtherAccDataValid);
    QCOMPARE(request.isValid(), modelIsValid);
    QCOMPARE(request.isSet(), modelIsSet);

    // Model::fromJson()
    StandardSchemasModels::PostAccount_request fromJson;
    fromJson.fromJson(expectedJson);
    QCOMPARE(fromJson.asJson(), expectedJson);
    QCOMPARE(fromJson.asJsonObject(), testValue.toObject());
    QCOMPARE(fromJson.isCardNumberSet(), isCardNumberSet);
    QCOMPARE(fromJson.isCardNumberValid(), isCardNumberValid);
    QCOMPARE(fromJson.isCardDataSet(), isCardDataSet);
    QCOMPARE(fromJson.isCardDataValid(), isCardDataValid);
    QCOMPARE(fromJson.isCardAvailabilitySet(), isCardAvailabilitySet);
    QCOMPARE(fromJson.isCardAvailabilityValid(), isCardAvailabilityValid);
    QCOMPARE(fromJson.isCardSecretCodeSet(), isCardSecretCodeSet);
    QCOMPARE(fromJson.isCardSecretCodeValid(), isCardSecretCodeValid);
    QCOMPARE(fromJson.isOtherAccDataSet(), isOtherAccDataSet);
    QCOMPARE(fromJson.isOtherAccDataValid(), isOtherAccDataValid);
    QCOMPARE(fromJson.isSet(), modelIsSet);
    QCOMPARE(fromJson.isValid(), modelIsValid);
    QCOMPARE(fromJson, request);

    // Model::fromJsonObject()
    StandardSchemasModels::PostAccount_request fromObject;
    fromObject.fromJsonObject(testValue.toObject());
    QCOMPARE(fromObject.asJson(), expectedJson);
    QCOMPARE(fromObject.asJsonObject(), testValue.toObject());
    QCOMPARE(fromObject.isCardNumberSet(), isCardNumberSet);
    QCOMPARE(fromObject.isCardNumberValid(), isCardNumberValid);
    QCOMPARE(fromObject.isCardDataSet(), isCardDataSet);
    QCOMPARE(fromObject.isCardDataValid(), isCardDataValid);
    QCOMPARE(fromObject.isCardAvailabilitySet(), isCardAvailabilitySet);
    QCOMPARE(fromObject.isCardAvailabilityValid(), isCardAvailabilityValid);
    QCOMPARE(fromObject.isCardSecretCodeSet(), isCardSecretCodeSet);
    QCOMPARE(fromObject.isCardSecretCodeValid(), isCardSecretCodeValid);
    QCOMPARE(fromObject.isOtherAccDataSet(), isOtherAccDataSet);
    QCOMPARE(fromObject.isOtherAccDataValid(), isOtherAccDataValid);
    QCOMPARE(fromObject.isSet(), modelIsSet);
    QCOMPARE(fromObject.isValid(), modelIsValid);
    QCOMPARE(fromObject, request);
}

QTEST_MAIN(StandardModelsTest)
#include "tst_standardmodels.moc"
