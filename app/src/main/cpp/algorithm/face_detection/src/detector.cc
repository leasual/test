#include "detector.h"
MTCNNDetector::MTCNNDetector(const ProposalNetwork::Config &pConfig,
                             const RefineNetwork::Config &rConfig,
                             const OutputNetwork::Config &oConfig) {
  _pnet = std::unique_ptr<ProposalNetwork>(new ProposalNetwork(pConfig));
  _rnet = std::unique_ptr<RefineNetwork>(new RefineNetwork(rConfig));
  _onet = std::unique_ptr<OutputNetwork>(new OutputNetwork(oConfig));
}

MTCNNDetector::MTCNNDetector(const std::string &mtcnn_dir, const float p_threshold, const float r_threshold, const float o_threshold) {
    ProposalNetwork::Config pConfig;
    pConfig.caffeModel = mtcnn_dir + "/det1.caffemodel";
    pConfig.protoText = mtcnn_dir + "/det1.prototxt";
    pConfig.threshold = p_threshold;

    RefineNetwork::Config rConfig;
    rConfig.caffeModel = mtcnn_dir + "/det2.caffemodel";
    rConfig.protoText = mtcnn_dir + "/det2.prototxt";
    rConfig.threshold = r_threshold;

    OutputNetwork::Config oConfig;
    oConfig.caffeModel = mtcnn_dir + "/det3.caffemodel";
    oConfig.protoText = mtcnn_dir + "/det3.prototxt";
    oConfig.threshold = o_threshold;

    _pnet = std::unique_ptr<ProposalNetwork>(new ProposalNetwork(pConfig));
    _rnet = std::unique_ptr<RefineNetwork>(new RefineNetwork(rConfig));
    _onet = std::unique_ptr<OutputNetwork>(new OutputNetwork(oConfig));
}

//std::vector<cv::Rect2i> MTCNNDetector::detect(const cv::Mat &img,
//                                        const float minFaceSize,
//                                        const float scaleFactor) {
//
//  cv::Mat rgbImg;
//  if (img.channels() == 3) {
//    cv::cvtColor(img, rgbImg, CV_BGR2RGB);
//  } else if (img.channels() == 4) {
//    cv::cvtColor(img, rgbImg, CV_BGRA2RGB);
//  }
//  if (rgbImg.empty()) {
//    return std::vector<cv::Rect2i>();
//  }
//  rgbImg.convertTo(rgbImg, CV_32FC3);
//  rgbImg = rgbImg.t();
//  // Run Proposal Network to find the initial set of faces
//  std::vector<Face> faces = _pnet->run(rgbImg, minFaceSize, scaleFactor);
//    if(faces.empty()){
//        return std::vector<cv::Rect2i>();
//    }
//  // Run Refine network on the output of the Proposal network
//  faces = _rnet->run(rgbImg, faces);
//    if(faces.empty()){
//        return std::vector<cv::Rect2i>();
//    }
//
//  // Run Output network on the output of the Refine network
//  faces = _onet->run(rgbImg, faces);
//    if(faces.empty()){
//        return std::vector<cv::Rect2i>();
//    }
//
//  for (size_t i = 0; i < faces.size(); ++i) {
//    std::swap(faces[i].bbox.x1, faces[i].bbox.y1);
//    std::swap(faces[i].bbox.x2, faces[i].bbox.y2);
//    for (int p = 0; p < NUM_PTS; ++p) {
//      std::swap(faces[i].ptsCoords[2 * p], faces[i].ptsCoords[2 * p + 1]);
//    }
//  }
//
//    std::vector<cv::Rect2i> rects;
//    for (size_t i = 0; i < faces.size(); i++) {
//        rects.emplace_back(faces[i].bbox.getRect());
//    }
//    return rects;
//}

std::vector<Face> MTCNNDetector::detect(const cv::Mat &img,
                                        const float minFaceSize,
                                        const float scaleFactor) {

    cv::Mat rgbImg;
    if (img.channels() == 3) {
        cv::cvtColor(img, rgbImg, CV_BGR2RGB);
    } else if (img.channels() == 4) {
        cv::cvtColor(img, rgbImg, CV_BGRA2RGB);
    }
    if (rgbImg.empty()) {
        return std::vector<Face>();
    }
    rgbImg.convertTo(rgbImg, CV_32FC3);
    rgbImg = rgbImg.t();
    // Run Proposal Network to find the initial set of faces
    std::vector<Face> faces = _pnet->run(rgbImg, minFaceSize, scaleFactor);
    if(faces.empty()){
        std::cout << "pnet not detect face!" << std::endl;
        return std::vector<Face>();
    }

    // Run Refine network on the output of the Proposal network
    faces = _rnet->run(rgbImg, faces);
    if(faces.empty()){
        std::cout << "rnet not detect face!" << std::endl;
        return std::vector<Face>();
    }

    // Run Output network on the output of the Refine network
    faces = _onet->run(rgbImg, faces);
    if(faces.empty()){
        std::cout << "onet not detect face!" << std::endl;
        return std::vector<Face>();
    }

    for (size_t i = 0; i < faces.size(); ++i) {
        std::swap(faces[i].bbox.x1, faces[i].bbox.y1);
        std::swap(faces[i].bbox.x2, faces[i].bbox.y2);
        for (int p = 0; p < NUM_PTS; ++p) {
            std::swap(faces[i].ptsCoords[2 * p], faces[i].ptsCoords[2 * p + 1]);
        }
    }

    return faces;
}

Face MTCNNDetector::get_largest_face(const std::vector<Face> &faces) {
    int max_area = 0;
    size_t ind = 0;
    for(size_t i = 0; i < faces.size(); i++) {
        int area = faces[i].bbox.getRect().area();
        if(area > max_area){
            max_area = area;
            ind = i;
        }
    }
    return faces[ind];
//    return make_square(faces[ind].bbox.getRect());
}

cv::Rect2i MTCNNDetector::make_square(const cv::Rect2i &rect) {
    int l = rect.x;
    int t = rect.y;
    int w = rect.width;
    int h = rect.height;
    int w_new = static_cast<int>(std::round(std::max(h*0.8, (double)w)));
    int l_new = static_cast<int>(l + w * 0.5 - w_new * 0.5);
    int t_new = t + h - w_new;
//    return cv::Rect2i(l_new, t_new, w_new, w_new);
    return cv::Rect2i(l, t_new, w_new, w_new);
}
