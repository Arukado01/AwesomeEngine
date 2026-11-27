add_executable(ae_collision_tests
    Core/CollisionTests.cpp
    ${PROJECT_SOURCE_DIR}/Engine/Source/Utils/SweptAabb.cpp
    ${PROJECT_SOURCE_DIR}/Engine/Source/Managers/CollisionManager.cpp
)

target_include_directories(ae_collision_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Engine/Include
)

target_compile_features(ae_collision_tests PRIVATE cxx_std_20)
target_link_libraries(ae_collision_tests PRIVATE SFML::Graphics)

if(MSVC)
    target_compile_options(ae_collision_tests PRIVATE /W4)
else()
    target_compile_options(ae_collision_tests PRIVATE -Wall -Wextra)
endif()

add_test(NAME core.collision COMMAND ae_collision_tests)
set_tests_properties(core.collision PROPERTIES
    LABELS "core;headless"
)

add_test(NAME core.collision.detects_failure COMMAND ae_collision_tests)
set_tests_properties(core.collision.detects_failure PROPERTIES
    ENVIRONMENT "AE_TEST_FORCE_FAIL=1"
    WILL_FAIL TRUE
    LABELS "core;headless;negative"
)

##################################################################################################

add_executable(ae_sprite_animator_tests
    Core/SpriteAnimatorTests.cpp
    ${PROJECT_SOURCE_DIR}/Engine/Source/Graphics/SpriteAnimator/SpriteAnimator.cpp
)

target_include_directories(ae_sprite_animator_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Engine/Include
)

target_compile_features(ae_sprite_animator_tests PRIVATE cxx_std_20)
target_link_libraries(ae_sprite_animator_tests PRIVATE SFML::Graphics)

if(MSVC)
    target_compile_options(ae_sprite_animator_tests PRIVATE /W4)
else()
    target_compile_options(ae_sprite_animator_tests PRIVATE -Wall -Wextra)
endif()

add_test(NAME core.sprite_animator COMMAND ae_sprite_animator_tests)
set_tests_properties(core.sprite_animator PROPERTIES
    LABELS "core;headless;animation"
)

##################################################################################################

add_executable(ae_save_manager_tests
    Core/SaveManagerTests.cpp
    ${PROJECT_SOURCE_DIR}/Engine/Source/Managers/SaveManager.cpp
)

target_include_directories(ae_save_manager_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/Engine/Include
)

target_compile_features(ae_save_manager_tests PRIVATE cxx_std_20)
target_link_libraries(ae_save_manager_tests PRIVATE nlohmann_json::nlohmann_json)

if(MSVC)
    target_compile_options(ae_save_manager_tests PRIVATE /W4)
else()
    target_compile_options(ae_save_manager_tests PRIVATE -Wall -Wextra)
endif()

add_test(NAME core.save_manager COMMAND ae_save_manager_tests)
set_tests_properties(core.save_manager PROPERTIES
    LABELS "core;headless;persistence"
)
