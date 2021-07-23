#ifndef GUIPANEL_H
#define GUIPANEL_H

#include <queue>
#include <wx/dir.h>
#include <wx/dirdlg.h>
#include "wx/wx.h"
#include "wx/socket.h"
#include "RadarDataCube.h"
#include "wxImagePanel.h"
#include "mathplot.h"
#include "MySvmClass.h"
#include "wx/msgqueue.h"

/** 枚举定义
 * @details 定义所有用到的窗口标志符
 */
enum {
    ID_ONLINE_DCA1000 = 1000,  ///< 对应 Online Page - Run DCA1000按钮
    ID_ONLINE_UDPCT,           ///< 对应 Online Page - UDP CONNECT按钮
    ID_ONLINE_AWR1642,         ///< 对应 Online Page - RUN AWR1642按钮
    ID_ONLINE_UDPDISCT,        ///< 对应 Online Page - UDP DISCONNECT按钮
    ID_ONLINE_SOCKET,          ///< 对应 Online Page - socket事件
    ID_ONLINE_PROCESS,         ///< 对应 Online Page - 子线程Packet处理
    ID_REPLAY_RUN,             ///< 对应 Replay Page - 子线程启动事件
    ID_REPLAY_PAUSE,           ///< 对应 Replay Page - 子线程暂停事件
    ID_REPLAY_RESUME,          ///< 对应 Replay Page - 子线程恢复事件
    ID_REPLAY_END,             ///< 对应 Replay Page - 子线程结束事件
    ID_REPLAY_PROCESS,         ///< 对应 Replay Page - GUI线程处理事件
    ID_REPLAY_BIN_PATH,        ///< 对应 Replay Page - Dialog - 设置BIN路径事件
    ID_REPLAY_VIDEO_PATH,      ///< 对应 Replay Page - Dialog - 设置VIDEO路径事件
    ID_REPLAY_RESET,           ///< 对应 Replay Page - Dialog - RESET事件
    ID_REPLAY_APPLY,           ///< 对应 Replay Page - Dialog - APPLY事件
    ID_OFFLINE_TRAIN,          ///< 对应 Offline Page - 训练集训练事件
    ID_OFFLINE_TEST,           ///< 对应 Offline Page - 测试集训练事件
    ID_OFFLINE_SVM,            ///< 对应 Offline Page - SVM Model产生事件
    ID_OFFLINE_DEMO,           ///< 对应 Offline Page - 单个Single演示事件
    ID_OFFLINE_BIN_PATH,       ///< 对应 Offline Page - Dialog - 设置BIN路径事件
    ID_OFFLINE_RESET,          ///< 对应 Offline Page - Dialog - RESET事件
    ID_OFFLINE_APPLY           ///< 对应 Offline Page - Dialog - APPLY事件
};

/* ---------------------------------------------------- MyApp类 ---------------------------------------------------- */

/** 应用类 */
class MyApp : public wxApp
{
public:
    /** MyApp程序入口函数
     * @return bool类型
     * @retval 1. true 运行成功
     * @retval 2. false 运行失败
     */
    bool OnInit() override;
};

/* --------------------------------------------------- MyFrame类 --------------------------------------------------- */

/** 主界面显示类
 * @details 主要是主界面的显示，所有功能已经被其他类封装
 */
class MyFrame : public wxFrame
{
public:
    /** 含参构造函数
     * @param title 主窗口名字
     */
    MyFrame(const wxString& title);

    // 其他事件处理函数
    /** OnQuit退出调用函数
     * @param event 触发事件
     */
    void OnQuit(wxCommandEvent& event);

    /** OnAbout关于调用函数
     * @param event 触发事件
     */
    void OnAbout(wxCommandEvent& event);
};

/* ------------------------------------------------------- Page 1 -----------------------------------------------------
 ------------------------------------------------- wxOnlinePagePanel类 ----------------------------------------------- */

class MyPlotEvent;
class PacketProcessThread;

/** 微多普勒在线数据处理类
 * @details 主要是用于在Frame下的Page1的布局和功能实现
 */
class wxOnlinePagePanel : public wxPanel
{
public:
    /** 含参构造函数
     * @param parent 父窗口指针
     */
    wxOnlinePagePanel(wxPanel *parent);

    // 按钮功能函数
    /** 启动DCA1000
     * @param 触发事件
     */
    void OnEnableDCA1000Click(wxCommandEvent& event);

