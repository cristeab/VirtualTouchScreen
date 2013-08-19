                         Virtual Touch Screen


                        author: Bogdan Cristea
                        e-mail: cristeab@gmail.com

  This application aims at improving user experience by providing a touch screen
experience with gestures. Currently touch screen enabled-applications have become
increasingly popular. However, classical PCs cannot benefit from this new type of
input. This application tries to fill this gap by providing touch screen experience
on any type of PC/device using a Creative Interactive Gesture Camera and perceptual
computing.
   The virtual touch screen is represented by the sphere centered on the camera
and of radius equal with a virtual touch screen threshold. Once the fingers are
on the sphere or inside a touch event is generated. Since both the thumb and the
index fingers are tracked, in theory, all eight gestures supported in Windows 8
are available:"
 - Press and Hold
 - Tap
 - Slide
 - Swipe
 - Pinch
 - Stretch
 - Swipe from Edge
 - Turn
However, in practice, with a single camera, the distance estimation has fluctuations
that might have a negative impact on virtual touch screen accuracy.
   The virtual touch screen threshold is configurable through the configuration
dialog. Finger icon (*.jpg and *.png image formats are supported) and
its size can also be changed. Both the thumb and the index fingers use the same
icon, but the thumb finger uses the icon rotated with 90 degrees counterclock wise.
If needed, the thumb finger icon can be hidden while the touch events from the thumb
are still received. The coordinates of the upper-left corner of the area covered
by the fingers can be changed, also the scaling factor used to make sure that
the entire screen area is covered.
   The following shortcut keys are available:
- F1: shows this help
- F2: shows the configuration dialog
- Ctrl+q: quits the application
In order to receive the shortcut keys the application needs to have
the focus. In order to have the focus either click on
the application icon on the main toolbar or click the pointer.


Installation and Utilisation

   For installation use the provided installer. Application executable and required libraries are installed
in the chosen location. Also, the drivers needed by Creative Interactive Perceptual Camera are installed 
and Intel Perceptual Computing (PC) SDK 2013. An uninstaller is also provided for the application, the 
camera drivers and PC SDK need to be uninstalled separately.
   The application has been tested on Windows 8 64. Please note that this application is a Windows 8 Desktop 
application. It is recommended not to use this application on a heavily loaded computer, that might slow
down the rate at which image frames are received from the camera.