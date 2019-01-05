

#include "stdafx.h"

#include "LandmarkDetectorFunc.h"
#include "RotationHelpers.h"
#include "ImageManipulationHelpers.h"

// OpenCV includes
#include <opencv2/core/core.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>

// System includes
#include <vector>
#include <numeric>

using namespace LandmarkDetector;

// Getting a head pose estimate from the currently detected landmarks, with appropriate correction due to the PDM assuming an orthographic camera
// which is only correct close to the centre of the image
// This method returns a corrected pose estimate with respect to world coordinates with camera at origin (0,0,0)
// The format returned is [Tx, Ty, Tz, Eul_x, Eul_y, Eul_z]
cv::Vec6f LandmarkDetector::GetPose(const CLNF& clnf_model, float fx, float fy, float cx, float cy)
{
	if (!clnf_model.detected_landmarks.empty() && clnf_model.params_global[0] != 0)
	{
		// This is used as an initial estimate for the iterative PnP algorithm
		float Z = fx / clnf_model.params_global[0];

		float X = ((clnf_model.params_global[4] - cx) * (1.0 / fx)) * Z;
		float Y = ((clnf_model.params_global[5] - cy) * (1.0 / fy)) * Z;

		// Correction for orientation

		// 2D points
		cv::Mat_<float> landmarks_2D = clnf_model.detected_landmarks;

		landmarks_2D = landmarks_2D.reshape(1, 2).t();

		// 3D points
		cv::Mat_<float> landmarks_3D;
		clnf_model.pdm.CalcShape3D(landmarks_3D, clnf_model.params_local);
		
		landmarks_3D = landmarks_3D.reshape(1, 3).t();

		// Solving the PNP model

		// The camera matrix
		cv::Matx33f camera_matrix(fx, 0, cx, 0, fy, cy, 0, 0, 1);

		cv::Vec3f vec_trans(X, Y, Z);
		cv::Vec3f vec_rot(clnf_model.params_global[1], clnf_model.params_global[2], clnf_model.params_global[3]);

		cv::solvePnP(landmarks_3D,
					 landmarks_2D, camera_matrix,
					 cv::Mat(), vec_rot, vec_trans, true);

		cv::Vec3f euler = Utilities::AxisAngle2Euler(vec_rot);


		return cv::Vec6f(vec_trans[0], vec_trans[1],
						 vec_trans[2], euler[0], euler[1], euler[2]);
	}
	else
	{
		return cv::Vec6f(0, 0, 0, 0, 0, 0);
	}
}

// Getting a head pose estimate from the currently detected landmarks, with appropriate correction due to perspective projection
// This method returns a corrected pose estimate with respect to a point camera (NOTE not the world coordinates), which is useful to find out if the person is looking at a camera
// The format returned is [Tx, Ty, Tz, Eul_x, Eul_y, Eul_z]
cv::Vec6f LandmarkDetector::GetPoseWRTCamera(const CLNF& clnf_model, float fx, float fy, float cx, float cy)
{
	if (!clnf_model.detected_landmarks.empty() && clnf_model.params_global[0] != 0)
	{

		float Z = fx / clnf_model.params_global[0];

		float X = ((clnf_model.params_global[4] - cx) * (1.0 / fx)) * Z;
		float Y = ((clnf_model.params_global[5] - cy) * (1.0 / fy)) * Z;

		// Correction for orientation

		// 3D points
		cv::Mat_<float> landmarks_3D;
		clnf_model.pdm.CalcShape3D(landmarks_3D, clnf_model.params_local);

		landmarks_3D = landmarks_3D.reshape(1, 3).t();

		// 2D points
		cv::Mat_<float> landmarks_2D = clnf_model.detected_landmarks;

		landmarks_2D = landmarks_2D.reshape(1, 2).t();

		// Solving the PNP model

		// The camera matrix
		cv::Matx33f camera_matrix(fx, 0, cx, 0, fy, cy, 0, 0, 1);

		cv::Vec3f vec_trans(X, Y, Z);
		cv::Vec3f vec_rot(clnf_model.params_global[1], clnf_model.params_global[2], clnf_model.params_global[3]);

		cv::solvePnP(landmarks_3D, landmarks_2D, camera_matrix, cv::Mat(), vec_rot, vec_trans, true);

		// Here we correct for the camera orientation, for this need to determine the angle the camera makes with the head pose
		float z_x = cv::sqrt(vec_trans[0] * vec_trans[0] + vec_trans[2] * vec_trans[2]);
		float eul_x = atan2(vec_trans[1], z_x);

		float z_y = cv::sqrt(vec_trans[1] * vec_trans[1] + vec_trans[2] * vec_trans[2]);
		float eul_y = -atan2(vec_trans[0], z_y);

		cv::Matx33f camera_rotation = Utilities::Euler2RotationMatrix(cv::Vec3f(eul_x, eul_y, 0));
		cv::Matx33f head_rotation = Utilities::AxisAngle2RotationMatrix(vec_rot);

		cv::Matx33f corrected_rotation = camera_rotation * head_rotation;

		cv::Vec3f euler_corrected = Utilities::RotationMatrix2Euler(corrected_rotation);

		return cv::Vec6f(vec_trans[0], vec_trans[1], vec_trans[2], euler_corrected[0], euler_corrected[1], euler_corrected[2]);
	}
	else
	{
		return cv::Vec6f(0, 0, 0, 0, 0, 0);
	}
}


