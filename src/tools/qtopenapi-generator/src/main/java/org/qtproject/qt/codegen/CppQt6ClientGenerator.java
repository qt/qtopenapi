// Copyright (C) 2018–2025 OpenAPI Generator contributors.
// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: Apache-2.0
// Qt-Security score:critical reason:data-parser

package org.qtproject.qt.codegen;

import lombok.Setter;
import org.openapitools.codegen.model.*;
import org.openapitools.codegen.CodegenConfig;
import org.openapitools.codegen.CodegenConstants;
import org.openapitools.codegen.CodegenType;
import org.openapitools.codegen.SupportingFile;
import org.openapitools.codegen.meta.features.DocumentationFeature;
import org.openapitools.codegen.meta.features.GlobalFeature;
import org.openapitools.codegen.meta.features.SecurityFeature;
import org.openapitools.codegen.templating.mustache.CamelCaseAndSanitizeLambda;
import static org.openapitools.codegen.utils.StringUtils.*;

import java.util.*;
import java.io.File;

public class CppQt6ClientGenerator extends CppQt6AbstractCodegen implements CodegenConfig {
    public static final String OPTIONAL_PROJECT_FILE_DESC = "Generate client.pri.";
    public static final String DEFAULT_PACKAGE_NAME = "Qt6OpenAPIClient";
    public static final String MAKE_OPERATIONS_VIRTUAL_NAME = "makeOperationsVirtual";
    public static final String MAKE_OPERATIONS_VIRTUAL_DESC =
            "Make all operations methods virtual. " +
            "This makes it easy to mock the generated API class for testing purposes.";
    public static final String MAKE_QML_ENABLED = "enableQmlCode";
    public static final String MAKE_QML_ENABLED_DESC = "Enable registering C++ Types with the QML Type System";
    protected String packageName = "";
    // source folder where to write the files
    protected String sourceFolder = "client";
    protected String apiVersion = "1.0.0";
    @Setter protected boolean optionalProjectFileFlag = true;
    @Setter protected boolean addDownloadProgress = false;
    @Setter protected boolean makeOperationsVirtual = true;
    @Setter protected boolean enableQmlCode = false;

    /**
     * Configures the type of generator.
     *
     * @return  the CodegenType for this generator
     * @see     org.openapitools.codegen.CodegenType
     */
    public CodegenType getTag() {
        return CodegenType.CLIENT;
    }

    /**
     * Configures a friendly name for the generator.  This will be used by the generator
     * to select the library with the -g flag.
     *
     * @return the friendly name for the generator
     */
    public String getName() {
        return "cpp-qt6-client";
    }

    /**
     * Provides an opportunity to inspect and modify operation data before the code is generated.
     */
    @Override
    public OperationsMap postProcessOperationsWithModels(OperationsMap objs, List<ModelMap> allModels) {
        objs = super.postProcessOperationsWithModels(objs, allModels);
        removeImport(objs, "#include <QList>");
        return objs;
    }

    /**
     * Returns human-friendly help for the generator.  Provide the consumer with help
     * tips, parameters here
     *
     * @return A string value for the help message
     */
    public String getHelp() {
        return "Generates a cpp-qt6-client client library.";
    }

