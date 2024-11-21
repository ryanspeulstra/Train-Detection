#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cstdio>
#include <chrono>
#include <iomanip>
#include <sstream>

// Function to parse command-line arguments
bool parseArguments(int argc, char* argv[], std::string& url, int& section, int& threshold) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <YouTube URL> -section <SECTION> -threshold <THRESHOLD>" << std::endl;
        return false;
    }

    url = argv[1];
    section = 1;      // Default to top-left section
    threshold = 20000; // Default threshold

    for (int i = 2; i < argc; i++) {
        if (std::string(argv[i]) == "-section" && i + 1 < argc) {
            section = std::stoi(argv[i + 1]);
            i++;
        } else if (std::string(argv[i]) == "-threshold" && i + 1 < argc) {
            threshold = std::stoi(argv[i + 1]);
            i++;
        }
    }

    return true;
}

int main(int argc, char* argv[]) {
    std::string url;
    int section, threshold;
    if (!parseArguments(argc, argv, url, section, threshold)) {
        return -1;
    }

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

    // Calculate cropping boundaries for the selected section
    int cols = 4; // Number of columns in the grid
    int rows = 4; // Number of rows in the grid
    int sectionWidth = frameWidth / cols;
    int sectionHeight = frameHeight / rows;

    int sectionX = ((section - 1) % cols) * sectionWidth;
    int sectionY = ((section - 1) / cols) * sectionHeight;

    std::cout << "Processing section: " << section
              << " (x: " << sectionX << ", y: " << sectionY
              << ", width: " << sectionWidth << ", height: " << sectionHeight << ")" << std::endl;

    while (true) {
        size_t bytesRead = fread(buffer.data(), 1, frameSize, pipe);
        if (bytesRead != frameSize) {
            std::cerr << "Error reading frame data: expected " << frameSize << " bytes, but got " << bytesRead << std::endl;
            break;
        }

        // Create current frame from the raw buffer data
        currentFrame = cv::Mat(frameHeight, frameWidth, CV_8UC3, buffer.data());

        if (currentFrame.empty()) {
            std::cerr << "Empty frame captured!" << std::endl;
            break;
        }

        // Crop the frame to the selected section
        cv::Rect cropRegion(sectionX, sectionY, sectionWidth, sectionHeight);
        cv::Mat croppedFrame = currentFrame(cropRegion);

        // Convert cropped frame to grayscale
        cv::cvtColor(croppedFrame, currentGray, cv::COLOR_BGR2GRAY);

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

            // Detection logic
            if (nonZeroPixels > 1000 && nonZeroPixels < 3000) {
                std::cout << "Detected Change: Car Likely" << std::endl;
            } else if (nonZeroPixels > 3000 && nonZeroPixels < 15000) {
                std::cout << "Detected Change: Multiple Cars Likely" << std::endl;
            } else if (nonZeroPixels > threshold) {
                std::cout << "Detected Change, Train Likely - Saving Frame" << std::endl;

                // Save the cropped frame as an image
                std::ostringstream filename;
                filename << "frames/Detected_Frame_" << std::put_time(&localTime, "%Y%m%d_%H%M%S") << ".png";
                cv::imwrite(filename.str(), croppedFrame);
            }
        }

        // Store the current cropped frame for the next iteration
        currentGray.copyTo(previousGray);

        // Display the cropped section
        cv::imshow("Selected Section", croppedFrame);

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