// This method uses basic template matching in order to allow for better tracking of fast moving faces
void CorrectGlobalParametersVideo(const cv::Mat_<uchar> &grayscale_image, CLNF& clnf_model, const FaceModelParameters& params)
{
	cv::Rect_<float> init_box;
	clnf_model.pdm.CalcBoundingBox(init_box, clnf_model.params_global, clnf_model.params_local);

	cv::Rect roi(init_box.x - init_box.width/2, init_box.y - init_box.height/2, init_box.width * 2, init_box.height * 2);
	roi = roi & cv::Rect(0, 0, grayscale_image.cols, grayscale_image.rows);

	int off_x = roi.x;
	int off_y = roi.y;

	float scaling = params.face_template_scale / clnf_model.params_global[0];
	cv::Mat_<uchar> image;
	if(scaling < 1)
	{
		cv::resize(clnf_model.face_template, clnf_model.face_template, cv::Size(), scaling, scaling);
		cv::resize(grayscale_image(roi), image, cv::Size(), scaling, scaling);
	}
	else
	{
		scaling = 1;
		image = grayscale_image(roi).clone();
	}
		
	// Resizing the template			
	cv::Mat corr_out;
	cv::matchTemplate(image, clnf_model.face_template, corr_out, CV_TM_CCOEFF_NORMED);

	// Actually matching it
	//double min, max;
	int max_loc[2];

	cv::minMaxIdx(corr_out, NULL, NULL, NULL, max_loc);

	cv::Rect_<float> out_bbox(max_loc[1]/scaling + off_x, max_loc[0]/scaling + off_y, clnf_model.face_template.rows / scaling, clnf_model.face_template.cols / scaling);

	float shift_x = out_bbox.x - init_box.x;
	float shift_y = out_bbox.y - init_box.y;
			
	clnf_model.params_global[4] = clnf_model.params_global[4] + shift_x;
	clnf_model.params_global[5] = clnf_model.params_global[5] + shift_y;
	
}





//================================================================================================================
// Landmark detection in image, need to provide an image and optionally CLNF model together with parameters (default values work well)
// Optionally can provide a bounding box in which detection is performed (this is useful if multiple faces are to be detected in images)
//================================================================================================================


// Helper index sorting function
template <typename T> std::vector<size_t> sort_indexes(const vector<T> &v) {

	// initialize original index locations
	vector<size_t> idx(v.size());
	std::iota(idx.begin(), idx.end(), 0);

	// sort indexes based on comparing values in v
	sort(idx.begin(), idx.end(),
		[&v](size_t i1, size_t i2) {return v[i1] > v[i2]; });

	return idx;
}
