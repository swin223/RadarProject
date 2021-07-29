#include "GuiPanel.h"
#include <wx/process.h>
#include <algorithm>
#include <wx/file.h>
#include <wx/wfstream.h>
#include <wx/notebook.h>
#include <memory>
#include <wx/stdpaths.h>

/* ----------------------------------------------------- 实现App ----------------------------------------------------- */
IMPLEMENT_APP(MyApp)

bool MyApp::OnInit()
{
    MyFrame *frame = new MyFrame(wxT("Real-Time Communication"));
    frame->Show(true);
    return true;
}

/* ---------------------------------------------------- MyFrame类 ---------------------------------------------------- */
MyFrame::MyFrame(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(1400, 800))
{
    /* ---------------------------------------------- 控件设定 + 页面布局 ----------------------------------------------- */
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

void MyFrame::OnQuit(wxCommandEvent& event)
{
    // 释放主窗口
    Close();
}

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
    EVT_SOCKET(ID_ONLINE_SOCKET, wxOnlinePagePanel::OnSocketEvent)
END_EVENT_TABLE()

wxOnlinePagePanel::wxOnlinePagePanel(wxPanel *parent)
    : wxPanel(parent)
{
    /* ---------------------------------------------- 控件设定 + 页面布局 ----------------------------------------------- */
    // 按钮
    m_logOutPut = new wxTextCtrl(this, wxID_ANY, wxT(""),wxPoint(-1, -1), wxSize(-1, -1), wxTE_MULTILINE);
    wxButton *btDca1000 = new wxButton(this, ID_ONLINE_DCA1000, wxT("RUN DCA1000"),wxDefaultPosition,wxSize(150,30));
    wxButton *btUdpConnect = new wxButton(this, ID_ONLINE_UDPCT, wxT("UDP CONNECT"),wxDefaultPosition,wxSize(150,30));
    wxButton *btAwr1642 = new wxButton(this, ID_ONLINE_AWR1642, wxT("RUN AWR1642"),wxDefaultPosition,wxSize(150,30));
    wxButton *btUdpDisconnect = new wxButton(this, ID_ONLINE_UDPDISCT, wxT("UDP DISCONNECT"),wxDefaultPosition,wxSize(150,30));
    wxButton *btDetectAction = new wxButton(this, ID_ONLINE_DECTACTION, wxT("DETECT ACTION"),wxDefaultPosition,wxSize(150,30));
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
    sBoxSizer->Add(btDetectAction,0,wxALIGN_CENTER_HORIZONTAL | wxALL,10);
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
    Connect(ID_ONLINE_DCA1000, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxOnlinePagePanel::OnEnableDCA1000Click));
    Connect(ID_ONLINE_UDPCT, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxOnlinePagePanel::OnConnectUDPClick));
    Connect(ID_ONLINE_AWR1642, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxOnlinePagePanel::OnEnableAWR1642Click));
    Connect(ID_ONLINE_UDPDISCT, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxOnlinePagePanel::OnDisconnectUDPClick));
    Connect(ID_ONLINE_DECTACTION, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxOnlinePagePanel::OnDetectActionClick));

    /* -------------------------------------------------- 参数初始化 --------------------------------------------------- */
    // 读取.ini文件中的配置来配置一些雷达参数
    // 参阅heatmap相关项目ini配置、官网wxFileConfig类的官方文档、wxWidgets Discussion Forum一些例子和讨论
    m_configIni = new wxFileConfig(wxEmptyString,
                                   wxEmptyString,
                                   wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPathWithSep() + _T("config.ini"));
#ifndef NDEBUG
    int groupNums = m_configIni->GetNumberOfGroups();
    std::cout << groupNums << std::endl;
#endif
    m_configIni->SetPath(_T("/OnlinePage"));

    // 雷达信号处理类相关变量初始设置
    m_udpParam = new UdpPacketParam;
    if (m_configIni->Read(_T("UdpPacketParam/bufSize"), &(m_udpParam->m_bufSize)));
    if (m_configIni->Read(_T("UdpPacketParam/bufScale"), &(m_udpParam->m_bufScale)));
    if (m_configIni->Read(_T("UdpPacketParam/bufOffset"), &(m_udpParam->m_bufOffset)));

#ifndef NDEBUG
    std::cout << m_udpParam->m_bufSize << " " << m_udpParam->m_bufScale << " " << m_udpParam->m_bufOffset << std::endl;
#endif

    // 摄像头捕获参数初始化
    m_capture = new cv::VideoCapture(0);             // 绑定摄像头
    wxString tempPath;
    if (m_configIni->Read(_T("Video/videoSavePath"), &tempPath))
    {
        m_videoPath = tempPath.ToStdString();              // 存视频的路径
    }
    m_outputVideo = new cv::VideoWriter;                   // 视频输出

#ifndef NDEBUG
    std::cout << m_videoPath << std::endl;
#endif
}

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
    m_insertPos = m_radarCube->GetFrame().begin();        // 定义插入帧数据中的位置初始化
    m_mdMapDrawFlag = true;                               // 表示开始绘制微多普勒图
    int needLostFrame = 5;                                // 需要丢弃的帧数 - 用于初始化参数修正类
    m_modifyFrame = new ModifyFrame(*m_radarParam,needLostFrame,*m_udpParam);
    m_FirstFixFlag = true;                                // 首次修正Flag

    // 存摄像头捕获的图片帧初始化
    m_singleFramePic = new cv::Mat;                       // 摄像头捕捉的单帧初始化
}

void PacketProcessThread::OnExit()
{

}

