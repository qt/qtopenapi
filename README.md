# OpenAPI Generator for the cpp-qt6-client library

## Overview
Our project is for initial development of improved OpenAPI cpp-qt6-client code-generator.
The project uses the existing [cpp-qt-client](https://openapi-generator.tech/docs/generators/cpp-qt-client) generator as a starting point.
The main ideas of a new implementation are:
- implementing a possibility easily to use the generated client code for any Qt UI solutions whether it QML, QWidgets, etc
- enabling the latest Qt6 enhancements, like QRESTful APIs and the latest QHttp APIs
- CMake-based workflows (for Qt application development)
- Support for OAS::anyOf(), OAS::allOf(), OAS::oneOf() and friends
- Support for OAS 3.1 features, as much as possible

The minimal supported Qt version is Qt6.8 LTS. The lowest supported C++ version is C++17.


## Prerequisites
1) OpenJDK
2) OpenAPI
3) Apache Maven
4) Qt


## What's OpenAPI
The goal of OpenAPI is to define a standard, language-agnostic interface to REST APIs which allows both humans and computers to discover and understand the capabilities of the service without access to source code, documentation, or through network traffic inspection.
When properly described with OpenAPI, a consumer can understand and interact with the remote service with a minimal amount of implementation logic.
Similar to what interfaces have done for lower-level programming, OpenAPI removes the guesswork in calling the service.

