cmake_minimum_required(VERSION 3.2.3) #using the continue instruction

macro(SetupAndInstallPdb)
  set(target ${ARGV0})

  #pdbs only for msvc
  if(NOT MSVC)
    return()
  endif()

  #lets find out what config we are using
  set(configuration_types)
  if(CMAKE_CONFIGURATION_TYPES)
    foreach(config_type ${CMAKE_CONFIGURATION_TYPES})
      set(configuration_types ${configuration_types} ${config_type})
    endforeach()
  else()
    set(configuration_types ${CMAKE_BUILD_TYPE})
  endif()

  get_target_property(target_type ${target} TYPE)
  if(target_type STREQUAL "EXECUTABLE")
    set(target_exe TRUE)
  else()
    set(target_exe FALSE)
  endif()

#  get_target_property(target_compile_flags ${target} COMPILE_FLAGS)
#  message(STATUS "target_compile_flags: ${target_compile_flags}")

  set(pdb_files)
  foreach(config_type ${configuration_types})
    string(TOUPPER ${config_type} config_type_upper)
    #only add pdb info for configs that have debug information
    if(NOT CMAKE_CXX_FLAGS_${config_type_upper} MATCHES "/ZI" AND NOT CMAKE_CXX_FLAGS_${config_type_upper} MATCHES "/Zi")
        continue()
    endif()

    if(target_exe)
        get_target_property(pdb_output_dir ${target} RUNTIME_OUTPUT_DIRECTORY)
    else()
        get_target_property(pdb_output_dir ${target} LIBRARY_OUTPUT_DIRECTORY)
    endif()
    set(pdb_output_dir "${pdb_output_dir}${config_type}/")
    set(pdb_name ${target}${CMAKE_${config_type_upper}_POSTFIX})

    set_target_properties(${target} PROPERTIES 
      COMPILE_PDB_NAME_${config_type_upper} ${pdb_name}
      COMPILE_PDB_OUTPUT_DIRECTORY_{config_type_upper} ${pdb_output_dir}
      )

    set(pdb_files ${pdb_files} ${pdb_output_dir}${pdb_name}.pdb)
  endforeach()

  include(GNUInstallDirs)

  if(target_exe)
    set(pdb_dst ${CMAKE_INSTALL_BINDIR})
  else()
    if(target_type STREQUAL "SHARED_LIBRARY")
      set(pdb_dst ${CMAKE_INSTALL_BINDIR})
    else()
      set(pdb_dst ${CMAKE_INSTALL_LIBDIR})
    endif()
  endif()

  install(
      FILES ${pdb_files}
      DESTINATION ${pdb_dst}
  )

  endmacro()