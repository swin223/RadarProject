#include "GuiPanel.h"
#include <wx/process.h>
#include <algorithm>
#include <wx/file.h>
#include <wx/wfstream.h>
#include <wx/notebook.h>
#include <memory>

/* ----------------------------------------------------- 实现App ----------------------------------------------------- */
IMPLEMENT_APP(MyApp)

/**
 * @brief MyApp程序入口函数
 * @return bool类型
 * @retval 1. true 运行成功
 * @retval 2. false 运行失败
 */
bool MyApp::OnInit()
{
    MyFrame *frame = new MyFrame(wxT("Real-Time Communication"));
    frame->Show(true);
    return true;
}

/* ---------------------------------------------------- MyFrame类 ---------------------------------------------------- */

/**
 * @brief MyFrame类 - 构造函数
 * @param title 主窗口名字
 */
MyFrame::MyFrame(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(1400, 800))
{
    // 创建菜单项 - 文件
    wxMenu *fileMenu = new wxMenu;
    wxMenu *aboutMenu = new wxMenu;
    fileMenu->Append(wxID_EXIT,wxT("Exit"),wxT("Exit the software"));
    aboutMenu->Append(wxID_ABOUT,wxT("About"),wxT("Message about the software"));
    wxMenuBar *menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, wxT("&File"));
    menuBar->Append(aboutMenu, wxT("&Help"));
    SetMenuBar(menuBar);
    // 添加两个状态条
    CreateStatusBar(2);
    SetStatusText(wxT("Real Time Communication"));
    // 创建页面notebook - 用于存3个Page
    wxNotebook *notebook = new wxNotebook(this, wxID_ANY);
    // 创建3个功能互异的Page页面
    wxPanel *onlinePage = new wxPanel(notebook,wxID_ANY);   // 实时功能聚集页
    wxPanel *replayPage = new wxPanel(notebook,wxID_ANY);   // bin文件回播功能聚集页
    wxPanel *offlinePage = new wxPanel(notebook,wxID_ANY);  // 离线功能聚集页
    notebook->AddPage(onlinePage,wxT("Online Page"), true);
    notebook->AddPage(replayPage,wxT("Replay Page"),false);
    notebook->AddPage(offlinePage,wxT("Offline Page"), false);

    // 构建三个Page类置于界面上
    // Page1 - 实时功能聚集页
    wxOnlinePagePanel *onlinePagePanel= new wxOnlinePagePanel(onlinePage);
    wxBoxSizer *hBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    hBoxSizer1->Add(onlinePagePanel,1,wxEXPAND | wxALL, 5);
    wxBoxSizer *vBoxSizer1 = new wxBoxSizer(wxVERTICAL);
    vBoxSizer1->Add(hBoxSizer1,1,wxEXPAND | wxALL,5);
    onlinePage->SetSizer(vBoxSizer1);
    // Page2 - bin文件回播功能聚集页
    wxBinReplayPagePanel *binReplayPagePanel = new wxBinReplayPagePanel(replayPage);
    wxBoxSizer *hBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    hBoxSizer2->Add(binReplayPagePanel,1,wxEXPAND | wxALL, 5);
    wxBoxSizer *vBoxSizer2 = new wxBoxSizer(wxVERTICAL);
    vBoxSizer2->Add(hBoxSizer2,1,wxEXPAND | wxALL,5);
    replayPage->SetSizer(vBoxSizer2);
    // Page3 - 离线功能聚集页
    wxOfflinePagePanel *offlinePagePanel = new wxOfflinePagePanel(offlinePage);
    wxBoxSizer *hBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    hBoxSizer3->Add(offlinePagePanel,1,wxEXPAND | wxALL, 5);
    wxBoxSizer *vBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    vBoxSizer3->Add(hBoxSizer3,1,wxEXPAND | wxALL,5);
    offlinePage->SetSizer(vBoxSizer3);

    // 添加动态事件响应
    Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED,wxCommandEventHandler(MyFrame::OnQuit));
    Connect(wxID_ABOUT, wxEVT_COMMAND_MENU_SELECTED,wxCommandEventHandler(MyFrame::OnAbout));

    // 在屏幕中居中显示
    Centre();
}

/**
 * @brief MyFrame类 - OnQuit函数
 * @param event 触发事件
 */
void MyFrame::OnQuit(wxCommandEvent& event)
{
    // 释放主窗口
    Close();
}

/**
 * @brief MyFrame类 - OnAbout函数
 * @param event 触发事件
 */
void MyFrame::OnAbout(wxCommandEvent& event)
{
    // 产生消息框
    wxString msgAbout;
    msgAbout.Printf(wxT("Real Time UDP Communication Test"));
    wxMessageBox(msgAbout, wxT("Related Information"),wxOK | wxICON_INFORMATION | wxCENTRE, this);
}

/* ----------------------------------------------------- Page 1 ------------------------------------------------------
 ------------------------------------------------ wxOnlinePagePanel类 ------------------------------------------------*/

// wxOnlinePagePanel类私有事件声明表
BEGIN_EVENT_TABLE(wxOnlinePagePanel,wxPanel)
    EVT_SOCKET(PAGE1_SOCKET_ID, wxOnlinePagePanel::OnSocketEvent)
END_EVENT_TABLE()

/**
 * @brief wxOnlinePagePanel类 - 构造函数
 * @param parent 父窗口指针
 */