    /** 启动AWR1642
     * @param 触发事件
     */
    void OnEnableAWR1642Click(wxCommandEvent& event);

    /** 进行UDP连接
     * @param 触发事件
     */
    void OnConnectUDPClick(wxCommandEvent& event);

    /** 进行UDP断开
     * @param 触发事件
     */
    void OnDisconnectUDPClick(wxCommandEvent& event);

    /** Socket触发函数
     * @param 触发事件
     */
    void OnSocketEvent(wxSocketEvent& event);

    /** 子线程Packet处理后绘图线程函数
     * @param 触发事件
     */
    void OnReceivePacketProcessThreadEvent(MyPlotEvent& event);

public:
    // 摄像头捕获相关
    cv::VideoCapture *m_capture;       ///< 摄像头捕捉指针
    std::string m_videoPath;           ///< 视频保存位置
    cv::VideoWriter *m_outputVideo;    ///< 保存输出的video

    // 使用wxMessageQueue用于在子线程间传递数据
    /// 用于给另一个线程处理的数据队列
    wxMessageQueue<UINT8 *> m_packetMsgQueue;

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

/** 处理的就是源源不断的UDP数据包的线程类 */
class PacketProcessThread : public wxThread
{
public:
    /** 含参构造函数
     * @param parent 父窗口指针
     */
    PacketProcessThread(wxOnlinePagePanel *parent);

    // 线程启动入口与退出
    /** 线程入口函数 */
    virtual void *Entry();

    /** 退出函数 */
    virtual void OnExit();

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
    std::vector<INT16>::iterator m_insertPos; ///< 指向雷达立方体类中的迭代器

    bool m_mdMapDrawFlag;              ///< 首次绘制微多普勒图flag
    ModifyFrame* m_modifyFrame;        ///< 修正参数类
    bool m_FirstFixFlag;               ///< 首次修正Flag

    // 存摄像头捕获的图片帧
    cv::Mat *m_singleFramePic;         ///< 摄像头单帧画面
};

/* ------------------------------------------------------- Page 2 -----------------------------------------------------
 ----------------------------------------------- wxBinReplayPagePanel类 --------------------------------------------- */

class BinReplayThread;

/** Bin文件重绘界面类
 * @details 主要是用于在Frame下的Page2的布局和功能实现
 */
class wxBinReplayPagePanel : public wxPanel
{
public:
    /** 含参构造函数
     * @param parent 父窗口指针
     */
    wxBinReplayPagePanel(wxPanel *parent);

    // 按钮功能函数
    /** bin回放开始函数
     * @param event 触发事件
     */
    void OnReplayRunClick(wxCommandEvent& event);

    /** bin回放暂停函数
     * @param event 触发事件
     */
    void OnReplayPauseClick(wxCommandEvent& event);

    /** bin回放恢复函数
    * @param event 触发事件
    */
    void OnReplayResumeClick(wxCommandEvent& event);

    /** bin回放结束函数
     * @param event 触发事件
     */
    void OnReplayEndClick(wxCommandEvent& event);

    /** bin回放处理线程函数
     * @param event 触发事件
     */
    void ReplayThreadProcess(MyPlotEvent& event);

public:
    // 离线的Bin文件和Video文件路径
    wxString m_binPathStr;       ///< 离线的Bin文件路径
    wxString m_videoPathStr;     ///< 离线的Video文件路径

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

/** 主要是用于在wxEVENTQUEUE存3个wxImage的指针 */
class MyPlotEvent : public wxEvent
{
public:
    /** 含参构造函数 */
    MyPlotEvent(wxEventType eventType, int id)
        : wxEvent(id, eventType)
    { }

    /** 析构函数 */
    ~MyPlotEvent()
    {}

    /** Clone函数 */
    virtual wxEvent *Clone() const { return new MyPlotEvent(*this); }

    /** 用于存三个wxImage矩阵
     * @param RdMatrix 用于存Range-doppler Image的wxImage指针
     * @param MdMatrix 用于存Micro-doppler Image的wxImage指针
     * @param videoMatrix 用于存视频中的每一帧的wxImage指针
     */
    void SetOfflineImage(wxImage* RdMatrix, wxImage* MdMatrix, wxImage* videoMatrix)
    {
        m_RdImage = RdMatrix;
        m_MdImage = MdMatrix;
        m_videoImage = videoMatrix;
    }

