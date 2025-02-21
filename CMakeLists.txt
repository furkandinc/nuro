cmake_minimum_required(VERSION 3.10)

project(nuro CXX)

find_package(assimp CONFIG REQUIRED) 
find_package(EnTT CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(unofficial-omniverse-physx-sdk CONFIG REQUIRED)

add_subdirectory(engine-core)
#add_subdirectory(engine-editor)
#add_subdirectory(examples)