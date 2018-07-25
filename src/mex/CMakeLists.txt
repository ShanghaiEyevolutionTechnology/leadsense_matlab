set(TARGET_NAME mexLeadSense)

include_directories(${MATLAB_INCLUDE_DIR})
include_directories(${EvoBinoSDK_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

link_directories(${OpenCV_LIBRARY_DIRS})

add_library(${TARGET_NAME} SHARED ${TARGET_NAME}.cpp ${CMAKE_SOURCE_DIR}/Matlabdef.def)

target_link_libraries(${TARGET_NAME} ${MATLAB_LIBRARIES} ${EvoBinoSDK_LIBRARIES} ${OpenCV_LIBRARIES})

if(WIN32)
    set_target_properties(${TARGET_NAME} PROPERTIES SUFFIX .mexw64)
else(WIN32)
    if (CMAKE_SIZEOF_VOID_P MATCHES "8")
        set_target_properties(${TARGET_NAME} PROPERTIES SUFFIX .mexa64 PREFIX "")
    else(CMAKE_SIZEOF_VOID_P MATCHES "8")
        set_target_properties(${TARGET_NAME} PROPERTIES SUFFIX .mexglx PREFIX "")
    endif (CMAKE_SIZEOF_VOID_P MATCHES "8")
endif(WIN32)

install(TARGETS ${TARGET_NAME} DESTINATION ../matlab)
