cmake_minimum_required(VERSION 3.23)
project(@ProjectName@)

find_package(qibuild) # Always required
qi_sanitize_compile_flags(HIDDEN_SYMBOLS) # To behave the same way on every platform
include_directories(".") # Exposes the headers locally
qi_stage_dir(".") # Exposes the IDLs locally

# Generate the specialized proxies. You can put several IDL files if you want.
find_package(qilang-tools) # Required for qi_gen_idl
set(@projectname@_idl "share/qi/idl/@projectname@/@projectname@.idl.qi")
qi_gen_idl(@projectname@_generated CPP "@projectname@" "${CMAKE_CURRENT_BINARY_DIR}" ${@projectname@_idl})
include_directories(${CMAKE_CURRENT_BINARY_DIR}) # Exposes the generated headers locally

# Install the headers so that people can use your proxies
qi_install_header(
  ${@projectname@_generated_INTERFACE}
  "@projectname@/api.hpp"
  KEEP_RELATIVE_PATHS)

# Create a lib with the proxies only
qi_create_lib(@projectname@
  ${@projectname@_generated}
  ${@projectname@_idl}
  DEPENDS
  qi)

# Create a module with your implementation of the MyService interface
# It is the easiest way to expose your service in NAOqi
find_package(qimodule) # Required only if you create a module

qi_add_module(@projectname@_module INSTALL)

target_sources(
  @projectname@_module
  PRIVATE
    src/@projectname@impl.hpp
    src/@projectname@_module.cpp
)

target_link_libraries(
  @projectname@_module
  PUBLIC
    @projectname@
)
