# This file defines:
# - POSTGRES_LIBRARIES
# - POSTGRES_INCLUDE_DIRS
# - POSTGRES_FOUND
# Taking into account:
# - POSTGRES_PREFIX

SET(POSTGRES_LIBRARY pq libpq)
FOREACH(l ${POSTGRES_LIBRARY})
  LIST(APPEND POSTGRES_DEBUG_LIBRARY "${l}d")
ENDFOREACH()

FIND_LIBRARY(POSTGRES_LIB
  NAMES
    ${POSTGRES_LIBRARY}
  PATHS
    ${POSTGRES_PREFIX}/lib
    ${POSTGRES_PREFIX}
    /usr/lib
    /usr/lib64
    /usr/local/lib
    /opt/local/lib
)

FIND_LIBRARY(POSTGRES_DEBUG_LIB
  NAMES
    ${POSTGRES_DEBUG_LIBRARY}
  PATHS
    ${POSTGRES_PREFIX}/lib
    ${POSTGRES_PREFIX}
    /usr/lib
    /usr/lib64
    /usr/local/lib
    /opt/local/lib
)

IF (POSTGRES_DEBUG_LIB)
  SET(POSTGRES_LIBRARIES optimized ${POSTGRES_LIB} debug ${POSTGRES_DEBUG_LIB})
ELSE (POSTGRES_DEBUG_LIB)
  SET(POSTGRES_LIBRARIES ${POSTGRES_LIB})
ENDIF (POSTGRES_DEBUG_LIB)

FIND_PATH(POSTGRES_INCLUDE libpq-fe.h
    ${POSTGRES_PREFIX}/include
    ${POSTGRES_PREFIX}/postgresql/include
    ${POSTGRES_PREFIX}/include/postgresql
    /usr/include
    /usr/include/pgsql
    /usr/include/postgresql
    /usr/local/include
    /usr/local/include/postgresql
    /opt/local/include	
    /opt/local/include/postgresql
)

SET(POSTGRES_FOUND FALSE)

IF(POSTGRES_LIBRARIES AND POSTGRES_INCLUDE)
  SET(POSTGRES_FOUND TRUE)
  SET(POSTGRES_INCLUDE_DIRS ${POSTGRES_INCLUDE})
  IF (WIN32)
    SET(POSTGRES_LIBRARIES secur32.lib ws2_32.lib ${POSTGRES_LIBRARIES})
  ENDIF (WIN32)
ENDIF(POSTGRES_LIBRARIES AND POSTGRES_INCLUDE)