    public CppQt6ClientGenerator() {
        super();
        modifyFeatureSet(features -> features
                .includeDocumentationFeatures(DocumentationFeature.Readme)
                .includeGlobalFeatures(GlobalFeature.ParameterizedServer)
                .includeGlobalFeatures(GlobalFeature.MultiServer)
                .includeSecurityFeatures(SecurityFeature.BasicAuth)
                .includeSecurityFeatures(SecurityFeature.ApiKey)
                .includeSecurityFeatures(SecurityFeature.BearerToken)
                .includeGlobalFeatures(GlobalFeature.ParameterStyling)
        );

        // set the output folder here
        outputFolder = "generated-code/cpp-qt6-client";

        /**
         * Models.  You can write model files using the modelTemplateFiles map.
         * if you want to create one template for file, you can do so here.
         * for multiple files for model, just put another entry in the `modelTemplateFiles` with
         * a different extension
         */
        modelTemplateFiles.put(
                    "model-header.mustache",
                    ".h");

        modelTemplateFiles.put(
                    "model-body.mustache",
                    ".cpp");

        /**
         * Api classes.  You can write classes for each Api file with the apiTemplateFiles map.
         * as with models, add multiple entries with different extensions for multiple files per
         * class
         */
        apiTemplateFiles.put(
                    "api-header.mustache",   // the template to use
                    ".h");       // the extension for each file to write

        apiTemplateFiles.put(
                   "api-body.mustache",   // the template to use
                    ".cpp");       // the extension for each file to write

        // CLI options
        addOption(CodegenConstants.PACKAGE_NAME, "C++ package (library) name.", DEFAULT_PACKAGE_NAME);
        addSwitch(CodegenConstants.OPTIONAL_PROJECT_FILE, OPTIONAL_PROJECT_FILE_DESC, this.optionalProjectFileFlag);
        addSwitch("addDownloadProgress", "Add support for Qt download progress", this.addDownloadProgress);
        addSwitch(MAKE_OPERATIONS_VIRTUAL_NAME, MAKE_OPERATIONS_VIRTUAL_DESC, this.makeOperationsVirtual);
        addSwitch(MAKE_QML_ENABLED, MAKE_QML_ENABLED_DESC, this.enableQmlCode);

        /**
         * Template Location.  This is the location which templates will be read from.  The generator
         * will use the resource stream to attempt to read the templates.
         */
        templateDir = "cpp-qt6-client";

        /**
         * Supporting Files.  You can write single files for the generator with the
         * entire object tree available.  If the input file has a suffix of `.mustache
         * it will be processed by the template engine.  Otherwise, it will be copied
         */
        supportingFiles.add(new SupportingFile("common/api-base-header.mustache", sourceFolder, PREFIX + "BaseApi.h"));
        supportingFiles.add(new SupportingFile("common/api-base-body.mustache", sourceFolder, PREFIX + "BaseApi.cpp"));
        supportingFiles.add(new SupportingFile("common/helpers-header.mustache", sourceFolder, PREFIX + "Helpers.h"));
        supportingFiles.add(new SupportingFile("common/helpers-body.mustache", sourceFolder, PREFIX + "Helpers.cpp"));
        supportingFiles.add(new SupportingFile("common/HttpRequest.h.mustache", sourceFolder, PREFIX + "HttpRequest.h"));
        supportingFiles.add(new SupportingFile("common/HttpRequest.cpp.mustache", sourceFolder, PREFIX + "HttpRequest.cpp"));
        supportingFiles.add(new SupportingFile("common/HttpFileElement.h.mustache", sourceFolder, PREFIX + "HttpFileElement.h"));
        supportingFiles.add(new SupportingFile("common/HttpFileElement.cpp.mustache", sourceFolder, PREFIX + "HttpFileElement.cpp"));
        supportingFiles.add(new SupportingFile("common/object.mustache", sourceFolder, PREFIX + "Object.h"));
        supportingFiles.add(new SupportingFile("common/enum.mustache", sourceFolder, PREFIX + "Enum.h"));
        supportingFiles.add(new SupportingFile("common/ServerConfiguration.mustache", sourceFolder, PREFIX + "ServerConfiguration.h"));
        supportingFiles.add(new SupportingFile("common/ServerVariable.mustache", sourceFolder, PREFIX + "ServerVariable.h"));
        supportingFiles.add(new SupportingFile("README.mustache", sourceFolder, "README.md"));
        supportingFiles.add(new SupportingFile("CMakeConfig.mustache", sourceFolder, "Config.cmake.in"));
        supportingFiles.add(new SupportingFile("CMakeLists.txt.mustache", sourceFolder, "CMakeLists.txt"));
        supportingFiles.add(new SupportingFile("doc/Doxyfile.in.mustache", sourceFolder, "doc/Doxyfile.in"));
        if (optionalProjectFileFlag) {
            supportingFiles.add(new SupportingFile("Project.mustache", sourceFolder, "client.pri"));
        }
        typeMapping.put("file", PREFIX + "HttpFileElement");
        typeMapping.put("AnyType", "QJsonValue");
        importMapping.put(PREFIX + "HttpFileElement", "#include \"" + PREFIX + "HttpFileElement.h\"");
        importMapping.put("QJsonValue", "#include <QtCore/qjsonvalue.h>");

        reservedWords.add("valid");
        reservedWords.add("set");

        /**
         * Additional Properties.  These values can be passed to the templates and
         * are available in models, apis, and supporting files
         */
        additionalProperties.put("apiVersion", apiVersion);
        additionalProperties.put("camelcase", new CamelCaseAndSanitizeLambda(false).generator(this));
    }

