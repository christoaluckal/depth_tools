// License: Apache 2.0. See LICENSE file in root directory.
// Copyright(c) 2017 Intel Corporation. All Rights Reserved.
// Compilation Need OpenGL g++ record.cpp imgui/*.cpp -lrealsense2 -lglfw -lGL -lGLU -lGLEW -lglut

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include "example.hpp"          // Include short list of convenience functions for rendering
#include <chrono>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"

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
    // Create a simple OpenGL window for rendering:
    window app(width, height, "RealSense Record and Playback Example");
    ImGui_ImplGlfw_Init(app, false);

    // Create booleans to control GUI (recorded - allow play button, recording - show 'recording to file' text)
    bool recorded = false;
    bool recording = false;

    // Declare a texture for the depth image on the GPU
    texture depth_image;

    // Declare frameset and frames which will hold the data from the camera
    rs2::frameset frames;
    rs2::frame depth;

    // Declare depth colorizer for pretty visualization of depth data
    rs2::colorizer color_map;

    // Create a shared pointer to a pipeline
    auto pipe = std::make_shared<rs2::pipeline>();

    // Start streaming with default configuration
    pipe->start();

    // Initialize a shared pointer to a device with the current device on the pipeline
    rs2::device device = pipe->get_active_profile().get_device();

    // Create a variable to control the seek bar
    int seek_pos;

    // While application is running

    while(app) {
        // std::cout << "It took me " << time_span.count() << " seconds.\n";
        // Flags for displaying ImGui window
        static const int flags = ImGuiWindowFlags_NoCollapse
            | ImGuiWindowFlags_NoScrollbar
            | ImGuiWindowFlags_NoSavedSettings
            | ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoMove;

        ImGui_ImplGlfw_NewFrame(1);
        ImGui::SetNextWindowSize({ app.width(), app.height() });
        ImGui::Begin("app", nullptr, flags);

        // If the device is sreaming live and not from a file
        if (!device.as<rs2::playback>())
        {
            frames = pipe->wait_for_frames(); // wait for next set of frames from the camera
            depth = color_map.process(frames.get_depth_frame()); // Find and colorize the depth data
        }

        // Set options for the ImGui buttons
        ImGui::PushStyleColor(ImGuiCol_TextSelectedBg, { 1, 1, 1, 1 });
        ImGui::PushStyleColor(ImGuiCol_Button, { 36 / 255.f, 44 / 255.f, 51 / 255.f, 1 });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 40 / 255.f, 170 / 255.f, 90 / 255.f, 1 });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 36 / 255.f, 44 / 255.f, 51 / 255.f, 1 });
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12);

        if (!device.as<rs2::playback>()) // Disable recording while device is playing
        {
            // std::cout << "1" << '\n';
            ImGui::SetCursorPos({ app.width() / 2 - 100, 3 * app.height() / 5 + 90});
            ImGui::Text("Click 'record' to start recording");
            ImGui::SetCursorPos({ app.width() / 2 - 100, 3 * app.height() / 5 + 110 });
            if (ImGui::Button("record", { 50, 50 }))
            {

                // std::cout << "2" << '\n';
                start = std::chrono::steady_clock::now();
                // If it is the start of a new recording (device is not a recorder yet)
                if (!device.as<rs2::recorder>())
                {
                    // std::cout << "3" << '\n';
                    pipe->stop(); // Stop the pipeline with the default configuration
                    pipe = std::make_shared<rs2::pipeline>();
                    rs2::config cfg; // Declare a new configuration
                    cfg.enable_stream(RS2_STREAM_DEPTH, width, height, RS2_FORMAT_Z16, 30);
                    cfg.enable_record_to_file(bag_name);
                    pipe->start(cfg); //File will be opened at this point
                    device = pipe->get_active_profile().get_device();
                }
                else
                { // If the recording is resumed after a pause, there's no need to reset the shared pointer
                // std::cout << "4" << '\n';
                    device.as<rs2::recorder>().resume(); // rs2::recorder allows access to 'resume' function
                }
                recording = true;
            }

            /*
            When pausing, device still holds the file.
            */
            if (device.as<rs2::recorder>())
            {
                // Calculate time between start and now
                std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start);
                // std::cout << "5" << '\n';
                if (recording)
                {
                    std::string output = "Recording to file "+bag_name;
                    const char *c = output.c_str();
                    // std::cout << "6" << '\n';
                    ImGui::SetCursorPos({ app.width() / 2 - 100, 3 * app.height() / 5 + 60 });
                    ImGui::TextColored({ 255 / 255.f, 64 / 255.f, 54 / 255.f, 1 }, c);
                }

                // Pause the playback if button is clicked
                ImGui::SetCursorPos({ app.width() / 2, 3 * app.height() / 5 + 110 });
                // if (ImGui::Button("pause\nrecord", { 50, 50 }))
                // {
                //     std::cout << "7" << '\n';
                //     device.as<rs2::recorder>().pause();
                //     recording = false;
                // }

                ImGui::SetCursorPos({ app.width() / 2 + 100, 3 * app.height() / 5 + 110 });
                if (time_span.count()>time_run)
                {
                    // Recording for longer than specified time. Save bag
                    // std::cout << "8" << '\n';
                    pipe->stop(); // Stop the pipeline that holds the file and the recorder
                    pipe = std::make_shared<rs2::pipeline>(); //Reset the shared pointer with a new pipeline
                    pipe->start(); // Resume streaming with default configuration
                    device = pipe->get_active_profile().get_device();
                    recorded = true; // Now we can run the file
                    recording = false;
                    break;
                }
            }
        }
        ImGui::PopStyleColor(4);
        ImGui::PopStyleVar();

        ImGui::End();
        ImGui::Render();

        // Render depth frames from the default configuration, the recorder or the playback
        depth_image.render(depth, { app.width() * 0.25f, app.height() * 0.25f, app.width() * 0.5f, app.height() * 0.75f  });
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
