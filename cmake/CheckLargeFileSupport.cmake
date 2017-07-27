# - Define macro to check large file support
#
#  check_large_file_support()
#
#  This macro sets the following variables:
#  _LARGE_FILES
#  _LARGEFILE_SOURCE
#  _FILE_OFFSET_BITS 64  
#
#  However, it is YOUR job to make sure these defines are set in a cmakedefine so they
#  end up in a config.h file that is included in your source if necessary!

get_filename_component(_selfdir_CheckLargeFileSupport "${CMAKE_CURRENT_LIST_FILE}" PATH)

macro(CHECK_LARGE_FILE_SUPPORT)
        # On most platforms it is probably overkill to first test the flags for 64-bit off_t,
        # and then separately fseeko. However, in the future we might have 128-bit filesystems
        # (ZFS), so it might be dangerous to indiscriminately set e.g. _FILE_OFFSET_BITS=64.

        message(STATUS "Checking for 64-bit off_t")

	# First check without any special flags
        try_compile(FILE64_OK "${CMAKE_BINARY_DIR}"    
                    "${_selfdir_CheckLargeFileSupport}/TestFileOffsetBits.c"
                      COMPILE_DEFINITIONS "-Werror")
        if(FILE64_OK)
	    message(STATUS "Checking for 64-bit off_t - present")			
      	endif(FILE64_OK)

        if(NOT FILE64_OK)	
	    # Test with _FILE_OFFSET_BITS=64
            try_compile(FILE64_OK "${CMAKE_BINARY_DIR}"
                        "${_selfdir_CheckLargeFileSupport}/TestFileOffsetBits.c"
                        COMPILE_DEFINITIONS "-D_FILE_OFFSET_BITS=64" )
            if(FILE64_OK)
	        message(STATUS "Checking for 64-bit off_t - present with _FILE_OFFSET_BITS=64")
                set(_FILE_OFFSET_BITS 64)
            endif(FILE64_OK)
        endif(NOT FILE64_OK)    

        if(NOT FILE64_OK)
            # Test with _LARGE_FILES
            try_compile(FILE64_OK "${CMAKE_BINARY_DIR}"
                        "${_selfdir_CheckLargeFileSupport}/TestFileOffsetBits.c"
                        COMPILE_DEFINITIONS "-D_LARGE_FILES" )
            if(FILE64_OK)
                message(STATUS "Checking for 64-bit off_t - present with _LARGE_FILES")
                set(_LARGE_FILES 1)
            endif(FILE64_OK)
        endif(NOT FILE64_OK)
	
        if(NOT FILE64_OK)
            # Test with _LARGEFILE_SOURCE
            try_compile(FILE64_OK "${CMAKE_BINARY_DIR}"
                        "${_selfdir_CheckLargeFileSupport}/TestFileOffsetBits.c"
                        COMPILE_DEFINITIONS "-D_LARGEFILE_SOURCE" )
            if(FILE64_OK)
                message(STATUS "Checking for 64-bit off_t - present with _LARGEFILE_SOURCE")
      		      	       set(_LARGEFILE_SOURCE 1)
            endif(FILE64_OK)
        endif(NOT FILE64_OK)
endmacro(CHECK_LARGE_FILE_SUPPORT)
