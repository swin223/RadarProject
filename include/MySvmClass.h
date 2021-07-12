#ifndef MYSVMCLASS_H
#define MYSVMCLASS_H

#include <fstream>
#include <armadillo>
#include <deque>
#include "svm.h"
#include "wx/wx.h"

/**
 * @brief DividePara类
 * 数据集划分类
 * @details 用于存储数据集中测试集/训练集个数及特征维度
 */
struct DividePara
{
    int m_trainSampleNum;               ///< 训练集的个数
    int m_testSampleNum;                ///< 测试集的个数
    int m_featureDim;                   ///< 特征总维度
};

/**
 * @brief MySvmClass类
 * svm相关调用类
 * @details 用于从训练集中得到svm模型,并可通过预测测试集来显示大体精度
 */
class MySvmClass
{
public:
    MySvmClass(DividePara &dividePara); ///< 构造函数
    ~MySvmClass();                      ///< 析构函数
    void GetSvmModel();                 ///< 生成svm模型
    void predictSvm();                  ///< 预测测试集数据

private:
    void InitParam();                   ///< 初始化svm训练参数
    void ReadTrainFeatureData();        ///< 读取已提取的训练集特征向量数据
    void ReadTestFeatureData();         ///< 读取已提取的测试集特征向量数据

private:
    DividePara m_dividePara;                 ///< 数据集划分参数
    std::deque<svm_node *> m_trainDataDeque; ///< 用于存储svm_node结构体双端队列(训练集)
    std::deque<svm_node *> m_testDataDeque;  ///< 用于存储svm_node结构体双端队列(测试集)
    std::deque<double> m_trainTagDeque;      ///< 用于记录标签数据的双端队列(训练集)
    std::deque<double> m_testTagDeque;       ///< 用于记录标签数据的双端队列(测试集)
    svm_parameter m_svmParam;                ///< svm超参
    svm_problem m_svmProb;                   ///< svm规范输入结构体(用于生成svmModel)
};
































#endif //MYSVMCLASS_H
