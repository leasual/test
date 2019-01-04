
#ifndef __LANDMARK_DETECTOR_UTILS_h_
#define __LANDMARK_DETECTOR_UTILS_h_

// OpenCV includes
#include <opencv2/core/core.hpp>

#include "LandmarkDetectorModel.h"



using namespace std;

namespace LandmarkDetector
{
	//===========================================================================	
	// Defining a set of useful utility functions to be used within CLNF

	//===========================================================================
	// Fast patch expert response computation (linear model across a ROI) using normalised cross-correlation
	//===========================================================================
	// This is a modified version of openCV code that allows for precomputed dfts of templates and for precomputed dfts of an image
	// _img is the input img, _img_dft it's dft (optional), _integral_img the images integral image (optional), squared integral image (optional), 
	// templ is the template we are convolving with, templ_dfts it's dfts at varying windows sizes (optional),  _result - the output, method the type of convolution
	void matchTemplate_m(const cv::Mat_<float>& input_img, cv::Mat_<double>& img_dft, cv::Mat& _integral_img, cv::Mat& _integral_img_sq, const cv::Mat_<float>&  templ, map<int, cv::Mat_<double> >& templ_dfts, cv::Mat_<float>& result, int method);

	//===========================================================================
	// Point set and landmark manipulation functions
	//===========================================================================
	// Using Kabsch's algorithm for aligning shapes
	//This assumes that align_from and align_to are already mean normalised
	cv::Matx22d AlignShapesKabsch2D(const cv::Mat_<double>& align_from, const cv::Mat_<double>& align_to);
	cv::Matx22f AlignShapesKabsch2D_f(const cv::Mat_<float>& align_from, const cv::Mat_<float>& align_to);

	//=============================================================================
	// Basically Kabsch's algorithm but also allows the collection of points to be different in scale from each other
	cv::Matx22d AlignShapesWithScale(cv::Mat_<double>& src, cv::Mat_<double> dst);
	cv::Matx22f AlignShapesWithScale_f(cv::Mat_<float>& src, cv::Mat_<float> dst);

	// Useful utility for grabing a bounding box around a set of 2D landmarks (as a 1D 2n x 1 vector of xs followed by doubles or as an n x 2 vector)
	void ExtractBoundingBox(const cv::Mat_<float>& landmarks, float &min_x, float &max_x, float &min_y, float &max_y);

	vector<cv::Point2f> CalculateVisibleLandmarks(const cv::Mat_<float>& shape2D, const cv::Mat_<int>& visibilities);
	vector<cv::Point2f> CalculateVisibleLandmarks(const CLNF& clnf_model);
	vector<cv::Point2f> CalculateVisibleEyeLandmarks(const CLNF& clnf_model);

	vector<cv::Point2f> CalculateAllLandmarks(const cv::Mat_<float>& shape2D);
	vector<cv::Point2f> CalculateAllLandmarks(const CLNF& clnf_model);
	vector<cv::Point2f> CalculateAllEyeLandmarks(const CLNF& clnf_model);
	vector<cv::Point3f> Calculate3DEyeLandmarks(const CLNF& clnf_model, float fx, float fy, float cx, float cy);

	//============================================================================
	// Face detection helpers
	//============================================================================




	//============================================================================
	// Matrix reading functionality
	//============================================================================

	// Reading a matrix written in a binary format
	void ReadMatBin(std::ifstream& stream, cv::Mat &output_mat);

	// Reading in a matrix from a stream
	void ReadMat(std::ifstream& stream, cv::Mat& output_matrix);

	// Skipping comments (lines starting with # symbol)
	void SkipComments(std::ifstream& stream);

	int MakeLmList(int totalnumber,std::vector<int> &KptLMIDX);
}
#endif