wxThread::ExitCode PacketProcessThread::Entry()
{
    // 线程自启动开始一直检查是否有包 - 有则处理无则等待
    while(!TestDestroy())
    {
        // 定义用于接收的指针
        UINT8* udpPacketPtr = nullptr;
        // 这里看了下Receive和ReceiveTimeOut的区别
        // Receive是阻塞的，一直等到队列中有可用数据，ReceiveTimeOut第一个参数设置为0后可以无阻塞调用
        wxMessageQueueError ret = m_fatherPanel->m_packetMsgQueue.ReceiveTimeout(0,udpPacketPtr);

        // 如果是超时事件，就跳过后续代码，继续等待
        if(ret == wxMSGQUEUE_TIMEOUT)
        {
            continue;
        }

        // 以下情况 - 包队列有UDP包数据
        // 由于是小端模式输入，seqNum表示序号 - 只取前两个
        unsigned long seqNum = udpPacketPtr[0] + udpPacketPtr[1] * 256;

        // 丢弃前面一些包
        if(m_FirstFixFlag && seqNum <= m_modifyFrame->GetRightByte().first)
        {
            delete[] udpPacketPtr;
            continue;
        }

        // 基于修正类的正确包和偏移量
        // 现在Seq是modifyFrame->getRightByte().first + 1
        if (m_FirstFixFlag)
        {
            int errorOffset = m_modifyFrame->GetRightByte().second;
            m_insertPos = std::copy_n(reinterpret_cast<int16_t *>(udpPacketPtr + m_udpParam->m_bufOffset + errorOffset),
                                      (m_udpParam->m_bufSize - m_udpParam->m_bufOffset - errorOffset) / m_udpParam->m_bufScale,
                                      m_insertPos);
            m_FirstFixFlag = false;  // 后面就正常了
            m_prePacketSeq = m_modifyFrame->GetRightByte().first + 1;
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
        outMessage.Printf(wxT(" 当前帧号 = %d ,  %d"), seqNum, m_udpPacketSeqFlag);
        wxLogMessage(outMessage);

        // 普通处理
        if(m_udpPacketSeqFlag)
        {
            // 帧数据流目前还剩余多少可以存放
            auto remainLength = m_radarCube->GetFrame().end() - m_insertPos;
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
                wxFile saveDataFile("data.bin", wxFile::write_append);
                // 使用wxFileOutputStream输出数据流 - 与创建出来的bin文件进行绑定
                wxFileOutputStream dataToFile(saveDataFile);
                dataToFile.Write(&(m_radarCube->GetFrame()[0]), m_radarParam->GetFrameBytes());

                // 雷达数据处理
                // 帧数据流存满 - 进行相应处理
                m_radarCube->CreatCube();
                m_radarCube->CreatRdm();
                // 用Bitmap创建wxImage对象
                int RdCols = m_radarCube->ConvertRdmToMap().cols, RdRows = m_radarCube->ConvertRdmToMap().rows;
                // 使用malloc分配一块动态内存 - 目的在于独立出这块图的数据
                void *RdData = malloc(3 * RdCols * RdRows);
                memcpy(RdData, (void *) m_radarCube->ConvertRdmToMap().data, 3 * RdCols * RdRows);
                wxImage *RdImage = new wxImage(RdCols, RdRows, (uchar *) RdData, false);

                // 微多普勒图也进行更新
                if (m_mdMapDrawFlag)
                {
                    m_radarCube->SetFlagForMap();
                    m_mdMapDrawFlag = false;
                }

                m_radarCube->UpdateMicroMap();
                int MdCols = m_radarCube->ConvertMdToMap().cols, MdRows = m_radarCube->ConvertMdToMap().rows;
                // 使用malloc分配一块动态内存 - 目的在于独立出这块图的数据
                void *MdData = malloc(3 * MdCols * MdRows);
                memcpy(MdData, (void *) m_radarCube->ConvertMdToMap().data, 3 * MdCols * MdRows);
                wxImage *MdImage = new wxImage(MdCols, MdRows, (uchar *) MdData, false);

                // 图像也进行存储
                // 注意:这里要特别说明一下，此块作用域内的代码均是得到一个完整的帧之后进行的处理，所以得到一个完整帧时，如下代码功能就是存储一帧摄像头数据
                // 目的:简单来讲，就是为了保持数据的存储处理和摄像头数据同步，处理得到Range Doppler和Micro Doppler时也得到摄像头一帧数据，回放时也就能完全匹配
                (*(m_fatherPanel->m_capture)) >> (*m_singleFramePic);
                // 存图像
                if (m_fatherPanel->m_outputVideo->isOpened())
                {
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
                MyPlotEvent* event = new MyPlotEvent( MY_PLOT_THREAD, ID_ONLINE_PROCESS );
                event->SetOfflineImage(RdImage,MdImage,VideoImage);

                // 线程安全
                wxQueueEvent( m_fatherPanel, event);

                // 一帧结束，重新设置insertPos 并把剩余的数据复制到帧数据流中存储
                m_insertPos = m_radarCube->GetFrame().begin();
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

void wxOnlinePagePanel::OnEnableDCA1000Click(wxCommandEvent& event)
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

void wxOnlinePagePanel::OnConnectUDPClick(wxCommandEvent& event)
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
    m_mySocket->SetEventHandler(*this, ID_ONLINE_SOCKET);
    // 定义的这个UDP socket只是用来接收的
    m_mySocket->SetNotify(wxSOCKET_INPUT_FLAG);
    // 监听wxSOCKET_INPUT_FLAG事件
    m_mySocket->Notify(true);
}

void wxOnlinePagePanel::OnEnableAWR1642Click(wxCommandEvent& event)
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
    Connect(ID_ONLINE_PROCESS, MY_PLOT_THREAD, wxMyPlotEventHandler(wxOnlinePagePanel::OnReceivePacketProcessThreadEvent));

    m_packetProcessThread = new PacketProcessThread(this);
    if ( m_packetProcessThread->Create() != wxTHREAD_NO_ERROR )
    {
        wxLogMessage(wxT("Can't open thread"));
        delete m_packetProcessThread;
        return;
    }
    m_packetProcessThread->Run();
    wxLogMessage(wxT("Run - Bin File Data Replay!"));
}

void wxOnlinePagePanel::OnDisconnectUDPClick(wxCommandEvent& event)
{
    m_mySocket->Destroy();
    m_mySocket->Notify(false);
    m_outputVideo->release();           // 摄像头保存设置为关闭
    m_packetProcessThread->Delete();    // 子线程关闭
    m_capture->release();               // 摄像头关闭
}

void wxOnlinePagePanel::OnDetectActionClick(wxCommandEvent& event)
{
    wxMessageBox(wxT("yes"));



}

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

        m_packetMsgQueue.Post(singleUdpBufPtr);

        // 上面wxSOCKET_LOST_FLAG 用以失去所有flag的bit位信息表示事件不触发
        // 现在要重新SetNotify wxSOCKET_INPUT_FLAG表示现在又要对socket数据缓存区的数据进行处理
        m_mySocket->SetNotify(wxSOCKET_INPUT_FLAG);
        break;
    }
    default:
        ;
    }
}