wxOnlinePagePanel::wxOnlinePagePanel(wxPanel *parent)
                 : wxPanel(parent)
{
    /* ---------------------------------------------- 控件设定 + 页面布局 ----------------------------------------------- */
    // 按钮
    m_logOutPut = new wxTextCtrl(this, wxID_ANY, wxT(""),wxPoint(-1, -1), wxSize(-1, -1), wxTE_MULTILINE);
    wxButton *btDca1000 = new wxButton(this, ID_PAGE1_DCA1000, wxT("RUN DCA1000"),wxDefaultPosition,wxSize(150,30));
    wxButton *btUdpConnect = new wxButton(this, ID_PAGE1_UDPCT, wxT("UDP CONNECT"),wxDefaultPosition,wxSize(150,30));
    wxButton *btAwr1642 = new wxButton(this, ID_PAGE1_AWR1642, wxT("RUN AWR1642"),wxDefaultPosition,wxSize(150,30));
    wxButton *btUdpDisconnect = new wxButton(this, ID_PAGE1_UDPDISCT, wxT("UDP DISCONNECT"),wxDefaultPosition,wxSize(150,30));
    wxStaticBox *sBox = new wxStaticBox(this,wxID_ANY,wxT("Function Button"));
    // 图窗和静态文本
    m_rdPicPanel = new wxImagePanel(this);
    m_mdPicPanel = new wxImagePanel(this);
    m_cameraPicPanel = new wxImagePanel(this);
    wxStaticText *rdPicText = new wxStaticText(this,wxID_STATIC,wxT("Range-Doppler Map"));
    wxStaticText *mdPicText = new wxStaticText(this,wxID_STATIC,wxT("Micro-Doppler Map"));
    wxStaticText *cameraPicText = new wxStaticText(this,wxID_STATIC,wxT("Camera Picture"));
    // 布局控件
    wxStaticBoxSizer * sBoxSizer = new wxStaticBoxSizer(sBox,wxVERTICAL);
    sBoxSizer->Add(btDca1000,0,wxALIGN_CENTER_HORIZONTAL | wxALL,10);
    sBoxSizer->Add(btUdpConnect,0,wxALIGN_CENTER_HORIZONTAL | wxALL,10);
    sBoxSizer->Add(btAwr1642,0,wxALIGN_CENTER_HORIZONTAL | wxALL,10);
    sBoxSizer->Add(btUdpDisconnect,0,wxALIGN_CENTER_HORIZONTAL | wxALL,10);
    wxBoxSizer *rdBoxSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *mdBoxSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *cameraBoxSizer = new wxBoxSizer(wxVERTICAL);
    rdBoxSizer->Add(rdPicText,0,wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
    rdBoxSizer->Add(m_rdPicPanel,1,wxEXPAND | wxALL, 5);
    mdBoxSizer->Add(mdPicText,0,wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
    mdBoxSizer->Add(m_mdPicPanel,1,wxEXPAND | wxALL, 5);
    cameraBoxSizer->Add(cameraPicText,0,wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
    cameraBoxSizer->Add(m_cameraPicPanel,1,wxEXPAND | wxALL, 5);
    wxBoxSizer *hBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    hBoxSizer->Add(rdBoxSizer,1,wxEXPAND | wxALL, 5);
    hBoxSizer->Add(mdBoxSizer,1,wxEXPAND | wxALL, 5);
    hBoxSizer->Add(cameraBoxSizer,1,wxEXPAND | wxALL, 5);
    hBoxSizer->Add(sBoxSizer,0,wxALIGN_CENTER_VERTICAL | wxALL,5);
    wxBoxSizer *vBoxSizer = new wxBoxSizer(wxVERTICAL);
    vBoxSizer->Add(hBoxSizer,3,wxEXPAND | wxALIGN_RIGHT | wxRIGHT,20);
    vBoxSizer->Add(m_logOutPut,2,wxEXPAND | wxALL,5);
    // 绑定布局控件和this窗口
    this->SetSizer(vBoxSizer);

    // 添加动态事件响应
    Connect(ID_PAGE1_DCA1000, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxOnlinePagePanel::EnableDCA1000));
    Connect(ID_PAGE1_UDPCT, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxOnlinePagePanel::ConnectUDP));
    Connect(ID_PAGE1_AWR1642, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxOnlinePagePanel::EnableAWR1642));
    Connect(ID_PAGE1_UDPDISCT, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxOnlinePagePanel::DisconnectUDP));

    /* -------------------------------------------------- 参数初始化 --------------------------------------------------- */
    // 摄像头捕获参数初始化
    m_capture = new cv::VideoCapture(0);             // 绑定摄像头
    m_videoPath = "RealTimeVideo\\test.avi";               // 存视频的路径
    m_outputVideo = new cv::VideoWriter;                   // 视频输出

    // 雷达信号处理类相关变量初始设置
    m_udpParam = new UdpPacketParam;                       // UdpPacketParam对象初始化
}

/**
 * @brief PacketProcessThread类 - 构造函数
 * @param parent 父窗口指针
 */
PacketProcessThread::PacketProcessThread(wxOnlinePagePanel *parent) : wxThread(wxTHREAD_DETACHED)
{
    // 父窗口成员
    m_fatherPanel = parent;

    // socket及udp参数初始化
    m_udpPacketSeqFlag = false;                           // UDP包序列是否连续标志符

    // 雷达信号处理类相关变量初始设置
    m_radarParam = new RadarParam;                        // RadarParam对象初始化
    m_udpParam = new UdpPacketParam;                      // UdpPacketParam对象初始化
    m_radarCube = new RadarDataCube(*m_radarParam);    // RadarDataCube对象初始化
    m_insertPos = m_radarCube->getFrame().begin();        // 定义插入帧数据中的位置初始化
    m_mdMapDrawFlag = true;                               // 表示开始绘制微多普勒图
    int needLostFrame = 5;                                // 需要丢弃的帧数 - 用于初始化参数修正类
    m_modifyFrame = new ModifyFrame(*m_radarParam,needLostFrame,*m_udpParam);
    m_FirstFixFlag = true;                                // 首次修正Flag

    // 存摄像头捕获的图片帧初始化
    m_singleFramePic = new cv::Mat;                       // 摄像头捕捉的单帧初始化
}

/**
 * @brief PacketProcessThread类 - 退出函数
 */
void PacketProcessThread::OnExit()
{

}

/**
 * @brief PacketProcessThread类 - 线程启动函数
 */
