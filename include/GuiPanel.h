#ifndef REALTIMECOMMUNICATION_GUIPANEL_H
#define REALTIMECOMMUNICATION_GUIPANEL_H

#include "wx/wx.h"
#include "wx/socket.h"
#include "RadarDataCube.h"
#include "wxImagePanel.h"
#include "mathplot.h"
#include <queue>

/**
 * @brief 枚举定义
 * @details 定义所有用到的窗口标志符
 */
enum {
    ID_PAGE1_DCA1000 = 1000,  ///< 对应 Online Page - Run DCA1000按钮
    ID_PAGE1_UDPCT,           ///< 对应 Online Page - UDP CONNECT按钮
    ID_PAGE1_AWR1642,         ///< 对应 Online Page - RUN AWR1642按钮
    ID_PAGE1_UDPDISCT,        ///< 对应 Online Page - UDP DISCONNECT按钮
    PAGE1_SOCKET_ID,          ///< 对应 Online Page - socket事件
    ID_PACKET_PROCESS,        ///< 对应 Online Page - 子线程Packet处理
    ID_PAGE2_RUN,             ///< 对应 Replay Page - 子线程启动事件
    ID_PAGE2_PAUSE,           ///< 对应 Replay Page - 子线程暂停事件
    ID_PAGE2_RESUME,          ///< 对应 Replay Page - 子线程恢复事件
    ID_PAGE2_END,             ///< 对应 Replay Page - 子线程结束事件
    ID_REPLAY_PROCESS,        ///< 对应 Replay Page - GUI线程处理事件
    ID_BIN_PATH,              ///< 对应 Replay Page - Dialog - 设置BIN路径事件
    ID_VIDEO_PATH,            ///< 对应 Replay Page - Dialog - 设置VIDEO路径事件
    ID_RESET,                 ///< 对应 Replay Page - Dialog - RESET事件
    ID_APPLY,                 ///< 对应 Replay Page - Dialog - APPLY事件
    ID_PAGE3_TRAIN,           ///< 对应 Offline Page - 训练集训练事件
    ID_PAGE3_TEST,            ///< 对应 Offline Page - 测试集训练事件
    ID_PAGE3_SVM,             ///< 对应 Offline Page - SVM Model产生事件
    ID_PAGE3_DEMO,            ///< 对应 Offline Page - 单个Single演示事件
    ID_OFFLINE_BIN_PATH,      ///< 对应 Offline Page - Dialog - 设置BIN路径事件
    ID_OFFLINE_RESET,         ///< 对应 Offline Page - Dialog - RESET事件
    ID_OFFLINE_APPLY          ///< 对应 Offline Page - Dialog - APPLY事件
};

/* ---------------------------------------------------- MyApp类 ---------------------------------------------------- */

/**
 * @brief MyApp类
 * 应用类
 */
class MyApp : public wxApp {
public:
    bool OnInit() override;
};

/* --------------------------------------------------- MyFrame类 --------------------------------------------------- */

/**
 * @brief MyFrame类
 * 主界面显示类
 * @details 主要是主界面的显示，所有功能已经被其他类封装
 */
class MyFrame : public wxFrame{
public:
    // 构造函数
    MyFrame(const wxString& title);      ///< 构造函数

    // 其他事件处理函数
    void OnQuit(wxCommandEvent& event);  ///< 退出调用函数
    void OnAbout(wxCommandEvent& event); ///< 关于调用函数

private :

};

/* ------------------------------------------------------- Page 1 -----------------------------------------------------
 ------------------------------------------------- wxOnlinePagePanel类 ----------------------------------------------- */

/**
 * @brief wxOnlinePagePanel类
 * 微多普勒在线数据处理类
 * @details 主要是用于在Frame下的Page1的布局和功能实现
 */
class MyPlotEvent;
class PacketProcessThread;

class wxOnlinePagePanel : public wxPanel
{
public:
    // 构造函数
    wxOnlinePagePanel(wxPanel *parent);         ///< 构造函数

    // 按钮功能函数
    void EnableDCA1000(wxCommandEvent& event);  ///< 启动DCA1000
    void EnableAWR1642(wxCommandEvent& event);  ///< 启动AWR1642
    void ConnectUDP(wxCommandEvent& event);     ///< 进行UDP连接
    void DisconnectUDP(wxCommandEvent& event);  ///< 断开UDP连接

    // Socket触发函数
    void OnSocketEvent(wxSocketEvent& event);   ///< UDP触发事件处理

    // 子线程Packet处理后绘图线程函数
    void PacketThreadProcess(MyPlotEvent& event); ///< 子线程函数-处理后绘图

public:
    // 摄像头捕获相关
    cv::VideoCapture *m_capture;       ///< 摄像头捕捉指针
    std::string m_videoPath;           ///< 视频保存位置
    cv::VideoWriter *m_outputVideo;    ///< 保存输出的video

