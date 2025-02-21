project(engine-core)

set(HEADER_FILES
	include/backend/api.h
	include/context/application_context.h
	include/diagnostics/diagnostics.h
	include/diagnostics/profiler.h
	include/ecs/components.h
	include/ecs/ecs.h
	include/ecs/ecs_collection.h
	include/ecs/entity_container.hpp
	include/ecs/reflection.h
	include/engine.h
	include/input/cursor.h
	include/input/input.h
	include/physics/core/physics_bridge.h
	include/physics/core/physics_context.h
	include/physics/physics.h
	include/physics/rigidbody/rigidbody.h
	include/physics/rigidbody/rigidbody_enums.h
	include/physics/utils/px_translator.h
	include/rendering/culling/bounding_volume.h
	include/rendering/gizmos/gizmos.h
	include/rendering/gizmos/gizmo_color.h
	include/rendering/gizmos/imgizmo.h
	include/rendering/icons/icon_pool.h
	include/rendering/material/imaterial.h
	include/rendering/material/lit/lit_material.h
	include/rendering/material/unlit/unlit_material.h
	include/rendering/model/mesh.h
	include/rendering/model/model.h
	include/rendering/passes/forward_pass.h
	include/rendering/passes/preprocessor_pass.h
	include/rendering/passes/pre_pass.h
	include/rendering/passes/ssao_pass.h
	include/rendering/postprocessing/bloom_pass.h
	include/rendering/postprocessing/motion_blur_pass.h
	include/rendering/postprocessing/post_processing.h
	include/rendering/postprocessing/post_processing_pipeline.h
	include/rendering/primitives/global_quad.h
	include/rendering/sceneview/scene_view_forward_pass.h
	include/rendering/shader/shader.h
	include/rendering/shader/shader_pool.h
	include/rendering/shadows/shadow_disk.h
	include/rendering/shadows/shadow_map.h
	include/rendering/skybox/cubemap.h
	include/rendering/skybox/skybox.h
	include/rendering/texture/texture.h
	include/rendering/transformation/transformation.h
	include/rendering/velocitybuffer/velocity_buffer.h
	include/resource/resource.h
	include/resource/resource_loader.h
	include/time/time.h
	include/transform/transform.h
	include/utils/console.h
	include/utils/iohandler.h
	include/utils/string_helper.h
	include/viewport/viewport.h
)

set(SOURCE_FILES
	src/context/application_context.cpp
	src/diagnostics/diagnostics.cpp
	src/diagnostics/profiler.cpp
	src/ecs/ecs.cpp
	src/ecs/reflection.cpp
	src/input/cursor.cpp
	src/input/input.cpp
	src/misc/stb_image.cpp
	src/physics/core/physics_bridge.cpp
	src/physics/core/physics_context.cpp
	src/physics/rigidbody/rigidbody.cpp
	src/physics/utils/px_translator.cpp
	src/rendering/culling/bounding_volume.cpp
	src/rendering/gizmos/imgizmo.cpp
	src/rendering/icons/icon_pool.cpp
	src/rendering/material/lit/lit_material.cpp
	src/rendering/material/unlit/unlit_material.cpp
	src/rendering/model/mesh.cpp
	src/rendering/model/model.cpp
	src/rendering/passes/forward_pass.cpp
	src/rendering/passes/preprocessor_pass.cpp
	src/rendering/passes/pre_pass.cpp
	src/rendering/passes/ssao_pass.cpp
	src/rendering/postprocessing/bloom_pass.cpp
	src/rendering/postprocessing/motion_blur_pass.cpp
	src/rendering/postprocessing/post_processing_pipeline.cpp
	src/rendering/primitives/global_quad.cpp
	src/rendering/sceneview/scene_view_forward_pass.cpp
	src/rendering/shader/shader.cpp
	src/rendering/shader/shader_pool.cpp
	src/rendering/shadows/shadow_disk.cpp
	src/rendering/shadows/shadow_map.cpp
	src/rendering/skybox/cubemap.cpp
	src/rendering/skybox/skybox.cpp
	src/rendering/texture/texture.cpp
	src/rendering/transformation/transformation.cpp
	src/rendering/velocitybuffer/velocity_buffer.cpp
	src/resource/resource_loader.cpp
	src/time/time.cpp
	src/transform/transform.cpp
	src/utils/console.cpp
	src/utils/iohandler.cpp
	src/utils/string_helper.cpp
	src/viewport/viewport.cpp
)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})
add_library(nuro::core ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} 
	PUBLIC 
		${CMAKE_CURRENT_LIST_DIR}/include
		${Stb_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} 
	PRIVATE 
		assimp::assimp
		EnTT::EnTT	
		glad::glad 
		glfw 
		glm::glm 
		unofficial::omniverse-physx-sdk::sdk
)

# Optional: import the defined target to copy over the GPU acceleration libraries (3rd party provided by NVIDIA)
if(TARGET unofficial::omniverse-physx-sdk::gpu-library)
    if(UNIX)
        # Add rpath setting to find .so libraries on unix based systems
        set_target_properties(${PROJECT_NAME} PROPERTIES
            BUILD_WITH_INSTALL_RPATH TRUE
            INSTALL_RPATH "$ORIGIN"
        )
    endif()
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:unofficial::omniverse-physx-sdk::gpu-library>
                    $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    if(WIN32)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        $<TARGET_FILE:unofficial::omniverse-physx-sdk::gpu-device-library>
                        $<TARGET_FILE_DIR:${PROJECT_NAME}>)
    endif()
else()
    message(WARNING "GPU acceleration library target not defined - GPU acceleration will NOT be available!")
endif()

