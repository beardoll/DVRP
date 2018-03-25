SET(SRC_LIST main.cpp
        algorithm/LNSBase.cpp
        algorithm/SSLR.cpp
        baseclass/Car.cpp
        baseclass/Route.cpp
        baseclass/Spot.cpp
        public/PublicFunction.cpp
        run/TxtRecorder.cpp
        run/Config.cpp
        dataset/TxtParser.cpp
        )

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
