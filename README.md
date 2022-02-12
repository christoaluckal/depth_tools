# depth_tools

## Make the bag using the C++ code
**Make sure you have OpenGL and RealSense and are in the `cpp` folder**
### GUI CODE
1.  `g++ record.cpp imgui/*.cpp -lrealsense2 -lglfw -lGL -lGLU -lGLEW -lglut`
2.  Execute with: `<exe> <time in seconds> <name of bag with .bag extension>` eg. `./a.out 10 test.bag`
### NON-GUI CODE <br>
1.  `g++ depth_nogui.cpp -lrealsense2`
2.  Execute with: `<exe> <time in seconds> <name of bag with .bag extension>` eg. `./a.out 10 test.bag`

## To Check total number of frames
1. `python3 readbag.py --input=<bag file> --frame=<framerate in multiples of 15>`

