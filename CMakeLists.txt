cmake_minimum_required(VERSION 3.17)
project(RealTimeCommunication)

set(CMAKE_CXX_STANDARD 11)

#查找wxWidgets库
find_package(wxWidgets COMPONENTS core base net REQUIRED)
include(${wxWidgets_USE_FILE})

#查找Armadillo库
find_package(Armadillo REQUIRED)
include_directories(${ARMADILLO_INCLUDE_DIRS})

add_executable(RealTimeCommunication
        GuiPanel.cpp
        RadarDataCube.cpp
        ParamClass.cpp
        imagepanel.cpp
        mathplot.cpp
        )

#查找Opencv库
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

#包含头文件路径
target_include_directories(RealTimeCommunication
        PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        )

#链接wxWidgets库
target_link_libraries(RealTimeCommunication
        PRIVATE
        ${wxWidgets_LIBRARIES}
        )

#链接arma库
target_link_libraries(RealTimeCommunication
        PRIVATE
        ${ARMADILLO_LIBRARIES}
        )

#链接opencv库
target_link_libraries(RealTimeCommunication
        PRIVATE
        ${OpenCV_LIBS}
        )