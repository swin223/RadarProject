#include "MySvmClass.h"

MySvm::MySvm(DividePara &dividePara)
{
    m_dividePara = dividePara;
}

MySvm::~MySvm()
{

}

void MySvm::InitParam()
{
    m_svmParam.svm_type = C_SVC;
    m_svmParam.kernel_type = POLY;
    m_svmParam.degree = 3;
    m_svmParam.gamma = 0.5;
    m_svmParam.coef0 = 0;
    m_svmParam.nu = 0.5;
    m_svmParam.cache_size = 50;
    m_svmParam.C = 500;
    m_svmParam.eps = 1e-3;
    m_svmParam.p = 0.1;
    m_svmParam.shrinking = 1;
    m_svmParam.nr_weight = 0;
    m_svmParam.weight = NULL;
    m_svmParam.weight_label = NULL;
}

void MySvm::ReadTrainFeatureData()
{
    // 读取指定文件
    std::string fileName = "../03_DataSet/trainData.txt";
    std::ifstream trainIfs(fileName, std::ios::in);
    if (!trainIfs.is_open())
    {
        wxLogMessage(wxT("文件" + fileName + "找不到!"));
        return;
    }
    // 指定一个临时值用于给trainIfs逐个读取数据
    double tempVal = 0.0;

    for(int i = 0; i < m_dividePara.m_trainSampleNum; ++i)
    {
        // 一个svm_node用于存一个Sample的一个维度的特征(最后一个用于标记结束)
        svm_node* singleSample = new svm_node[m_dividePara.m_featureDim + 1];
        for(int j = 0; j < m_dividePara.m_featureDim; ++j)
        {
            trainIfs >> tempVal;
            singleSample[j].index = j + 1;      // index成员表示第N维度，从1开始
            singleSample[j].value = tempVal;    // value成员表示第N维度的值
        }
        // svm_node向量的最后一个数据用于标记结束(index = -1)
        singleSample[m_dividePara.m_featureDim].index = -1;

        // 读取对应该Sample的分类结果 - 9类动作对应(1-9)
        trainIfs >> tempVal;
        double tag = tempVal;

        // 将单个Sample的维度数据向量指针 + 分类结果保存到双端队列中
        m_trainDataDeque.push_back(singleSample);
        m_trainTagDeque.push_back(tag);
    }

    // 读取结束
    trainIfs.close();
}

void MySvm::ReadTestFeatureData()
{

    std::string fileName = "../03_DataSet/testData.txt";
    // 读取指定文件
    std::ifstream testIfs(fileName, std::ios::in);
    if (!testIfs.is_open())
    {
        wxLogMessage(wxT("文件" + fileName + "找不到!"));
        return;
    }
    // 指定一个临时值用于给testIfs逐个读取数据
    double tempVal = 0.0;

    for(int i = 0; i < m_dividePara.m_testSampleNum; ++i)
    {
        // 一个svm_node用于存一个Sample的一个维度的特征(最后一个用于标记结束)
        svm_node* singleSample = new svm_node[m_dividePara.m_featureDim + 1];
        for(int j = 0; j < m_dividePara.m_featureDim; ++j)
        {
            testIfs >> tempVal;
            singleSample[j].index = j + 1;      // index成员表示第N维度，从1开始
            singleSample[j].value = tempVal;    // value成员表示第N维度的值
        }
        // svm_node向量的最后一个数据用于标记结束(index = -1)
        singleSample[m_dividePara.m_featureDim].index = -1;

        // 读取对应该Sample的分类结果 - 9类动作对应(1-9)
        testIfs >> tempVal;
        double tag = tempVal;

        // 将单个Sample的维度数据向量指针 + 分类结果保存到双端队列中
        m_testDataDeque.push_back(singleSample);
        m_testTagDeque.push_back(tag);
    }

    // 读取结束
    testIfs.close();
}

void MySvm::TrainSvmModel()
{
    // 初始化相应的超参数
    InitParam();
    // 读取训练集数据
    ReadTrainFeatureData();
    // 整理svm规范输入结构体
    m_svmProb.l = m_dividePara.m_trainSampleNum;
    m_svmProb.x = new svm_node*[m_svmProb.l];
    m_svmProb.y = new double[m_svmProb.l];

    // 将双端队列中数据整理至svm规范输入结构体中，以用来训练
    // 同时，释放已用完的双端队列中的动态内存数据
    for(int i = 0;i < m_svmProb.l;++i)
    {
        svm_node *singleSamplePtr = m_trainDataDeque.front();
        m_svmProb.x[i] = singleSamplePtr;
        m_trainDataDeque.pop_front();
        // pop后继续存末尾，最后一次性delete，不然容易造成内存泄漏
        m_trainDataDeque.push_back(singleSamplePtr);

        double singleSampleTag = m_trainTagDeque.front();
        m_svmProb.y[i] = singleSampleTag;
        m_trainTagDeque.pop_front();
    }

    // svm模型训练 + 保存
    wxLogMessage(wxT(" ---------------------------- "));
    wxLogMessage(wxT(" -------- 开始训练 --------- "));
    svm_model* SvmModel = svm_train(&m_svmProb, &m_svmParam);
    wxLogMessage(wxT(" ----- 正在保存模型 ----- "));
    std::string svmModelName = "SvmModel";
    svm_save_model(svmModelName.c_str(), SvmModel);
    wxLogMessage(wxT(" -------- 保存完毕 -------- "));
    wxLogMessage(wxT(" ---------------------------- "));

    // 释放动态内存
    while(!m_trainDataDeque.empty())
    {
        svm_node *temp = m_trainDataDeque.front();
        m_trainDataDeque.pop_front();
        delete[] temp;
    }
    delete[] m_svmProb.x;
    delete[] m_svmProb.y;
}

void MySvm::PredictSvm()
{
    // 搜索保存的svm模型并打开
    wxLogMessage(wxT(" ------------------------------------------ "));
    wxLogMessage(wxT(" -------- 正在打开已训练的模型 -------- "));
    std::string svmModelName = "SvmModel";
    svm_model* svmModel = svm_load_model(svmModelName.c_str());
    // 读取测试集数据
    ReadTestFeatureData();

    // 计算相关正确率
    // 与训练不同，测试需要单文件测试
    wxLogMessage(wxT(" -------- 已开始预测整体正确率 -------- "));
    int trueNum = 0;
    for(int i = 0;i < m_dividePara.m_testSampleNum;++i)
    {
        // 将双端队列中样本特征数组指针直接输入，以用来预测
        // 同时，释放已用完的双端队列中的动态内存数据
        svm_node *singleSamplePtr = m_testDataDeque.front();
        double predictTag = svm_predict(svmModel,singleSamplePtr);
        m_testDataDeque.pop_front();
        delete[] singleSamplePtr;

        // 得到真实Tag
        double trueTag = m_testTagDeque.front();
        m_testTagDeque.pop_front();

        // double有精度问题，精确比较采用int
        if(static_cast<int>(predictTag) == static_cast<int>(trueTag)) ++trueNum;
    }

    // 计算正确率
    wxLogMessage(wxT(" --------------- 预测完毕 ---------------- "));
    double score = static_cast<double>(trueNum) / m_dividePara.m_testSampleNum * 100;
    wxString outMessage;
    outMessage.Printf(wxT("--------- 预测正确率 = %.2lf"),score);
    outMessage += "%%---------";
    wxLogMessage(outMessage);
    wxLogMessage(wxT(" ------------------------------------------ "));
}