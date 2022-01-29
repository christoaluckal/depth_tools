// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.
// Compilation Need OpenGL g++ record.cpp imgui/*.cpp -lrealsense2 -lglfw -lGL -lGLU -lGLEW -lglut

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include "example.hpp"          // Include short list of convenience functions for rendering
#include <chrono>


// Includes for time display
#include <sstream>
#include <iostream>
#include <iomanip>

int width = 640;
int height = 480;

// Initial definitions, will be overwitten in function
double time_run = 10;
auto start = std::chrono::steady_clock::now();

int main(int argc, char * argv[]) try
{
    // Time in seconds
    time_run=atof(argv[1]);
    std::string bag_name = argv[2];

    // Declare frameset and frames which will hold the data from the camera
    rs2::frameset frames;
    rs2::frame depth;

    // Declare depth colorizer for pretty visualization of depth data
    rs2::colorizer color_map;

    // Create a shared pointer to a pipeline
    auto pipe = std::make_shared<rs2::pipeline>();

    // Start streaming with default configuration
    pipe->start();

    pipe = std::make_shared<rs2::pipeline>();
    rs2::config cfg; // Declare a new configuration
    cfg.enable_stream(RS2_STREAM_DEPTH, width, height, RS2_FORMAT_Z16, 30);
    cfg.enable_record_to_file(bag_name);
    pipe->start(cfg); //File will be opened at this point

    // Initialize a shared pointer to a device with the current device on the pipeline
    rs2::device device = pipe->get_active_profile().get_device();

    // While application is running
    start = std::chrono::steady_clock::now();
    std::cout << "Recording....\n";
    while(1) {
        frames = pipe->wait_for_frames(); // wait for next set of frames from the camera
        if (device.as<rs2::recorder>())
        {
            // Calculate time between start and now
            std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start);

            if (time_span.count()>time_run)
            {
                pipe->stop(); // Stop the pipeline that holds the file and the recorder
                pipe = std::make_shared<rs2::pipeline>(); //Reset the shared pointer with a new pipeline
                pipe->start(); // Resume streaming with default configuration
                device = pipe->get_active_profile().get_device();
                break;
            }
        }
    }
    // std::cout << "17" << '\n';
    return EXIT_SUCCESS;
}
catch (const rs2::error & e)
{
    std::cout << "RealSense error calling " << e.get_failed_function() << "(" << e.get_failed_args() << "):\n    " << e.what() << std::endl;
    return EXIT_FAILURE;
}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
}