void wxOnlinePagePanel::OnReceivePacketProcessThreadEvent(MyPlotEvent& event)
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
wxBinReplayPagePanel::wxBinReplayPagePanel(wxPanel *parent)
    : wxPanel(parent)
{
    /* ---------------------------------------------- 控件设定 + 页面布局 ----------------------------------------------- */
    // 按钮
    m_logOutPut = new wxTextCtrl(this, wxID_ANY, wxT(""),wxPoint(-1, -1), wxSize(-1, -1), wxTE_MULTILINE);
    wxButton *btRun = new wxButton(this, ID_REPLAY_RUN, wxT("RUN"),wxDefaultPosition,wxSize(150,30));
    wxButton *btPause = new wxButton(this, ID_REPLAY_PAUSE, wxT("PAUSE"),wxDefaultPosition,wxSize(150,30));
    wxButton *btResume = new wxButton(this, ID_REPLAY_RESUME, wxT("RESUME"),wxDefaultPosition,wxSize(150,30));
    wxButton *btEnd = new wxButton(this, ID_REPLAY_END, wxT("END"),wxDefaultPosition,wxSize(150,30));
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
    Connect(ID_REPLAY_RUN, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxBinReplayPagePanel::OnReplayRunClick));
    Connect(ID_REPLAY_PAUSE, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxBinReplayPagePanel::OnReplayPauseClick));
    Connect(ID_REPLAY_RESUME, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxBinReplayPagePanel::OnReplayResumeClick));
    Connect(ID_REPLAY_END, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxBinReplayPagePanel::OnReplayEndClick));

    /* -------------------------------------------------- 参数初始化 -------------------------------------------------- */

}

BinReplayThread::BinReplayThread(wxBinReplayPagePanel *parent) : wxThread(wxTHREAD_DETACHED)
{
    m_fatherPanel = parent;
    m_mdMapDrawFlag = true;
}

void BinReplayThread::OnExit()
{

}

wxThread::ExitCode BinReplayThread::Entry()
{
    // 获取路径
    std::string binFileNameStr = m_fatherPanel->m_binPathStr.ToStdString();
    std::string videoFileNameStr = m_fatherPanel->m_videoPathStr.ToStdString();
    // avi频流地址
    cv::VideoCapture aviCapture(videoFileNameStr);
    // 用于暂存Video中的每一帧数据定义
    cv::Mat singleFrameInBin;

    // 离线数据 - 雷达信号处理类相关变量初始设置
    // todo - 可以设置为智能指针
    RadarParam *radarParam = new RadarParam;                            // RadarParam对象初始化
    RadarDataCube *radarCube = new RadarDataCube(*radarParam);          // RadarDataCube对象初始化
    int16_t *preBuf = new int16_t[radarParam->GetFrameBytes() / 2];     // 初始化buff
    std::vector<INT16>::iterator insertPos;                             // 设置初始复制位置
    insertPos = radarCube->GetFrame().begin();                          // 定义插入帧数据中的位置初始化

    // 获取帧数 - totalFrame
    std::ifstream ifs(binFileNameStr,std::ios::binary | std::ios::in);
    if(!ifs.is_open())
    {
        return (wxThread::ExitCode)0;
    }
    ifs.seekg(0, std::ios_base::end);
    long long nFileLen = ifs.tellg();
    long long totalFrame = nFileLen / radarParam->GetFrameBytes();
    // 文件指针回到开头
    ifs.clear();                 // 文件指针重定位前对流状态标志进行清除操作
    ifs.seekg(0,std::ios::beg);  // 文件指针重定位

    // 执行读取的文件
    while (totalFrame-- && !TestDestroy())
    {
        ifs.read((char *) preBuf, radarParam->GetFrameBytes());
        std::copy_n(preBuf, radarParam->GetFrameBytes() / 2, insertPos);

        // 雷达数据处理
        // 帧数据流存满 - 进行相应处理
        radarCube->CreatCube();
        radarCube->CreatRdm();
        int RdCols = radarCube->ConvertRdmToMap().cols, RdRows = radarCube->ConvertRdmToMap().rows;
        // 使用malloc分配一块动态内存 - 目的在于独立出这块图的数据
        void* RdData = malloc(3 * RdCols * RdRows);
        memcpy(RdData, (void *) radarCube->ConvertRdmToMap().data, 3 * RdCols * RdRows);
        wxImage* RdImage = new wxImage(RdCols, RdRows, (uchar *) RdData, false);

        // 微多普勒图也进行更新
        if (m_mdMapDrawFlag)
        {
            radarCube->SetFlagForMap();
            m_mdMapDrawFlag = false;
        }

        radarCube->UpdateMicroMap();
        int MdCols = radarCube->ConvertMdToMap().cols, MdRows = radarCube->ConvertMdToMap().rows;
        // 使用malloc分配一块动态内存 - 目的在于独立出这块图的数据
        void *MdData = malloc(3 * MdCols * MdRows);
        memcpy(MdData, (void *) radarCube->ConvertMdToMap().data, 3 * MdCols * MdRows);
        wxImage* MdImage = new wxImage(MdCols, MdRows, (uchar *) MdData, false);

        // 读取保存的视频
        aviCapture >> singleFrameInBin;
        cv::cvtColor(singleFrameInBin, singleFrameInBin, cv::COLOR_RGB2BGR);
        int VideoCols = singleFrameInBin.cols, VideoRows = singleFrameInBin.rows;
        // 使用malloc分配一块动态内存 - 目的在于独立出这块图的数据
        void *VideoData = malloc(3 * VideoCols * VideoRows);
        memcpy(VideoData, (void *) singleFrameInBin.data, 3 * VideoCols * VideoRows);
        wxImage* VideoImage = new wxImage(VideoCols, VideoRows, (uchar *) VideoData, false);

        // 两个图像矩阵处理结束，可以通知主线程进行绘制
        MyPlotEvent* event = new MyPlotEvent( MY_PLOT_THREAD, ID_REPLAY_PROCESS );
        event->SetOfflineImage(RdImage, MdImage, VideoImage);

        // 线程安全
        wxQueueEvent(m_fatherPanel, event );
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

void wxBinReplayPagePanel::OnReplayRunClick(wxCommandEvent& event)
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
        wxLogMessage(wxT("Can't open thread"));
        delete m_binReplayThread;
        return;
    }
    m_binReplayThread->Run();
    wxLogMessage(wxT("Run the thread successfully!"));
}

