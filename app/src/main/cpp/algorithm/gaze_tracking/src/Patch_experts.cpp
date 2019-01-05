

#include "stdafx.h"

#include "Patch_experts.h"

// TBB includes
#include <tbb/tbb.h>

// Math includes
#define _USE_MATH_DEFINES
#include <cmath>

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

#include "LandmarkDetectorUtils.h"

using namespace LandmarkDetector;

static const int _TotalLandMark_= 68;

// A copy constructor

Patch_experts::Patch_experts(const Patch_experts& other) : patch_scaling(other.patch_scaling), centers(other.centers),
														ccnf_expert_intensity(other.ccnf_expert_intensity),
														early_term_weights(other.early_term_weights), early_term_biases(other.early_term_biases), early_term_cutoffs(other.early_term_cutoffs),
														mirror_inds(other.mirror_inds),mirror_views(other.mirror_views)
{

	// Make sure the matrices are allocated properly
	this->sigma_components.resize(other.sigma_components.size());
	for (size_t i = 0; i < other.sigma_components.size(); ++i)
	{
		this->sigma_components[i].resize(other.sigma_components[i].size());

		for (size_t j = 0; j < other.sigma_components[i].size(); ++j)
		{
			// Make sure the matrix is copied.
			this->sigma_components[i][j] = other.sigma_components[i][j].clone();
		}
	}

	// Make sure the matrices are allocated properly
	this->visibilities.resize(other.visibilities.size());
	for (size_t i = 0; i < other.visibilities.size(); ++i)
	{
		this->visibilities[i].resize(other.visibilities[i].size());

		for (size_t j = 0; j < other.visibilities[i].size(); ++j)
		{
			// Make sure the matrix is copied.
			this->visibilities[i][j] = other.visibilities[i][j].clone();
		}
	}

	preallocated_im2col.resize(other.preallocated_im2col.size());
}

// Returns indices to landmarks that need to have patch responses computed (omits mirrored frontal landmarks for CEN as they will be computed together with their mirrored pair)
std::vector<int> Patch_experts::Collect_visible_landmarks(vector<vector<cv::Mat_<int> > > visibilities, int scale, int view_id, int n)
{
	std::vector<int> vis_lmk;
	for (int i = 0; i < n; i++)
	{
		if (visibilities[scale][view_id].rows == n)
		{
			if (visibilities[scale][view_id].at<int>(i, 0) != 0)
			{
					vis_lmk.push_back(i);
			}
		}
	}
	return vis_lmk;

}

