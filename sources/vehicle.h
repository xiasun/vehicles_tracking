#ifndef VEHICLE_H
#define VEHICLE_H

#include <vector>
#include <opencv2/opencv.hpp>
#include "box2.h"

class Vehicle {
public:
    explicit Vehicle();
    explicit Vehicle(box firstBox);
    ~Vehicle();

    std::string brand;
    std::string size;
    int numFrames; // total number of frames, to calculate speed
    std::vector<box> boxes; // boxes for each frame
    std::vector<cv::Mat> screenShots;
    bool out; // vehicle already out of boundary
};

#endif // VEHICLE_H