wxThread::ExitCode PacketProcessThread::Entry()
{
    // 线程自启动开始一直检查是否有包 - 有则处理无则等待
    while(!TestDestroy()){

        // 若包队列无UDP包数据
        if(m_fatherPanel->m_packetQueue.empty()) continue;

        // 以下情况 - 包队列有UDP包数据
        UINT8 * udpPacketPtr = m_fatherPanel->m_packetQueue.front();
        m_fatherPanel->m_packetQueue.pop();

        // 由于是小端模式输入，seqNum表示序号 - 只取前两个
        unsigned long seqNum = udpPacketPtr[0] + udpPacketPtr[1] * 256;

        // 丢弃前面一些包
        if(m_FirstFixFlag && seqNum <= m_modifyFrame->getRightByte().first)
        {
            delete[] udpPacketPtr;
            continue;
        }

        // 基于修正类的正确包和偏移量
        // 现在Seq是modifyFrame->getRightByte().first + 1
        if (m_FirstFixFlag)
        {
            int errorOffset = m_modifyFrame->getRightByte().second;
            m_insertPos = std::copy_n(reinterpret_cast<int16_t *>(udpPacketPtr + m_udpParam->m_bufOffset + errorOffset),
                                    (m_udpParam->m_bufSize - m_udpParam->m_bufOffset - errorOffset) / m_udpParam->m_bufScale,
                                    m_insertPos);
            m_FirstFixFlag = false;  // 后面就正常了
            m_prePacketSeq = m_modifyFrame->getRightByte().first + 1;
            delete[] udpPacketPtr;
            continue;
        }

        // 判断udp包是否是连续的
        // 重置
        m_udpPacketSeqFlag = false;
        if ( seqNum == (m_prePacketSeq + 1) % 65536)
        {
            m_udpPacketSeqFlag = true;
            m_prePacketSeq = seqNum;
        }

        // 输出日志
        wxString outMessage;
        outMessage.Printf(wxT(" 当前帧号 = %d ,  %d"),seqNum,m_udpPacketSeqFlag);
        wxLogMessage(outMessage);

        // 普通处理
        if(m_udpPacketSeqFlag)
        {
            // 帧数据流目前还剩余多少可以存放
            auto remainLength = m_radarCube->getFrame().end() - m_insertPos;
            // 分支1 - 能存下当前整个数据包
            if (remainLength >= (m_udpParam->m_bufSize - m_udpParam->m_bufOffset) / m_udpParam->m_bufScale)
            {
                m_insertPos = std::copy_n(reinterpret_cast<int16_t *>(udpPacketPtr + m_udpParam->m_bufOffset),      // 源地址
                                        (m_udpParam->m_bufSize - m_udpParam->m_bufOffset) / m_udpParam->m_bufScale,   // 复制元素个数
                                        m_insertPos);                                                              // 目的起始地址
            }
            // 分支2 - 帧数据流目前没有空间存下当前所有数据
            else
            {
                // 将一个UDP包的部分内容填充进帧数据流
                m_insertPos = std::copy_n(reinterpret_cast<int16_t *>(udpPacketPtr + m_udpParam->m_bufOffset),
                                        remainLength,
                                        m_insertPos);

                // 保存相应的数据到bin文件中
                // 创建一个用于保存整数帧数据的data.bin文件 - 存在即使用append
                wxFile saveDataFile("data.bin",wxFile::write_append);
                // 使用wxFileOutputStream输出数据流 - 与创建出来的bin文件进行绑定
                wxFileOutputStream dataToFile(saveDataFile);
                dataToFile.Write(&(m_radarCube->getFrame()[0]),m_radarParam->getFrameBytes());

                // 雷达数据处理
                // 帧数据流存满 - 进行相应处理
                m_radarCube->creatCube();
                m_radarCube->creatRdm();
                // 用Bitmap创建wxImage对象
                int RdCols = m_radarCube->convertRdmToMap().cols, RdRows = m_radarCube->convertRdmToMap().rows;
                // 使用malloc分配一块动态内存 - 目的在于独立出这块图的数据
                void *RdData = malloc(3 * RdCols * RdRows);
                memcpy(RdData, (void *) m_radarCube->convertRdmToMap().data, 3 * RdCols * RdRows);
                wxImage *RdImage = new wxImage(RdCols, RdRows, (uchar *) RdData, false);

                // 微多普勒图也进行更新
                if (m_mdMapDrawFlag)
                {
                    m_radarCube->setFlagForMap();
                    m_mdMapDrawFlag = false;
                }

                m_radarCube->updateMicroMap();
                int MdCols = m_radarCube->convertMdToMap().cols, MdRows = m_radarCube->convertMdToMap().rows;
                // 使用malloc分配一块动态内存 - 目的在于独立出这块图的数据
                void *MdData = malloc(3 * MdCols * MdRows);
                memcpy(MdData, (void *) m_radarCube->convertMdToMap().data, 3 * MdCols * MdRows);
                wxImage *MdImage = new wxImage(MdCols, MdRows, (uchar *) MdData, false);

                // 图像也进行存储
                (*(m_fatherPanel->m_capture)) >> (*m_singleFramePic);
                // 存图像
                if (m_fatherPanel->m_outputVideo->isOpened()) {
                    (*m_fatherPanel->m_outputVideo) << (*m_singleFramePic);
                }
                // 转颜色
                cv::cvtColor(*m_singleFramePic, *m_singleFramePic, cv::COLOR_RGB2BGR);

                int VideoCols = m_singleFramePic->cols, VideoRows = m_singleFramePic->rows;
                // 使用malloc分配一块动态内存 - 目的在于独立出这块图的数据
                void *VideoData = malloc(3 * VideoCols * VideoRows);
                memcpy(VideoData, (void *) m_singleFramePic->data, 3 * VideoCols * VideoRows);
                wxImage *VideoImage = new wxImage(VideoCols, VideoRows, (uchar *) VideoData, false);

                // 两个图像矩阵处理结束，可以通知主线程进行绘制
                MyPlotEvent event( MY_PLOT_THREAD, ID_PACKET_PROCESS );
                event.SetOfflineImage(RdImage,MdImage,VideoImage);

                // 线程安全
                wxQueueEvent( m_fatherPanel, event.Clone() );

                // 一帧结束，重新设置insertPos 并把剩余的数据复制到帧数据流中存储
                m_insertPos = m_radarCube->getFrame().begin();
                m_insertPos = std::copy_n(
                        reinterpret_cast<int16_t *>(udpPacketPtr + m_udpParam->m_bufOffset + remainLength * m_udpParam->m_bufScale),
                        ((m_udpParam->m_bufSize - m_udpParam->m_bufOffset) / m_udpParam->m_bufScale) - remainLength,
                        m_insertPos);
            }
        }
        // 删除那一个在动态内存中new出来数据部分
        delete[] udpPacketPtr;
    }

    // 不返回这个的话会异常退出
    return (wxThread::ExitCode)0;
}

/**
 * @brief wxOnlinePagePanel类 - EnableDCA1000函数
 * @param 触发事件
 */
void wxOnlinePagePanel::EnableDCA1000(wxCommandEvent& event)
{
    // 输出消息重定位
    wxLogTextCtrl *console = new wxLogTextCtrl(m_logOutPut);
    wxLog::SetActiveTarget(console);

    // 获得运行程序当前文件路径 + DCA1000EVM.exe路径
    wxString exePath = wxGetCwd();
    wxString filePath(exePath + "\\DCA1000\\DCA1000EVM.exe");

#ifndef NDEBUG
    wxLogMessage(exePath);
    wxLogMessage(filePath);
#endif

    // 与wxExecute一起使用的函数
    // 异步通知程序进程终止，并且还在异步执行的情况下从wxExecute中获取其退出状态
    wxProcess * proc = new wxProcess(this);
    // wxExecuteEnv结构体第一个参数cwd - 新进程的工作目录
    wxExecuteEnv env;
    wxString myCwd(exePath + "\\DCA1000");
    env.cwd = myCwd;

    // wxExecute - 允许在Unix或Windows中执行另一个程序
    // wxExecute第一个参数command - command要执行的命令以及要作为单个字符串传递给它的任何参数，即“ emacs file.txt”
    // wxExecute第二个参数flags -  wxEXEC_ASYNC 或 wxEXEC_SYNC 设置异步/同步 wxEXEC_SHOW_CONSOLE 始终显示控制台
    // wxExecute第三个参数callback - 指向wxProcess的可选指针
    // wxExecute第四个参数env - 指向子进程的其他参数的可选指针，例如其初始工作目录和环境变量
    wxExecute(filePath, wxEXEC_ASYNC | wxEXEC_SHOW_CONSOLE, proc, &env);
}

