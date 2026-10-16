cmake_minimum_required(VERSION 3.16)
project(AbdullahTool VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ضمان قراءة النصوص العربية في MSVC بدون رموز غريبة
if(MSVC)
    add_compile_options(/utf-8)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

add_executable(AbdullahTool
    main.cpp
)

# إخفاء نافذة CMD السوداء عند تشغيل البرنامج على ويندوز
if(MSVC)
    set_target_properties(AbdullahTool PROPERTIES
        WIN32_EXECUTABLE TRUE
        LINK_FLAGS "/ENTRY:mainCRTStartup"
    )
endif()

target_link_libraries(AbdullahTool PRIVATE 
    Qt6::Core 
    Qt6::Gui 
    Qt6::Widgets
)
