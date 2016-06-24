#include "LeapHandler.h"

Leap::Vector getPalmVelocity(const Leap::Frame& frame) {
	Leap::Hand hand = frame.hands().rightmost();
	Leap::PointableList pointables = hand.pointables().extended();

	return hand.palmVelocity();
}

int getSwipeGesture(const Leap::Frame& frame) {
	Leap::GestureList gl = frame.gestures();
	for (Leap::Gesture gesture : gl) {
		if (gesture.type() == Leap::SwipeGesture::classType()) {
			Leap::SwipeGesture swipe = Leap::SwipeGesture(gesture);
			if (swipe.direction().x > 0) {
				std::cout << "Left swipe" << std::endl;
				return -1;
			}
			else {
				std::cout << "Right swipe" << std::endl;
				return 1;
			}
		}
	}
	return 0;
}

bool isHandClosed(const Leap::Frame& frame) {
	Leap::Hand hand = frame.hands().rightmost();
	Leap::PointableList pointables = hand.pointables().extended();
	return pointables.isEmpty() && !frame.hands().isEmpty();
}