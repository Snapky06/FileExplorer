include("D:/proyectos qt/FileExplorer/build/Desktop_Qt_6_9_3_MinGW_64_bit-Debug/.qt/QtDeploySupport.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/FileExplorer-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase")

qt6_deploy_runtime_dependencies(
    EXECUTABLE "D:/proyectos qt/FileExplorer/build/Desktop_Qt_6_9_3_MinGW_64_bit-Debug/FileExplorer.exe"
    GENERATE_QT_CONF
)
