// Copyright (C) 2018–2025 OpenAPI Generator contributors.
// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: Apache-2.0
// Qt-Security score:critical reason:data-parser

package org.qtproject.qt.codegen;

import lombok.Setter;
import org.openapitools.codegen.*;
import org.openapitools.codegen.model.*;
import org.openapitools.codegen.meta.features.DocumentationFeature;
import org.openapitools.codegen.meta.features.GlobalFeature;
import org.openapitools.codegen.meta.features.SecurityFeature;
import org.openapitools.codegen.templating.mustache.CamelCaseAndSanitizeLambda;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import static org.openapitools.codegen.utils.StringUtils.*;

import java.util.*;
import java.io.File;

public class CppQt6ClientGenerator extends CppQt6AbstractCodegen implements CodegenConfig {
    public static final String DEFAULT_PACKAGE_NAME = "Qt6OpenAPIClient";
    public static final String COMMON_LIB_NAME_OPTION = "commonLibraryName";
    public static final String DEFAULT_COMMON_LIB_NAME = "QtOpenAPICommon";
    public static final String COMMON_LIB_OPTION = "commonLibrary";
    public static final String MAKE_OPERATIONS_VIRTUAL_NAME = "makeOperationsVirtual";
    public static final String MAKE_OPERATIONS_VIRTUAL_DESC =
            "Make all operations methods virtual. " +
            "This makes it easy to mock the generated API class for testing purposes.";
    public static final String MAKE_QML_ENABLED = "enableQmlCode";
    public static final String MAKE_QML_ENABLED_DESC = "Enable registering C++ Types with the QML Type System";

    public enum GENERATION_TYPE {
        COMMON_LIB("Use-Common-Lib"),
        NO_FILES("Skip-Common-Files");

        public final String value;
        GENERATION_TYPE(String value) {
            this.value = value;
        }
    }
    protected String packageName = "";
    // source folder where to write the files
    protected String sourceFolder = "client";
    // source folder where to write the 'common' files
    protected String commonLibrarySourceFolder = "common";
    protected String apiVersion = "1.0.0";
    protected static final String USE_COMMON_LIBRARY = "enableCommonLibGeneration";
    private final Logger LOGGER = LoggerFactory.getLogger(CppQt6ClientGenerator.class);
    @Setter protected boolean addDownloadProgress = false;
    @Setter protected boolean makeOperationsVirtual = true;
    @Setter protected boolean enableQmlCode = false;
    @Setter protected String commonLibrary = GENERATION_TYPE.COMMON_LIB.value;
    @Setter protected String commonLibraryName = DEFAULT_PACKAGE_NAME;

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
        addSwitch("addDownloadProgress", "Add support for Qt download progress", this.addDownloadProgress);
        addSwitch(MAKE_OPERATIONS_VIRTUAL_NAME, MAKE_OPERATIONS_VIRTUAL_DESC, this.makeOperationsVirtual);
        addSwitch(MAKE_QML_ENABLED, MAKE_QML_ENABLED_DESC, this.enableQmlCode);
        // Common library name allows to choose a unique name for 'commonLibrary=COMMON_LIB' case.
        addOption(COMMON_LIB_NAME_OPTION, "Name of the common client library, if generated.",
                  DEFAULT_COMMON_LIB_NAME);
        // 'commonLibrary' option allows to choose the generation mode for common resources.
        // Possible generation modes:
        // 'COMMON_LIB' - generates common files as a separate library.
        // 'NO_FILES' - doesn't generate common files at all.
        CliOption commonLib = new CliOption(COMMON_LIB_OPTION,
                "Generate common library for the client or not.");
        Map<String, String> commonLibOptions = new HashMap<>();
        commonLibOptions.put(GENERATION_TYPE.COMMON_LIB.value,
                "The common resources will be generated as a Common library.");
        commonLibOptions.put(GENERATION_TYPE.NO_FILES.value,
                "The Client will be generated without common files at all.");
        commonLib.setEnum(commonLibOptions);
        commonLib.setDefault(this.commonLibrary);
        this.cliOptions.add(commonLib);

