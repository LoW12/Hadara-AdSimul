cmake_minimum_required(VERSION 3.2)
project(AdSimul)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

set(GEN_SOURCE_FILES
    Source/tinyxml2/tinyxml2.cpp
    Source/CVars/CVarManager.cpp
    Source/CGraph/CGraph.cpp
    Source/CGraph/CGraphUtility.cpp
    Source/CGraph/CPetriNet.cpp
    Source/CWFGenerator.cpp
    Source/mainGen.cpp
 )

add_executable(Hadara_AdSimul_Gen ${GEN_SOURCE_FILES})

set(RED_SOURCE_FILES
    Source/tinyxml2/tinyxml2.cpp
    Source/CVars/CVarManager.cpp
    Source/CGraph/CGraph.cpp
    Source/CGraph/CGraphUtility.cpp
    Source/CGraph/CPetriNet.cpp
    Source/CWFReducer.cpp
    Source/mainRed.cpp
 )

set(RED_WEB_SOURCE_FILES
    Source/tinyxml2/tinyxml2.cpp
    Source/CVars/CVarManager.cpp
    Source/CGraph/CGraph.cpp
    Source/CGraph/CGraphUtility.cpp
    Source/CGraph/CPetriNet.cpp
    Source/CWFReducer.cpp
    Source/mainRedWeb.cpp
 )

set(RED_BATCH_SOURCE_FILES
    Source/tinyxml2/tinyxml2.cpp
    Source/CVars/CVarManager.cpp
    Source/CGraph/CGraph.cpp
    Source/CGraph/CGraphUtility.cpp
    Source/CGraph/CPetriNet.cpp
    Source/CWFReducer.cpp
    Source/mainRedBatch.cpp
 )

include_directories("/home/low/Tools/z3-4.3.2.6/include")
find_library(z3 z3 /home/low/Tools/z3-4.3.2.6/bin NO_DEFAULT_PATH)
add_executable(Hadara_AdSimul_Red ${RED_SOURCE_FILES})
target_link_libraries(Hadara_AdSimul_Red ${z3})
add_executable(Hadara_AdSimul_Red_Web ${RED_WEB_SOURCE_FILES})
target_link_libraries(Hadara_AdSimul_Red_Web ${z3})
add_executable(Hadara_AdSimul_Red_Batch ${RED_BATCH_SOURCE_FILES})
target_link_libraries(Hadara_AdSimul_Red_Batch ${z3})
