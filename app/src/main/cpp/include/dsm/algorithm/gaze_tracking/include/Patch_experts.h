
#ifndef __Patch_experts_h_
#define __Patch_experts_h_

// OpenCV includes
#include <opencv2/core/core.hpp>
#include <map>
#include "CCNF_patch_expert.h"
#include "PDM.h"
using namespace std;
namespace LandmarkDetector
{
//===========================================================================
/** 
    Combined class for all of the patch experts
*/
class Patch_experts
{

public:
	// The collection of LNF (CCNF) patch experts (for intensity images), the experts are laid out scale->view->landmark
	vector<vector<vector<CCNF_patch_expert> > >			ccnf_expert_intensity;

	// The node connectivity for CCNF experts, at different window sizes and corresponding to separate edge features
	vector<vector<cv::Mat_<float> > >					sigma_components;


	//Useful to pre-allocate data for im2col so that it is not allocated for every iteration and every patch
	vector< map<int, cv::Mat_<float> > > preallocated_im2col;

	// The available scales for intensity patch experts
	vector<double>							patch_scaling;

	// The available views for the patch experts at every scale (in radians)
	vector<vector<cv::Vec3d> >               centers;

	// Landmark visibilities for each scale and view
    vector<vector<cv::Mat_<int> > >          visibilities;


	cv::Mat_<int>							mirror_inds;
	cv::Mat_<int>							mirror_views;

	// Early termination calibration values, 
	//useful for CE-CLM model to speed up the multi-hypothesis setup
	vector<double> early_term_weights;
	vector<double> early_term_biases;
	vector<double> early_term_cutoffs;

    int _TtLandMarkNumEPX_;

    vector<int> KptLMIDX;


    int WrtCtViews()
    {
        cv::FileStorage fs("Centerviews.xml", cv::FileStorage::WRITE);
        fs<<"scalenumber"<<int(visibilities.size());
        for(size_t scidx=0;scidx<visibilities.size();scidx++)
        {
            char viewstorage[100];
            sprintf(viewstorage,"viewnumber%02d",int(scidx));
            fs<<viewstorage<<int(visibilities[scidx].size());

            for(size_t vidx=0;vidx<visibilities[scidx].size();vidx++)
            {
                char centername[260],viewname[260];
                sprintf(centername,"CENTER%02dscale%02dview",int(scidx),int(vidx));
                sprintf(viewname,"VIEW%02dscale%02dview",int(scidx),int(vidx));
                //printf("centername:%s,viewname:%s\n",centername,viewname);
                fs<<centername<<centers[scidx][vidx];
                fs<<viewname<<visibilities[scidx][vidx];
            }
        }

        fs.release();
        return 0;
    }




    int TnsMirIdsFrom68(cv::Mat_<int>&Mirror_inds68,vector<int>&Result)
    {
        vector<int> &KeepLMList=KptLMIDX;
        map<int,int> maplist;
        for(size_t idx=0;idx<KeepLMList.size();idx++)
        maplist[KeepLMList[idx]]=idx;

        for(size_t idx=0;idx<KeepLMList.size();idx++)
        {
            int Src=KeepLMList[idx];
            int Des=Mirror_inds68.at<int>(0,Src);
            int newMapped=maplist[Des];
            Result.push_back(newMapped);
        }
        return 0;
    }


	// A default constructor
	Patch_experts(){;}

	// A copy constructor
	Patch_experts(const Patch_experts& other);


	// Returns the patch expert responses given a grayscale image.
	// Additionally returns the transform from the image coordinates to the response coordinates (and vice versa).
	// The computation also requires the current landmark locations to compute response around, the PDM corresponding to the desired model, and the parameters describing its instance
	// Also need to provide the size of the area of interest and the desired scale of analysis
	void Response(vector<cv::Mat_<float> >& patch_expert_responses, cv::Matx22f& sim_ref_to_img, cv::Matx22f& sim_img_to_ref, const cv::Mat_<float>& grayscale_image, 
							 const PDM& pdm, const cv::Vec6f& params_global, const cv::Mat_<float>& params_local, int window_size, int scale);

	// Getting the best view associated with the current orientation
	int GetViewIdx(const cv::Vec6f& params_global, int scale) const;

	// The number of views at a particular scale
	inline int nViews(size_t scale = 0) const { return (int)centers[scale].size(); };

	// Reading in all of the patch experts
	bool Read(vector<string> intensity_svr_expert_locations,
              vector<string> intensity_ccnf_expert_locations,
              vector<string> intensity_cen_expert_locations,
              string early_term_loc = "");

    bool Read11(vector<string> intensity_svr_expert_locations,
              vector<string> intensity_ccnf_expert_locations,
              vector<string> intensity_cen_expert_locations,
              string early_term_loc = "");


private:
	bool Read_CCNF_patch_experts(
                string patchesFileLocation,
                std::vector<cv::Vec3d>& centers,
                std::vector<cv::Mat_<int> >& visibility,
                std::vector<std::vector<CCNF_patch_expert> >& patches,
                double& patchScaling);


    bool Read_CCNF_patch_experts11(
            string patchesFileLocation,
            std::vector<cv::Vec3d>& centers,
            std::vector<cv::Mat_<int> >& visibility,
            std::vector<std::vector<CCNF_patch_expert> >& patches,
            double& patchScaling);

    // Helper for collecting visibilities
	std::vector<int> Collect_visible_landmarks(vector<vector<cv::Mat_<int> > > visibilities,
											   int scale, int view_id, int n);
};




}
#endif
