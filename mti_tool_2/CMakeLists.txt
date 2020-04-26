cmake_minimum_required(VERSION 3.16)

project(mti_tool_2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
find_package(Qt5 COMPONENTS Core Widgets Sql Network Gui LinguistTools REQUIRED)

add_definitions(-DUNICODE -D_UNICODE)

set(qt_path "C:/Qt/Qt5.12.4/5.12.4/msvc2017/include")
include_directories(${qt_path}/QtNetwork ${qt_path}/QtSql ${qt_path}/QtWidgets)

#set(province_files CpdfFileWatcher_hebei.cpp CpdfFileWatcher_shanxi.cpp FileSystemWatcher_hebei.cpp FileSystemWatcher_shanxi.cpp)  
file(GLOB province_files ../Province_Files/*.cpp ./*.cpp)
#message("province_files = ${province_files}")
source_group("Provice Files" FILES ${province_files})

#set(head_files CFtpClient.h CLock.h CLog.h Config.h CPdfFileWatcher.h CSetDlg.h FileSystemWatcher.h mti_tool_2.h resource.h)
file(GLOB head_files ./*.h)
source_group("Header Files" ${head_files})

#set(sour_files CFtpClient.cpp CLock.cpp CLog.cpp Config.cpp CPdfFileWatcher.cpp CSetDlg.cpp FileSystemWatcher.cpp main.cpp mti_tool_2.cpp)
file(GLOB sour_files ./*.cpp)
source_group("Source Files" ${sour_files})

#set(form_files mti_tool_2.ui CSetDlg.ui)
file(GLOB form_files ./*.ui)
source_group("Form Files" FILES ${form_files})

#set(resource_files mti_tool_2.qrc)
file(GLOB resource_files ./*.qrc)
source_group("Resource Files" ${resource_files})

set(src_list
 ${province_files}
 ${head_files}
 ${sour_files}
 ${form_files}
 ${resource_files}

  ./mti_tool_2.rc
  ./Resources/images/bitbug_favicon.ico
)


#设置输出文件存放路径
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/bin) 
add_executable(${PROJECT_NAME} ${src_list})
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Widgets Qt5::Sql Qt5::Network Qt5::Gui)
set_Target_Properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")

add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD 
 COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/Resources/images/star.png  ${PROJECT_SOURCE_DIR}/build/Resources/images/star.png
 COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/Resources/images/bitbug_favicon.ico  ${PROJECT_SOURCE_DIR}/build/Resources/images/bitbug_favicon.ico
)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD 
 COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/Config.ini  $<TARGET_FILE_DIR:${PROJECT_NAME}>
)