#ifndef MYSVMCLASS_H
#define MYSVMCLASS_H

#include <fstream>
#include <armadillo>
#include <deque>
#include "svm.h"
#include "wx/wx.h"

/** 数据集划分类
 * @details 用于存储数据集中测试集/训练集个数及特征维度
 */
struct DividePara
{
    int m_trainSampleNum;               ///< 训练集的个数
    int m_testSampleNum;                ///< 测试集的个数
    int m_featureDim;                   ///< 特征总维度
};

/** 自定义svm类
 * @details 用于从训练集中得到svm模型,并可通过预测测试集来显示大体精度
 */
class MySvm
{
public:
    /** MySvm构造函数
     * @param dividePara 数据集划分参数
     * @details 用数据集划分参数来进行有参构造
     */
    MySvm(DividePara &dividePara);

    /** 析构函数 */
    ~MySvm();

    /** 生成svm模型
    * @details 将训练集整合后的规范输入结构体、svm超参数作为输入，输出svm模型
    */
    void TrainSvmModel();

    /** 预测测试集数据
    * @details 用于大体预测svm的精度
    */
    void PredictSvm();

private:
    /** 初始化svm训练参数
     * @details 在生成svm的模型时所要传递的一个参数
     * @note 可修改，但是一般情况下默认调用即可
     */
    void InitParam();

    /** 读取已提取的训练集特征向量数据
    * @details 从用于存储训练集特征向量的txt文件中读取特征数据以及分类结果，
    * 用于svm模型的训练以及生成
    */
    void ReadTrainFeatureData();

    /** 读取已提取的测试集特征向量数据
     * @details 从用于存储测试集特征向量的txt文件中读取特征数据以及分类结果，
     * 用于对已生成的svm模型进行正确率大体检测
     */
    void ReadTestFeatureData();

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