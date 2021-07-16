#ifndef REALTIMECOMMUNICATION_RADARDATACUBE_H
#define REALTIMECOMMUNICATION_RADARDATACUBE_H

#include "ParamClass.h"
#include<vector>
#include<cstdint>
#include<armadillo>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>

# define NDEBUG

/**
 * @brief RadarDataCube类
 * @details 处理雷达帧数据流
 */
class RadarDataCube
{
public:
    // 构造函数相关
    /// 默认构造函数
    RadarDataCube()
        : m_singleFrame(m_param.getFrameBytes() / 2),
          m_realTimeMapStart(false),
          m_frameCount(0)
    {}
    /// 含参构造函数
    RadarDataCube(RadarParam &radarConf)
        : m_param(radarConf),
          m_singleFrame(m_param.getFrameBytes() / 2),
          m_realTimeMapStart(false),
          m_frameCount(0)
    {}

    // 功能函数相关
    /// 获得单帧数据流
    std::vector<int16_t> & getFrame()
    {
        return m_singleFrame;
    }

    // 转换函数相关
    arma::cx_cube & creatCube();     ///< 构建数据立方体
    void creatRdm();                 ///< 创建RDM
    void setFlagForMap()             ///< 表示开始转化微多普勒图Flag
    {
        m_realTimeMapStart = true;
    }
    void updateMicroMap();           ///< 更新微多普勒图
    cv::Mat& convertRdmToMap();      ///< 转换成Mat图
    cv::Mat& convertMdToMap();       ///< 转换成微多普勒图

    std::vector<arma::rowvec> extractFeature();   ///< 从微多普勒图中抽取向量
    arma::vec readVelocity();        // todo - 读取速度
    void updateStaticMicroMap(long long frameCount);
    cv::Mat& convertMdToStaticMap(long long frameCount);



protected:
    arma::vec creatHanningCol(int length);    ///< 创建hanning窗 - 纵向
    arma::rowvec creatHanningRow(int length); ///< 创建hanning窗 - 横向

protected:
    RadarParam m_param;                     ///< 雷达参数
    std::vector<int16_t> m_singleFrame;     ///< 单帧数据流
    arma::cx_cube m_radarCube;              ///< 雷达数据立方体
    arma::mat m_radarRdmReal;               ///< 雷达RDM
    bool m_realTimeMapStart;
    arma::mat m_realTimeMdMap;              ///< 实时微多普勒矩阵(待更新)
    cv::Mat m_rdmMap;                       ///< Map图
    cv::Mat m_mdMap;                        ///< 实时微多普勒矩阵(待更新)
    long m_frameCount;                      ///< 用于实时微多普勒矩阵更新的frameCount
    double m_globalMin;                     ///< 自适应图像参数 - 前N帧最小值
    double m_globalMax;                     ///< 自适应图像参数 - 前N帧最大值
};






















#endif //REALTIMECOMMUNICATION_RADARDATACUBE_H
