// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include <k4a/k4a.h>
#include <math.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

static void export_xy_table(const k4a_calibration_t* calibration, std::string file_name)
{
    int width = calibration->depth_camera_calibration.resolution_width;
    int height = calibration->depth_camera_calibration.resolution_height;

    k4a_float2_t p;
    k4a_float3_t ray;
    int valid;

    std::ofstream outputFile(file_name + ".lut");
    if (!outputFile.is_open())
    {
        std::cout << "Unable to open file.";
        return;
    }

    for (int y = 0, idx = 0; y < height; y++)
    {
        p.xy.y = (float)y;
        for (int x = 0; x < width; x++, idx++)
        {
            p.xy.x = (float)x;

            k4a_calibration_2d_to_3d(
                calibration, &p, 1.f, K4A_CALIBRATION_TYPE_DEPTH, K4A_CALIBRATION_TYPE_DEPTH, &ray, &valid);

            if (valid)
                outputFile << ray.xyz.x << "," << ray.xyz.y << " ";
            else
                outputFile << "NaN" << "," << "NaN" << " ";
        }
        outputFile << std::endl;
    }

    outputFile.close();
}

int main(int argc, char** argv)
{
    int returnCode = 1;
    k4a_device_t device = NULL;
    const int32_t TIMEOUT_IN_MS = 1000;
    std::string file_name;
    uint32_t device_count = 0;
    k4a_device_configuration_t config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
    k4a_image_t xy_table = NULL;

    if (argc != 2)
    {
        printf("k4a_lut_generator.exe <K4A_DEPTH_MODE>\n");
        returnCode = 2;
        goto Exit;
    }

    file_name = argv[1];

    if (file_name == "WFOV_2X2BINNED")
        config.depth_mode = K4A_DEPTH_MODE_WFOV_2X2BINNED;
    else if (file_name == "NFOV_UNBINNED")
        config.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
    else
    {
        std::cout << "Invalid depth mode argument." << std::endl;
        std::cout << "You must specify \"WFOV_2X2BINNED\" or \"NFOV_UNBINNED\"" << std::endl;
        return 0;
    }

    config.camera_fps = K4A_FRAMES_PER_SECOND_30;

    device_count = k4a_device_get_installed_count();

    if (device_count == 0)
    {
        printf("No K4A devices found\n");
        return 0;
    }

    if (K4A_RESULT_SUCCEEDED != k4a_device_open(K4A_DEVICE_DEFAULT, &device))
    {
        printf("Failed to open device\n");
        goto Exit;
    }

    k4a_calibration_t calibration;
    if (K4A_RESULT_SUCCEEDED !=
        k4a_device_get_calibration(device, config.depth_mode, config.color_resolution, &calibration))
    {
        printf("Failed to get calibration\n");
        goto Exit;
    }

    k4a_image_create(K4A_IMAGE_FORMAT_CUSTOM,
        calibration.depth_camera_calibration.resolution_width,
        calibration.depth_camera_calibration.resolution_height,
        calibration.depth_camera_calibration.resolution_width * (int)sizeof(k4a_float2_t),
        &xy_table);

    export_xy_table(&calibration, file_name);

    k4a_image_release(xy_table);

    returnCode = 0;
Exit:
    if (device != NULL)
    {
        k4a_device_close(device);
    }

    return returnCode;
}
