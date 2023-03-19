#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <opencv2/opencv.hpp>
#include <vpi/OpenCVInterop.hpp>
#include <vpi/VPI.h>

int main(int argc, char** argv)
{
    // Remove previous keyframes
    system("rm ./keyframes/*");

    // Load video file
    cv::VideoCapture cap("input_home1.mp4");
    if (!cap.isOpened()) {
        printf("Error: Failed to open video file\n");
        return -1;
    }

    // Create VPI context
    VPIImage vpiFrame1 = NULL;
    VPIImage vpiFrame2 = NULL;
    VPIStream vpiStream = NULL;
    if (VPI_SUCCESS != vpiStreamCreate(&vpiStream)) {
        printf("Error: Failed to create VPI stream\n");
        return -1;
    }

    // Extract keyframes
    std::vector<cv::Mat> keyframes;
    int num_keyframes = 200;
    int keytry_cnt = 0;
    while (keyframes.size() < num_keyframes) {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) {
            break;
        }

        // Convert to YUV420 format for VPI processing
        VPIImageFormat vpiFormat = {
            .color = VPI_COLOR_FORMAT_YUV420,
            .width = frame.cols,
            .height = frame.rows,
            .num_planes = 3,
            .bit_depth = 8,
            .data_type = VPI_DATA_TYPE_UNSIGNED_BYTE,
            .flags = VPI_IMAGE_FLIPPED
        };
        if (VPI_SUCCESS != vpiImageCreateOpenCVMatWrapper(&frame, 0, &vpiFormat, &vpiFrame1)) {
            printf("Error: Failed to create VPI image wrapper\n");
            break;
        }

        // Compute frame difference
        if (!keyframes.empty()) {
            if (VPI_SUCCESS != vpiImageAbsDiff(vpiFrame1, vpiFrame2, vpiStream)) {
                printf("Error: Failed to compute absolute difference\n");
                break;
            }
            float mean_diff;
            if (VPI_SUCCESS != vpiImageMeanStdDev(vpiFrame2, NULL, &mean_diff, NULL, vpiStream)) {
                printf("Error: Failed to compute mean difference\n");
                break;
            }
            if (mean_diff < 40) {
                continue;
            }
        }

        // Compute focus measure using Laplacian variance
        float fm;
        if (VPI_SUCCESS != vpiImageLaplacianVar(vpiFrame1, &fm, vpiStream)) {
            printf("Error: Failed to compute Laplacian variance\n");
            break;
        }
        if (fm < 300) {
            continue;
        }

        // Add frame as keyframe
        cv::Mat keyframe;
        if (VPI_SUCCESS != vpiImageCreateOpenCVWrapper(&keyframe, 0, &vpiFormat, vpiFrame1, vpiStream)) {
            printf("Error: Failed to create OpenCV image wrapper\n");
            break;
        }
        keyframes.push_back(keyframe.clone());

        // Save keyframe as JPEG file
        char filename[256];
        sprintf(filename, "keyframes/keyframe%d
