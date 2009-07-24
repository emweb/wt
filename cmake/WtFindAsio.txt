# Attention: must have done WtFindBoost.txt before this one!
#
# This defines:
# - ASIO_INCLUDE_DIRS
# - ASIO_LIBRARIES
# - ASIO_DEFINITIONS
# - ASIO_FOUND
#  
# This file will search for both plain and boost kinds of asio, and
# set up the correct includes and libraries
# ASIO_LIBRARIES is set to the boost system library, if the boost
# version of asio was detected. Otherwise, it remains empty.


#FIND_PATH(PLAIN_ASIO_INCLUDE
#  NAMES
#    asio.hpp
#  PATHS
#    ${BOOST_INCLUDE_DIRS}
#    /usr/include
#    /usr/local/include
#)

IF(BOOST_WTHTTP_FOUND)
  FIND_PATH(BOOST_ASIO_INCLUDE
    NAMES
      boost/asio.hpp
    PATHS
      ${BOOST_INCLUDE_DIRS}
    NO_DEFAULT_PATH
  )
ENDIF(BOOST_WTHTTP_FOUND)

SET(ASIO_FOUND FALSE)
SET(ASIO_DEFINITIONS  "")
SET(ASIO_LIBRARIES    "")
SET(ASIO_INCLUDE_DIRS "")

IF(PLAIN_ASIO_INCLUDE)
  SET (ASIO_INCLUDE_DIRS ${PLAIN_ASIO_INCLUDE})
  SET (ASIO_LIBRARIES "")
  SET(ASIO_FOUND TRUE)
ELSE(PLAIN_ASIO_INCLUDE)
  IF(BOOST_ASIO_INCLUDE
      AND DEFINED BOOST_WT_SYSTEM_LIBRARY)
    SET (ASIO_INCLUDE ${BOOST_ASIO_INCLUDE})
    SET(ASIO_DEFINITIONS -DBOOST_ASIO)
    SET(ASIO_LIBRARIES ${BOOST_WT_SYSTEM_LIBRARY})
    SET(ASIO_FOUND TRUE)
  ENDIF(BOOST_ASIO_INCLUDE
      AND DEFINED BOOST_WT_SYSTEM_LIBRARY)
ENDIF(PLAIN_ASIO_INCLUDE)

