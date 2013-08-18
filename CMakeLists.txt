cmake_minimum_required (VERSION 2.8.10)

project (VirtualTouchScreen)

set (CMAKE_PREFIX_PATH "D:/Qt/Qt5.1.0/5.1.0/msvc2012")
SET (PCSDK_DIR "D:/Program Files (x86)/Intel/PCSDK")
SET (OpenCV_DIR "D:/opencv/build")

find_package (Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package (OpenCV REQUIRED core video)

include_directories (${CMAKE_SOURCE_DIR}
					 ${QT_QTDECLARATIVE_INCLUDE_DIR} 
					 ${PCSDK_DIR}/sample/common/include
					 ${PCSDK_DIR}/include
					 ${OpenCV_INCLUDE_DIRS})

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

qt5_wrap_ui(UIS_HDRS configuration.ui)

set (VirtualTouchScreen_HDRS VirtualTouchScreen.h GestureThread.h TouchInputEmulator.h ConfigDialog.h)
set (VirtualTouchScreen_SRCS VirtualTouchScreen.cpp GestureThread.cpp GestureAlgos.cpp 
	TouchInputEmulator.cpp ConfigDialog.cpp main.cpp VirtualTouchScreen.rc)

set (VirtualTouchScreen_RSC icons.qrc)
QT5_ADD_RESOURCES (RSCS ${VirtualTouchScreen_RSC})

IF (CMAKE_BUILD_TYPE MATCHES "[Rr]elease")
	SET (PCSDK_LIBS "libpxc.lib;libpxcutils.lib")
ELSE()
	SET (PCSDK_LIBS "libpxc_d.lib;libpxcutils_d.lib")
ENDIF()

LINK_DIRECTORIES(${PCSDK_DIR}/lib/Win32/;${PCSDK_DIR}/sample/common/lib/Win32/v110/)
add_executable (${CMAKE_PROJECT_NAME} WIN32 ${VirtualTouchScreen_SRCS} 
	${RSCS} ${UIS_HDRS})
target_link_libraries (${CMAKE_PROJECT_NAME} Qt5::WinMain ${PCSDK_LIBS} ${OpenCV_LIBS})

qt5_use_modules (${CMAKE_PROJECT_NAME} Widgets)

#add unit tests
ADD_SUBDIRECTORY(tests)

install(TARGETS VirtualTouchScreen RUNTIME DESTINATION .)
install(FILES ${CMAKE_SOURCE_DIR}/icons/VirtualTouchScreen.ico DESTINATION .)
install(FILES ${CMAKE_SOURCE_DIR}/README.md DESTINATION .)
set(OPENCV_DLLS "opencv_video246.dll;opencv_core246.dll;opencv_imgproc246.dll")
foreach(line ${OPENCV_DLLS})
  install(FILES D:/opencv/build/x86/vc11/bin/${line} DESTINATION .)
endforeach()
set(QT5_DLLS "Qt5Widgets.dll;Qt5Gui.dll;Qt5Core.dll;libGLESv2.dll;icuin51.dll;icuuc51.dll;icudt51.dll;libegl.dll")
foreach(line ${QT5_DLLS})
  install(FILES D:/Qt/Qt5.1.0_32/5.1.0/msvc2012/bin/${line} DESTINATION .)
endforeach()
install(FILES D:/Qt/Qt5.1.0_32/5.1.0/msvc2012/plugins/platforms/qwindows.dll DESTINATION platforms)
install(FILES D:/Qt/Qt5.1.0_32/5.1.0/msvc2012/plugins/imageformats/qjpeg.dll DESTINATION imageformats)
SET(MSVC_DLLS "msvcp110.dll;msvcr110.dll")
foreach(line ${MSVC_DLLS})
  install(FILES "D:/Program Files (x86)/Microsoft Visual Studio 11.0/VC/redist/x86/Microsoft.VC110.CRT/${line}" DESTINATION .)
endforeach()
INSTALL (FILES "D:/Users/b/Documents/C++/PerceptivePresenter/DepthSenseSDK-1.2.1.840-win32-VS2010-Release-redist_signed.exe" DESTINATION runtime)
INSTALL (FILES "D:/Users/b/Documents/C++/PerceptivePresenter/intel_pc_sdk_runtime_ia32_8779.msi" DESTINATION runtime)

#setup installers
set(CPACK_PACKAGE_NAME "VirtualTouchScreen")
set(CPACK_PACKAGE_VENDOR "Bogdan Cristea")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "uses gesture camera in order to implmeent a virtual touch screen")
set(CPACK_PACKAGE_VERSION "1.0")
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/Copyright.txt)
set(CPACK_GENERATOR "NSIS")
set(CPACK_NSIS_CONTACT "Bogdan Cristea <cristeab\@gmail.com>")
set(CPACK_NSIS_INSTALLED_ICON_NAME "VirtualTouchScreen.exe")
set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/icons/VirtualTouchScreen.ico")
set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/icons/VirtualTouchScreen.ico")
set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "ExecWait '\\\"$INSTDIR\\\\runtime\\\\DepthSenseSDK-1.2.1.840-win32-VS2010-Release-redist_signed.exe\\\"'
  ExecWait 'msiexec /i \\\"$INSTDIR\\\\runtime\\\\intel_pc_sdk_runtime_ia32_8779.msi\\\"'
  CreateShortCut \\\"$DESKTOP\\\\VirtualTouchScreen.lnk\\\" \\\"$INSTDIR\\\\VirtualTouchScreen.exe\\\" \\\"\\\" \\\"$INSTDIR\\\\VirtualTouchScreen.ico\\\"
  CreateShortCut \\\"$SMPROGRAMS\\\\VirtualTouchScreen.lnk\\\" \\\"$INSTDIR\\\\VirtualTouchScreen.exe\\\" \\\"\\\" \\\"$INSTDIR\\\\VirtualTouchScreen.ico\\\"")
set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "Delete \\\"$DESKTOP\\\\VirtualTouchScreen.lnk\\\"
  Delete \\\"$SMPROGRAMS\\\\VirtualTouchScreen.lnk\\\"")

include(CPack)