void wxBinReplayPagePanel::OnReplayPauseClick(wxCommandEvent& event)
{
    if (m_binReplayThread) // 线程是否仍旧存在
    {
        if (m_binReplayThread->Pause() != wxTHREAD_NO_ERROR)
            wxLogError("Can't pause the thread!");
        else  wxLogMessage(wxT("Pause the thread successfully!"));
    }
    else wxLogError("The thread has ended, please run again!");
}

void wxBinReplayPagePanel::OnReplayResumeClick(wxCommandEvent& event)
{
    if (m_binReplayThread) // 线程是否仍旧存在
    {
        if (m_binReplayThread->Resume() != wxTHREAD_NO_ERROR)
            wxLogError("Can't resume the thread!");
        else  wxLogMessage(wxT("Resume the thread successfully!"));
    }
    else wxLogError("The thread has ended, please run again!");
}

void wxBinReplayPagePanel::OnReplayEndClick(wxCommandEvent& event)
{
    if (m_binReplayThread) // 线程是否仍旧存在
    {
        if (m_binReplayThread->Delete() != wxTHREAD_NO_ERROR)
            wxLogError("Can't delete the thread!");
        else  wxLogMessage(wxT("Delete the thread successfully!"));
        // 在调用END后再调用Pause和Resume无效，除非重新调用Run开始
        m_binReplayThread = nullptr;
    }
    else wxLogError("The thread has ended, please run again!");
}

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
    wxButton *btBinPath = new wxButton(this, ID_REPLAY_BIN_PATH, wxT("Select Path..."),wxDefaultPosition,wxSize(150,30));
    wxButton *btVideoPath = new wxButton(this, ID_REPLAY_VIDEO_PATH, wxT("Select Path..."),wxDefaultPosition,wxSize(150,30));
    wxButton *btReset = new wxButton(this, ID_REPLAY_RESET, wxT("RESET"),wxDefaultPosition,wxSize(150,30));
    wxButton *btApply = new wxButton(this, ID_REPLAY_APPLY, wxT("APPLY"),wxDefaultPosition,wxSize(150,30));
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
    Connect(ID_REPLAY_BIN_PATH, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxReplayFileDialog::OnSetBinPathClick));
    Connect(ID_REPLAY_VIDEO_PATH, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxReplayFileDialog::OnSetVideoPathClick));
    Connect(ID_REPLAY_RESET, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxReplayFileDialog::OnResetClick));
    Connect(ID_REPLAY_APPLY, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxReplayFileDialog::OnApplyClick));

    // 在屏幕中居中显示
    Centre();
    // 显示消息对话框
    this->ShowModal();
}

void wxReplayFileDialog::OnSetBinPathClick(wxCommandEvent& event)
{
    // 创建wxFileDialog类获取文件路径 - 用于在界面中显示
    wxFileDialog * openFileDialog = new wxFileDialog(this);
    if (openFileDialog->ShowModal() == wxID_OK)
    {
        wxString fileName = openFileDialog->GetPath();
        m_binPath->SetValue(fileName);
    }
    delete openFileDialog;
}

void wxReplayFileDialog::OnSetVideoPathClick(wxCommandEvent& event)
{
    // 创建wxFileDialog类获取文件路径 - 用于在界面中显示
    wxFileDialog * openFileDialog = new wxFileDialog(this);
    if (openFileDialog->ShowModal() == wxID_OK)
    {
        wxString fileName = openFileDialog->GetPath();
        m_videoPath->SetValue(fileName);
    }
    delete openFileDialog;
}

void wxReplayFileDialog::OnResetClick(wxCommandEvent& event)
{
    m_binPath->Clear();
    m_videoPath->Clear();
}

void wxReplayFileDialog::OnApplyClick(wxCommandEvent& event)
{
    // 获取路径 - 用于代码
    m_father->m_binPathStr = m_binPath->GetValue();
    m_father->m_videoPathStr = m_videoPath->GetValue();
    this->Destroy();
}

/* ----------------------------------------------------- Page 3 ------------------------------------------------------
 ----------------------------------------------- wxOfflinePagePanel类 ---------------------------------------------- */
wxOfflinePagePanel::wxOfflinePagePanel(wxPanel *parent) : wxPanel(parent,wxID_ANY)
{
    /* ----------------------------------------------- 控件设定 + 页面布局 --------------------------------------------- */
    // 按钮
    m_logOutPut = new wxTextCtrl(this, wxID_ANY, wxT(""),wxPoint(-1, -1), wxSize(-1, -1), wxTE_MULTILINE);
    wxButton *btTrainExtra = new wxButton(this, ID_OFFLINE_TRAIN, wxT("TrainSet Extract"),wxDefaultPosition,wxSize(150,30));
    wxButton *btTestExtra = new wxButton(this, ID_OFFLINE_TEST, wxT("TestSet Extract"),wxDefaultPosition,wxSize(150,30));
    wxButton *btSvmModel = new wxButton(this, ID_OFFLINE_SVM, wxT("SVM Model Build"),wxDefaultPosition,wxSize(150,30));
    wxButton *btBinDemo = new wxButton(this, ID_OFFLINE_DEMO, wxT("Single Bin Test"),wxDefaultPosition,wxSize(150,30));
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
    wxStaticText *vmdWinText = new wxStaticText(this,wxID_STATIC,wxT("SVD Vector"));
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
    Connect(ID_OFFLINE_TRAIN, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxOfflinePagePanel::TrainDataSet));
    Connect(ID_OFFLINE_TEST, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxOfflinePagePanel::TestDataSet));
    Connect(ID_OFFLINE_SVM, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxOfflinePagePanel::SvmModel));
    Connect(ID_OFFLINE_DEMO, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxOfflinePagePanel::SingleBinDemo));

    /* ----------------------------------------------- 参数初始化 --------------------------------------------- */
    // 读取.ini文件中的配置来配置一些雷达参数
    m_configIni = new wxFileConfig(wxEmptyString,
                                   wxEmptyString,
                                   wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPathWithSep() + _T("config.ini"));
    m_configIni->SetPath(_T("/OfflinePage"));
}

void wxOfflinePagePanel::TrainDataSet(wxCommandEvent& event)
{
    // 输出消息重定位
    wxLog::SetActiveTarget(m_console);

    FeatureExtraThread * featureExtraThread = new FeatureExtraThread(this);
    if ( featureExtraThread->Create() != wxTHREAD_NO_ERROR )
    {
        wxLogMessage(wxT("Can't open thread"));
        delete featureExtraThread;
        return;
    }
    featureExtraThread->trainFlag = true;        // 设置为true表示训练训练集
    featureExtraThread->Run();
    wxLogMessage(wxT("Run the thread successfully!"));
}

