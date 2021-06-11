#include"RadarDataCube.h"

/**
 * @brief RadarDataCube类 - creatCube函数
 * @details 构建雷达数据立方体
 * @return 返回雷达数据立方体(Complex)
 */
arma::cx_cube & RadarDataCube::creatCube()
{
    // 转换成double类型的arma向量
    arma::vec frameDataStrem(m_param.getFrameBytes() / 2);
    for (int i = 0;i < m_param.getFrameBytes() / 2;++i)
        frameDataStrem.at(i) = (double)m_singleFrame[i];

    // 数据 Complex 化
    arma::mat iqMat = reshape(frameDataStrem, 4, m_param.getFrameBytes() / 2 / 4);
    arma::cx_mat preRadarCube(iqMat.rows(0, 1), iqMat.rows(2, 3));

    // 构建雷达数据立方体
    arma::cx_cube tempCube(preRadarCube.n_rows, preRadarCube.n_cols, 1);
    tempCube.slice(0) = preRadarCube;
    m_radarCube = reshape(tempCube, m_param.m_adcSample, m_param.m_nRx, m_param.m_nChirp);
    return m_radarCube;
}

/**
 * @brief RadarDataCube类 - creatHanningCol
 * @details 创建纵向Hanning窗
 * @param length 创建纵向Hanning窗的长度
 * @return 纵向Hanning窗向量
 */
arma::vec RadarDataCube::creatHanningCol(int length)
{
    const double PI = 3.14159265359;
    arma::vec hanWin(length,arma::fill::zeros);
    for(int i = 0;i < length;++i)
        hanWin.at(i) = 0.54 - 0.46 * cos(2*PI*i/length);
    return hanWin;
}

/**
 * @brief RadarDataCube类 - creatHanningRow
 * @details 创建横向Hanning窗
 * @param length 创建横向Hanning窗的长度
 * @return 横向Hanning窗向量
 */
arma::rowvec RadarDataCube::creatHanningRow(int length)
{
    const double PI = 3.14159265359;
    arma::rowvec hanWin(length,arma::fill::zeros);
    for(int i = 0;i < length;++i)
        hanWin.at(i) = 0.54 - 0.46 * cos(2*PI*i/length);
    return hanWin;
}

/**
 * @brief RadarDataCube类 - creatRdm
 * @details 创建RDM矩阵
 */
void RadarDataCube::creatRdm()
{
    // 这个函数中只取RX0的数据

    // 创建汉宁窗
    arma::vec colHanning = creatHanningCol(m_param.m_adcSample);  // adcSample列汉宁窗
    arma::rowvec rowHanning = creatHanningRow(m_param.m_nChirp);  // nchirp行汉宁窗

    // 加窗
    for(int i = 0;i < m_param.m_nChirp;++i)
        for(int j = 0;j < m_param.m_nRx;++j)
            m_radarCube.slice(i).col(j) = m_radarCube.slice(i).col(j) % colHanning;

    // 由n通道合成RDM
    // todo channels 合成参数
    int nChannel = 4;
    arma::mat radarRdmRealTotal(m_param.m_adcSample, m_param.m_nChirp, arma::fill::zeros);
    for(int i = 0;i < nChannel;++i){
        arma::cx_mat radarRdm(m_param.m_adcSample, m_param.m_nChirp,arma::fill::zeros);  // 暂存数据
        for(int j = 0;j < m_param.m_nChirp;++j)
            radarRdm.col(j) = m_radarCube.slice(j).col(i);

        // Range-FFT
        radarRdm = fft(radarRdm);

        // 在Doppler-FFT前加窗
        for(int j = 0;j < m_param.m_adcSample;++j)
            radarRdm.row(j) = radarRdm.row(j) % rowHanning;

        // Doppler FFT
        radarRdm = fft(radarRdm.st());
        radarRdm = radarRdm.st();

        // 取模 - 平方
        m_radarRdmReal = abs(radarRdm);
        m_radarRdmReal = m_radarRdmReal % m_radarRdmReal;

        // FFT shift
        m_radarRdmReal = join_rows(m_radarRdmReal.cols(m_param.m_nChirp / 2,m_param.m_nChirp - 1),
                                   m_radarRdmReal.cols(0, m_param.m_nChirp / 2 - 1));

        // 四路信号相加
        radarRdmRealTotal = radarRdmRealTotal + m_radarRdmReal;
    }

    // 四路信号取平均
    m_radarRdmReal = radarRdmRealTotal / nChannel;

    // 陷波滤波 + 插值
    bool deleteBin0 = true;                    // 是否0-bin去除flag

    if(deleteBin0) {
        int dopplerBin0 = m_param.m_nChirp / 2;
        arma::vec interpF = m_radarRdmReal.col(dopplerBin0 - 2);
        arma::vec interpB = m_radarRdmReal.col(dopplerBin0 + 2);
        arma::vec interPerDiff = (interpB - interpF) / 4;
        m_radarRdmReal.col(dopplerBin0 - 1) = interpF + interPerDiff;
        m_radarRdmReal.col(dopplerBin0) = interpF + interPerDiff * 2;
        m_radarRdmReal.col(dopplerBin0 + 1) = interpF + interPerDiff * 3;
    }

    // 对数化元素
    arma::mat::iterator it = m_radarRdmReal.begin();
    arma::mat::iterator itEnd = m_radarRdmReal.end();
    for(;it != itEnd;++it)
        (*it) = 10 * log10(1.0 + (*it));
}

/**
 * @brief RadarDataCube类 - updateMicroMap函数
 * @details 更新微多普勒图
 */