    @Override
    public void processOpts() {
        super.processOpts();

        packageName = (String) additionalProperties.getOrDefault(CodegenConstants.PACKAGE_NAME, DEFAULT_PACKAGE_NAME);

        if (additionalProperties.containsKey(CodegenConstants.OPTIONAL_PROJECT_FILE)) {
            setOptionalProjectFileFlag(convertPropertyToBooleanAndWriteBack(CodegenConstants.OPTIONAL_PROJECT_FILE));
        } else {
            additionalProperties.put(CodegenConstants.OPTIONAL_PROJECT_FILE, optionalProjectFileFlag);
        }

        if (additionalProperties.containsKey(MAKE_OPERATIONS_VIRTUAL_NAME)) {
            setMakeOperationsVirtual(convertPropertyToBooleanAndWriteBack(MAKE_OPERATIONS_VIRTUAL_NAME));
        } else {
            additionalProperties.put(MAKE_OPERATIONS_VIRTUAL_NAME, makeOperationsVirtual);
        }

        if (additionalProperties.containsKey(MAKE_QML_ENABLED)) {
            setEnableQmlCode(convertPropertyToBooleanAndWriteBack(MAKE_QML_ENABLED));
        } else {
            additionalProperties.put(MAKE_QML_ENABLED, enableQmlCode);
        }

        additionalProperties.put(CodegenConstants.PACKAGE_NAME, packageName);

        if (additionalProperties.containsKey("modelNamePrefix")) {
            supportingFiles.clear();
            supportingFiles.add(new SupportingFile("common/api-base-header.mustache", sourceFolder, modelNamePrefix + "BaseApi.h"));
            supportingFiles.add(new SupportingFile("common/api-base-body.mustache", sourceFolder, modelNamePrefix + "BaseApi.cpp"));
            supportingFiles.add(new SupportingFile("common/helpers-header.mustache", sourceFolder, modelNamePrefix + "Helpers.h"));
            supportingFiles.add(new SupportingFile("common/helpers-body.mustache", sourceFolder, modelNamePrefix + "Helpers.cpp"));
            supportingFiles.add(new SupportingFile("common/HttpRequest.h.mustache", sourceFolder, modelNamePrefix + "HttpRequest.h"));
            supportingFiles.add(new SupportingFile("common/HttpRequest.cpp.mustache", sourceFolder, modelNamePrefix + "HttpRequest.cpp"));
            supportingFiles.add(new SupportingFile("common/HttpFileElement.h.mustache", sourceFolder, modelNamePrefix + "HttpFileElement.h"));
            supportingFiles.add(new SupportingFile("common/HttpFileElement.cpp.mustache", sourceFolder, modelNamePrefix + "HttpFileElement.cpp"));
            supportingFiles.add(new SupportingFile("common/object.mustache", sourceFolder, modelNamePrefix + "Object.h"));
            supportingFiles.add(new SupportingFile("common/enum.mustache", sourceFolder, modelNamePrefix + "Enum.h"));
            supportingFiles.add(new SupportingFile("common/ServerConfiguration.mustache", sourceFolder, modelNamePrefix + "ServerConfiguration.h"));
            supportingFiles.add(new SupportingFile("common/ServerVariable.mustache", sourceFolder, modelNamePrefix + "ServerVariable.h"));
            supportingFiles.add(new SupportingFile("README.mustache", sourceFolder, "README.md"));
            supportingFiles.add(new SupportingFile("CMakeConfig.mustache", sourceFolder, "Config.cmake.in"));
            supportingFiles.add(new SupportingFile("CMakeLists.txt.mustache", sourceFolder, "CMakeLists.txt"));
            supportingFiles.add(new SupportingFile("doc/Doxyfile.in.mustache", sourceFolder, "doc/Doxyfile.in"));


            typeMapping.put("file", modelNamePrefix + "HttpFileElement");
            typeMapping.put("AnyType", "QJsonValue");
            importMapping.put(modelNamePrefix + "HttpFileElement", "#include \"" + modelNamePrefix + "HttpFileElement.h\"");
            importMapping.put("QJsonValue", "#include <QtCore/qjsonvalue.h>");
            if (optionalProjectFileFlag) {
                supportingFiles.add(new SupportingFile("Project.mustache", sourceFolder, "client.pri"));
            }
        }
    }

    /**
     * Escapes a reserved word as defined in the `reservedWords` array. Handle escaping
     * those terms here.  This logic is only called if a variable matches the reserved words
     *
     * @return the escaped term
     */
    @Override
    public String escapeReservedWord(String name) {
        return "_" + name;  // add an underscore to the name
    }

    /**
     * Location to write model files.  You can use the modelPackage() as defined when the class is
     * instantiated
     */
    public String modelFileFolder() {
        return outputFolder + "/" + sourceFolder + "/" + modelPackage().replace("::", File.separator);
    }

    /**
     * Location to write api files.  You can use the apiPackage() as defined when the class is
     * instantiated
     */
    @Override
    public String apiFileFolder() {
        return outputFolder + "/" + sourceFolder + "/" + apiPackage().replace("::", File.separator);
    }

    /**
     * override with any special text escaping logic to handle unsafe
     * characters so as to avoid code injection
     *
     * @param input String to be cleaned up
     * @return string with unsafe characters removed or escaped
     */
    @Override
    public String escapeUnsafeCharacters(String input) {
        //TODO: check that this logic is safe to escape unsafe characters to avoid code injection
        return input;
    }

    /**
     * Escape single and/or double quote to avoid code injection
     *
     * @param input String to be cleaned up
     * @return string with quotation mark removed or escaped
     */
    public String escapeQuotationMark(String input) {
        //TODO: check that this logic is safe to escape quotation mark to avoid code injection
        return input.replace("\"", "\\\"");
    }

    @Override
    public String toApiFilename(String name) {
        return modelNamePrefix + sanitizeName(camelize(name)) + "Api";
    }
}