void wxOfflinePagePanel::TestDataSet(wxCommandEvent& event)
{
    // 输出消息重定位
    wxLog::SetActiveTarget(m_console);

    FeatureExtraThread * featureExtraThread = new FeatureExtraThread(this);
    if ( featureExtraThread->Create() != wxTHREAD_NO_ERROR )
    {
        wxLogMessage(wxT("Can't open thread"));
        delete featureExtraThread;
        return;
    }
    featureExtraThread->trainFlag = false;        // 设置为false表示训练测试集
    featureExtraThread->Run();
    wxLogMessage(wxT("Run the thread successfully!"));
}

void wxOfflinePagePanel::SvmModel(wxCommandEvent& event)
{
    // 输出消息重定位
    wxLog::SetActiveTarget(m_console);
    OfflineFunction offlineFunc(this);
    offlineFunc.SvmPrediction();

    wxLogMessage(wxT("svm模型建立及预测处理完毕..."));
}

void wxOfflinePagePanel::SingleBinDemo(wxCommandEvent& event)
{
    // 输出消息重定向到TextCtrl窗口
    wxLog::SetActiveTarget(m_console);

    // 创建Dialog用于输入bin文件的路径
    wxOfflineDemoFileDialog* DemoBinFileDialog = new wxOfflineDemoFileDialog(this,wxT("Bin Demo File Path Setting"));

    // 得到Demo文件路径
    std::string binFileNameStr = m_binPathStr.ToStdString();

    // 创建离线操作类
    OfflineFunction offlineFunc(this);
    offlineFunc.SingleBinProcess(binFileNameStr);

    wxLogMessage(wxT("Bin文件微多普勒图生成及特征提取输出处理完毕..."));
}

wxOfflineDemoFileDialog::wxOfflineDemoFileDialog(wxOfflinePagePanel* parent,const wxString& title)
    :wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxSize(700, 140))
{
    // 设置父亲
    m_father = parent;
    // 静态文本设置
    wxStaticText *binPathText = new wxStaticText(this, wxID_ANY, wxT("Bin File Path"),wxDefaultPosition,wxSize(150,30));
    // 路径显示
    m_binPath = new wxTextCtrl(this, wxID_ANY, wxT(""),wxDefaultPosition,wxSize(300,30));
    // 按钮设置
    wxButton *btBinPath = new wxButton(this, ID_OFFLINE_BIN_PATH, wxT("Select Path..."),wxDefaultPosition,wxSize(150,30));
    wxButton *btReset = new wxButton(this, ID_OFFLINE_RESET, wxT("RESET"),wxDefaultPosition,wxSize(150,30));
    wxButton *btApply = new wxButton(this, ID_OFFLINE_APPLY, wxT("APPLY"),wxDefaultPosition,wxSize(150,30));
    // 设置布局控件
    wxBoxSizer *hBoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *hBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    hBoxSizer1->Add(binPathText,0,wxLEFT,10);
    hBoxSizer1->Add(m_binPath,0,wxLEFT,10);
    hBoxSizer1->Add(btBinPath,0,wxLEFT | wxRIGHT,10);
    hBoxSizer2->Add(btReset,0,wxLEFT,10);
    hBoxSizer2->Add(btApply,0,wxLEFT | wxRIGHT,10);
    wxBoxSizer *vBoxSizer = new wxBoxSizer(wxVERTICAL);
    vBoxSizer->Add(hBoxSizer1,0,wxALL | wxALIGN_CENTRE_HORIZONTAL,10);
    vBoxSizer->Add(hBoxSizer2,0,wxALL | wxALIGN_CENTRE_HORIZONTAL,10);
    this->SetSizer(vBoxSizer);

    // 链接响应事件
    Connect(ID_OFFLINE_BIN_PATH, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxOfflineDemoFileDialog::OnSetBinPathClick));
    Connect(ID_OFFLINE_RESET, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxOfflineDemoFileDialog::OnResetClick));
    Connect(ID_OFFLINE_APPLY, wxEVT_COMMAND_BUTTON_CLICKED,wxCommandEventHandler(wxOfflineDemoFileDialog::OnApplyClick));

    // 在屏幕中居中显示
    Centre();
    // 显示消息对话框
    this->ShowModal();
}

void wxOfflineDemoFileDialog::OnSetBinPathClick(wxCommandEvent& event)
{
    // 创建wxFileDialog类获取文件路径 - 用于在界面中显示
    wxFileDialog * openFileDialog = new wxFileDialog(this);
    if (openFileDialog->ShowModal() == wxID_OK)
    {
        wxString fileName = openFileDialog->GetPath();
        m_binPath->SetValue(fileName);
    }
    delete openFileDialog;
}

void wxOfflineDemoFileDialog::OnResetClick(wxCommandEvent& event)
{
    m_binPath->Clear();
}

void wxOfflineDemoFileDialog::OnApplyClick(wxCommandEvent& event)
{
    // 获取路径 - 用于代码
    m_father->m_binPathStr = m_binPath->GetValue();
    this->Destroy();
}

OfflineFunction::OfflineFunction(wxOfflinePagePanel *parent)
{
    // 得到父窗口指针
    m_father = parent;
    // 数据集划分参数初始化
    if (m_father->m_configIni->Read(_T("DataSet/trainSampleNum"), &(m_dividePara.m_trainSampleNum)));
    if (m_father->m_configIni->Read(_T("DataSet/testSampleNum"), &(m_dividePara.m_testSampleNum)));
    if (m_father->m_configIni->Read(_T("DataSet/featureDim"), &(m_dividePara.m_featureDim)));

#ifndef NDEBUG
    std::cout << m_dividePara.m_trainSampleNum << " " << m_dividePara.m_testSampleNum << " " << m_dividePara.m_featureDim << std::endl;
#endif
}