void RadarDataCube::updateMicroMap()
{
    int rowDiffDefault = 10;
    arma::rowvec rangeSum = arma::sum(m_radarRdmReal.rows(rowDiffDefault,
                                                        m_param.m_adcSample - rowDiffDefault));
    arma::vec rangeSumCol = rangeSum.as_col();

    if(m_realTimeMapStart)
    {
        int frameDefault = 200;
        double rdmRealMin = rangeSumCol.min();
        m_realTimeMdMap = arma::mat(m_param.m_nChirp,frameDefault);
        m_realTimeMdMap.fill(rdmRealMin);

        m_realTimeMdMap.col(frameDefault - 1) = rangeSumCol;

        m_realTimeMapStart = false;
        return;
    }

    m_realTimeMdMap.cols(0,m_realTimeMdMap.n_cols - 2)
            = m_realTimeMdMap.cols(1,m_realTimeMdMap.n_cols - 1);

    m_realTimeMdMap.col(m_realTimeMdMap.n_cols - 1) = rangeSumCol;
}

/**
 * @brief RadarDataCube类 - convertRdmToMap函数
 * @details 转换成Mat图
 * @note 并不会改变radarRdmReal
 * @return 返回Range Doppler Map矩阵 - cv::Mat
 */
cv::Mat& RadarDataCube::convertRdmToMap()
{
    // realTimeMdMap是实时微多普勒矩阵，数据不应该改变
    auto tempRdmReal = m_radarRdmReal;
    // 创建预处理矩阵
    cv::Mat preMap(m_param.m_adcSample, m_param.m_nChirp, CV_8UC1);

    // 调整值
    unsigned int minSetValue = 130;    // 颜色规整 - 下界
    unsigned int maxSetValue = 210;    // 颜色规整 - 上界

    double rdmRealMax = tempRdmReal.max();
    double rdmRealMin = tempRdmReal.min();
    double rdmRealDiff = rdmRealMax - rdmRealMin;

    tempRdmReal = (tempRdmReal - rdmRealMin) / rdmRealDiff * 254 + 1;
    for(auto it = tempRdmReal.begin(),itEnd = tempRdmReal.end();it != itEnd;++it){
        if((*it) < minSetValue) (*it) = minSetValue;
        else if((*it) > maxSetValue) (*it) = maxSetValue;
    }

    tempRdmReal = (tempRdmReal - minSetValue) / (maxSetValue - minSetValue) * 254 + 1;

    // 创建图像Mat
    for(int i = 0;i < m_param.m_adcSample;++i)
        for(int j = 0;j < m_param.m_nChirp;++j){
            unsigned int tempVal = unsigned(tempRdmReal.at(i,j));
            preMap.at<uchar>(i,j) = tempVal;
        }

    cv::applyColorMap(preMap, m_rdmMap, cv::COLORMAP_JET);
    // 将 RGB 图转化成 BGR 图
    cv::cvtColor(m_rdmMap, m_rdmMap, cv::COLOR_RGB2BGR);
    return m_rdmMap;
}

/**
 * @brief RadarDataCube类 - convertMdToMap函数
 * @details 转换成微多普勒图
 * @note 并不会改变realTimeMdMap
 * @return 返回micro doppler矩阵 - cv::Mat
 */
cv::Mat& RadarDataCube::convertMdToMap()
{
    // realTimeMdMap是实时微多普勒矩阵，数据不应该改变
    auto tempReadTimeMd = m_realTimeMdMap;
    // 创建预处理矩阵
    cv::Mat preMap(m_param.m_nChirp, 200, CV_8UC1);

    // 更新帧
    ++m_frameCount;

    // 前100帧用于调试图像
    if(m_frameCount <= 100){
        if(m_frameCount == 1){
            m_globalMin = tempReadTimeMd.min();
            m_globalMax = tempReadTimeMd.max();
        }
        m_globalMin = std::min(m_globalMin,tempReadTimeMd.min());
        m_globalMax = std::max(m_globalMax,tempReadTimeMd.max());
    }

    // 将值锁定在[globalMdMin,globalMdMax]中
    for(auto it = tempReadTimeMd.begin(),itEnd = tempReadTimeMd.end();it != itEnd;++it){
        if((*it) < m_globalMin) (*it) = m_globalMin;
        else if((*it) > m_globalMax) (*it) = m_globalMax;
    }

    // 转化值
    // 调色上下界
    unsigned int minSetValue = 60;    // 颜色规整 - 上界
    unsigned int maxSetValue = 170;    // 颜色规整 - 下届

    double mdRealDiff = m_globalMax - m_globalMin;
    tempReadTimeMd = (tempReadTimeMd - m_globalMin) / mdRealDiff * 254 + 1;
    tempReadTimeMd = tempReadTimeMd - 15;
    for(auto it = tempReadTimeMd.begin(),itEnd = tempReadTimeMd.end();it != itEnd;++it){
        if((*it) < minSetValue) (*it) = minSetValue;
        else if((*it) > maxSetValue) (*it) = maxSetValue;
    }

    tempReadTimeMd = (tempReadTimeMd - minSetValue) / (maxSetValue - minSetValue) * 254 + 1;

    // 转换到Mat
    for(int i = 0;i < m_param.m_nChirp;++i)
        for(int j = 0;j < 200;++j){
            unsigned int tempVal = unsigned(tempReadTimeMd.at(i,j));
            preMap.at<uchar>(i,j) = tempVal;
        }

    cv::applyColorMap(preMap, m_mdMap, cv::COLORMAP_JET);
    // 将 RGB 图转化成 BGR 图
    cv::cvtColor(m_mdMap, m_mdMap, cv::COLOR_RGB2BGR);
    return m_mdMap;
}


