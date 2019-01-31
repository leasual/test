
#ifndef __LANDMARK_DETECTOR_FUNC_h_
#define __LANDMARK_DETECTOR_FUNC_h_

// OpenCV includes
#include <opencv2/core/core.hpp>

#include <LandmarkDetectorParameters.h>
#include <LandmarkDetectorUtils.h>
#include <LandmarkDetectorModel.h>

using namespace std;

namespace LandmarkDetector
{


	//================================================================
	// Helper function for getting head pose from CLNF parameters

	// Return the current estimate of the head pose in world coordinates with camera at origin (0,0,0)
	// The format returned is [Tx, Ty, Tz, Eul_x, Eul_y, Eul_z]
	cv::Vec6f GetPose(const CLNF& clnf_model, float fx, float fy, float cx, float cy);

	// Return the current estimate of the head pose in world coordinates with camera at origin (0,0,0), but with rotation representing if the head is looking at the camera
	// The format returned is [Tx, Ty, Tz, Eul_x, Eul_y, Eul_z]
	cv::Vec6f GetPoseWRTCamera(const CLNF& clnf_model, float fx, float fy, float cx, float cy);
}
#endif