    /** 用于读两个Radar数据生成的wxImage矩阵
     * @return 用于存Range-doppler Image和Micro-doppler Image的wxImage指针的pair对
     */
    std::pair<wxImage*, wxImage*> GetOfflineRadarPic()
    {
        return std::make_pair(m_RdImage, m_MdImage);
    }

    /** 用于读取视频中的帧数据wxImage矩阵
     * @return 用于存视频中的每一帧的wxImage指针
     */
    wxImage* GetOfflineVideoPic()
    {
        return m_videoImage;
    }

protected:
    wxImage* m_RdImage;      ///< 用于存Range-doppler Image
    wxImage* m_MdImage;      ///< 用于存Micro-doppler Image
    wxImage* m_videoImage;   ///< 用于存视频中的每一帧
};

// 定义事件类型，此处只有一个，如果有多个写多行即可（这里就定义了自定义事件 MY_PLOT_CLICKED）
wxDEFINE_EVENT(MY_PLOT_THREAD, MyPlotEvent);
typedef void (wxEvtHandler::* MyPlotEventFunction)(MyPlotEvent&);
#define wxMyPlotEventHandler(func) wxEVENT_HANDLER_CAST(MyPlotEventFunction,func)

/** 用于选择bin文件和video文件的对话框窗口 */
class wxReplayFileDialog : public wxDialog
{
public:
    /** 含参构造函数
     * @param parent 父窗口指针
     * @param title 窗口名称
     */
    wxReplayFileDialog(wxBinReplayPagePanel* parent, const wxString& title);

    // 事件处理函数
    /** 设置bin文件路径
     * @param event 触发事件
     */
    void OnSetBinPathClick(wxCommandEvent& event);

    /** 设置Video文件路径
     * @param event 触发事件
     */
    void OnSetVideoPathClick(wxCommandEvent& event);

    /** 重置文件路径
     * @param event 触发事件
     */
    void OnResetClick(wxCommandEvent& event);

    /** 应用文件路径
     * @brief wxReplayFileDialog类 - Apply函数
     * @param event 触发事件
     */
    void OnApplyClick(wxCommandEvent& event);

private:
    // 窗口控件相关
    wxTextCtrl* m_binPath;                  ///< bin文件路径 - 用于text显示
    wxTextCtrl* m_videoPath;                ///< video文件路径 - 用于text显示
    wxBinReplayPagePanel* m_father;         ///< 父窗口指针
};

/** 处理的离线的bin文件和Video文件的线程类 */
class BinReplayThread : public wxThread
{
public:
    /** 含参构造函数
    * @param parent 父窗口指针
    */
    BinReplayThread(wxBinReplayPagePanel* parent);

    // 线程启动入口和退出
    /** 线程入口函数 */
    virtual void* Entry();

    /** 线程退出函数 */
    virtual void OnExit();

public:
    wxBinReplayPagePanel* m_fatherPanel;           ///< 父亲Panel指针

    // qu : 下面这个变量的定义，md什么意思？OL什么意思，请在注释里面对应说清楚，或者把变量名字修改完整
    // note : 这里原先是m_mdMapDrawFlagOL，现在是m_mdMapDrawFlag
    // 之前由于所有参数写在一起，所以OL是offline的意思，为了区分Online Page的m_mdMapDrawFlag标志
    // 但是现在位于不同的类中，不需要OL来区分;md表示Micro doppler的意思
    // 这个flag用于表示是否开始绘制微多普勒图？因为要用前100帧来自适应图像颜色，其为true后，往后接收100帧用于自适应。
    bool m_mdMapDrawFlag;                         ///< 表示开始绘制标志位
};

/* ------------------------------------------------------- Page 3 -----------------------------------------------------
 ------------------------------------------------- wxOfflinePagePanel类 --------------------------------------------- */

/** 微多普勒离线数据处理类
 * @details 主要是用于在Frame下的Page3的布局和功能实现
 */
class wxOfflinePagePanel : public wxPanel
{
    // 定义友元类(用以访问图窗)
    friend class OfflineFunction;
public:
    /** 含参构造函数
     * @param parent 父窗口指针
     */
    wxOfflinePagePanel(wxPanel *parent);

    // 按钮功能函数
    /** 训练数据集
     * @param event 触发事件
     */
    void TrainDataSet(wxCommandEvent& event);

    /** 训练测试集
     * @param event 触发事件
     */
    void TestDataSet(wxCommandEvent& event);

