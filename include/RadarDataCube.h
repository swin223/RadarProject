#ifndef RADARDATACUBE_H
#define RADARDATACUBE_H

#include "ParamClass.h"
#include <vector>
#include <cstdint>
#include <armadillo>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#define NDEBUG

/** RadarDataCube类
 * @details 处理雷达帧数据流，构建Range Doppler Map、
 * 以及Micro Doppler Map
 * @todo 可以将创建静态/动态微多普勒图和距离速度图用一个flag标识版本放在一起实现
 */
class RadarDataCube
{
public:
    /** 默认构造函数 */
    RadarDataCube()
        : m_singleFrame(m_param.GetFrameBytes() / 2),
          m_realTimeMapStart(false),
          m_frameCount(0)
    {}

    /** 含参构造函数
     * @param radarConf RadarParam类对象
     */
    RadarDataCube(RadarParam &radarConf)
        : m_param(radarConf),
          m_singleFrame(m_param.GetFrameBytes() / 2),
          m_realTimeMapStart(false),
          m_frameCount(0)
    {}

    // 功能函数相关
    /** 获取单帧数据流
     * @return 整个单帧的数据流vector
     */
    std::vector<int16_t> & GetFrame()
    {
        return m_singleFrame;
    }

    // 转换函数相关
    /** 构建雷达数据立方体
     * @return 返回雷达数据立方体(Complex)
     */
    arma::cx_cube & CreatCube();

    /** 创建RDM矩阵
     * @details RDM - Range Doppler Map
     */
    void CreatRdm();

    /** 将RDM矩阵转换成Mat
     * @note 并不会改变radarRdmReal
     * @return 返回Range Doppler Map矩阵 - cv::Mat
     */
    cv::Mat& ConvertRdmToMap();

    /** 设置表示开始转化微多普勒图的Flag */
    void SetFlagForMap()
    {
        m_realTimeMapStart = true;
    }

    /** 更新微多普勒图(动态)
     * @details 用于Online Page、Replay Page得到实时更新的微多普勒图像
     */
    void UpdateMicroMap();

    /** 更新微多普勒图(静态)
    * @details 用于Offline Page得到静态的微多普勒图像
    * @param frameCount 静态的微多普勒图像构建所需要的帧数
    */
    void UpdateStaticMicroMap(long long frameCount);

    /** 转换成微多普勒图(动态)
     * @note 并不会改变realTimeMdMap
     * @return 返回micro doppler矩阵 - cv::Mat
     */
    cv::Mat& ConvertMdToMap();

    /** 转换成微多普勒图(静态)
     * @param frameCount 静态的速度距离图构建所需要的帧数
     * @return 返回micro doppler矩阵 - cv::Mat
     */
    cv::Mat& ConvertMdToStaticMap(long long frameCount);

    /** 从微多普勒频谱中提取相应的特征向量
     * @return 包含三大特征向量的vector
     */
    std::vector<arma::rowvec> ExtractFeature();

protected:
    /** 创建纵向Hanning窗
     * @param length 创建纵向Hanning窗的长度
     * @return 纵向Hanning窗向量
     */
    arma::vec CreatHanningCol(int length);

    /** 创建横向Hanning窗
     * @param length 创建横向Hanning窗的长度
     * @return 横向Hanning窗向量
     */
    arma::rowvec CreatHanningRow(int length);

    /** 通过计算得到用于Range Bin对应的速度序列
     * @details 得到用于Range Bin对应的速度序列来计算躯干特征矢量
     * @return 速度序列(向量)
     */
    arma::vec ReadVelocity();

protected:
    RadarParam m_param;                     ///< 雷达参数
    std::vector<int16_t> m_singleFrame;     ///< 单帧数据流
    arma::cx_cube m_radarCube;              ///< 雷达数据立方体
    arma::mat m_radarRdmReal;               ///< 雷达RDM
    bool m_realTimeMapStart;                ///< 标志微多普勒图是否开始转换
    arma::mat m_realTimeMdMap;              ///< 实时微多普勒矩阵(待更新)
    cv::Mat m_rdmMap;                       ///< Map图
    cv::Mat m_mdMap;                        ///< 实时微多普勒矩阵(待更新)
    long m_frameCount;                      ///< 用于实时微多普勒矩阵更新的frameCount
    double m_globalMin;                     ///< 自适应图像参数 - 前N帧最小值
    double m_globalMax;                     ///< 自适应图像参数 - 前N帧最大值
};

#endif //RADARDATACUBE_H