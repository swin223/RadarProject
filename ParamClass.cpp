#include "ParamClass.h"

unsigned long RadarParam::getFrameBytes() const
{
    // IQ两个通道，每个通道16bit位的数据，因此2*2
    unsigned long singleFrameBytes = m_adcSample * m_nChirp * m_nRx * 2 * 2;
    return singleFrameBytes;
}

ModifyFrame::ModifyFrame(RadarParam &para, int frameLost, UdpPacketParam &udpPara)
{
    m_singleFrameBytes = para.getFrameBytes();
    m_frameLost = frameLost;
    m_udpPackageSize = udpPara.m_bufSize;
    m_udpPackageOffset = udpPara.m_bufOffset;
}

std::pair<int,int> ModifyFrame::getRightByte() const
{
    std::pair<int,int> ret;
    ret.first = m_singleFrameBytes*m_frameLost / (m_udpPackageSize - m_udpPackageOffset);
    ret.second = m_singleFrameBytes*m_frameLost - ret.first*(m_udpPackageSize - m_udpPackageOffset);
    return ret;
}