/**
 * @brief wxOnlinePagePanel类 - ConnectUDP函数
 * @param 触发事件
 */
void wxOnlinePagePanel::ConnectUDP(wxCommandEvent& event)
{
    // 定义一个IPV4地址类对象 - 绑定本地
    // DCA1000只将数据传到指定 IP + 端口
    wxIPV4address localAddr;
    wxString hostname("192.168.33.30");
    localAddr.Hostname(hostname);
    localAddr.Service(4098);

    // 创建一个本地UDP-Socket，并且绑定上述建立的IPV4地址
    m_mySocket = new wxDatagramSocket(localAddr);

    // 分配1024*1024字节的缓存给udp的socket
    // SetOption第一个参数level - SOL_SOCKET 或者 IPPROTO_TCP
    // SetOption第二个参数optname - 需要设置的参数名称
    // SetOption第三个参数optval - 指向缓冲区的指针(实际上就是int *)
    // SetOption第四个参数optlen - 缓存区大小
    long newBufferSize = 1024*1024*200;
    m_mySocket->SetOption(SOL_SOCKET, SO_RCVBUF, &newBufferSize, sizeof(newBufferSize));

    // 设置UDP Socket
    // 将事件处理器和事件标志符SOCKET_ID绑定
    m_mySocket->SetEventHandler(*this, PAGE1_SOCKET_ID);
    // 定义的这个UDP socket只是用来接收的
    m_mySocket->SetNotify(wxSOCKET_INPUT_FLAG);
    // 监听wxSOCKET_INPUT_FLAG事件
    m_mySocket->Notify(true);
}

/**
 * @brief wxOnlinePagePanel类 - EnableAWR1642函数
 * @param 触发事件
 */
void wxOnlinePagePanel::EnableAWR1642(wxCommandEvent& event)
{
    // 注释同上
    wxString exePath = wxGetCwd();
    wxString filePath(exePath + "\\AWR1642\\mmwavelink_example.exe");

#ifndef NDEBUG
    wxLogMessage(exePath);
    wxLogMessage(filePath);
#endif

    wxProcess * proc = new wxProcess(this);
    wxExecuteEnv env;
    wxString myCwd(exePath + "\\AWR1642");
    env.cwd = myCwd;

    wxExecute(filePath, wxEXEC_ASYNC | wxEXEC_SHOW_CONSOLE, proc, &env);

    // 打开存储视频
    cv::Size size = cv::Size(m_capture->get(cv::CAP_PROP_FRAME_WIDTH), m_capture->get(cv::CAP_PROP_FRAME_HEIGHT));
    m_outputVideo->open(m_videoPath, m_outputVideo->fourcc('M', 'J', 'P', 'G'), 25.0, size, true);

    // log信息输出
    // 重定向以向log文件中输出 - 输出是否包连续
    m_logFile = fopen("trace.log","wa");
    // 构造一个发送log信息的目标，将所有的log信息都发送到给定的FILE中
    m_logOutput = new wxLogStderr(m_logFile);
    // 将指定的日志目标设置为活动目标
    wxLog::SetActiveTarget(m_logOutput);

    // 用于处理Packet的子线程启动
    // bin回放开始 - 线程启动
    // 绑定事件
    Connect(ID_PACKET_PROCESS,MY_PLOT_THREAD,wxMyPlotEventHandler(wxOnlinePagePanel::PacketThreadProcess));

    m_packetProcessThread = new PacketProcessThread(this);
    if ( m_packetProcessThread->Create() != wxTHREAD_NO_ERROR )
    {
        wxLogMessage(_("Can't open thread"));
        delete m_packetProcessThread;
        return;
    }
    m_packetProcessThread->Run();
    wxLogMessage(_("Run - Bin File Data Replay!"));
}

/**
 * @brief wxOnlinePagePanel类 - DisconnectUDP函数
 * @param 触发事件
 */
void wxOnlinePagePanel::DisconnectUDP(wxCommandEvent& event)
{
    m_mySocket->Destroy();
    m_mySocket->Notify(false);
    m_outputVideo->release();           // 摄像头保存设置为关闭
    m_capture->release();               // 摄像头关闭

    m_packetProcessThread->Delete();    // 子线程关闭
}

/**
 * @brief wxOnlinePagePanel类 - OnSocketEvent函数
 * @param 触发事件
 */
void wxOnlinePagePanel::OnSocketEvent(wxSocketEvent& event)
{
    // 事件类型触发
    switch (event.GetSocketEvent())
    {
        case wxSOCKET_INPUT:
        {
            // wxSOCKET_INPUT_FLAG标志会在数据到来时触发，会引起中断
            // 我们SetNotify wxSOCKET_LOST_FLAG 用以失去所有flag的bit位信息表示事件不触发
            m_mySocket->SetNotify(wxSOCKET_LOST_FLAG);

            // 一个UDP包的大小为1466，头部有验证信息，且要丢弃前10帧
            UINT8 *singleUdpBufPtr = new UINT8[m_udpParam->m_bufSize];

            // 从socket缓存中读取1466字节udp数据(一个包的大小)到buf
            m_mySocket->Read(singleUdpBufPtr, m_udpParam->m_bufSize);

            m_packetQueue.push(singleUdpBufPtr);

            // 上面wxSOCKET_LOST_FLAG 用以失去所有flag的bit位信息表示事件不触发
            // 现在要重新SetNotify wxSOCKET_INPUT_FLAG表示现在又要对socket数据缓存区的数据进行处理
            m_mySocket->SetNotify(wxSOCKET_INPUT_FLAG);
            break;
        }
        default:
            ;
    }
}

/**
 * @brief wxOnlinePagePanel类 - PacketThreadProcess函数
 * @param 触发事件
 */
void wxOnlinePagePanel::PacketThreadProcess(MyPlotEvent& event)
{
    // 主线程实时绘图 - Range Doppler绘图
    m_rdPicPanel->SetImage(*event.GetOfflineRadarPic().first);
    m_rdPicPanel->StretchImage();
    m_rdPicPanel->PaintNow();

    // 主线程实时绘图 - Micro Doppler绘图
    m_mdPicPanel->SetImage(*event.GetOfflineRadarPic().second);
    m_mdPicPanel->StretchImage();
    m_mdPicPanel->PaintNow();

    // 主线程实时绘图 - 视频逐帧播放绘图
    m_cameraPicPanel->SetImage(*event.GetOfflineVideoPic());
    m_cameraPicPanel->StretchImage();
    m_cameraPicPanel->PaintNow();

    delete event.GetOfflineRadarPic().first;
    delete event.GetOfflineRadarPic().second;
    delete event.GetOfflineVideoPic();
}

/* ----------------------------------------------------- Page 2 ------------------------------------------------------
 ----------------------------------------------- wxBinReplayPagePanel类 --------------------------------------------- */

/**
 * @brief wxBinReplayPagePanel类 - 构造函数
 * @param parent 父窗口指针
 */
