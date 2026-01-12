// Copyright (C) 2018–2025 OpenAPI Generator contributors.
// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: Apache-2.0
// Qt-Security score:critical reason:data-parser

package org.qtproject.qt.codegen;

import io.swagger.v3.oas.models.OpenAPI;
import io.swagger.v3.oas.models.info.Info;
import lombok.Setter;
import org.openapitools.codegen.*;
import org.openapitools.codegen.model.*;
import org.openapitools.codegen.meta.features.DocumentationFeature;
import org.openapitools.codegen.meta.features.GlobalFeature;
import org.openapitools.codegen.meta.features.SecurityFeature;
import org.openapitools.codegen.templating.mustache.CamelCaseAndSanitizeLambda;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import io.swagger.v3.oas.models.info.License;

import static org.openapitools.codegen.utils.StringUtils.*;

import java.io.BufferedWriter;
import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.*;
import java.util.stream.Collectors;
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
    protected static final String CPP_COMMON_NAMESPACE = "cppCommonNamespace";
    protected static final String CPP_COMMON_NAMESPACE_DESC
            = "C++ namespace (convention: name::space::for::api) for the common library.";
    protected String cppCommonNamespace = "QtCommonOpenAPI";

    public enum GENERATION_TYPE {
        COMMON_LIB("Gen-Common-Lib"),
        CLIENT_LIB("Gen-Client-Lib");

        public final String value;
        GENERATION_TYPE(String value) {
            this.value = value;
        }
    }
    protected String namePrefix = "";
    public static final String USE_CMAKE_FUNCTION = "useCmakeMacro";
    public static final String USE_CMAKE_FUNCTION_DESC
            = "The 'qt6_add_openapi_client' function uses the option for CombinedModelsAndAPIs.cpp file generation";
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
    @Setter protected String commonLibrary = GENERATION_TYPE.CLIENT_LIB.value;
    @Setter protected String commonLibraryName = DEFAULT_PACKAGE_NAME;
    @Setter protected boolean useCmakeMacro = false;
    @Setter protected String licenseName = "";

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
        addOption(CPP_COMMON_NAMESPACE, CPP_COMMON_NAMESPACE_DESC, this.cppCommonNamespace);
        addOption(CodegenConstants.PACKAGE_NAME, "C++ package (library) name.", DEFAULT_PACKAGE_NAME);
        addSwitch("addDownloadProgress", "Add support for Qt download progress", this.addDownloadProgress);
        addSwitch(MAKE_OPERATIONS_VIRTUAL_NAME, MAKE_OPERATIONS_VIRTUAL_DESC, this.makeOperationsVirtual);
        addSwitch(MAKE_QML_ENABLED, MAKE_QML_ENABLED_DESC, this.enableQmlCode);
        addSwitch(USE_CMAKE_FUNCTION, USE_CMAKE_FUNCTION_DESC, this.useCmakeMacro);
        // Common library name allows to choose a unique name for 'commonLibrary=COMMON_LIB' case.
        addOption(COMMON_LIB_NAME_OPTION, "Name of the common client library, if generated.",
                  DEFAULT_COMMON_LIB_NAME);
        // 'commonLibrary' option allows to choose the generation mode for common resources.
        // Possible generation modes:
        // 'COMMON_LIB' - generates common files as a separate library.
        // 'CLIENT_LIB' - generate client lib as a separate library.
        CliOption commonLib = new CliOption(COMMON_LIB_OPTION,
                "Generate common library for the client or not.");
        Map<String, String> commonLibOptions = new HashMap<>();
        commonLibOptions.put(GENERATION_TYPE.COMMON_LIB.value,
                "The common resources will be generated as a Common library.");
        commonLibOptions.put(GENERATION_TYPE.CLIENT_LIB.value,
                "The Client will be generated without common files at all.");
        commonLib.setEnum(commonLibOptions);
        commonLib.setDefault(this.commonLibrary);
        this.cliOptions.add(commonLib);
        this.cliOptions.add(new CliOption(CodegenConstants.LICENSE_NAME,
            CodegenConstants.LICENSE_NAME_DESC).defaultValue(this.licenseName));

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
        additionalProperties.put("prefix", namePrefix);
        additionalProperties.put("camelcase", new CamelCaseAndSanitizeLambda(false).generator(this));
        additionalProperties.put("cppCommonNamespace", cppCommonNamespace);
    }

    @Override
    public void preprocessOpenAPI(OpenAPI openAPI) {
        super.preprocessOpenAPI(openAPI);
        if (openAPI.getInfo() != null) {
            Info info = openAPI.getInfo();
            // when licenceName is not specified, use info.license
            if (additionalProperties.get(CodegenConstants.LICENSE_NAME) == null && info.getLicense() != null) {
                License license = info.getLicense();
                licenseName = license.getName();
            }
        }

        additionalProperties.put(CodegenConstants.LICENSE_NAME, licenseName);
    }

    @Override
    public void processOpts() {
        super.processOpts();

        String genVersion = (String) additionalProperties.get("generatorVersion");
        // Currently the generatorVersion property is populated *after* processOpts()
        // is called, so we need to take it into account, and extract the
        // version information on our own. However, that might change in future, and
        // that's why we still read the property above.
        if (genVersion == null)
            genVersion = DefaultGenerator.class.getPackage().getImplementationVersion();

        final String[] versionParts = genVersion.split("\\.");
        additionalProperties.put("generatorVersionMajor",
                                 versionParts.length > 0 ? versionParts[0] : "0");
        additionalProperties.put("generatorVersionMinor",
                                 versionParts.length > 1 ? versionParts[1] : "0");
        additionalProperties.put("generatorVersionPatch",
                                 versionParts.length > 2 ? versionParts[2] : "0");

        if (additionalProperties.containsKey("cppCommonNamespace")) {
            cppCommonNamespace = (String) additionalProperties.get("cppCommonNamespace");
        }
        additionalProperties.put("cppCommonNamespaceDeclarations", cppCommonNamespace.split("\\::"));

        packageName = (String) additionalProperties.getOrDefault(CodegenConstants.PACKAGE_NAME, DEFAULT_PACKAGE_NAME);
        commonLibraryName = (String) additionalProperties.getOrDefault(COMMON_LIB_NAME_OPTION,
                                                                       DEFAULT_COMMON_LIB_NAME);

        if (additionalProperties.containsKey(CodegenConstants.LICENSE_NAME)) {
            setLicenseName(((String) additionalProperties.get(CodegenConstants.LICENSE_NAME)));
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

        if (additionalProperties.containsKey(USE_CMAKE_FUNCTION)) {
            setUseCmakeMacro(convertPropertyToBooleanAndWriteBack(USE_CMAKE_FUNCTION));
        } else {
            additionalProperties.put(USE_CMAKE_FUNCTION, useCmakeMacro);
        }

        additionalProperties.put(CodegenConstants.PACKAGE_NAME, packageName);
        final String lowerPackageName = packageName.toLowerCase();
        additionalProperties.put("packageNameLowerCase", lowerPackageName);
        additionalProperties.put("packageNameUpperCase", packageName.toUpperCase());
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

        String lowerCasePrefix = "";
        if (additionalProperties.containsKey("prefix")) {
            namePrefix = additionalProperties.get("prefix").toString();
            lowerCasePrefix = namePrefix.toLowerCase();
            additionalProperties.put("prefixLowerCase", lowerCasePrefix);
        }
        supportingFiles.clear();
        supportingFiles.add(new SupportingFile("README.mustache",
                sourceFolder, "README.md"));
        supportingFiles.add(new SupportingFile("CMakeConfig.mustache",
                sourceFolder, "config.cmake.in"));
        if (!this.useCmakeMacro) {
            supportingFiles.add(new SupportingFile("CMakeLists.txt.mustache",
                    sourceFolder, "CMakeLists.txt"));
        }
        supportingFiles.add(new SupportingFile("exports.mustache",
                sourceFolder, lowerPackageName + "exports.h"));
        supportingFiles.add(new SupportingFile("doc/Doxyfile.in.mustache",
                sourceFolder, "doc/Doxyfile.in"));
        typeMapping.put("object", namePrefix + "Object");
        typeMapping.put("file", namePrefix + "HttpFileElement");
        importMapping.put(namePrefix + "HttpFileElement", "#include \""
                + lowerCasePrefix + "httpfileelement.h\"");

        if (commonLibrary.equals(GENERATION_TYPE.CLIENT_LIB.value)) {
            LOGGER.info("Skipping ./common/* templates generation. 'Skip-Common-Files' is ON.");
            return;
        } else {
            if (this.useCmakeMacro) {
                modelTemplateFiles.clear();
                apiTemplateFiles.clear();
            }
            supportingFiles.add(new SupportingFile("common/api-base-header.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "baseapi.h"));
            supportingFiles.add(new SupportingFile("common/api-base-body.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "baseapi.cpp"));
            supportingFiles.add(new SupportingFile("common/helpers-header.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "helpers.h"));
            supportingFiles.add(new SupportingFile("common/helpers-body.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "helpers.cpp"));
            supportingFiles.add(new SupportingFile("common/httprequest.h.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "httprequest.h"));
            supportingFiles.add(new SupportingFile("common/httprequest.cpp.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "httprequest.cpp"));
            supportingFiles.add(new SupportingFile("common/httpfileelement.h.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "httpfileelement.h"));
            supportingFiles.add(new SupportingFile("common/httpfileelement.cpp.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "httpfileelement.cpp"));
            supportingFiles.add(new SupportingFile("common/object-header.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "object.h"));
                    supportingFiles.add(new SupportingFile("common/object-body.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "object.cpp"));
            supportingFiles.add(new SupportingFile("common/enum-header.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "enum.h"));
                    supportingFiles.add(new SupportingFile("common/enum-body.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "enum.cpp"));
            supportingFiles.add(new SupportingFile("common/common-exports.mustache",
                    commonLibrarySourceFolder, lowerPackageName + "commonexports.h"));
            supportingFiles.add(new SupportingFile("common/common-global.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "commonglobal.h"));
            supportingFiles.add(new SupportingFile("common/serverconfiguration.h.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "serverconfiguration.h"));
            supportingFiles.add(new SupportingFile("common/serverconfiguration.cpp.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "serverconfiguration.cpp"));
            supportingFiles.add(new SupportingFile("common/servervariable.h.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "servervariable.h"));
            supportingFiles.add(new SupportingFile("common/servervariable.cpp.mustache",
                    commonLibrarySourceFolder, lowerCasePrefix + "servervariable.cpp"));
            supportingFiles.add(new SupportingFile("common/CMakeConfig.mustache",
                    commonLibrarySourceFolder, "config.cmake.in"));
            if (!this.useCmakeMacro) {
                supportingFiles.add(new SupportingFile("common/CMakeLists.txt.mustache",
                        commonLibrarySourceFolder, "CMakeLists.txt"));
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
        final String apiFile = modelNamePrefix + sanitizeName(name) + "Api";
        return apiFile.toLowerCase();
    }

    public Map<String, Object> postProcessSupportingFileData(Map<String, Object> objs) {
        objs = super.postProcessSupportingFileData(objs);
        // `qt6_add_openapi_client` macro needs to know all file names on configuration step.
        // The problem is Models and APIs file names are not known before running the generator.
        // We tried to create a single file with a static name, that includes all Models
        // and APIs cpp files inside. It doesn't help, because of moc files absence.
        // The MOC doesn't generate moc files for cpp files included into the resulting cpp file
        // (there is no such feature).
        // So, the current solution is to have a single file with a static name which duplicates
        // the content of Models and APIs files.
        // The file is re-generated fully each time when generation is called.
        // The file is being generated only if `useCmakeMacro` option is ON.
        // The option description warns users that the option is required only for
        // `qt6_add_openapi_client` macro.
        // The macro always enables the option. Users don't need to do it manually.
        // By default, the option is OFF.
        if (this.useCmakeMacro && commonLibrary.equals(GENERATION_TYPE.CLIENT_LIB.value)) {
            final String lowerPackageName = packageName.toLowerCase();
            List<Path> apiClassFiles = new ArrayList<>();
            String apiDir = apiPackage.replace('.', File.separatorChar);
            ApiInfoMap apiInfo = (ApiInfoMap) objs.get("apiInfo");
            for (OperationsMap api : apiInfo.getApis()) {
                OperationMap opsApi = api.getOperations();
                final String apiFileName = opsApi.getClassname().toLowerCase();
                Path headerPath = Paths.get(outputFolder, apiDir,
                        sourceFolder
                                + File.separator + apiFileName + ".h");
                Path cppPath = Paths.get(outputFolder,
                        apiDir,
                        sourceFolder + File.separator
                                + apiFileName + ".cpp");
                apiClassFiles.add(headerPath);
                apiClassFiles.add(cppPath);
            }

            List<ModelMap> models = (List<ModelMap>) objs.get("models");
            List<CodegenModel> codegenModelList = models.stream()
                    .map(ModelMap::getModel)
                    .collect(Collectors.toList());
            List<Path> modelCppFilePaths = new ArrayList<>();
            List<Path> modelHeaderFilePaths = new ArrayList<>();
            Path combinedFile = Paths.get(outputFolder,
                    sourceFolder
                            + File.separator + lowerPackageName + "combinedmodelsandapis.cpp");
            String modelDir = modelPackage.replace('.', File.separatorChar);

            for (CodegenModel codeMod : codegenModelList) {
                final String modelFileName = codeMod.getClassFilename().toLowerCase();
                Path headerPath = Paths.get(outputFolder,
                        modelDir, sourceFolder + File.separator
                                + modelFileName + ".h");
                Path cppPath = Paths.get(outputFolder, modelDir, sourceFolder
                        + File.separator + modelFileName + ".cpp");
                modelHeaderFilePaths.add(headerPath);
                modelCppFilePaths.add(cppPath);
            }

            try {
                BufferedWriter writer = Files.newBufferedWriter(combinedFile, StandardCharsets.UTF_8);
                List<String> filteredOrdering = new ArrayList<>();
                Map<String, String> contentMap = new HashMap<>();
                for (Path path : modelHeaderFilePaths) {
                    if (Files.exists(path)) {
                        String content = new String(Files.readAllBytes(path));
                        contentMap.put(path.getFileName().toString(), content);
                    } else {
                        LOGGER.warn("Missing the generated Model file: {}", path);
                    }
                }

                // Models should be included in the proper ordering:
                // If a model includes header of the another model,
                // included one should be added first.
                // Result of such filtering is stored into 'filteredOrdering' list.
                List<String> includedHeaders = new ArrayList<>();
                for (Map.Entry<String, String> entry : contentMap.entrySet()) {
                    for (Path path : modelHeaderFilePaths) {
                        String includeHeader = "#include " + "\"" + path.getFileName().toString() + "\"";
                        // Model has included models
                        if (entry.getValue().contains(includeHeader)) {
                            if (!includedHeaders.contains(path.getFileName().toString())) {
                                if (!includedHeaders.contains(entry.getKey())) {
                                    filteredOrdering.add(contentMap.get(path.getFileName().toString()));
                                    includedHeaders.add(path.getFileName().toString());
                                } else {
                                    // if current model was included as a dependency of the another
                                    // model, but current one also has a dependency,
                                    // then insert required dependency.
                                    int index = includedHeaders.indexOf(entry.getKey());
                                    if (index >= 0 && index < includedHeaders.size()) {
                                        filteredOrdering.add(index, contentMap.get(path.getFileName().toString()));
                                        includedHeaders.add(index, path.getFileName().toString());
                                    }
                                }
                            }
                        }
                    }
                    // When includes were checked and included, we can insert the header.
                    if (!includedHeaders.contains(entry.getKey())) {
                        filteredOrdering.add(entry.getValue());
                        includedHeaders.add(entry.getKey());
                    }
                }

                // Writing the result of filtering.
                for (int i = 0; i < filteredOrdering.size(); i++) {
                    writer.write(filteredOrdering.get(i));
                    writer.write("\n\n");
                }

                for (Path path : modelCppFilePaths) {
                    if (Files.exists(path)) {
                        writer.write("// ===== " + path.getFileName() + " =====\n");
                        String content = new String(Files.readAllBytes(path));
                        writer.write(content);
                        writer.write("\n\n");
                    } else {
                        LOGGER.warn("Missing the generated Model file: {}", path);
                    }
                }

                for (Path path : apiClassFiles) {
                    if (Files.exists(path)) {
                        writer.write("// ===== " + path.getFileName() + " =====\n");
                        String content = new String(Files.readAllBytes(path), StandardCharsets.UTF_8);
                        writer.write(content);
                        writer.write("\n\n");
                    } else {
                        LOGGER.warn("Missing the generated API file: {}", path);
                    }
                }
                writer.write("#include \"" + lowerPackageName + "combinedmodelsandapis.moc\"");
                writer.write("\n\n");
                writer.close();
            } catch (IOException e) {
                throw new RuntimeException("Failed to generate combinedmodelsandapis.cpp file, when the option useCmakeMacro is enabled.", e);
            }
        }
        return objs;
    }
}