void OfflineFunction::GetFileNamesAndTag()
{
    // 选择部分数据集所在的文件夹位置
    wxDirDialog *openDirDialog = new wxDirDialog(m_father,wxT("Please select the folder where the data set is located..."));
    if(openDirDialog->ShowModal() != wxID_OK)
    {
        return;
    }

    size_t fileCount = 0;                             // 文件夹中对应类型的文件总数
    wxString dataSetPath = openDirDialog->GetPath();  // 获取指定文件夹路径
    wxDir dir(dataSetPath);                           // wxDir类允许枚举目录中的文件

    if(dir.IsOpened())
    {
        wxString filter = wxT("*.bin");             // 过滤指定文件
        fileCount = dir.GetAllFiles(dataSetPath,&m_filesArray,filter,wxDIR_DEFAULT);
    }

    // 每个bin文件的文件名中第二个字符表示动作类别
    // 例如 a1p1r1.bin 表示 动作类别1 - 第1个人 - 第1次重复执行
    // 读出tag表示该bin文件处理结果
    for(int i = 0;i < fileCount;++i)
    {
        wxString::reverse_iterator rIter = m_filesArray[i].rbegin();
        if(*(rIter + 7) != 'p'){
            int tag = *(rIter + 9) - '0';
            m_tagArray.push_back(tag);
        }
        else
        {
            int tag = *(rIter + 8) - '0';
            m_tagArray.push_back(tag);
        }
    }

#ifndef NDEBUG
    for(int i = 0;i < fileCount;++i)
    {
        std::cout << m_filesArray[i].ToStdString() << std::endl;
        std::cout << m_tagArray[i] << std::endl;
    }
#endif

    wxLogMessage(wxT("已读取指定文件夹的所有文件的路径及类别..."));
}

void OfflineFunction::TrainSetNormalized(arma::mat &sampleFeatureMat)
{
    // 得到训练集特征矩阵的每一行列的最大最小值
    arma::rowvec featureMax = max(sampleFeatureMat, 0);
    arma::rowvec featureMin = min(sampleFeatureMat, 0);

    // 标准化最大最小值
    int yMax = 1,yMin = -1;

    // 标准化
    for (arma::uword i = 0; i < m_dividePara.m_featureDim; i++)
    {
        sampleFeatureMat.col(i) = (yMax - yMin) * (sampleFeatureMat.col(i) - featureMin(i))
                                / (featureMax(i) - featureMin(i)) + yMin;
    }

    // 输出最大最小值文件(.txt)以保证测试集按照最大最小值来进行标准化
    std::ofstream maxMinFs("MaxMinOut.txt", std::ios::out);
    arma::rowvec::iterator it = featureMin.begin();
    arma::rowvec::iterator itEnd = featureMin.end();
    for(;it < itEnd;++it)
    {
        maxMinFs << (*it) << " ";
    }
    maxMinFs << std::endl;

    it = featureMax.begin();
    itEnd = featureMax.end();
    for (; it < itEnd; it++)
    {
        maxMinFs << (*it) << " ";
    }
    maxMinFs << std::endl;

    // 输出后，关闭文件描述符
    maxMinFs.close();

    wxLogMessage(wxT("训练集特征矩阵标准化完毕..."));
}

void OfflineFunction::TestSetNormalized(arma::mat &sampleFeatureMat)
{
    // 得到测试集特征矩阵的每一行列的最大最小值
    arma::rowvec featureMax = max(sampleFeatureMat, 0);
    arma::rowvec featureMin = min(sampleFeatureMat, 0);

    // 标准化最大最小值
    int yMax = 1,yMin = -1;

    // 读取训练集最大最小值数据
    std::ifstream maxMinFs("MaxMinOut.txt", std::ios::in);
    for (arma::uword i = 0; i < sampleFeatureMat.n_cols; i++)
    {
        maxMinFs >> featureMin(i);
    }
    for (arma::uword i = 0; i < sampleFeatureMat.n_cols; i++)
    {
        maxMinFs >> featureMax(i);
    }

    // 标准化
    for (arma::uword i = 0; i < m_dividePara.m_featureDim; i++)
    {
        sampleFeatureMat.col(i) = (yMax - yMin) * (sampleFeatureMat.col(i) - featureMin(i))
                                / (featureMax(i) - featureMin(i)) + yMin;
    }

    // 输出后，关闭文件描述符
    maxMinFs.close();

    wxLogMessage(wxT("测试集特征矩阵标准化完毕..."));
}

void OfflineFunction::PrintFeatureData(arma::mat& sampleFeatureMat,const std::string& outputFileName)
{
    std::ofstream outFs(outputFileName,std::ios::out);
    for(arma::uword i = 0;i < sampleFeatureMat.n_rows;++i)
    {
        for(arma::uword j = 0;j < sampleFeatureMat.n_cols;++j)
        {
            outFs << sampleFeatureMat.at(i,j) << " ";
        }
        outFs << std::endl;
    }

    // 文件输出完毕
    outFs.close();
}

void OfflineFunction::TrainSetProcess()
{
    // 创建特征矩阵用于存所有样本的特征 - 最后一维度为了存分类结果
    arma::mat sampleFeature(m_dividePara.m_trainSampleNum,m_dividePara.m_featureDim + 1,arma::fill::zeros);

    // 初始化数据
    RadarParam *radarParam = new RadarParam;                             // RadarParam对象初始化
    RadarDataCube *trainSingleCube = new RadarDataCube(*radarParam);  // RadarDataCube对象初始化
    int16_t *preBuf = new int16_t[radarParam->GetFrameBytes() / 2];      // 初始化buff
    std::vector<INT16>::iterator insertPos;                              // 设置初始复制位置

    wxLogMessage(wxT("开始从训练集对应文件夹中提取所有文件的特征..."));

    // 遍历所有训练集数据，用以得到特征数据 + 分类类别
    for(int i = 0;i < m_dividePara.m_trainSampleNum;++i)
    {
        std::string binFileName = m_filesArray[i].ToStdString();         // 获取要处理的训练集文件名
        insertPos = trainSingleCube->GetFrame().begin();                 // 定义插入帧数据中的位置初始化
        // 获取帧数 - totalFrame
        std::ifstream ifs(binFileName, std::ios::binary | std::ios::in);
        if(!ifs.is_open())
            return;
        ifs.seekg(0, std::ios_base::end);
        long long nFileLen = ifs.tellg();
        long long totalFrame = nFileLen / radarParam->GetFrameBytes();
        // 文件指针回到开头
        ifs.clear();                 // 文件指针重定位前对流状态标志进行清除操作
        ifs.seekg(0,std::ios::beg);  // 文件指针重定位
        bool m_mdMapDrawFlag = true;
        int frameCount = totalFrame;

        // 执行读取的文件
        while (frameCount--)
        {
            ifs.read((char *) preBuf, radarParam->GetFrameBytes());
            std::copy_n(preBuf, radarParam->GetFrameBytes() / 2, insertPos);

            // 雷达数据处理
            // 帧数据流存满 - 进行相应处理
            trainSingleCube->CreatCube();
            trainSingleCube->CreatRdm();

            // 微多普勒图也进行更新
            if (m_mdMapDrawFlag)
            {
                trainSingleCube->SetFlagForMap();
                m_mdMapDrawFlag = false;
            }

            trainSingleCube->UpdateStaticMicroMap(totalFrame);
        }

        // 所有处理结束后，会生成完整的微多普勒频谱，提取特征
        std::vector<arma::rowvec> featureVec = trainSingleCube->ExtractFeature();
        // 将提取的特征向量放在特征矩阵的第i行前部分
        sampleFeature.row(i).cols(0,m_dividePara.m_featureDim - 1) =
                join_rows(join_rows(featureVec[0],featureVec[1]),featureVec[2]);
        // 将提取的特征向量放在特征矩阵的第i行末尾
        sampleFeature.at(i,m_dividePara.m_featureDim) = m_tagArray[i];

        // 输出处理进度信息
        wxString outMessage;
        outMessage.Printf(wxT("--- 当前处理进度: %d / %d ---"),i + 1,m_dividePara.m_trainSampleNum);
        wxLogMessage(outMessage);
    }

    // 输出训练集的文件
    const std::string trainSetFileName = "trainData.txt";
    // 训练集特征数据标准化
    TrainSetNormalized(sampleFeature);
    // 输出所有特征数据
    PrintFeatureData(sampleFeature,trainSetFileName);

    // 释放动态内存
    delete radarParam;
    delete trainSingleCube;
    delete[] preBuf;
}