wxBinReplayPagePanel::wxBinReplayPagePanel(wxPanel *parent)
                    : wxPanel(parent)
{
    /* ---------------------------------------------- 控件设定 + 页面布局 ----------------------------------------------- */
    // 按钮
    m_logOutPut = new wxTextCtrl(this, wxID_ANY, wxT(""),wxPoint(-1, -1), wxSize(-1, -1), wxTE_MULTILINE);
    wxButton *btRun = new wxButton(this, ID_PAGE2_RUN, wxT("RUN"),wxDefaultPosition,wxSize(150,30));
    wxButton *btPause = new wxButton(this, ID_PAGE2_PAUSE, wxT("PAUSE"),wxDefaultPosition,wxSize(150,30));
    wxButton *btResume = new wxButton(this, ID_PAGE2_RESUME, wxT("RESUME"),wxDefaultPosition,wxSize(150,30));
    wxButton *btEnd = new wxButton(this, ID_PAGE2_END, wxT("END"),wxDefaultPosition,wxSize(150,30));
    wxStaticBox *sBox = new wxStaticBox(this,wxID_ANY,wxT("Function Button"));
    // 输出log信息
    m_console = new wxLogTextCtrl(m_logOutPut);
    // 图窗和静态文本
    m_rdPicPanel = new wxImagePanel(this);
    m_mdPicPanel = new wxImagePanel(this);
    m_cameraPicPanel = new wxImagePanel(this);
    wxStaticText *rdPicText = new wxStaticText(this,wxID_STATIC,wxT("Range-Doppler Map"));
    wxStaticText *mdPicText = new wxStaticText(this,wxID_STATIC,wxT("Micro-Doppler Map"));
    wxStaticText *cameraPicText = new wxStaticText(this,wxID_STATIC,wxT("Camera Picture"));
    // 布局控件
    wxStaticBoxSizer * sBoxSizer = new wxStaticBoxSizer(sBox,wxVERTICAL);
    sBoxSizer->Add(btRun,0,wxALIGN_CENTER_HORIZONTAL | wxALL,10);
    sBoxSizer->Add(btPause,0,wxALIGN_CENTER_HORIZONTAL | wxALL,10);
    sBoxSizer->Add(btResume,0,wxALIGN_CENTER_HORIZONTAL | wxALL,10);
    sBoxSizer->Add(btEnd,0,wxALIGN_CENTER_HORIZONTAL | wxALL,10);
    wxBoxSizer *rdBoxSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *mdBoxSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *cameraBoxSizer = new wxBoxSizer(wxVERTICAL);
    rdBoxSizer->Add(rdPicText,0,wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
    rdBoxSizer->Add(m_rdPicPanel,1,wxEXPAND | wxALL, 5);
    mdBoxSizer->Add(mdPicText,0,wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
    mdBoxSizer->Add(m_mdPicPanel,1,wxEXPAND | wxALL, 5);
    cameraBoxSizer->Add(cameraPicText,0,wxALIGN_CENTER_HORIZONTAL | wxALL, 5);
    cameraBoxSizer->Add(m_cameraPicPanel,1,wxEXPAND | wxALL, 5);
    wxBoxSizer *hBoxSizer = new wxBoxSizer(wxHORIZONTAL);
    hBoxSizer->Add(rdBoxSizer,1,wxEXPAND | wxALL, 5);
    hBoxSizer->Add(mdBoxSizer,1,wxEXPAND | wxALL, 5);
    hBoxSizer->Add(cameraBoxSizer,1,wxEXPAND | wxALL, 5);
    hBoxSizer->Add(sBoxSizer,0,wxALIGN_CENTER_VERTICAL | wxALL,5);
    wxBoxSizer *vBoxSizer = new wxBoxSizer(wxVERTICAL);
    vBoxSizer->Add(hBoxSizer,3,wxEXPAND | wxALIGN_RIGHT | wxRIGHT,20);
    vBoxSizer->Add(m_logOutPut,2,wxEXPAND | wxALL,5);
    // 绑定布局控件和this窗口
    this->SetSizer(vBoxSizer);

    // 添加动态事件响应
    Connect(ID_PAGE2_RUN, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxBinReplayPagePanel::ReplayRun));
    Connect(ID_PAGE2_PAUSE, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxBinReplayPagePanel::ReplayPause));
    Connect(ID_PAGE2_RESUME, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxBinReplayPagePanel::ReplayResume));
    Connect(ID_PAGE2_END, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxBinReplayPagePanel::ReplayEnd));

    /* -------------------------------------------------- 参数初始化 -------------------------------------------------- */



}

/**
 * @brief BinReplayThread类 - 构造函数
 * @param parent 父窗口指针
 */
BinReplayThread::BinReplayThread(wxBinReplayPagePanel *parent) : wxThread(wxTHREAD_DETACHED)
{
    m_fatherPanel = parent;
    m_mdMapDrawFlagOL = true;
}

/**
 * @brief BinReplayThread类 - 退出函数
 */
void BinReplayThread::OnExit()
{

}

/**
 * @brief BinReplayThread类 - 启动函数
 */
