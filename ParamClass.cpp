#include "ParamClass.h"

/**
 * @brief RadarParam类 - getFrameBytes函数
 * @return 返回一帧数据的字节大小值
 */
unsigned long RadarParam::getFrameBytes() const
{
    // IQ通道 - 2  +  16bit - 2
    unsigned long singleFrameBytes = m_adcSample * m_nChirp * m_nRx * 2 * 2;
    return singleFrameBytes;
}

/**
 * @brief ModifyFrame类 - 构造函数
 * @param para 雷达参数对象
 * @param frameLost 需要丢弃的帧数
 * @param udpPara udp参数对象
 */
ModifyFrame::ModifyFrame(RadarParam &para, int frameLost, UdpPacketParam &udpPara)
{
    m_singleFrameBytes = para.getFrameBytes();
    m_frameLost = frameLost;
    m_udpPackageSize = udpPara.m_bufSize;
    m_udpPackageOffset = udpPara.m_bufOffset;
}

/**
 * @brief ModifyFrame类 - getRightByte函数
 * @return 返回该丢弃的包和剩余字节数的pair对
 */
std::pair<int,int> ModifyFrame::getRightByte() const
{
    std::pair<int,int> ret;
    ret.first = m_singleFrameBytes*m_frameLost / (m_udpPackageSize - m_udpPackageOffset);
    ret.second = m_singleFrameBytes*m_frameLost - ret.first*(m_udpPackageSize - m_udpPackageOffset);
    return ret;
}