void OfflineFunction::TestSetProcess()
{
    // 创建特征矩阵用于存所有样本的特征 - 最后一维度为了存分类结果
    arma::mat sampleFeature(m_dividePara.m_testSampleNum,m_dividePara.m_featureDim + 1,arma::fill::zeros);

    // 初始化数据
    RadarParam *radarParam = new RadarParam;                             // RadarParam对象初始化
    RadarDataCube *testSingleCube = new RadarDataCube(*radarParam);      // RadarDataCube对象初始化
    int16_t *preBuf = new int16_t[radarParam->GetFrameBytes() / 2];      // 初始化buff
    std::vector<INT16>::iterator insertPos;                              // 设置初始复制位置

    wxLogMessage(wxT("开始从测试集对应文件夹中提取所有文件的特征..."));

    // 遍历所有训练集数据，用以得到特征数据 + 分类类别
    for(int i = 0;i < m_dividePara.m_testSampleNum;++i)
    {
        std::string binFileName = m_filesArray[i].ToStdString();             // 获取要处理的训练集文件名
        insertPos = testSingleCube->GetFrame().begin();                      // 定义插入帧数据中的位置初始化
        // 获取帧数 - totalFrame
        std::ifstream ifs(binFileName, std::ios::binary | std::ios::in);
        if(!ifs.is_open())
            return;
        ifs.seekg(0, std::ios_base::end);
        long long nFileLen = ifs.tellg();
        long long totalFrame = nFileLen / radarParam->GetFrameBytes();
        // 文件指针回到开头
        ifs.clear();                 // 文件指针重定位前对流状态标志进行清除操作
        ifs.seekg(0,std::ios::beg);  // 文件指针重定位
        bool m_mdMapDrawFlag = true;
        int frameCount = totalFrame;

        // 执行读取的文件
        while (frameCount--)
        {
            ifs.read((char *) preBuf, radarParam->GetFrameBytes());
            std::copy_n(preBuf, radarParam->GetFrameBytes() / 2, insertPos);

            // 雷达数据处理
            // 帧数据流存满 - 进行相应处理
            testSingleCube->CreatCube();
            testSingleCube->CreatRdm();

            // 微多普勒图也进行更新
            if (m_mdMapDrawFlag)
            {
                testSingleCube->SetFlagForMap();
                m_mdMapDrawFlag = false;
            }

            testSingleCube->UpdateStaticMicroMap(totalFrame);
        }

        // 所有处理结束后，会生成完整的微多普勒频谱，提取特征
        std::vector<arma::rowvec> featureVec = testSingleCube->ExtractFeature();
        // 将提取的特征向量放在特征矩阵的第i行前部分
        sampleFeature.row(i).cols(0,m_dividePara.m_featureDim - 1) =
                join_rows(join_rows(featureVec[0],featureVec[1]),featureVec[2]);
        // 将提取的特征向量放在特征矩阵的第i行末尾
        sampleFeature.at(i,m_dividePara.m_featureDim) = m_tagArray[i];

        // 输出处理进度信息
        wxString outMessage;
        outMessage.Printf(wxT("--- 当前处理进度: %d / %d ---"),i + 1,m_dividePara.m_testSampleNum);
        wxLogMessage(outMessage);
    }

    // 输出测试集的文件
    const std::string testSetFileName = "testData.txt";
    // 测试集特征数据标准化
    TestSetNormalized(sampleFeature);
    // 输出所有特征数据
    PrintFeatureData(sampleFeature,testSetFileName);

    // 释放动态内存
    delete radarParam;
    delete testSingleCube;
    delete[] preBuf;
}

void OfflineFunction::SvmPrediction()
{
    MySvm mySvm(m_dividePara);       // 建立svm相关处理类
    mySvm.TrainSvmModel();           // 训练svm以得到svm模型
    mySvm.PredictSvm();              // 用测试集大体预测
}

