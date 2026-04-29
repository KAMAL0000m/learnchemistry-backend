cmake_minimum_required(VERSION 3.20)
project(LearnChemistry LANGUAGES CXX)

# -----------------------------
# C++ Standard
# -----------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------
# Options
# -----------------------------
option(ENABLE_MYSQL "Enable MySQL Connector/C++" ON)

# -----------------------------
# Dependencies (via vcpkg)
# -----------------------------
find_package(Boost CONFIG REQUIRED COMPONENTS system thread)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(unofficial-sodium CONFIG REQUIRED)

if (ENABLE_MYSQL)
    find_package(unofficial-mysql-connector-cpp CONFIG REQUIRED)
endif()

# -----------------------------
# Executable
# -----------------------------
add_executable(learnChemistry
    main.cpp
    config/AppConfig.cpp
    server/HttpServer.cpp
    server/Session.cpp
    routing/Router.cpp
    middleware/MiddlewareChain.cpp
    middleware/LoggingMiddleware.cpp
    middleware/JwtAuthMiddleware.cpp
    controllers/AuthController.cpp
    services/AuthService.cpp
    repositories/UserRepository.cpp
    db/MySqlPool.cpp
    security/JwtService.cpp
    security/PasswordHasher.cpp
    utils/HttpResponse.cpp
    controllers/CourseController.cpp
    services/CourseService.cpp
    repositories/CourseRepository.cpp

 "models/Course.h" "repositories/CourseRepository.h" "repositories/CourseRepository.cpp" "services/CourseService.h" "services/CourseService.cpp" "controllers/CourseController.h" "controllers/CourseController.cpp")

# -----------------------------
# Include directories
# -----------------------------
target_include_directories(learnChemistry PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# -----------------------------
# Link libraries
# -----------------------------
target_link_libraries(learnChemistry PRIVATE
    Boost::system
    Boost::thread
    nlohmann_json::nlohmann_json
    OpenSSL::Crypto
    unofficial-sodium::sodium
)

# -----------------------------
# MySQL (vcpkg - CLEAN)
# -----------------------------
if (ENABLE_MYSQL)
    target_link_libraries(learnChemistry PRIVATE
        unofficial::mysql-connector-cpp::connector
    )

    target_compile_definitions(learnChemistry PRIVATE ENABLE_MYSQL)
endif()

# -----------------------------
# Windows-specific
# -----------------------------
if (MSVC)
    target_compile_definitions(learnChemistry PRIVATE
        _WIN32_WINNT=0x0A00
        NOMINMAX
    )
endif()