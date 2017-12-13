SET(SRC_LIST main.cpp
        ALNS.cpp
        Car.cpp
        Dispatcher.cpp
        LNS_rel.cpp
        LoadFile.cpp
        OutputFile.cpp
        PublicFunction.cpp
        Route.cpp
        SetBench.cpp
        Simulator.cpp
        SSALNS.cpp
        Timer.cpp
        LoadBench.cpp
        SaveBench.cpp
        tinystr.cpp
        tinyxml.cpp
        tinyxmlerror.cpp
        tinyxmlparser.cpp
        TxtRecorder.cpp)

ADD_EXECUTABLE(main ${SRC_LIST})
SET(CMAKE_EXE_LINKER_FLAGS "-pthread")

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)

if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
	message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler")
endif()