// Returns the patch expert responses given a grayscale image.
// Additionally returns the transform from the image coordinates to the response coordinates (and vice versa).
// The computation also requires the current landmark locations to compute response around, the PDM corresponding to the desired model, and the parameters describing its instance
// Also need to provide the size of the area of interest and the desired scale of analysis
void Patch_experts::Response(vector<cv::Mat_<float> >& patch_expert_responses, cv::Matx22f& sim_ref_to_img, cv::Matx22f& sim_img_to_ref, const cv::Mat_<float>& grayscale_image,
	const PDM& pdm, const cv::Vec6f& params_global, const cv::Mat_<float>& params_local, int window_size, int scale)
{

	int view_id = GetViewIdx(params_global, scale);

	int n = pdm.NumberOfPoints();

	// Compute the current landmark locations (around which responses will be computed)
	cv::Mat_<float> landmark_locations;

	pdm.CalcShape2D(landmark_locations, params_local, params_global);

	cv::Mat_<float> reference_shape;

	// Initialise the reference shape on which we'll be warping
	cv::Vec6f global_ref(patch_scaling[scale], 0, 0, 0, 0, 0);

	// Compute the reference shape
	pdm.CalcShape2D(reference_shape, params_local, global_ref);

	// similarity and inverse similarity transform to and from image and reference shape
	cv::Mat_<float> reference_shape_2D = (reference_shape.reshape(1, 2).t());
	cv::Mat_<float> image_shape_2D = landmark_locations.reshape(1, 2).t();

	sim_img_to_ref = AlignShapesWithScale_f(image_shape_2D, reference_shape_2D);
	sim_ref_to_img = sim_img_to_ref.inv(cv::DECOMP_LU);
	
	float a1 = sim_ref_to_img(0, 0);
	float b1 = -sim_ref_to_img(0, 1);



	// If using CCNF patch experts might need to precalculate Sigmas

		vector<cv::Mat_<float> > sigma_components;

		// Retrieve the correct sigma component size
		for (size_t w_size = 0; w_size < this->sigma_components.size(); ++w_size)
		{
			if (!this->sigma_components[w_size].empty())
			{
				if (window_size*window_size == this->sigma_components[w_size][0].rows)
				{
					sigma_components = this->sigma_components[w_size];
				}
			}
		}

		// Go through all of the landmarks and compute the Sigma for each
		for (int lmark = 0; lmark < n; lmark++)
		{
			// Only for visible landmarks
			if (visibilities[scale][view_id].at<int>(lmark, 0))
			{
				// Precompute sigmas if they are not computed yet
				ccnf_expert_intensity[scale][view_id][lmark].ComputeSigmas(sigma_components, window_size);
			}
		}



	// If using CEN precalculate interpolation matrix
	cv::Mat_<float> interp_mat;


	// We do not want to create threads for invisible landmarks, so construct an index of visible ones
	std::vector<int> vis_lmk = Collect_visible_landmarks(visibilities, scale, view_id, n);

	// calculate the patch responses for every landmark, Actual work happens here. If openMP is turned on it is possible to do this in parallel,
	// this might work well on some machines, while potentially have an adverse effect on others
	//#ifdef _OPENMP
	//#pragma omp parallel for
	//#endif
	tbb::parallel_for(0, (int)vis_lmk.size(), [&](int i) {
	//for (int i = 0; i < vis_lmk.size(); i++)
	{

		// Work out how big the area of interest has to be to get a response of window size
		int area_of_interest_width;
		int area_of_interest_height;
		int ind = vis_lmk.at(i);


			area_of_interest_width = window_size + ccnf_expert_intensity[scale][view_id][ind].width - 1;
			area_of_interest_height = window_size + ccnf_expert_intensity[scale][view_id][ind].height - 1;

		// scale and rotate to mean shape to reference frame
		cv::Mat sim = (cv::Mat_<float>(2, 3) << a1, -b1, landmark_locations.at<float>(ind, 0) - a1 * (area_of_interest_width - 1.0f) / 2.0f + b1 * (area_of_interest_width - 1.0f) / 2.0f, b1, a1, landmark_locations.at<float>(ind + n, 0) - a1 * (area_of_interest_width - 1.0f) / 2.0f - b1 * (area_of_interest_width - 1.0f) / 2.0f);

		// Extract the region of interest around the current landmark location
		cv::Mat_<float> area_of_interest(area_of_interest_height, area_of_interest_width, 0.0f);

		cv::warpAffine(grayscale_image, area_of_interest, sim, area_of_interest.size(), cv::WARP_INVERSE_MAP + CV_INTER_LINEAR);		



			// get the correct size response window			
			patch_expert_responses[ind] = cv::Mat_<float>(window_size, window_size);

			int im2col_size = area_of_interest_width * area_of_interest_height;

			cv::Mat_<float> prealloc_mat = preallocated_im2col[ind][im2col_size];

			ccnf_expert_intensity[scale][view_id][ind].ResponseOpenBlas(area_of_interest, patch_expert_responses[ind], prealloc_mat);

			preallocated_im2col[ind][im2col_size] = prealloc_mat;

			// Below is an alternative way to compute the same, but that uses FFT instead of OpenBLAS
			// ccnf_expert_intensity[scale][view_id][ind].Response(area_of_interest, patch_expert_responses[ind]);
	}
	});
}


//=============================================================================
// Getting the closest view center based on orientation
int Patch_experts::GetViewIdx(const cv::Vec6f& params_global, int scale) const
{	
	int idx = 0;
	
	float dbest;

	for(int i = 0; i < this->nViews(scale); i++)
	{
		float v1 = params_global[1] - centers[scale][i][0]; 
		float v2 = params_global[2] - centers[scale][i][1];
		float v3 = params_global[3] - centers[scale][i][2];
			
		float d = v1*v1 + v2*v2 + v3*v3;

		if(i == 0 || d < dbest)
		{
			dbest = d;
			idx = i;
		}
	}
	return idx;
}


//===========================================================================
bool Patch_experts::Read(vector<string> intensity_svr_expert_locations, vector<string> intensity_ccnf_expert_locations, vector<string> intensity_cen_expert_locations, string early_term_loc)
{

	// initialise the SVR intensity patch expert parameters
	int num_intensity_svr = intensity_svr_expert_locations.size();
	centers.resize(num_intensity_svr);
	visibilities.resize(num_intensity_svr);
	patch_scaling.resize(num_intensity_svr);
	

	// Initialise and read CCNF patch experts (currently only intensity based), 
	int num_intensity_ccnf = intensity_ccnf_expert_locations.size();

	// CCNF experts override the SVR ones
	if(num_intensity_ccnf > 0)
	{
		centers.resize(num_intensity_ccnf);
		visibilities.resize(num_intensity_ccnf);
		patch_scaling.resize(num_intensity_ccnf);
		ccnf_expert_intensity.resize(num_intensity_ccnf);
	}

	for(int scale = 0; scale < num_intensity_ccnf; ++scale)
	{		
		string location = intensity_ccnf_expert_locations[scale];
		cout << "Reading the intensity CCNF patch experts from: " << location << "....";
		bool success_read = Read_CCNF_patch_experts(location,  centers[scale], visibilities[scale], ccnf_expert_intensity[scale], patch_scaling[scale]);

		if (!success_read)
		{
			return false;
		}

		if (scale == 0)
		{
			preallocated_im2col.resize(ccnf_expert_intensity[0][0].size());
		}
	}

	return true;
}









