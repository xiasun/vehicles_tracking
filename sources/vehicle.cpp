#include "vehicle.h"

Vehicle::Vehicle() {
    numFrames = 0;
    out = false;
}

Vehicle::Vehicle(box firstBox) {
    boxes.push_back(firstBox);
    numFrames = 1; // at least one frame
    out = false;
}

Vehicle::~Vehicle() {
    delete this;
}
