#ifndef PARAMCLASS_H
#define PARAMCLASS_H

#include <utility>

/** RadarParam类
 * @details 单帧参数设定类
 */
struct RadarParam
{
    /** 默认构造函数 */
    RadarParam()
        : m_adcSample(256),
          m_nChirp(128),
          m_nRx(4)
    {}

    /** 含参构造函数 */
    RadarParam(const int nS, const int nC, const int nR)
        : m_adcSample(nS),
          m_nChirp(nC),
          m_nRx(nR)
    {}

    /** RadarParam类 - getFrameBytes函数
    * @return 返回一帧数据的字节大小值
    */
    unsigned long getFrameBytes() const;

    int m_adcSample;                      ///< 采样个数
    int m_nChirp;                         ///< 扫频个数
    int m_nRx;                            ///< 天线个数
};

/** UdpPacketParam类
 * @details UDP参数设定类
 * @note 16bit - bufScale(2)  头10个包为信息包 - bufOffset(10)
 */
struct UdpPacketParam
{
    /** 默认构造函数 */
    UdpPacketParam()
        : m_bufSize(1466),
          m_bufScale(2),
          m_bufOffset(10)
    {}

    /** 含参构造函数 */
    UdpPacketParam(const int bs, const int bsl, const int bo)
        : m_bufSize(bs),
          m_bufScale(bsl),
          m_bufOffset(bo)
    {}

    int m_bufSize;                        ///< udp一次读入的buf大小
    int m_bufScale;                       ///< 与16bit的缩放关系比例
    int m_bufOffset;                      ///< udp Packet的偏移量(正式数据)
};

/** ModifyFrame类
 * @details 参数修正类
 */
class ModifyFrame
{
public:
    /** ModifyFrame类 - 构造函数
     * @param para 雷达参数对象
     * @param frameLost 需要丢弃的帧数
     * @param udpPara udp参数对象
     */
    ModifyFrame(RadarParam &para, int frameLost, UdpPacketParam &udpPara);

    /** ModifyFrame类 - getRightByte函数
     * @return 返回该丢弃的包和剩余字节数的pair对
     */
    std::pair<int,int> getRightByte() const;

private:
    unsigned long m_singleFrameBytes;       ///< 一帧字节数
    int m_frameLost;                        ///< 需要丢弃的frame个数
    int m_udpPackageSize;                   ///< udp Packet字节大小
    int m_udpPackageOffset;                 ///< udp Packet的字节偏移
};

#endif //PARAMCLASS_H