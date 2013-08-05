set (VirtualTouchScreen_TEST_SCRS unit_tests.cpp ../GestureAlgos.cpp)

find_package (Qt5Test REQUIRED)

add_executable(VtcUnitTests ${VirtualTouchScreen_TEST_SCRS})
target_link_libraries (VtcUnitTests ${PCSDK_LIBS} ${OpenCV_LIBS})

qt5_use_modules(VtcUnitTests Test)