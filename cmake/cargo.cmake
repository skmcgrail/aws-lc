# Copyright Amazon.com Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0 OR ISC

# Builds Cargo `staticlib` crates so they can be linked into C targets. Included
# when ENABLE_RUST is on.

find_program(CARGO_EXECUTABLE NAMES cargo
             HINTS "$ENV{CARGO_HOME}/bin" "$ENV{HOME}/.cargo/bin")
if(NOT CARGO_EXECUTABLE)
  message(FATAL_ERROR "ENABLE_RUST is set but cargo was not found. Install a "
                      "Rust toolchain (https://rustup.rs) or set CARGO_EXECUTABLE.")
endif()

execute_process(
  COMMAND ${CARGO_EXECUTABLE} --version
  OUTPUT_VARIABLE CARGO_VERSION_OUTPUT
  OUTPUT_STRIP_TRAILING_WHITESPACE
  RESULT_VARIABLE CARGO_VERSION_RESULT)
if(NOT CARGO_VERSION_RESULT EQUAL 0)
  message(FATAL_ERROR "Failed to run ${CARGO_EXECUTABLE} --version")
endif()
message(STATUS "Found cargo: ${CARGO_EXECUTABLE} (${CARGO_VERSION_OUTPUT})")

set(RUST_TARGET "" CACHE STRING
    "Rust target triple to build for when ENABLE_RUST is on (default: host)")
if(CMAKE_CROSSCOMPILING AND NOT RUST_TARGET)
  message(FATAL_ERROR "ENABLE_RUST requires RUST_TARGET to be set when "
                      "cross-compiling, e.g. -DRUST_TARGET=aarch64-unknown-linux-gnu")
endif()

# cargo_add_staticlib(<target> MANIFEST_PATH <path> PACKAGE <crate>
#                     LIB_NAME <name>)
#
# Defines the IMPORTED STATIC library <target>, built from the Cargo package
# PACKAGE in the workspace at MANIFEST_PATH. LIB_NAME is the crate's [lib]
# name. Sets <target>_FILE_NAME in the caller's scope to the archive's file
# name.
#
# Cargo is run on every build and does its own dependency tracking, so Rust
# source changes are picked up without re-running CMake. Debug builds (and
# builds with no CMAKE_BUILD_TYPE) use Cargo's `dev` profile; all others use
# `release`.
function(cargo_add_staticlib target)
  cmake_parse_arguments(arg "" "MANIFEST_PATH;PACKAGE;LIB_NAME" "" ${ARGN})
  if(NOT arg_MANIFEST_PATH OR NOT arg_PACKAGE OR NOT arg_LIB_NAME)
    message(FATAL_ERROR "cargo_add_staticlib requires MANIFEST_PATH, PACKAGE and LIB_NAME")
  endif()

  set(target_dir "${AWSLC_BINARY_DIR}/cargo")
  set(out_dir "${target_dir}")
  set(cargo_target_args "")
  if(RUST_TARGET)
    set(cargo_target_args --target "${RUST_TARGET}")
    set(out_dir "${out_dir}/${RUST_TARGET}")
  endif()

  if(MSVC)
    set(file_name "${arg_LIB_NAME}.lib")
  else()
    set(file_name "lib${arg_LIB_NAME}.a")
  endif()

  add_library(${target} STATIC IMPORTED)

  if(CMAKE_CONFIGURATION_TYPES)
    # Multi-config generators choose the profile at build time.
    set(profile "$<$<CONFIG:Debug>:dev>$<$<NOT:$<CONFIG:Debug>>:release>")
    set(lib_path "${out_dir}/$<$<CONFIG:Debug>:debug>$<$<NOT:$<CONFIG:Debug>>:release>/${file_name}")
    foreach(config ${CMAKE_CONFIGURATION_TYPES})
      string(TOUPPER "${config}" config_upper)
      if(config_upper STREQUAL "DEBUG")
        set(config_dir "debug")
      else()
        set(config_dir "release")
      endif()
      set_target_properties(${target} PROPERTIES
        IMPORTED_LOCATION_${config_upper} "${out_dir}/${config_dir}/${file_name}")
    endforeach()
    # Generator expressions in BYPRODUCTS need CMake 3.20. Only the Ninja
    # Multi-Config generator needs them.
    set(byproducts "")
    if(NOT CMAKE_VERSION VERSION_LESS "3.20")
      set(byproducts BYPRODUCTS "${lib_path}")
    endif()
  else()
    string(TOLOWER "${CMAKE_BUILD_TYPE}" build_type_lower)
    if(build_type_lower STREQUAL "debug" OR build_type_lower STREQUAL "")
      set(profile "dev")
      set(lib_path "${out_dir}/debug/${file_name}")
    else()
      set(profile "release")
      set(lib_path "${out_dir}/release/${file_name}")
    endif()
    set_target_properties(${target} PROPERTIES IMPORTED_LOCATION "${lib_path}")
    set(byproducts BYPRODUCTS "${lib_path}")
  endif()

  add_custom_target(${target}_cargo_build
    COMMAND ${CARGO_EXECUTABLE} build
            --locked
            --manifest-path "${arg_MANIFEST_PATH}"
            --target-dir "${target_dir}"
            --package "${arg_PACKAGE}"
            --profile "${profile}"
            ${cargo_target_args}
    ${byproducts}
    COMMENT "Building Rust crate ${arg_PACKAGE}"
    USES_TERMINAL
    VERBATIM)
  add_dependencies(${target} ${target}_cargo_build)

  set(${target}_FILE_NAME "${file_name}" PARENT_SCOPE)
endfunction()