    /** SVM模型训练
     * @param event 触发事件
     */
    void SvmModel(wxCommandEvent& event);

    /** 单个Bin文件Demo
     * @param event 触发事件
     */
    void SingleBinDemo(wxCommandEvent& event);

    /** 所有图更新
     * @note 目前尚未使用
     */
    void RefreshAll(arma::rowvec&, wxBitmap&);

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

/** 线程类要处理的训练集和数据集的大量文件提取(为了不卡死主线程) */
class FeatureExtraThread : public wxThread
{
public:
    /** 含参构造函数
     * @param parent 父窗口指针
     */
    FeatureExtraThread(wxOfflinePagePanel *parent);

    // 线程启动入口和退出
    /** 线程入口函数 */
    virtual void* Entry();

    /** 线程退出函数 */
    virtual void OnExit();

public:
    wxOfflinePagePanel* m_fatherPanel;                ///< 父亲Panel指针
    bool trainFlag;                                   ///< 标记处理训练集(1)/测试集(0)
};

/** 用于选择 Demo bin文件的对话框窗口 */
class wxOfflineDemoFileDialog : public wxDialog
{
public:
    /** 含参构造函数
     * @param parent 父窗口指针
     * @param title 窗口名称
     */
    wxOfflineDemoFileDialog(wxOfflinePagePanel* parent, const wxString& title);

    // 事件处理函数
    /** 设置bin文件路径
     * @param event 触发事件
     */
    void OnSetBinPathClick(wxCommandEvent& event);

    /** 重置文件路径
     * @param event 触发事件
     */
    void OnResetClick(wxCommandEvent& event);

    /** 应用文件路径
     * @param event 触发事件
     */
    void OnApplyClick(wxCommandEvent& event);

private:
    // 窗口控件相关
    wxTextCtrl *m_binPath;                   ///< 离线的Bin Demo文件路径 - 用于text显示
    wxOfflinePagePanel *m_father;            ///< 父窗口指针
};

/** 实现Offline Page下的所有功能 */
class OfflineFunction
{
public:
    /** 含参构造函数
     * @param parent 父窗口指针
     */
    OfflineFunction(wxOfflinePagePanel *parent);

    /** 获取文件夹下的全文件名以及对应的分类
     * @details 为后续训练集和测试集bin文件读取及提取特征做预备
     */
    void GetFileNamesAndTag();

    /** 训练集处理
     * @details 提取特征 + 得到标准化最大最小值文件(用于测试集处理) + 输出
     */
    void TrainSetProcess();

    /** 测试集处理
     * @details 提取特征 + 标准化(按最大最小值文件标准化) + 输出
     */
    void TestSetProcess();

    /** svm预测精度
     * @details 通过读取testData.txt和trainData.txt完成svm模型创建及输出
     */
    void SvmPrediction();

    /** 单个Bin文件Demo演示具体实现代码
     * @details 在图窗中输出微多普勒图像及特征曲线
     */
    void SingleBinProcess(std::string binFileNameStr);

private:
    /** 训练集数据集数据标准化
     * @param sampleFeatureMat 训练集的特征值矩阵
     * @details 对训练集中提取的特征数据进行数据标准化，会输出一个最大最小值文件来用于对测试集也进行标准化
     */
    void TrainSetNormalized(arma::mat &sampleFeatureMat);

    /** 测试集数据集数据标准化
     * @param sampleFeatureMat 测试集的特征值矩阵
     * @details 对测试集中提取的特征数据进行数据标准化，会读取训练集标准化后输出的一个最大最小值文件来用于对测试集进行标准化
     */
    void TestSetNormalized(arma::mat &sampleFeatureMat);

    /** 用于输出特征值数据到txt文件
     * @param sampleFeatureMat 训练集/测试集特征值矩阵
     * @param outputFileName 输出的文件名
     */
    void PrintFeatureData(arma::mat& sampleFeatureMat,const std::string& outputFileName);

private:
    DividePara m_dividePara;                 ///< 数据集划分参数
    wxOfflinePagePanel *m_father;            ///< 父窗口指针
    wxArrayString m_filesArray;              ///< 对应文件夹下的全文件名数组
    std::vector<int> m_tagArray;             ///< 对应文件夹下的全文件对应的分类类别
};

// todo - 一些new出来的要去清理

#endif //GUIPANEL_H