void OfflineFunction::SingleBinProcess(std::string binFileNameStr)
{
    // 输出消息
    wxLogMessage(wxT("请耐心等待结果输出 ... "));

    // 雷达信号处理类相关变量初始设置
    RadarParam *radarParam = new RadarParam;                          // RadarParam对象初始化
    RadarDataCube *radarCube = new RadarDataCube(*radarParam);     // RadarDataCube对象初始化
    int16_t *preBuf = new int16_t[radarParam->GetFrameBytes() / 2];   // 初始化buff

    std::vector<INT16>::iterator insertPos;                           // 设置初始复制位置
    insertPos = radarCube->GetFrame().begin();                        // 定义插入帧数据中的位置初始化

    // 获取帧数 - totalFrame
    std::ifstream ifs(binFileNameStr, std::ios::binary | std::ios::in);
    if(!ifs.is_open())
        return;
    ifs.seekg(0, std::ios_base::end);
    long long nFileLen = ifs.tellg();
    long long totalFrame = nFileLen / radarParam->GetFrameBytes();
    // 文件指针回到开头
    ifs.clear();                 // 文件指针重定位前对流状态标志进行清除操作
    ifs.seekg(0,std::ios::beg);  // 文件指针重定位

    bool m_mdMapDrawFlag = true;

    int frameCount = totalFrame;
    // 执行读取的文件
    while (frameCount--)
    {
        ifs.read((char *) preBuf, radarParam->GetFrameBytes());
        std::copy_n(preBuf, radarParam->GetFrameBytes() / 2, insertPos);

        // 雷达数据处理
        // 帧数据流存满 - 进行相应处理
        radarCube->CreatCube();
        radarCube->CreatRdm();

        // 微多普勒图也进行更新
        if (m_mdMapDrawFlag)
        {
            radarCube->SetFlagForMap();
            m_mdMapDrawFlag = false;
        }

        radarCube->UpdateStaticMicroMap(totalFrame);
    }

    // 所有处理结束后，会生成完整的微多普勒频谱，提取特征
    std::vector<arma::rowvec> featureVec = radarCube->ExtractFeature();

#ifndef NDEBUG
    // cv::imshow("aa",radarCube->convertMdToStaticMap(totalFrame));
    // cv::waitKey(0);
#endif

    int RdCols = radarCube->ConvertMdToStaticMap(totalFrame).cols, RdRows = radarCube->ConvertMdToStaticMap(totalFrame).rows;
    // 使用malloc分配一块动态内存 - 目的在于独立出这块图的数据
    void *RdData = malloc(3 * RdCols * RdRows);
    memcpy(RdData, (void *) radarCube->ConvertMdToStaticMap(totalFrame).data, 3 * RdCols * RdRows);
    wxImage *RdImage = new wxImage(RdCols, RdRows, (uchar *) RdData, false);

    m_father->m_mdPic->SetBitmap(*RdImage,0,0,128,150);

    m_father->m_mdWin->UpdateAll();
    m_father->m_mdWin->Fit();


#ifndef NDEBUG
    featureVec[0].print();
    std::cout << "-------------------------" << std::endl;
    featureVec[1].print();
    std::cout << "-------------------------" << std::endl;
    featureVec[2].print();
    std::cout << "-------------------------" << std::endl;
#endif

    // 进行绘制 - 设置三个特征矢量的y坐标
    std::vector<double> vecCur1y, vecCur2y, vecCur3y;
    arma::rowvec::iterator mIt = featureVec[0].begin();
    arma::rowvec::iterator mIt_end = featureVec[0].end();
    for (; mIt != mIt_end; mIt++)
        vecCur1y.push_back(*mIt);

    mIt = featureVec[1].begin();
    mIt_end = featureVec[1].end();
    for (; mIt != mIt_end; mIt++)
        vecCur2y.push_back(*mIt);

    mIt = featureVec[2].begin();
    mIt_end = featureVec[2].end();
    for (; mIt != mIt_end; mIt++)
        vecCur3y.push_back(*mIt);
    // 进行绘制 - 设置三个特征矢量的x坐标
    std::vector<double> vecCur1x, vecCur2x, vecCur3x;
    double timeRes = 0.03;
    for (int p = 0; p < vecCur1y.size(); ++p)
        vecCur1x.push_back(p*timeRes);

    for (int p = 0; p < vecCur2y.size(); ++p)
        vecCur2x.push_back(p*timeRes);

    for (int p = 0; p < vecCur3y.size(); ++p)
        vecCur3x.push_back(p);

    m_father->m_torsoCurve->SetData(vecCur1x,vecCur1y);
    m_father->m_torsoCurve->SetContinuity(true);
    m_father->m_limbsCurve->SetData(vecCur2x,vecCur2y);
    m_father->m_limbsCurve->SetContinuity(true);
    m_father->m_vmdCurve->SetData(vecCur3x,vecCur3y);
    m_father->m_vmdCurve->SetContinuity(true);

    // 坐标轴相关更新
    auto minMax1y = std::minmax_element(vecCur1y.begin(),vecCur1y.end());
    auto minMax2y = std::minmax_element(vecCur2y.begin(),vecCur2y.end());
    auto minMax3y = std::minmax_element(vecCur3y.begin(),vecCur3y.end());

    m_father->m_torsoWin->Update();
    m_father->m_torsoWin->Fit(-15*timeRes,vecCur1x.size()*timeRes,
                    (*minMax1y.first)*1.5,(*minMax1y.second)*1.5);
    m_father->m_limbsWin->Update();
    m_father->m_limbsWin->Fit(-15*timeRes,vecCur2x.size()*timeRes,
                    -(*minMax2y.second)*0.5,(*minMax2y.second)*1.5);
    m_father->m_vmdWin->Update();
    m_father->m_vmdWin->Fit(-15,vecCur3x.size(),
                  (*minMax3y.first)*1.5,-(*minMax3y.first)*0.5);

    // 关闭文件 + 释放内存
    // 关闭文件
    ifs.close();
    // 释放内存
    delete[] preBuf;
    delete radarParam;
    delete radarCube;
}

FeatureExtraThread::FeatureExtraThread(wxOfflinePagePanel *parent) : wxThread(wxTHREAD_DETACHED)
{
    m_fatherPanel = parent;
    trainFlag = true;
}

wxThread::ExitCode FeatureExtraThread::Entry()
{
    // 情况1 - 处理训练集
    if(trainFlag)
    {
        // 创建离线操作类
        OfflineFunction offlineFunc(m_fatherPanel);
        offlineFunc.GetFileNamesAndTag();
        offlineFunc.TrainSetProcess();

        wxLogMessage(wxT("训练集处理完毕..."));
        wxLogMessage(wxT("请您继续完成测试集的特征提取..."));
        return (wxThread::ExitCode)0;
    }

    // 情况2 - 处理测试集
    // 创建离线操作类
    OfflineFunction offlineFunc(m_fatherPanel);
    offlineFunc.GetFileNamesAndTag();
    offlineFunc.TestSetProcess();

    wxLogMessage(wxT("测试集处理完毕..."));
    wxLogMessage(wxT("可以继续执行svm分类等操作..."));

    return (wxThread::ExitCode)0;
}

void FeatureExtraThread::OnExit()
{

}