    // 用于在子线程间传递数据
    std::queue<UINT8 *> m_packetQueue; ///< 用于给另一个线程处理的数据队列

private:
    // 窗口控件相关
    wxTextCtrl *m_logOutPut;           ///< 输出log信息
    wxImagePanel *m_rdPicPanel;        ///< 存放Range-Doppler图
    wxImagePanel *m_mdPicPanel;        ///< 存放Micro-Doppler图
    wxImagePanel *m_cameraPicPanel;    ///< 存放摄像头拍摄图
    FILE *m_logFile;                   ///< log信息输出文件(创建文件以输出) - 用于调试查看是否丢帧
    wxLogStderr *m_logOutput;          ///< log信息输出

    // socket及udp相关
    wxDatagramSocket* m_mySocket;      ///< 用于接收数据的socket(UDP)

    // 雷达信号处理类
    UdpPacketParam* m_udpParam;        ///< UDP参数类

    // 子线程相关
    PacketProcessThread *m_packetProcessThread; ///< 子线程对象-处理UDP Packet

    // 声明私有事件表
    DECLARE_EVENT_TABLE()
};

/**
 * @brief wxOnlinePagePanel 下
 * PacketProcessThread类
 * @details 实际上这个线程类要处理的就是源源不断的UDP数据包
 */
class PacketProcessThread : public wxThread
{
public:
    // 构造函数
    PacketProcessThread(wxOnlinePagePanel *parent); ///< 构造函数

    // 线程启动入口与退出
    virtual void *Entry();                          ///< 线程入口函数
    virtual void OnExit();                          ///< 线程退出函数

private:
    // 父窗口成员
    wxOnlinePagePanel *m_fatherPanel;  ///< 父窗口成员指针

    // socket及udp相关
    bool m_udpPacketSeqFlag;           ///< UDP包序列是否连续标志符
    unsigned long m_prePacketSeq;      ///< UDP包当前的帧序号(理应该)

    // 雷达参数类成员
    RadarParam* m_radarParam;          ///< 雷达参数类
    UdpPacketParam* m_udpParam;        ///< UDP参数类
    RadarDataCube* m_radarCube;        ///< 雷达立方体类
    std::vector<INT16>::iterator m_insertPos;
    ///< 指向雷达立方体类中的迭代器
    bool m_mdMapDrawFlag;              ///< 首次绘制微多普勒图flag
    ModifyFrame* m_modifyFrame;        ///< 修正参数类
    bool m_FirstFixFlag;               ///< 首次修正Flag

    // 存摄像头捕获的图片帧
    cv::Mat *m_singleFramePic;         ///< 摄像头单帧画面
};

/* ------------------------------------------------------- Page 2 -----------------------------------------------------
 ----------------------------------------------- wxBinReplayPagePanel类 --------------------------------------------- */

/**
 * @brief wxBinReplayPagePanel类
 * Bin文件重绘界面类
 * @details 主要是用于在Frame下的Page2的布局和功能实现
 */
class BinReplayThread;

class wxBinReplayPagePanel : public wxPanel
{
public:
    // 构造函数
    wxBinReplayPagePanel(wxPanel *parent);    ///< bin回放开始

    // 按钮功能函数
    void ReplayRun(wxCommandEvent& event);    ///< bin回放开始
    void ReplayPause(wxCommandEvent& event);  ///< bin回放暂停
    void ReplayResume(wxCommandEvent& event); ///< bin回放恢复
    void ReplayEnd(wxCommandEvent& event);    ///< bin回放结束

    // bin回放处理线程函数
    void ReplayThreadProcess(MyPlotEvent& event);  ///< 子线程函数 - bin回放

public:
    // 离线的Bin文件和Video文件路径
    wxString m_binPathStr,m_videoPathStr;     ///< 离线的Bin文件和Video文件路径

private:
    // 窗口控件相关
    wxTextCtrl *m_logOutPut;       ///< 输出log信息框
    wxLogTextCtrl *m_console;      ///< 输出log信息
    wxImagePanel *m_rdPicPanel;    ///< 存放Range-Doppler图
    wxImagePanel *m_mdPicPanel;    ///< 存放Micro-Doppler图
    wxImagePanel *m_cameraPicPanel;///< 存放摄像头拍摄图

    // 子线程相关
    BinReplayThread *m_binReplayThread;   ///< 子线程对象-用于重播文件

};

/**
 * @brief wxBinReplayPagePanel类 下
 * MyPlotEvent类
 * @details 主要是用于在wxEVENTQUEUE存3个wxImage
 */
class MyPlotEvent : public wxEvent
{
public:
    /// 构造函数
    MyPlotEvent(wxEventType eventType,int id) : wxEvent(id,eventType) { }

    /// 析构函数
    ~MyPlotEvent() {}

    /// Clone函数
    virtual wxEvent *Clone() const { return new MyPlotEvent(*this); }

    /// 用于存三个wxImage矩阵
    void SetOfflineImage(wxImage *RdMatrix,wxImage *MdMatrix,wxImage *videoMatrix)
    {
        m_RdImage = RdMatrix;
        m_MdImage = MdMatrix;
        m_videoImage = videoMatrix;
    }

    /// 用于读两个Radar数据生成的wxImage矩阵
    std::pair<wxImage *,wxImage *> GetOfflineRadarPic()
    {
        return std::make_pair(m_RdImage,m_MdImage);
    }