bool Patch_experts::Read11(
vector<string> intensity_svr_expert_locations,
vector<string> intensity_ccnf_expert_locations,
vector<string> intensity_cen_expert_locations,
string early_term_loc)
{
	///MakeLmList(_TotalLandMark_);
	MakeLmList(_TtLandMarkNumEPX_,KptLMIDX);


	// Initialise and read CCNF patch experts (currently only intensity based),
	int num_intensity_ccnf = intensity_ccnf_expert_locations.size();

	// CCNF experts override the SVR ones
	if(num_intensity_ccnf > 0)
	{
		centers.resize(num_intensity_ccnf);
		visibilities.resize(num_intensity_ccnf);
		patch_scaling.resize(num_intensity_ccnf);
		ccnf_expert_intensity.resize(num_intensity_ccnf);
	}

	for(int scale = 0; scale < num_intensity_ccnf; ++scale)
	{
		string location = intensity_ccnf_expert_locations[scale];
		cout << "Reading the intensity CCNF patch experts from: " << location << "....";
		bool success_read = Read_CCNF_patch_experts(
				location,
				centers[scale],
				visibilities[scale],
				ccnf_expert_intensity[scale],
				patch_scaling[scale]);

		if (!success_read)
		{
			return false;
		}

		if (scale == 0)
		{
			preallocated_im2col.resize(ccnf_expert_intensity[0][0].size());
		}
	}

	return true;
}








//======================= Reading the SVR patch experts =========================================//



//======================= Reading the CCNF patch experts =========================================//
bool Patch_experts::Read_CCNF_patch_experts(string patchesFileLocation, std::vector<cv::Vec3d>& centers, std::vector<cv::Mat_<int> >& visibility, std::vector<std::vector<CCNF_patch_expert> >& patches, double& patchScaling)
{

	ifstream patchesFile(patchesFileLocation.c_str(), ios::in | ios::binary);

	if(patchesFile.is_open())
	{
		patchesFile.read ((char*)&patchScaling, 8);
		
		int numberViews;		
		patchesFile.read ((char*)&numberViews, 4);

		// read the visibility
		centers.resize(numberViews);
		visibility.resize(numberViews);
  
		patches.resize(numberViews);
		
		// centers of each view (which view corresponds to which orientation)
		for(size_t i = 0; i < centers.size(); i++)
		{
			cv::Mat center;
			LandmarkDetector::ReadMatBin(patchesFile, center);	
			center.copyTo(centers[i]);
			centers[i] = centers[i] * M_PI / 180.0;
		}

		// the visibility of points for each of the views (which verts are visible at a specific view
		for(size_t i = 0; i < visibility.size(); i++)
		{
			LandmarkDetector::ReadMatBin(patchesFile, visibility[i]);				
		}
		int numberOfPoints = visibility[0].rows;

		// Read the possible SigmaInvs (without beta), this will be followed by patch reading (this assumes all of them have the same type, and number of betas)
		int num_win_sizes;
		int num_sigma_comp;
		patchesFile.read ((char*)&num_win_sizes, 4);

		vector<int> windows;
		windows.resize(num_win_sizes);

		vector<vector<cv::Mat_<float> > > sigma_components;
		sigma_components.resize(num_win_sizes);

		for (int w=0; w < num_win_sizes; ++w)
		{
			patchesFile.read ((char*)&windows[w], 4);

			patchesFile.read ((char*)&num_sigma_comp, 4);

			sigma_components[w].resize(num_sigma_comp);

			for(int s=0; s < num_sigma_comp; ++s)
			{
				LandmarkDetector::ReadMatBin(patchesFile, sigma_components[w][s]);
			}
		}
		
		this->sigma_components = sigma_components;

		// read the patches themselves
		for(size_t i = 0; i < patches.size(); i++)
		{
			// number of patches for each view
			patches[i].resize(numberOfPoints);
			// read in each patch
			for(int j = 0; j < numberOfPoints; j++)
			{
				patches[i][j].Read(patchesFile, windows, sigma_components);
			}
		}
		cout << "Done" << endl;
		return true;
	}
	else
	{
		cout << "Can't find/open the patches file" << endl;
		return false;
	}
}

