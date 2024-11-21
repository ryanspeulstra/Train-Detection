This brief C++ program provides the ability to parse a livestream for significant changes. This was originally intended to parse railcams in order to detect when trains pass by the camera, but it may have other usecases.

It compares frame by frame of the livestream for changed pixels, and displays within certain intervals the program's best guess as to what is happening

When a certain threshold is reached, it captures and saves the frame. 

Compilation:
``` g++ -o traindetect train_detect.cpp 'pkg-config --cflags --libs opencv4' ```

``` g++ -o traincrop traincrop.cpp 'pkg-config --cflags --libs opencv4' ``` BETA

``` g++ -o test test.cpp 'pkg-config --cflags --libs opencv4' ``` ALPHA


Running:
``` ./traindetect [URL] ```

``` ./test(traincrop) [URL] -section [SECTION] -threshold [THRESHOLD] ```

URL - The url of the livestream that you would like to use.

OPTIONAL TEST FLAGS
SECTION - The section of the livestream that you wish to analyze, split into 16 pieces, with 1 as the top left. test.cpp will analyze whatever is in the frame, traincrop.cpp will analyze the whole stream but only display the frame.

THRESHOLD - The amount of pixel changes that fires off a capture command for the frame.

Features:
> Detection of Pixel changes, 
> Prints frame when threshold reached

Future Features:
> Custom Detection Thresholds,
> Custom Frame (only capturing a section of the stream),
> External output,
> Proper Error Handling


