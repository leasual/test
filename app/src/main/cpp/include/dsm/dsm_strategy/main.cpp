#include <iostream>
#include <unistd.h>
#include "TotalFlow.h"

int main() {
    TotalFlow total_flow;
    total_flow.run();
    std::cout << "succeed to finish the total_flow\n";
    return 0;

//    BaseTimeDetect calPercent(0.5f, 0.5f, 2, 4);
//    int result;
//    int a;
//    srand(time(NULL));
//    for(int i = 0; i < 500; i++)
//    {
//        a = rand()%2;
//        std::cout<<"(main)a = "<<a<<std::endl;
//        result = calPercent.BTDetect((bool)a);
//
//        std::cout<<"(main)percent = "<<result<<std::endl;
//        usleep(100*1000);
//    }

//    Action action;
//    srand(time(NULL));
//
//    for(int i = -30; i < 30; i++)
//    {
//        for(int j = 0; j < 10; j++)
//        {
//            int status = action.check(rand()%10, i);
////            if(status!=0)
//                std::cout<<"the left and right is: "<<status<<std::endl;
////            else
////            {
////                std::cout<<"the left and right is: "<<status<<std::endl;
////                i = 30;
////                break;
////            }
//        }
//
//    }
//    TrackerConfig config("../configure.yaml");
//    config.loadData();
//    cv::VideoCapture cap(0);
////    cv::VideoCapture cap("/home/zhangbo/workspace/cpp/files/ljj.avi");
//    Show showimage;
//    StrategyList strategyList;
//    strategyList.distraction = NORMAL;
//    strategyList.sleeping = NORMAL;
//    strategyList.smoking = NORMAL;
//    strategyList.calling = NORMAL;
//    strategyList.suspicious = NORMAL;
//    strategyList.speaking = NORMAL;
//    if (!cap.isOpened()) {
//        std::cout << "can't open camera!" << std::endl;
//        return -1;
//    }
//    cv::namedWindow("show");
//    cv::Mat img;
//    while (cap.read(img)) {
////        img = showimage.show_strategy_list(img, strategyList);
//        IplImage image(img);
////        cv::Mat result_img = showimage.show_strategy_list(&image,strategyList);
//        cv::Mat result_img = showimage.showActionTime(img, true, 7);
//        cv::imshow("image", result_img);
//        char key = cv::waitKey(1);
//        if (key == 'q')
//            break;
//    }
//
//    cv::destroyAllWindows();
//    std::cout << "Hello, World!" << std::endl;
//    return 0;
}