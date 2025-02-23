cmake_minimum_required(VERSION 3.10)

project(nuro CXX)

set(CMAKE_CXX_STANDARD 17)

#engine packages
find_package(assimp CONFIG REQUIRED) 
find_package(EnTT CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(unofficial-omniverse-physx-sdk CONFIG REQUIRED)

#editor packages
find_package(nlohmann_json CONFIG REQUIRED)
# find_package(imgui CONFIG REQUIRED) # imgui is imported from local directory
#find_package(implot CONFIG REQUIRED)
#find_package(imguizmo CONFIG REQUIRED)

add_subdirectory(engine-core)
add_subdirectory(engine-editor)
#add_subdirectory(examples)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/engine-core/shaders/ DESTINATION ${CMAKE_BINARY_DIR}/shaders)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/engine-editor/examples/ DESTINATION ${CMAKE_BINARY_DIR}/engine-editor/examples)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/resources/ DESTINATION ${CMAKE_BINARY_DIR}/resources)