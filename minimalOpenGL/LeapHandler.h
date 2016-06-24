#pragma once
#include "Leap.h"

/* Gets the palm velocity of right most hand */
Leap::Vector getPalmVelocity(const Leap::Frame& frame);

/* Returns -1 for left swipe, 1 for right swipe and 0 otherwise */
int getSwipeGesture(const Leap::Frame& frame);

/* Check if hand is closed */
bool isHandClosed(const Leap::Frame& frame);