wxThread::ExitCode BinReplayThread::Entry()
{
    // 获取路径
    std::string binFileNameStr = this->m_fatherPanel->m_binPathStr.ToStdString();
    std::string videoFileNameStr = this->m_fatherPanel->m_videoPathStr.ToStdString();
    // avi频流地址
    cv::VideoCapture aviCapture(videoFileNameStr);
    // 用于暂存Video中的每一帧数据定义
    cv::Mat singleFrameInBin;

    // 离线数据 - 雷达信号处理类相关变量初始设置
    // todo - 可以设置为智能指针
    RadarParam *radarParam = new RadarParam;                          // RadarParam对象初始化
    RadarDataCube *radarCube = new RadarDataCube(*radarParam);   // RadarDataCube对象初始化
    int16_t *preBuf = new int16_t[radarParam->getFrameBytes() / 2];   // 初始化buff
    std::vector<INT16>::iterator insertPos;                           // 设置初始复制位置
    insertPos = radarCube->getFrame().begin();                      // 定义插入帧数据中的位置初始化

    // 获取帧数 - totalFrame
    std::ifstream ifsCountFrame(binFileNameStr,std::ios::binary | std::ios::in);
    if(!ifsCountFrame.is_open()) {return (wxThread::ExitCode)0;}
    ifsCountFrame.seekg(0, std::ios_base::end);
    long long nFileLen = ifsCountFrame.tellg();
    long long totalFrame = nFileLen / radarParam->getFrameBytes();
    ifsCountFrame.close();

    // 打开文件
    std::ifstream ifs(binFileNameStr,std::ios::binary | std::ios::in);
    if(!ifs.is_open()){ return (wxThread::ExitCode)0;}

    // 执行读取的文件
    while (totalFrame-- && !TestDestroy())
    {
        ifs.read((char *) preBuf, radarParam->getFrameBytes());
        std::copy_n(preBuf, radarParam->getFrameBytes() / 2, insertPos);

        // 雷达数据处理
        // 帧数据流存满 - 进行相应处理
        radarCube->creatCube();
        radarCube->creatRdm();
        int RdCols = radarCube->convertRdmToMap().cols, RdRows = radarCube->convertRdmToMap().rows;
        // 使用malloc分配一块动态内存 - 目的在于独立出这块图的数据
        void *RdData = malloc(3 * RdCols * RdRows);
        memcpy(RdData, (void *) radarCube->convertRdmToMap().data, 3 * RdCols * RdRows);
        wxImage *RdImage = new wxImage(RdCols, RdRows, (uchar *) RdData, false);

        // 微多普勒图也进行更新
        if (m_mdMapDrawFlagOL) {
            radarCube->setFlagForMap();
            m_mdMapDrawFlagOL = false;
        }

        radarCube->updateMicroMap();
        int MdCols = radarCube->convertMdToMap().cols, MdRows = radarCube->convertMdToMap().rows;
        // 使用malloc分配一块动态内存 - 目的在于独立出这块图的数据
        void *MdData = malloc(3 * MdCols * MdRows);
        memcpy(MdData, (void *) radarCube->convertMdToMap().data, 3 * MdCols * MdRows);
        wxImage *MdImage = new wxImage(MdCols, MdRows, (uchar *) MdData, false);

        // 读取保存的视频
        aviCapture >> singleFrameInBin;
        cv::cvtColor(singleFrameInBin, singleFrameInBin, cv::COLOR_RGB2BGR);
        int VideoCols = singleFrameInBin.cols, VideoRows = singleFrameInBin.rows;
        // 使用malloc分配一块动态内存 - 目的在于独立出这块图的数据
        void *VideoData = malloc(3 * VideoCols * VideoRows);
        memcpy(VideoData, (void *) singleFrameInBin.data, 3 * VideoCols * VideoRows);
        wxImage *VideoImage = new wxImage(VideoCols, VideoRows, (uchar *) VideoData, false);

        // 两个图像矩阵处理结束，可以通知主线程进行绘制
        MyPlotEvent event( MY_PLOT_THREAD, ID_REPLAY_PROCESS );
        event.SetOfflineImage(RdImage,MdImage,VideoImage);

        // 线程安全
        wxQueueEvent( m_fatherPanel, event.Clone() );
    }

    // 关闭文件 + 释放内存
    // 关闭文件
    ifs.close();
    // 释放内存
    delete[] preBuf;
    delete radarParam;
    delete radarCube;

    // 不返回这个的话会异常退出
    return (wxThread::ExitCode)0;
}

/**
 * @brief wxBinReplayPagePanel类 - bin回放开始函数
 * @param event 触发事件
 */
void wxBinReplayPagePanel::ReplayRun(wxCommandEvent& event)
{
    // 输出消息重定位
    wxLog::SetActiveTarget(m_console);

    // 创建Dialog用于输入路径
    wxReplayFileDialog* replayFileDialog = new wxReplayFileDialog(this,wxT("Bin File And Video File Path Setting"));

    // bin回放开始 - 线程启动
    // 绑定事件
    Connect(ID_REPLAY_PROCESS,MY_PLOT_THREAD,wxMyPlotEventHandler(wxBinReplayPagePanel::ReplayThreadProcess));

    m_binReplayThread = new BinReplayThread(this);
    if ( m_binReplayThread->Create() != wxTHREAD_NO_ERROR )
    {
        wxLogMessage(_("Can't open thread"));
        delete m_binReplayThread;
        return;
    }
    m_binReplayThread->Run();
    wxLogMessage(_("Run the thread successfully!"));
}

/**
 * @brief wxBinReplayPagePanel类 - bin回放暂停函数
 * @param event 触发事件
 */
void wxBinReplayPagePanel::ReplayPause(wxCommandEvent& event)
{
    if (m_binReplayThread) // 线程是否仍旧存在
    {
        if (m_binReplayThread->Pause() != wxTHREAD_NO_ERROR)
            wxLogError("Can't pause the thread!");
        else  wxLogMessage(_("Pause the thread successfully!"));
    }
    else wxLogError("The thread has ended, please run again!");
}

/**
 * @brief wxBinReplayPagePanel类 - bin回放恢复函数
 * @param event 触发事件
 */
void wxBinReplayPagePanel::ReplayResume(wxCommandEvent& event)
{
    if (m_binReplayThread) // 线程是否仍旧存在
    {
        if (m_binReplayThread->Resume() != wxTHREAD_NO_ERROR)
            wxLogError("Can't resume the thread!");
        else  wxLogMessage(_("Resume the thread successfully!"));
    }
    else wxLogError("The thread has ended, please run again!");
}

/**
 * @brief wxBinReplayPagePanel类 - bin回放结束函数
 * @param event 触发事件
 */
void wxBinReplayPagePanel::ReplayEnd(wxCommandEvent& event)
{
    if (m_binReplayThread) // 线程是否仍旧存在
    {
        if (m_binReplayThread->Delete() != wxTHREAD_NO_ERROR)
            wxLogError("Can't delete the thread!");
        else  wxLogMessage(_("Delete the thread successfully!"));
        // 在调用END后再调用Pause和Resume无效，除非重新调用Run开始
        m_binReplayThread = nullptr;
    }
    else wxLogError("The thread has ended, please run again!");
}

/**
 * @brief wxBinReplayPagePanel类 - bin回放处理线程函数
 * @param event 触发事件
 */
void wxBinReplayPagePanel::ReplayThreadProcess(MyPlotEvent& event)
{
    // 主线程实时绘图 - Range Doppler绘图
    m_rdPicPanel->SetImage(*event.GetOfflineRadarPic().first);
    m_rdPicPanel->StretchImage();
    m_rdPicPanel->PaintNow();

    // 主线程实时绘图 - Micro Doppler绘图
    m_mdPicPanel->SetImage(*event.GetOfflineRadarPic().second);
    m_mdPicPanel->StretchImage();
    m_mdPicPanel->PaintNow();

    // 主线程实时绘图 - 视频逐帧播放绘图
    m_cameraPicPanel->SetImage(*event.GetOfflineVideoPic());
    m_cameraPicPanel->StretchImage();
    m_cameraPicPanel->PaintNow();

    delete event.GetOfflineRadarPic().first;
    delete event.GetOfflineRadarPic().second;
    delete event.GetOfflineVideoPic();
}

/**
 * @brief wxReplayFileDialog类 - 构造函数
 * @param parent 父窗口指针
 * @param title 窗口名称
 */
