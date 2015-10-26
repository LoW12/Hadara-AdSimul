cmake_minimum_required(VERSION 2.8)
project(Hadara_AdSimul)

add_definitions(-std=c++0x)

set(SOURCE_FILES_GEN
        Source/tinyxml2/tinyxml2.cpp
        Source/CVars/CVarManager.cpp
        Source/mainGen.cpp
        Source/CGraph.cpp
        Source/CGraphUtility.cpp
        Source/CPetriNet.cpp
        Source/CWFGenerator.cpp
 )

set(SOURCE_FILES_RED
        Source/tinyxml2/tinyxml2.cpp
        Source/mainRed.cpp
        Source/CGraph.cpp
        Source/CGraphUtility.cpp
        Source/CPetriNet.cpp
        Source/CWFReducer.cpp
 )

set(SOURCE_FILES_TOSPIN
         Source/tinyxml2/tinyxml2.cpp
         Source/CVars/CVarManager.cpp
         Source/mainToSpin.cpp
         Source/CGraph.cpp
         Source/CGraphUtility.cpp
         Source/CPetriNet.cpp
  )

add_executable(Hadara_AdSimul_Gen ${SOURCE_FILES_GEN})
add_executable(Hadara_AdSimul_Red ${SOURCE_FILES_RED})
add_executable(Hadara_AdSimul_ToSpin ${SOURCE_FILES_TOSPIN})
