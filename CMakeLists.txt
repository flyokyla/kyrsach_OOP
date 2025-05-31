cmake_minimum_required(VERSION 3.16)
project(office_equipment_manager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets PrintSupport Charts REQUIRED)

file(GLOB SOURCES "*.cpp")
file(GLOB HEADERS "*.h")
file(GLOB FORMS "*.ui")

set(SOURCES
    main.cpp
    mainwindow.cpp
    equipmentmodel.cpp
    equipmentrecord.cpp
    aboutdialog.cpp
    chartwindow.cpp
    comboboxdelegate.cpp
    datedelegate.cpp
    equipmentdialog.cpp
    settings.cpp
)

set(HEADERS
    mainwindow.h
    equipmentmodel.h
    equipmentrecord.h
    aboutdialog.h
    chartwindow.h
    comboboxdelegate.h
    datedelegate.h
    equipmentdialog.h
    settings.h
)

set(TS_FILES
    translations/office_equipment_en.ts
    translations/office_equipment_ru.ts
    translations/office_equipment_es.ts
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${FORMS} ${TS_FILES})

target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::PrintSupport
    Qt6::Charts
)

# Обработка переводов
find_package(Qt6 COMPONENTS LinguistTools REQUIRED)

# Создаем .qm файлы из .ts файлов
qt_add_translations(${PROJECT_NAME} 
    TS_FILES ${TS_FILES}
    QM_FILES_OUTPUT_VARIABLE QM_FILES
)

# Копируем переводы в папку сборки
foreach(qm_file ${QM_FILES})
    get_filename_component(qm_filename ${qm_file} NAME)
    configure_file(${qm_file} ${CMAKE_CURRENT_BINARY_DIR}/${qm_filename} COPYONLY)
endforeach()

# Установка путей для переводов
target_compile_definitions(${PROJECT_NAME} PRIVATE
    TRANSLATIONS_DIR="${CMAKE_CURRENT_BINARY_DIR}"
)