wxReplayFileDialog::wxReplayFileDialog(wxBinReplayPagePanel* parent,const wxString& title)
                  :wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxSize(700, 200))
{
    // 设置父亲
    m_father = parent;
    // 静态文本设置
    wxStaticText *binPathText = new wxStaticText(this, wxID_ANY, wxT("Bin File Path"),wxDefaultPosition,wxSize(150,30));
    wxStaticText *videoPathText = new wxStaticText(this, wxID_ANY, wxT("Video File Path"),wxDefaultPosition,wxSize(150,30));
    // 路径显示
    m_binPath = new wxTextCtrl(this, wxID_ANY, wxT(""),wxDefaultPosition,wxSize(300,30));
    m_videoPath = new wxTextCtrl(this, wxID_ANY, wxT(""),wxDefaultPosition,wxSize(300,30));
    // 按钮设置
    wxButton *btBinPath = new wxButton(this, ID_BIN_PATH, wxT("Select Path..."),wxDefaultPosition,wxSize(150,30));
    wxButton *btVideoPath = new wxButton(this, ID_VIDEO_PATH, wxT("Select Path..."),wxDefaultPosition,wxSize(150,30));
    wxButton *btReset = new wxButton(this, ID_RESET, wxT("RESET"),wxDefaultPosition,wxSize(150,30));
    wxButton *btApply = new wxButton(this, ID_APPLY, wxT("APPLY"),wxDefaultPosition,wxSize(150,30));
    // 设置布局控件
    wxBoxSizer *hBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *hBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *hBoxSizer3 = new wxBoxSizer(wxHORIZONTAL);
    hBoxSizer1->Add(binPathText,0,wxLEFT,10);
    hBoxSizer1->Add(m_binPath,0,wxLEFT,10);
    hBoxSizer1->Add(btBinPath,0,wxLEFT | wxRIGHT,10);
    hBoxSizer2->Add(videoPathText,0,wxLEFT,10);
    hBoxSizer2->Add(m_videoPath,0,wxLEFT,10);
    hBoxSizer2->Add(btVideoPath,0,wxLEFT | wxRIGHT,10);
    hBoxSizer3->Add(btReset,0,wxLEFT,10);
    hBoxSizer3->Add(btApply,0,wxLEFT | wxRIGHT,10);
    wxBoxSizer *vBoxSizer = new wxBoxSizer(wxVERTICAL);
    vBoxSizer->Add(hBoxSizer1,0,wxALL | wxALIGN_CENTRE_HORIZONTAL,10);
    vBoxSizer->Add(hBoxSizer2,0,wxALL | wxALIGN_CENTRE_HORIZONTAL,10);
    vBoxSizer->Add(hBoxSizer3,0,wxALL | wxALIGN_CENTRE_HORIZONTAL,10);
    this->SetSizer(vBoxSizer);

    // 链接响应事件
    Connect(ID_BIN_PATH, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxReplayFileDialog::setBinPath));
    Connect(ID_VIDEO_PATH, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxReplayFileDialog::setVideoPath));
    Connect(ID_RESET, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxReplayFileDialog::Reset));
    Connect(ID_APPLY, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxReplayFileDialog::Apply));

    // 在屏幕中居中显示
    Centre();
    // 显示消息对话框
    this->ShowModal();
}

/**
 * @brief wxReplayFileDialog类 - setBinPath函数
 * @param event 触发事件
 */
void wxReplayFileDialog::setBinPath(wxCommandEvent& event)
{
    // 创建wxFileDialog类获取文件路径 - 用于在界面中显示
    wxFileDialog * openFileDialog = new wxFileDialog(this);
    if (openFileDialog->ShowModal() == wxID_OK){
        wxString fileName = openFileDialog->GetPath();
        m_binPath->SetValue(fileName);
    }
    delete openFileDialog;
}

/**
 * @brief wxReplayFileDialog类 - setVideoPath函数
 * @param event 触发事件
 */
void wxReplayFileDialog::setVideoPath(wxCommandEvent& event)
{
    // 创建wxFileDialog类获取文件路径 - 用于在界面中显示
    wxFileDialog * openFileDialog = new wxFileDialog(this);
    if (openFileDialog->ShowModal() == wxID_OK){
        wxString fileName = openFileDialog->GetPath();
        m_videoPath->SetValue(fileName);
    }
    delete openFileDialog;
}

/**
 * @brief wxReplayFileDialog类 - Reset函数
 * @param event 触发事件
 */
void wxReplayFileDialog::Reset(wxCommandEvent& event)
{
    m_binPath->Clear();
    m_videoPath->Clear();
}

/**
 * @brief wxReplayFileDialog类 - Apply函数
 * @param event 触发事件
 */
void wxReplayFileDialog::Apply(wxCommandEvent& event)
{
    // 获取路径 - 用于代码
    m_father->m_binPathStr = m_binPath->GetValue();
    m_father->m_videoPathStr = m_videoPath->GetValue();
    this->Destroy();
}

/* ----------------------------------------------------- Page 3 ------------------------------------------------------
 ----------------------------------------------- wxOfflinePagePanel类 ---------------------------------------------- */

/**
 * @brief wxOfflinePagePanel类 - 构造函数
 * @param parent 父窗口指针
 */
