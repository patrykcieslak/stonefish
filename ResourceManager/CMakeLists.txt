# Copyright (c) 2018 Johnny Borov <JohnnyBorov@gmail.com>. Released under MIT License.

add_executable(embed_resource embed_resource.cpp)

set(RESOURCE_MANAGER_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE)

# this file contains defenition for ResourceHandle constructor
set(RESOURCE_MANAGER_RESOURCES_CONFIG_FILE "RM_generated_files/__resources__config.cpp" PARENT_SCOPE)

function(rm_embed_resources output_resources_list)
  if(${ARGC} EQUAL 1)
    message(FATAL_ERROR "ResourceManager::rm_embed_resources: No resources provided")
  else()
    set(input_files_list)
    set(output_files_list)
    foreach(input_file_name ${ARGN})
      file(RELATIVE_PATH relative_to_binary_input_file_name "${CMAKE_BINARY_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/${input_file_name}")
      set(output_file_name "RM_generated_files/${input_file_name}.cpp")
      get_filename_component(output_file_dir ${output_file_name} DIRECTORY)
      
      # this runs only if input_file_name file changed since last time or output_file_name doesnt exist
      add_custom_command(
        OUTPUT "${output_file_name}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${output_file_dir}"
        COMMAND embed_resource "-data" "${input_file_name}" "${relative_to_binary_input_file_name}" "${output_file_name}"
        DEPENDS "${input_file_name}"
        VERBATIM)

      list(APPEND input_files_list "${input_file_name}")
      list(APPEND output_files_list "${output_file_name}")
    endforeach()
    
    # this runs only if one or more output_file_name file from the loop before changed since last time
    # or if RESOURCE_MANAGER_RESOURCES_CONFIG_FILE doesnt exist
    # there's no need to call cmake -E make_directory because its already created by this time
    add_custom_command(
      OUTPUT "${RESOURCE_MANAGER_RESOURCES_CONFIG_FILE}"
      COMMAND embed_resource "-config" "${input_files_list}" "${RESOURCE_MANAGER_RESOURCES_CONFIG_FILE}"
      DEPENDS ${output_files_list}
      VERBATIM)

    list(APPEND output_files_list "${RESOURCE_MANAGER_RESOURCES_CONFIG_FILE}")
    set(${output_resources_list} ${output_files_list} PARENT_SCOPE)
  endif()
endfunction()