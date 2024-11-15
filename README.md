This brief C++ program provides the ability to parse a livestream for significant changes. This was originally intended to parse railcams in order to detect when trains pass by the camera, but it may have other usecases.

It compares frame by frame of the livestream for changed pixels, and displays within certain intervals the program's best guess as to what is happening

When a certain threshold is reached, it captures and saves the frame. 

Compilation:
``` g++ -o traindetect train_detect.cpp 'pkg-config --cflags --libs opencv4' ```

Running:
``` train_detect.cpp [URL] ```

Features:
> Detection of Pixel changes, 
> Prints frame when threshold reached

Future Features:
> Custom Detection Thresholds,
> Custom Frame (only capturing a section of the stream),
> External output,
> Proper Error Handling


