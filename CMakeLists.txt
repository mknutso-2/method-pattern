cmake_minimum_required(VERSION 3.24)

project(
   Method
   VERSION 0.1.0
   LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(CMakePackageConfigHelpers)
include(FetchContent)

option(METHOD_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer." OFF)
option(METHOD_ENABLE_INSTALL "Install and export the Method CMake package." OFF)

set(METHOD_BUILD_EXAMPLES_DEFAULT OFF)
if(PROJECT_IS_TOP_LEVEL)
   set(METHOD_BUILD_EXAMPLES_DEFAULT ON)
endif()
option(METHOD_BUILD_EXAMPLES "Build Method example projects." "${METHOD_BUILD_EXAMPLES_DEFAULT}")

set(METHOD_BUILD_TESTING_DEFAULT OFF)
if(PROJECT_IS_TOP_LEVEL)
   set(METHOD_BUILD_TESTING_DEFAULT ON)
endif()
option(METHOD_BUILD_TESTING "Build Method tests." "${METHOD_BUILD_TESTING_DEFAULT}")

function(enable_project_warnings target_name)
   # Apply the same warning policy to every project target: highest practical warning level, warnings as errors, and
   # only documented low-signal suppressions.
   if(MSVC)
      target_compile_options(
         ${target_name}
         PRIVATE /Wall
                 /WX
                 /wd4710
                 /wd4711
                 /wd4820
                 /wd5045
                 /permissive-
                 /external:anglebrackets
                 /external:W0
      )
   elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
      target_compile_options(
         ${target_name} PRIVATE -Weverything -Werror -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded
                                -Wno-pre-c++23-compat
      )
   else()
      target_compile_options(
         ${target_name}
         PRIVATE -Wall
                 -Wextra
                 -Wpedantic
                 -Werror
                 -Wconversion
                 -Wsign-conversion
                 -Wshadow
                 -Wformat=2
                 -Wcast-align
                 -Wcast-qual
                 -Wdouble-promotion
                 -Wduplicated-branches
                 -Wduplicated-cond
                 -Wimplicit-fallthrough=5
                 -Wlogical-op
                 -Wmisleading-indentation
                 -Wnon-virtual-dtor
                 -Wnull-dereference
                 # GCC emits false positives through nlohmann/json iterators in optimized builds.
                 $<$<CONFIG:Release>:-Wno-null-dereference>
                 -Wold-style-cast
                 -Woverloaded-virtual
                 -Wredundant-decls
                 -Wuseless-cast
      )
   endif()
endfunction()

function(enable_project_sanitizers target_name)
   if(NOT METHOD_ENABLE_SANITIZERS)
      return()
   endif()

   if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
      message(FATAL_ERROR "METHOD_ENABLE_SANITIZERS requires Clang or GCC.")
   endif()

   target_compile_options(${target_name} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
   target_link_options(${target_name} PRIVATE -fsanitize=address,undefined)
endfunction()

function(add_system_includes_from_target target_name dependency_target_name)
   if(TARGET ${dependency_target_name})
      get_target_property(include_dirs ${dependency_target_name} INTERFACE_INCLUDE_DIRECTORIES)
      if(include_dirs)
         target_include_directories(${target_name} SYSTEM PRIVATE ${include_dirs})
      endif()
   endif()
endfunction()

function(add_public_header_self_check self_check_target dependency_target include_root public_header_glob)
   file(GLOB public_headers CONFIGURE_DEPENDS "${public_header_glob}")
   set(public_header_self_check_sources)
   set(public_header_self_check_dir "${CMAKE_CURRENT_BINARY_DIR}/${self_check_target}")
   file(MAKE_DIRECTORY "${public_header_self_check_dir}")
   foreach(public_header IN LISTS public_headers)
      file(RELATIVE_PATH public_header_relative "${include_root}" "${public_header}")
      string(REPLACE "/" "_" public_header_source_name "${public_header_relative}")
      set(public_header_check_source "${public_header_self_check_dir}/${public_header_source_name}.cpp")
      file(WRITE "${public_header_check_source}" "#include \"${public_header_relative}\"\n")
      list(APPEND public_header_self_check_sources "${public_header_check_source}")
   endforeach()

   add_library(${self_check_target} OBJECT ${public_header_self_check_sources})
   target_link_libraries(${self_check_target} PRIVATE ${dependency_target})
   enable_project_warnings(${self_check_target})
endfunction()

find_package(nlohmann_json CONFIG QUIET)
set(METHOD_USING_FETCHED_NLOHMANN_JSON OFF)
if(NOT nlohmann_json_FOUND)
   set(METHOD_USING_FETCHED_NLOHMANN_JSON ON)
   fetchcontent_declare(
      nlohmann_json
      URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
      URL_HASH SHA256=d6c65aca6b1ed68e7a182f4757257b107ae403032760ed6ef121c9d55e81757d
   )
   fetchcontent_makeavailable(nlohmann_json)
endif()

add_library(MethodCore STATIC)
add_library(Method::Method ALIAS MethodCore)

target_sources(
   MethodCore
   PRIVATE src/Declaration.cpp src/Definition.cpp src/Invocation.cpp
   PUBLIC FILE_SET
          HEADERS
          BASE_DIRS
          include
          FILES
          include/Method/Declaration.h
          include/Method/Definition.h
          include/Method/Invocation.h
          include/Method/Json.h
          include/Method/Kind.h
          include/Method/Version.h
)
set_target_properties(MethodCore PROPERTIES EXPORT_NAME Method OUTPUT_NAME Method)
target_include_directories(
   MethodCore PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>
)
target_compile_features(MethodCore PUBLIC cxx_std_23)
target_link_libraries(MethodCore PUBLIC nlohmann_json::nlohmann_json)
add_system_includes_from_target(MethodCore nlohmann_json::nlohmann_json)
enable_project_warnings(MethodCore)
enable_project_sanitizers(MethodCore)

add_public_header_self_check(
   MethodPublicHeaderSelfCheck Method::Method "${CMAKE_CURRENT_SOURCE_DIR}/include"
   "${CMAKE_CURRENT_SOURCE_DIR}/include/Method/*.h"
)
add_system_includes_from_target(MethodPublicHeaderSelfCheck nlohmann_json::nlohmann_json)

if(PROJECT_IS_TOP_LEVEL)
   add_custom_target(PublicHeaderSelfCheck DEPENDS MethodPublicHeaderSelfCheck)
endif()

if(METHOD_BUILD_EXAMPLES)
   add_library(CounterExampleLibrary STATIC examples/Counter/CounterMethods.cpp examples/Counter/CounterModel.cpp)
   target_include_directories(CounterExampleLibrary PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/examples/Counter")
   target_compile_features(CounterExampleLibrary PUBLIC cxx_std_23)
   target_link_libraries(CounterExampleLibrary PUBLIC Method::Method)
   add_system_includes_from_target(CounterExampleLibrary nlohmann_json::nlohmann_json)
   enable_project_warnings(CounterExampleLibrary)
   enable_project_sanitizers(CounterExampleLibrary)

   add_executable(CounterExample examples/Counter/main.cpp)
   target_link_libraries(CounterExample PRIVATE CounterExampleLibrary)
   add_system_includes_from_target(CounterExample nlohmann_json::nlohmann_json)
   enable_project_warnings(CounterExample)
   enable_project_sanitizers(CounterExample)

   add_library(TodoListCoreExample STATIC examples/TodoList/core/src/TodoListModel.cpp)
   target_include_directories(TodoListCoreExample PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/examples/TodoList/core/include")
   target_compile_features(TodoListCoreExample PUBLIC cxx_std_23)
   enable_project_warnings(TodoListCoreExample)
   enable_project_sanitizers(TodoListCoreExample)

   add_library(TodoListMethodsExample STATIC examples/TodoList/methods/src/TodoListSurface.cpp)
   target_include_directories(
      TodoListMethodsExample PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/examples/TodoList/methods/include"
   )
   target_compile_features(TodoListMethodsExample PUBLIC cxx_std_23)
   target_link_libraries(
      TodoListMethodsExample
      PUBLIC Method::Method
      PRIVATE TodoListCoreExample
   )
   add_system_includes_from_target(TodoListMethodsExample nlohmann_json::nlohmann_json)
   enable_project_warnings(TodoListMethodsExample)
   enable_project_sanitizers(TodoListMethodsExample)

   add_executable(TodoListCliExample examples/TodoList/cli/main.cpp)
   target_link_libraries(TodoListCliExample PRIVATE TodoListMethodsExample)
   add_system_includes_from_target(TodoListCliExample nlohmann_json::nlohmann_json)
   enable_project_warnings(TodoListCliExample)
   enable_project_sanitizers(TodoListCliExample)

   add_executable(TodoListAutomationExample examples/TodoList/automation/main.cpp)
   target_link_libraries(TodoListAutomationExample PRIVATE TodoListMethodsExample)
   add_system_includes_from_target(TodoListAutomationExample nlohmann_json::nlohmann_json)
   enable_project_warnings(TodoListAutomationExample)
   enable_project_sanitizers(TodoListAutomationExample)
endif()

if(METHOD_ENABLE_INSTALL)
   if(METHOD_USING_FETCHED_NLOHMANN_JSON)
      message(FATAL_ERROR "METHOD_ENABLE_INSTALL requires nlohmann_json to be available through find_package.")
   endif()

   install(
      TARGETS MethodCore
      EXPORT MethodTargets
      FILE_SET HEADERS
      DESTINATION include
      ARCHIVE DESTINATION lib
      LIBRARY DESTINATION lib
      RUNTIME DESTINATION bin
   )
   install(
      EXPORT MethodTargets
      FILE MethodTargets.cmake
      NAMESPACE Method::
      DESTINATION lib/cmake/Method
   )

   configure_package_config_file(
      cmake/MethodConfig.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/MethodConfig.cmake" INSTALL_DESTINATION lib/cmake/Method
   )
   write_basic_package_version_file(
      "${CMAKE_CURRENT_BINARY_DIR}/MethodConfigVersion.cmake"
      VERSION "${PROJECT_VERSION}"
      COMPATIBILITY SameMajorVersion
   )
   install(FILES "${CMAKE_CURRENT_BINARY_DIR}/MethodConfig.cmake"
                 "${CMAKE_CURRENT_BINARY_DIR}/MethodConfigVersion.cmake" DESTINATION lib/cmake/Method
   )
endif()

if(METHOD_BUILD_TESTING)
   include(CTest)

   if(BUILD_TESTING)
      if(METHOD_BUILD_EXAMPLES)
         add_test(NAME CounterExample COMMAND CounterExample)
         set_tests_properties(CounterExample PROPERTIES PASS_REGULAR_EXPRESSION "After reset: 0")

         add_test(NAME TodoListCliExample COMMAND TodoListCliExample add "Write docs" add "Run tests" complete 1 list)
         set_tests_properties(TodoListCliExample PROPERTIES PASS_REGULAR_EXPRESSION "Run tests")

         add_test(
            NAME TodoListAutomationExample
            COMMAND
               TodoListAutomationExample
               [[
[{"method":"todo.add","parameters":{"title":"Write docs"}},{"method":"todo.add","parameters":{"title":"Run tests"}},{"method":"todo.complete","parameters":{"id":1}},{"method":"todo.list","parameters":{}}]
]]
         )
         set_tests_properties(TodoListAutomationExample PROPERTIES PASS_REGULAR_EXPRESSION "Run tests")
      endif()

      find_package(Catch2 3 CONFIG QUIET)
      if(NOT Catch2_FOUND)
         fetchcontent_declare(
            Catch2
            URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.7.1.tar.gz
            URL_HASH SHA256=c991b247a1a0d7bb9c39aa35faf0fe9e19764213f28ffba3109388e62ee0269c
         )
         fetchcontent_makeavailable(Catch2)
      endif()

      add_executable(MethodTests tests/MethodTests.cpp)
      target_link_libraries(MethodTests PRIVATE Method::Method Catch2::Catch2WithMain)
      add_system_includes_from_target(MethodTests nlohmann_json::nlohmann_json)
      add_system_includes_from_target(MethodTests Catch2::Catch2)
      add_system_includes_from_target(MethodTests Catch2::Catch2WithMain)
      enable_project_warnings(MethodTests)
      enable_project_sanitizers(MethodTests)

      if(METHOD_BUILD_EXAMPLES)
         add_executable(ExampleTests tests/ExampleTests.cpp)
         target_link_libraries(ExampleTests PRIVATE CounterExampleLibrary TodoListMethodsExample Catch2::Catch2WithMain)
         add_system_includes_from_target(ExampleTests nlohmann_json::nlohmann_json)
         add_system_includes_from_target(ExampleTests Catch2::Catch2)
         add_system_includes_from_target(ExampleTests Catch2::Catch2WithMain)
         enable_project_warnings(ExampleTests)
         enable_project_sanitizers(ExampleTests)
      endif()

      if(DEFINED catch2_SOURCE_DIR)
         list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
      endif()
      include(Catch)
      catch_discover_tests(MethodTests)
      if(METHOD_BUILD_EXAMPLES)
         catch_discover_tests(ExampleTests)
      endif()
   endif()
endif()