        /**
         * Template Location.  This is the location which templates will be read from.  The generator
         * will use the resource stream to attempt to read the templates.
         */
        templateDir = "cpp-qt6-client";
        typeMapping.put("AnyType", "QJsonValue");
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
        commonLibraryName = (String) additionalProperties.getOrDefault(COMMON_LIB_NAME_OPTION,
                                                                       DEFAULT_COMMON_LIB_NAME);

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
        if (additionalProperties.containsKey(COMMON_LIB_OPTION)
                && !additionalProperties.get(COMMON_LIB_OPTION).toString().isEmpty()) {
            setCommonLibrary(additionalProperties.get(COMMON_LIB_OPTION).toString());
        } else {
            additionalProperties.put(COMMON_LIB_OPTION, this.commonLibrary);
        }
        // If common library mode is ON, then make sense to handle the common library name.
        if (commonLibrary.equals(GENERATION_TYPE.COMMON_LIB.value)) {
            if (additionalProperties.containsKey(COMMON_LIB_NAME_OPTION)) {
                setCommonLibraryName(additionalProperties.get(COMMON_LIB_NAME_OPTION).toString());
            } else {
                additionalProperties.put(COMMON_LIB_NAME_OPTION, this.commonLibraryName);
            }
        }
        // The 'enableCommonLibGeneration' mustache-key required to enable/disable
        // common library generation.
        additionalProperties.put(USE_COMMON_LIBRARY,
                                 commonLibrary.equals(GENERATION_TYPE.COMMON_LIB.value));
        supportingFiles.clear();
        final String namePrefix = additionalProperties.containsKey("modelNamePrefix")
                ? modelNamePrefix : PREFIX;
        supportingFiles.add(new SupportingFile("README.mustache",
                sourceFolder, "README.md"));
        supportingFiles.add(new SupportingFile("CMakeConfig.mustache",
                sourceFolder, "Config.cmake.in"));
        supportingFiles.add(new SupportingFile("CMakeLists.txt.mustache",
                sourceFolder, "CMakeLists.txt"));
        supportingFiles.add(new SupportingFile("doc/Doxyfile.in.mustache",
                sourceFolder, "doc/Doxyfile.in"));
        typeMapping.put("file", namePrefix + "HttpFileElement");
        importMapping.put(namePrefix + "HttpFileElement", "#include \""
                          + namePrefix + "HttpFileElement.h\"");
        if (commonLibrary.equals(GENERATION_TYPE.NO_FILES.value)) {
            LOGGER.info("Skipping ./common/* templates generation. 'Skip-Common-Files' is ON.");
            return;
        }
        supportingFiles.add(new SupportingFile("common/api-base-header.mustache",
                commonLibrarySourceFolder, namePrefix + "BaseApi.h"));
        supportingFiles.add(new SupportingFile("common/api-base-body.mustache",
                commonLibrarySourceFolder, namePrefix + "BaseApi.cpp"));
        supportingFiles.add(new SupportingFile("common/helpers-header.mustache",
                commonLibrarySourceFolder, namePrefix + "Helpers.h"));
        supportingFiles.add(new SupportingFile("common/helpers-body.mustache",
                commonLibrarySourceFolder, namePrefix + "Helpers.cpp"));
        supportingFiles.add(new SupportingFile("common/HttpRequest.h.mustache",
                commonLibrarySourceFolder, namePrefix + "HttpRequest.h"));
        supportingFiles.add(new SupportingFile("common/HttpRequest.cpp.mustache",
                commonLibrarySourceFolder, namePrefix + "HttpRequest.cpp"));
        supportingFiles.add(new SupportingFile("common/HttpFileElement.h.mustache",
                commonLibrarySourceFolder, namePrefix + "HttpFileElement.h"));
        supportingFiles.add(new SupportingFile("common/HttpFileElement.cpp.mustache",
                commonLibrarySourceFolder, namePrefix + "HttpFileElement.cpp"));
        supportingFiles.add(new SupportingFile("common/object.mustache",
                commonLibrarySourceFolder, namePrefix + "Object.h"));
        supportingFiles.add(new SupportingFile("common/enum.mustache",
                commonLibrarySourceFolder, namePrefix + "Enum.h"));
        supportingFiles.add(new SupportingFile("common/ServerConfiguration.mustache",
                commonLibrarySourceFolder, namePrefix + "ServerConfiguration.h"));
        supportingFiles.add(new SupportingFile("common/ServerVariable.mustache",
                commonLibrarySourceFolder, namePrefix + "ServerVariable.h"));
        supportingFiles.add(new SupportingFile("common/CMakeConfig.mustache",
                commonLibrarySourceFolder, "Config.cmake.in"));
        supportingFiles.add(new SupportingFile("common/CMakeLists.txt.mustache",
                commonLibrarySourceFolder, "CMakeLists.txt"));
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
