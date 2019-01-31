
//  Parameters of the CLNF, CLM-Z and CLM trackers
#ifndef __LANDMARK_DETECTOR_PARAM_H
#define __LANDMARK_DETECTOR_PARAM_H

#include <vector>

using namespace std;

namespace LandmarkDetector
{

struct FaceModelParameters
{
	int num_optimisation_iteration;
	bool limit_pose;
	vector<int> window_sizes_small;

	float face_template_scale;
	// Used when initialising or tracking fails
	vector<int> window_sizes_init;
	
	// Used for the current frame
	vector<int> window_sizes_current;
	// Where to load the model from
	string model_location;

	float sigma;

	float reg_factor;	// weight put to regularisation
	float weight_factor; // factor for weighted least squares

	bool multi_view;
	enum LandmarkDetector { CLM_DETECTOR, CLNF_DETECTOR, CECLM_DETECTOR };
	LandmarkDetector curr_landmark_detector;

	enum FaceDetector{HAAR_DETECTOR, HOG_SVM_DETECTOR, MTCNN_DETECTOR};

	bool quiet_mode;
	bool refine_hierarchical;
	bool refine_parameters;

	FaceModelParameters();

	FaceModelParameters(vector<string> &arguments);

	private:
		void init();
		void check_model_path(const std::string& root = "/");

};

}

#endif // __LANDMARK_DETECTOR_PARAM_H