Check out [OpenAPI-Spec](https://github.com/OAI/OpenAPI-Specification) for additional information about the OpenAPI project, including additional libraries with support for other languages and more.

## How to get started with OpenAPI
There are a number of ways starting to use OpenAPI Generator.
The easiest one is to use the `Bash Launcher Script`:

```
mkdir -p ~/bin/openapitools
curl https://raw.githubusercontent.com/OpenAPITools/openapi-generator/master/bin/utils/openapi-generator-cli.sh > ~/bin/openapitools/openapi-generator-cli
chmod u+x ~/bin/openapitools/openapi-generator-cli
export PATH=$PATH:~/bin/openapitools/

```
To see more ways of getting started with OpenAPI, check the [installation instructions](https://openapi-generator.tech/docs/installation).

## The cpp-qt6-client structure
```
.
|- README.md      // this file
|- pom.xml        // java build script
|- yaml_files/*   // yaml specification's storage
|-- colorpalette.yaml
|-- petstore.yaml
|- target/*       // maven build
|- generator.sh   // bash script to manipulate the project
|-- openapi_client_generators
|--- openapi-generator-cli     //openapi generator
|-- src
|--- main
|---- java
|----- org.qtproject.qt.codegen.CppQt6AbstractCodegen.java // java interface class, that has initial Qt6 type mapping
|----- org.qtproject.qt.codegen.CppQt6ClientGenerator.java // main generator class
|---- resources
|----- cpp-qt6-client // *.mustache template files
|----- META-INF
|------ services
|------- org.openapitools.codegen.CodegenConfig
|--- test
|---- java
|----- org.qtproject.qt.codegen.CppQt6ClientGeneratorTest.java // Java Unit Tests. It checks the generator can be launched(need to be extended in future)
|- tests   // C++ test apppications
|-- auto
|--- colorpalette
|---- qmlclient  // QML-based client test app
|----- client // Contains the auto-generated QML-enabled client code for the colorpalette.yaml file. See `./generate.sh` for details
|------ doc   // Contains the auto-generated doxygen documentation for QML-enabled 'colorpalette' library
|---- client  // client test app
|----- client // Contains the auto-generated client code for the colorpalette.yaml file. See `./generate.sh` for details
|------ doc   // Contains the auto-generated doxygen documentation for 'colorpalette' library
|--- petstore
|---- qmlclient  // QML-based client test app
|----- client // Contains the auto-generated QML-enabled client code for the petstore.yaml specification. See `./generate.sh` for details
|------ doc   // Contains the auto-generated doxygen documentation for QML-enabled 'petstore' library
|---- client  // client test app
|----- client // Default storage for auto-generated client code from the petstore.yaml specification. See `./generate.sh` for details
|------ doc   // Contains the auto-generated doxygen documentation for 'petstore' library
|---- server  // server test app
```

## How to get started with the cpp-qt6-client
To build and run the cpp-qt6-client, use the `generator.sh` script.
The script expects Java environment to be set, and uses `openapi-generator-cli.jar`.
to perform the actual code generation. A copy of this JAR is downloaded for convenience, but any recent stock installation should work as well.

# Usage
```
./generate.sh <command> <specification> <loglevel>
```

The script has the following options of \<command\> as the first argument:

```
'cg' Compile the generator and generate the client code
'doc' Compile doxygen documentation for auto-generated client application
'l' List options provided by the generator
'qmlcg' Compile the generator with Qml-enabled option set, and generate the Qml-enabled client code
'qmldoc' Compile doxygen documentation for auto-generated client application that can be used in QML
'qmltest' Compile and run re-/generated code with QML test applications
'test' Compile and run re-/generated code with C++ test applications
```

All commands (except 'l') accept the following optional arguments:
- \<specification\>: The name of the OpenAPI spec to generate.
  Available options:
  - `petstore` to use use the petstore.yaml specification (default, used if not specified).
  - `colorpalette` to use the colorpalette.yaml specification.

- \<loglevel\>: The desired logging level.
  Available options:
  - `DEBUG`
  - `INFO` (default, used if not specified)
  - `WARN`
  - `ERROR`


To build and run the cpp-qt6-client generator, use:
```
./generate.sh cg <specification> <loglevel>
```

## How to test a generated client code
### ColorPalette
#### QtCore
To compile the `client code` generated by the `cpp-qt6-client` generator, run:
```
./generate.sh test colorpalette
```
Expected test result:
```
generated client files compiles succesfully.
```

#### QML
To compile the `client code` with possibility to be used in QML applications, run:
```
./generate.sh qmltest colorpalette
```
Expected test result:
```
generated client files compiles succesfully.
```

### PetStore
#### QtCore
For first testing of the Qt-based client code, we inspired by following openapi project-samples:
```
openapi-generator/samples/client/petstore/cpp-qt
openapi-generator/samples/server/petstore/cpp-qt-qhttpengine-server
```
For now, samples are part of cpp-qt6-client-project structure:
```
openapi-generator/samples/client/petstore/cpp-qt                     ==> cpp-qt6-client/tests/auto/petstore/client/
openapi-generator/samples/server/petstore/cpp-qt-qhttpengine-server  ==> cpp-qt6-client/tests/auto/petstore/server/
```
By default, the `client code` generated by `cpp-qt6-client` is stored directly into:
`cpp-qt6-client/tests/auto/petstore/client/` and being used as a part of a `cpp-qt-petstore-test` application.

The `cpp-qt-petstore-test` is a client application, that interacts with the server test application using `http://127.0.0.1:9080/v2` url.
The specification used for petstore client generation: `cpp-qt6-client/yaml_files/petstore.yaml`.

To compile the `client code` generated by the `cpp-qt6-client` generator and launch client and server tests, run:
```
./generate.sh test
```
This will use the petstore specification by default.

Expected tests execution result:
```
Start 1: cpp-qt-petstore-test
.........................................
.........................................
.........................................
1/1 Test #1: cpp-qt-petstore-test .............   Passed    0.02 sec

100% tests passed, 0 tests failed out of 1
Total Test time (real) =   0.02 sec
```

#### QML
To compile the `client code` with possibility to be used in QML applications, run:
```
./generate.sh qmltest
```
Expected test result:
```
1/1 Test #1: qml-qt-petstore-test .............   Passed    1.97 sec

100% tests passed, 0 tests failed out of 1

Total Test time (real) =   1.98 sec
```

## To add your changes to the project repo
```
cd existing_repo
git remote add origin git@git.qt.io:tatiana.borisova/cpp-qt6-client.git
git branch -M main
git push -u origin main
```

## How to build documentation for Petstore and Colorpalette test applications
To build Client's documentation use a `doxygen`. The application can be installed by the following command:
```
sudo apt-get install doxygen
```
After installation run the generate.sh script with following option:
```
./generate.sh doc
firefox tests/auto/petstore/client/client/doc/html/index.html

./generate.sh doc colorpalette
firefox tests/auto/colorpalette/client/client/doc/html/index.html

```
or
```
./generate.sh qmldoc
firefox tests/auto/petstore/qmlclient/client/doc/html/index.html

./generate.sh qmldoc colorpalette
firefox tests/auto/colorpalette/qmlclient/client/doc/html/index.html
```
