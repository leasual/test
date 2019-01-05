
#include "stdafx.h"

#include "LandmarkDetectorParameters.h"

// Boost includes
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

// System includes
#include <sstream>
#include <iostream>
#include <cstdlib>

#ifndef CONFIG_DIR
#define CONFIG_DIR "~"
#endif

using namespace std;

using namespace LandmarkDetector;

FaceModelParameters::FaceModelParameters() {
    // initialise the default values
    init();
    check_model_path();

}

FaceModelParameters::FaceModelParameters(vector<string> &arguments) {
    // initialise the default values
    init();

    // First element is reserved for the executable location (useful for finding relative model locs)
//	boost::filesystem::path root = boost::filesystem::path(arguments[0]).parent_path();
    boost::filesystem::path root = boost::filesystem::path(arguments[0]);

    bool *valid = new bool[arguments.size()];
    valid[0] = true;

    for (size_t i = 1; i < arguments.size(); ++i) {
        valid[i] = true;

        if (arguments[i].compare("-mloc") == 0) {
            string model_loc = arguments[i + 1];
            model_location = model_loc;
            valid[i] = false;
            valid[i + 1] = false;
            i++;

        }
        if (arguments[i].compare("-fdloc") == 0) {
            string face_detector_loc = arguments[i + 1];
            valid[i] = false;
            valid[i + 1] = false;
            i++;
        }
        if (arguments[i].compare("-sigma") == 0) {
            stringstream data(arguments[i + 1]);
            data >> sigma;
            valid[i] = false;
            valid[i + 1] = false;
            i++;
        } else if (arguments[i].compare("-w_reg") == 0) {
            stringstream data(arguments[i + 1]);
            data >> weight_factor;
            valid[i] = false;
            valid[i + 1] = false;
            i++;
        } else if (arguments[i].compare("-reg") == 0) {
            stringstream data(arguments[i + 1]);
            data >> reg_factor;
            valid[i] = false;
            valid[i + 1] = false;
            i++;
        } else if (arguments[i].compare("-multi_view") == 0) {

            stringstream data(arguments[i + 1]);
            int m_view;
            data >> m_view;

            multi_view = (bool) (m_view != 0);
            valid[i] = false;
            valid[i + 1] = false;
            i++;
        } else if (arguments[i].compare("-n_iter") == 0) {
            stringstream data(arguments[i + 1]);
            data >> num_optimisation_iteration;

            valid[i] = false;
            valid[i + 1] = false;
            i++;
        } else if (arguments[i].compare("-q") == 0) {

            quiet_mode = true;

            valid[i] = false;
        } else if (arguments[i].compare("-wild") == 0) {
            // For in the wild fitting these parameters are suitable
            window_sizes_init = vector<int>(4);
            window_sizes_init[0] = 15;
            window_sizes_init[1] = 13;
            window_sizes_init[2] = 11;
            window_sizes_init[3] = 11;

            sigma = 1.25;
            reg_factor = 35;
            weight_factor = 2.5;
            num_optimisation_iteration = 10;

            valid[i] = false;

            // Use multi-view hypotheses if in-the-wild setting
            multi_view = true;
        }
    }

    for (int i = (int) arguments.size() - 1; i >= 0; --i) {
        if (!valid[i]) {
            arguments.erase(arguments.begin() + i);
        }
    }


    // Make sure model_location is valid
    // First check working directory, then the executable's directory, then the config path set by the build process.
    boost::filesystem::path config_path = boost::filesystem::path(CONFIG_DIR);
    boost::filesystem::path model_path = boost::filesystem::path(model_location);
    std::cout << "***************   " << root.string() << std::endl;
    std::cout << "***************   " << model_path.string() << std::endl;
    std::cout << "***************   " << (root / model_path).string() << std::endl;

    if (boost::filesystem::exists(model_path)) {
        model_location = model_path.string();
    } else if (boost::filesystem::exists(root / model_path)) {
        model_location = (root / model_path).string();
    } else if (boost::filesystem::exists(config_path / model_path)) {
        model_location = (config_path / model_path).string();
    } else {
        std::cout << "Could not find the landmark detection model to load" << std::endl;
    }

    if (model_path.stem().string().compare("main_ceclm_general") == 0) {
        curr_landmark_detector = CECLM_DETECTOR;
        sigma = 1.5f * sigma;
        reg_factor = 0.9f * reg_factor;
    } else if (model_path.stem().string().compare("main_clnf_general") == 0) {
        curr_landmark_detector = CLNF_DETECTOR;
    } else if (model_path.stem().string().compare("main_clm_general") == 0) {
        curr_landmark_detector = CLM_DETECTOR;
    }


    check_model_path(root.string());
}

void FaceModelParameters::check_model_path(const std::string &root) {
    // Make sure model_location is valid
    // First check working directory, then the executable's directory, then the config path set by the build process.
    boost::filesystem::path config_path = boost::filesystem::path(CONFIG_DIR);
    boost::filesystem::path model_path = boost::filesystem::path(model_location);
    boost::filesystem::path root_path = boost::filesystem::path(root);

    if (boost::filesystem::exists(model_path)) {
        model_location = model_path.string();
    } else if (boost::filesystem::exists(root_path / model_path)) {
        model_location = (root_path / model_path).string();
    } else if (boost::filesystem::exists(config_path / model_path)) {
        model_location = (config_path / model_path).string();
    } else {
        std::cout << "Could not find the landmark detection model to load" << std::endl;
    }
}

void FaceModelParameters::init() {

    // number of iterations that will be performed at each scale
    num_optimisation_iteration = 5;


    refine_hierarchical = true;
    refine_parameters = true;

    window_sizes_small = vector<int>(4);
    window_sizes_init = vector<int>(4);

    // For fast tracking
    window_sizes_small[0] = 0;
    window_sizes_small[1] = 9;
    window_sizes_small[2] = 7;
    window_sizes_small[3] = 0;

    // Just for initialisation
    window_sizes_init.at(0) = 11;
    window_sizes_init.at(1) = 9;
    window_sizes_init.at(2) = 7;
    window_sizes_init.at(3) = 5;

    face_template_scale = 0.3f;

    // For first frame use the initialisation
    window_sizes_current = window_sizes_init;

//	model_location = "model/main_clnf_general11.txt";
    model_location = "main_clnf_general11.txt";
    curr_landmark_detector = CLNF_DETECTOR;

    sigma = 1.5f;
    reg_factor = 25.0f;
    weight_factor = 0.0f;


    limit_pose = true;
    multi_view = false;

    quiet_mode = false;
}

