#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cstdio>
#include <chrono>
#include <iomanip>
#include <sstream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <YouTube URL>" << std::endl;
        return -1;
    }

    std::string url = argv[1];
    std::string command =
        "yt-dlp -o - " + url +
        " | ffmpeg -loglevel quiet -i pipe:0 -f rawvideo -pix_fmt bgr24 pipe:1";

    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        std::cerr << "Failed to open video stream." << std::endl;
        return -1;
    }

    int frameWidth = 1920;   // Adjust according to the livestream resolution
    int frameHeight = 1080;  // Adjust according to the livestream resolution
    int frameSize = frameWidth * frameHeight * 3;

    cv::Mat previousGray, currentFrame, currentGray, frameDifference;
    std::vector<unsigned char> buffer(frameSize);

    int frameCounter = 0;        // Counter to skip every other frame

    while (true) {
        size_t bytesRead = fread(buffer.data(), 1, frameSize, pipe);
        if (bytesRead != frameSize) {
            std::cerr << "Error reading frame data: expected " << frameSize << " bytes, but got " << bytesRead << std::endl;
            break;
        }

        frameCounter++;
        if (frameCounter % 2 != 0) {
            continue;  // Skip every other frame
        }

        // Create current frame from the raw buffer data
        currentFrame = cv::Mat(frameHeight, frameWidth, CV_8UC3, buffer.data());

        if (currentFrame.empty()) {
            std::cerr << "Empty frame captured!" << std::endl;
            break;
        }

        // Resize the frame to a smaller size for output
        int outputWidth = 640;   // New width for the resized output stream
        int outputHeight = 360;  // New height for the resized output stream
        cv::Mat resizedFrame;
        cv::resize(currentFrame, resizedFrame, cv::Size(outputWidth, outputHeight));

        // Display the resized frame
        cv::imshow("Livestream (Resized)", resizedFrame);

        // Convert current frame to grayscale
        cv::cvtColor(currentFrame, currentGray, cv::COLOR_BGR2GRAY);

        if (!previousGray.empty()) {
            // Calculate absolute difference between the current and previous grayscale frame
            cv::absdiff(previousGray, currentGray, frameDifference);

            // Threshold the difference to detect significant changes
            cv::threshold(frameDifference, frameDifference, 50, 255, cv::THRESH_BINARY);

            // Count non-zero pixels (indicating changes)
            int nonZeroPixels = cv::countNonZero(frameDifference);

            // Get the current time as a timestamp
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            auto localTime = *std::localtime(&time);

            std::cerr << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S")
                      << " - Non-zero pixels: " << nonZeroPixels << std::endl;

            if (nonZeroPixels > 1000 && nonZeroPixels < 3000) {
                std::cout << "Detected Change: Car Likely" << std::endl;
            } else if (nonZeroPixels > 3000 && nonZeroPixels < 15000) {
                std::cout << "Detected Change: Multiple Cars Likely" << std::endl;
            } else if (nonZeroPixels > 20000) {  // Higher threshold for significant change
                std::cout << "Detected Change, Train Likely - Saving Frame" << std::endl;

                // Save the frame as an image
                std::ostringstream filename;
                filename << "frames" << "/Detected_Frame_" << std::put_time(&localTime, "%Y%m%d_%H%M%S") << ".png";
                cv::imwrite(filename.str(), currentFrame);

                }
        }

        // Store the current frame for the next iteration
        currentGray.copyTo(previousGray);

        // Exit if the window is closed
        if (cv::waitKey(1) == 'q') {
            std::cout << "Exiting due to user request (Press 'q' to quit)." << std::endl;
            break;
        }
    }

    pclose(pipe);
    cv::destroyAllWindows();  // Close any open OpenCV windows
    return 0;
}
