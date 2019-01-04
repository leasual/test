
/*! \file enum_types.h
*  \brief 定义DSM项目中使用到的各种枚举类型
*  \author zhangbo
*  \date 2018-5-22
*/
#ifndef DSM_STRACTEGY_ENUM_TYPES_H
#define DSM_STRACTEGY_ENUM_TYPES_H

//! \brief 定义规定动作过程中的不同动作
//enum class Action {
//    END = 0,            //!< 完成规定动作
//    HEAD_MIDDLE = 1,    //!< 头在中间位置
//    HEAD_LEFT = 2,      //!< 向左转头
//    HEAD_RIGHT = 3,     //!< 向右转头
//    HEAD_UP = 4,        //!< 向上转头
//    HEAD_DOWN = 5,      //!< 向下转头
//};


//! \brief 定义不同的检测策略
//enum class Strategy {
//    DISTRACTION = 1,   //!< 分神检测策略
//    FATIGUE = 2,    //!< 疲劳检测策略
//    SMOKE = 3,    //!< 吸烟检测策略
//    CALL = 4,    //!<  打电话检测策略
//    PALM = 5,    //!< 手掌（可疑行为）检测策略
//    CHAT = 6,    //!< 聊天检测策略
//};

//! \brief 策略检测得到的不同结果
enum class StrategyResult {
    NORMAL = 0,     //!< 正常
    WARN = 1,    //!< 警告
    JUDGEMENT = 2,  //!< 判决
};


//! \brief 判断程序当前运行的阶段
enum RunStep {

    RequiredRoutineStep = 1,    ///< 规定动作阶段
    CalibrationStep = 2,    ///< 校正参数阶段
    MainRunStep = 3,  ///< 正式运行阶段

    DistractDetectionStep = 5,  ///< 分神检测阶段
    FatigueDetectionStep = 6,   ///< 疲劳检测阶段
    SmokeDetectionStep = 7, ///< 吸烟检测阶段
    CallDetectionStep = 8,  ///< 打电话检测阶段
    YawnDetectionStep = 9,  ///< 打哈欠检测阶段
    EndDetectionStep = 11,  ///< 完成检测
};


/// \brief 程序运行过程中的检测状态
enum DetectionState {
    DistractionWarn = 1,  ///< 分神预警状态
    DistractionJudge = 2,  ///< 分神判决状态
    FatigueWarn = 3,  ///< 疲劳预警状态
    FatigueJudge = 4,  ///< 疲劳判决状态
    SmokeJudge = 6,  ///< 吸烟判决状态
    CallWarn = 7,  ///< 打电话预警状态
    CallJudge = 8,  ///< 打电话判决状态
    YawnWarn = 9,  ///< 打哈欠（疲劳）预警状态
    YawnJudge = 10,  ///< 打哈欠（疲劳）判决状态
    Normal = 13,  ///< 正常状态
};


/// \brief 程序运行过程中显示图片信息的不同阶段
enum ShowStep {
//    StartRoutine = 0,   ///<显示“开始运行”
    StartCalibration = 5,   ///<显示“开始校准”和策略列表
    ShowCount = 6,      ///< 显示“校准倒计时”和策略列表
    StartRunning = 7,       ///< 显示“开始运行”和策略列表
    Running = 8,        ///< 只显示策略列表
};

/// \brief 自定义枚举类型的哈希函数
struct EnumClassHash {
    template<typename T>
    std::size_t operator()(T t) const {
        return static_cast<std::size_t >(t);
    }
};


#endif //DSM_STRACTEGY_ENUM_TYPES_H