    /// 用于读取视频中的帧数据wxImage矩阵
    wxImage * GetOfflineVideoPic()
    {
        return m_videoImage;
    }

protected:
    wxImage *m_RdImage;           ///< 用于存Range-doppler Image
    wxImage *m_MdImage;           ///< 用于存Micro-doppler Image
    wxImage *m_videoImage;        ///< 用于存视频中的每一帧
};
// 定义事件类型，此处只有一个，如果有多个写多行即可（这里就定义了自定义事件 MY_PLOT_CLICKED）
wxDEFINE_EVENT(MY_PLOT_THREAD,MyPlotEvent);
typedef void (wxEvtHandler::*MyPlotEventFunction)(MyPlotEvent&);
#define wxMyPlotEventHandler(func) wxEVENT_HANDLER_CAST(MyPlotEventFunction,func)

/**
 * @brief wxBinReplayPagePanel类 下
 * wxReplayFileDialog类
 * @details 主要是用于选择bin文件和video文件的消息窗口
 */
class wxReplayFileDialog : public wxDialog
{
public:
    // 构造函数
    wxReplayFileDialog(wxBinReplayPagePanel* parent,const wxString& title); ///< 构造函数

    // 事件处理函数
    void setBinPath(wxCommandEvent& event);   ///< 设置bin文件路径
    void setVideoPath(wxCommandEvent& event); ///< 设置Video文件路径
    void Reset(wxCommandEvent& event);        ///< 重置文件路径
    void Apply(wxCommandEvent& event);        ///< 应用文件路径

private:
    // 窗口控件相关
    wxTextCtrl *m_binPath,*m_videoPath;      ///< bin、video文件路径 - 用于text显示
    wxBinReplayPagePanel *m_father;          ///< 父窗口指针
};

/**
 * @brief wxBinReplayPagePanel 下
 * BinReplayThread类
 * @details 实际上这个线程类要处理的离线的bin文件和Video文件的回放
 */
class BinReplayThread : public wxThread
{
public:
    // 构造函数
    BinReplayThread(wxBinReplayPagePanel *parent); ///< 构造函数

    // 线程启动入口和退出
    virtual void *Entry();                         ///< 线程入口函数
    virtual void OnExit();                         ///< 线程退出函数

public:
    wxBinReplayPagePanel *m_fatherPanel;           ///< 父亲Panel指针
    bool m_mdMapDrawFlagOL;                        ///< 表示开始绘制标志位
};

/* ------------------------------------------------------- Page 3 -----------------------------------------------------
 ------------------------------------------------- wxOfflinePagePanel类 --------------------------------------------- */

/**
 * @brief wxOfflinePagePanel类
 * 微多普勒离线数据处理类
 * @details 主要是用于在Frame下的Page3的布局和功能实现
 */
class wxOfflinePagePanel : public wxPanel
{
public:
    // 构造函数
    wxOfflinePagePanel(wxPanel *parent);          ///< 构造函数

    // 按钮功能函数
    void TrainDataSet(wxCommandEvent& event);     ///< 训练数据集
    void TestDataSet(wxCommandEvent& event);      ///< 训练测试集
    void SvmModel(wxCommandEvent& event);         ///< SVM模型训练
    void SingleBinDemo(wxCommandEvent& event);    ///< 单个Bin文件Demo演示

    // 其他函数
    void refreshAll(arma::rowvec&, wxBitmap&);    ///< 所有图更新

public:
    // 离线的Bin文件和
    wxString m_binPathStr;                        ///< 离线的Bin文件路径

private:
    // 窗口控件相关
    wxTextCtrl *m_logOutPut;                               ///< 输出log信息框
    wxLogTextCtrl *m_console;                              ///< 输出log信息
    mpWindow *m_mdWin,*m_torsoWin,*m_limbsWin,*m_vmdWin;   ///< 曲线图窗
    mpFXYVector *m_torsoCurve,*m_limbsCurve,*m_vmdCurve;   ///< 三个曲线layer
    mpBitmapLayer *m_mdPic;                                ///< 一个图layer
};

/**
 * @brief wxOfflinePagePanel类 下
 * wxOfflineDemoFileDialog类
 * @details 主要是用于选择 Demo bin文件的消息窗口
 */
class wxOfflineDemoFileDialog : public wxDialog
{
public:
    // 构造函数
    wxOfflineDemoFileDialog(wxOfflinePagePanel* parent,const wxString& title); ///< 构造函数

    // 事件处理函数
    void setBinPath(wxCommandEvent& event);   ///< 设置bin文件路径
    void Reset(wxCommandEvent& event);        ///< 重置文件路径
    void Apply(wxCommandEvent& event);        ///< 应用文件路径

private:
    // 窗口控件相关
    wxTextCtrl *m_binPath;                   ///< 离线的Bin Demo文件路径 - 用于text显示
    wxOfflinePagePanel *m_father;            ///< 父窗口指针
};

// todo - 一些new出来的要去清理

#endif //REALTIMECOMMUNICATION_GUIPANEL_H