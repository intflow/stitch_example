#include <opencv2/opencv.hpp>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <unistd.h>

using namespace cv;
using namespace std;

int main()
{
    // Remove previous keyframes
    system("rm ./keyframes/*");

    // Load video file
    VideoCapture cap("input_home1.mp4");

    // Extract keyframes
    int num_keyframes = 200;
    vector<Mat> keyframes;
    int keytry_cnt = 0;
    while (keyframes.size() < num_keyframes)
    {
        Mat frame;
        cap >> frame;
        if (frame.empty())
        {
            break;
        }
        // Compute frame difference
        double mean_diff;
        if (!keyframes.empty())
        {
            Mat diff;
            absdiff(frame, keyframes.back(), diff);
            Scalar mean_val = mean(diff);
            mean_diff = mean_val[0] + mean_val[1] + mean_val[2];
            mean_diff /= 3.0;
        }
        else
        {
            mean_diff = 0.0;
        }
        // Compute focus measure using Laplacian variance
        Mat gray_frame;
        cvtColor(frame, gray_frame, COLOR_BGR2GRAY);
        Mat lap_frame;
        Laplacian(gray_frame, lap_frame, CV_64F);
        Scalar lap_var = mean(lap_frame.mul(lap_frame));
        double fm = lap_var[0];

        // Add frame as keyframe if it's visually different enough from previous keyframes and has sufficient focus
        if (keyframes.empty() || (mean_diff > 40.0 && fm > 300.0))
        {
            keyframes.push_back(frame);
            imwrite("keyframes/keyframe" + to_string(keyframes.size() - 1) + ".jpg", frame);
            cout << "Keyframe " << keyframes.size() - 1 << " saved as keyframe" << keyframes.size() - 1 << ".jpg" << endl;
        }
    }

    // Stitch keyframes
    Stitcher::Mode mode = Stitcher::PANORAMA;
    Ptr<Stitcher> stitcher = Stitcher::create(mode);
    Stitcher::Status status;
    Mat stitched;
    status = stitcher->stitch(keyframes, stitched);

    // Save result as JPEG file
    if (status == Stitcher::OK)
    {
        imwrite("output.jpg", stitched);
        cout << "Stitched panorama saved as output.jpg" << endl;
    }
    else
    {
        cout << "Error stitching images: " << status << endl;
    }

    // Print execution times
    double extraction_time, save_time, stitching_time;
    clock_t start_time = clock();
    extraction_time = (double)(clock() - start_time) / CLOCKS_PER_SEC;
    start_time = clock();
    save_time = (double)(clock() - start_time) / CLOCKS_PER_SEC;
    start_time = clock();
    stitching_time = (double)(clock() - start_time) / CLOCKS_PER_SEC;
    cout << "Keyframe extraction time: " << extraction_time << " seconds" << endl;
    cout << "Saving keyframes time: " << save_time << " seconds" << endl;
    cout << "Stitching time: " << stitching_time << " seconds" << endl;
    cout << "Saving stitched panorama time: " << save_time << " seconds" << endl;

    // Release resources
    cap.release();

    return
}