wxOfflinePagePanel::wxOfflinePagePanel(wxPanel *parent) : wxPanel(parent,wxID_ANY)
{
    /* ----------------------------------------------- 控件设定 + 页面布局 --------------------------------------------- */
    // 按钮
    m_logOutPut = new wxTextCtrl(this, wxID_ANY, wxT(""),wxPoint(-1, -1), wxSize(-1, -1), wxTE_MULTILINE);
    wxButton *btTrainExtra = new wxButton(this, ID_PAGE3_TRAIN, wxT("TrainSet Extract"),wxDefaultPosition,wxSize(150,30));
    wxButton *btTestExtra = new wxButton(this, ID_PAGE3_TEST, wxT("TestSet Extract"),wxDefaultPosition,wxSize(150,30));
    wxButton *btSvmModel = new wxButton(this, ID_PAGE3_SVM, wxT("SVM Model Build"),wxDefaultPosition,wxSize(150,30));
    wxButton *btBinDemo = new wxButton(this, ID_PAGE3_DEMO, wxT("Single Bin Test"),wxDefaultPosition,wxSize(150,30));
    wxStaticBox *sbBox = new wxStaticBox(this,wxID_ANY,wxT("Function Button"));
    // 输出log信息 - 用于重定位
    m_console = new wxLogTextCtrl(m_logOutPut);
    // 图窗和静态文本
    m_mdWin = new mpWindow(this,wxID_ANY,wxDefaultPosition,wxSize(100,100));
    m_torsoWin = new mpWindow(this,wxID_ANY,wxDefaultPosition,wxSize(100,100));
    m_limbsWin = new mpWindow(this,wxID_ANY,wxDefaultPosition,wxSize(100,100));
    m_vmdWin = new mpWindow(this,wxID_ANY,wxDefaultPosition,wxSize(100,100));
    wxStaticText *mdWinText = new wxStaticText(this,wxID_STATIC,wxT("Micro-Doppler Map"));
    wxStaticText *torsoWinText = new wxStaticText(this,wxID_STATIC,wxT("Torso Curve"));
    wxStaticText *limbsWinText = new wxStaticText(this,wxID_STATIC,wxT("Limbs Curve"));
    wxStaticText *vmdWinText = new wxStaticText(this,wxID_STATIC,wxT("VMD Vector"));
    // 图像layer
    m_mdPic = new mpBitmapLayer;
    m_mdWin->AddLayer(m_mdPic);
    // torso layer
    mpScaleX *torsoCurveXScale = new mpScaleX();
    mpScaleY *torsoCurveYScale = new mpScaleY();
    m_torsoCurve = new mpFXYVector();
    m_torsoWin->AddLayer(torsoCurveXScale);
    m_torsoWin->AddLayer(torsoCurveYScale);
    m_torsoWin->AddLayer(m_torsoCurve);
    // limbs layer
    mpScaleX *limbsCurveXScale = new mpScaleX();
    mpScaleY *limbsCurveYScale = new mpScaleY();
    m_limbsCurve = new mpFXYVector();
    m_limbsWin->AddLayer(limbsCurveXScale);
    m_limbsWin->AddLayer(limbsCurveYScale);
    m_limbsWin->AddLayer(m_limbsCurve);
    // vmd layer
    mpScaleX *vmdCurveXScale = new mpScaleX();
    mpScaleY *vmdCurveYScale = new mpScaleY();
    m_vmdCurve = new mpFXYVector();
    m_vmdWin->AddLayer(vmdCurveXScale);
    m_vmdWin->AddLayer(vmdCurveYScale);
    m_vmdWin->AddLayer(m_vmdCurve);
    // 布局控件
    wxStaticBoxSizer * sBoxSizer = new wxStaticBoxSizer(sbBox,wxVERTICAL);
    sBoxSizer->Add(btTrainExtra,0,wxALIGN_CENTER_HORIZONTAL | wxALL,10);
    sBoxSizer->Add(btTestExtra,0,wxALIGN_CENTER_HORIZONTAL | wxALL,10);
    sBoxSizer->Add(btSvmModel,0,wxALIGN_CENTER_HORIZONTAL | wxALL,10);
    sBoxSizer->Add(btBinDemo,0,wxALIGN_CENTER_HORIZONTAL | wxALL,10);
    wxBoxSizer *mdBoxSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *torsoBoxSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *limbsBoxSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *vmdBoxSizer = new wxBoxSizer(wxVERTICAL);
    mdBoxSizer->Add(mdWinText,0,wxALIGN_CENTER_HORIZONTAL | wxALL, 2);
    mdBoxSizer->Add(m_mdWin,1,wxEXPAND | wxALL, 2);
    torsoBoxSizer->Add(torsoWinText,0,wxALIGN_CENTER_HORIZONTAL | wxALL, 2);
    torsoBoxSizer->Add(m_torsoWin,1,wxEXPAND | wxALL, 2);
    limbsBoxSizer->Add(limbsWinText,0,wxALIGN_CENTER_HORIZONTAL | wxALL, 2);
    limbsBoxSizer->Add(m_limbsWin,1,wxEXPAND | wxALL, 2);
    vmdBoxSizer->Add(vmdWinText,0,wxALIGN_CENTER_HORIZONTAL | wxALL, 2);
    vmdBoxSizer->Add(m_vmdWin,1,wxEXPAND | wxALL, 2);
    wxBoxSizer *hBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *hBoxSizer2 = new wxBoxSizer( wxHORIZONTAL );
    wxBoxSizer *hBoxSizer3 = new wxBoxSizer( wxHORIZONTAL );
    wxBoxSizer *vBoxSizer1 = new wxBoxSizer( wxVERTICAL );
    wxBoxSizer *vBoxSizer2 = new wxBoxSizer( wxVERTICAL );
    hBoxSizer1->Add(mdBoxSizer,1,wxALL | wxEXPAND,5);
    hBoxSizer1->Add(torsoBoxSizer,1,wxALL | wxEXPAND,5);
    hBoxSizer2->Add(limbsBoxSizer,1,wxALL | wxEXPAND,5);
    hBoxSizer2->Add(vmdBoxSizer,1,wxALL | wxEXPAND,5);
    vBoxSizer1->Add(hBoxSizer1,1,wxALL | wxEXPAND,5);
    vBoxSizer1->Add(hBoxSizer2,1,wxALL | wxEXPAND,5);
    hBoxSizer3->Add(vBoxSizer1,1,wxALL | wxEXPAND,5);
    hBoxSizer3->Add(sBoxSizer,0,wxALIGN_CENTER_VERTICAL | wxALL,5);
    vBoxSizer2->Add(hBoxSizer3,3,wxEXPAND | wxALIGN_RIGHT | wxRIGHT,20);
    vBoxSizer2->Add(m_logOutPut,2,wxEXPAND | wxALL,5);
    this->SetSizer(vBoxSizer2);

    // 添加动态事件响应
    Connect(ID_PAGE3_TRAIN, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxOfflinePagePanel::TrainDataSet));
    Connect(ID_PAGE3_TEST, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxOfflinePagePanel::TestDataSet));
    Connect(ID_PAGE3_SVM, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxOfflinePagePanel::SvmModel));
    Connect(ID_PAGE3_DEMO, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxOfflinePagePanel::SingleBinDemo));

    /* ----------------------------------------------- 参数初始化 --------------------------------------------- */


}

/**
 * @brief wxOfflinePagePanel类 - TrainDataSet函数
 * @param event 触发事件
 */
void wxOfflinePagePanel::TrainDataSet(wxCommandEvent& event)
{
    // 输出消息重定位
    wxLog::SetActiveTarget(m_console);

    wxLogMessage(_("In development !"));
}

/**
 * @brief wxOfflinePagePanel类 - TestDataSet函数
 * @param event 触发事件
 */
void wxOfflinePagePanel::TestDataSet(wxCommandEvent& event)
{
    // 输出消息重定位
    wxLog::SetActiveTarget(m_console);

    wxLogMessage(_("In development !"));
}

/**
 * @brief wxOfflinePagePanel类 - SvmModel函数
 * @param event 触发事件
 */
void wxOfflinePagePanel::SvmModel(wxCommandEvent& event)
{
    // 输出消息重定位
    wxLog::SetActiveTarget(m_console);

    wxLogMessage(_("In development !"));
}

/**
 * @brief wxOfflinePagePanel类 - SingleBinDemo函数
 * @param event 触发事件
 */
void wxOfflinePagePanel::SingleBinDemo(wxCommandEvent& event)
{
    // 输出消息重定位
    wxLog::SetActiveTarget(m_console);

    wxLogMessage(_("In development !"));
}




