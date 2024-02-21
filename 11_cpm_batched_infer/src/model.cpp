#include "model.hpp"
#include "logger.hpp"
#include "utils.hpp"
#include <vector>
#include <future>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <queue>
#include <chrono>
#include "opencv2/highgui.hpp"
#include "opencv2/opencv.hpp"

using namespace std;

namespace model{

struct Job{
    string path;
    shared_ptr<promise<img>> tar;
};

class ModelImpl : public Model{

public:
    ModelImpl(string listPath, int batchSize):
        m_listPath(listPath), m_batchSize(batchSize)
    {
        m_imgPaths = loadDataList(m_listPath);
        m_imgPaths.resize(static_cast<int>(m_imgPaths.size() / m_batchSize) * m_batchSize);
    };

    /* 
     * 析构函数: 
     *  当ModelImpl可以释放资源的时候，也自动的把线程同步
     */
    ~ModelImpl() {
        stop();
    };

    void stop() {
        /* 如果正在执行，那么就停止, 并且通知所有正在阻塞的consumer*/
        if (m_running){
            m_running = false;
            m_cv.notify_all();
        }

        /* 对于所有线程进行join处理 */
        for (int i = 0; i < m_batchSize; i ++){
            if (m_workers[i].joinable()){
                LOGV(DGREEN"[consumer] consumer%d release" CLEAR, i);
                m_workers[i].join();
            }
        }
    }

    /*
     * 初始化:
     *  创建consumer所用的线程，个数为batchSize个
     *  并为每一个consumer线程绑定消费者函数inference
    */
    bool initialization(){
        m_running = true;

        m_workers.reserve(m_batchSize);
        m_batchedImgPaths.reserve(m_imgIndex);

        for (int i = 0; i < m_batchSize; i ++){
            m_workers.push_back(thread(&ModelImpl::inference, this));
            LOGV(GREEN"[producer]created consumer%d" CLEAR, i);
        }
        return true;
    }

    /*
     * 前向推理:
     *  在模型内部进行生产者和消费者的交互
     *  每获取以及处理完一个batch的图片之后，进行一次同步，之后再获取下一个batch的图片
     *  让生产者只要imageList没有读完数据，就进行commit
    */
    void forward() override {
        while (getBatch()){
            auto results = commits();
            for (auto& res: results) {
                img info = res.get();
            }
            m_batchedImgPaths.clear();
        }
        stop();
    }

    /*
     * 获取一个batch的数据, 给放到batchedImgPaths中
    */
    bool getBatch(){
        if (m_imgIndex + m_batchSize >= m_imgPaths.size() + 1)
            return false;

        LOG("%3d/%3d: %s",
            m_imgIndex + 1, m_imgPaths.size(), m_imgPaths.at(m_imgIndex).c_str());
        
        for (int i = 0; i < m_batchSize; i ++) {
            m_batchedImgPaths.emplace_back(m_imgPaths.at(m_imgIndex++));
        }
        return true;
    }

    /* 
     * 生产者: producer
     *  负责分配job, 每一个job负责管理一个图片数据，以及对应的future
     *  返回一个future的vector, 分配对应每一个job
    */
    vector<shared_future<img>> commits() {
        vector<Job> jobs(m_batchSize);
        vector<shared_future<img>> futures(m_batchSize);

        /* 设置一批job, 并对每一个job的promise设置对应的future*/
        for (int i = 0; i < m_batchSize; i ++){
            jobs[i].path = m_batchedImgPaths[i];
            jobs[i].tar.reset(new promise<img>());
            futures[i] = jobs[i].tar->get_future();
        }

        /* 对jobQueue进行原子push */
        {
            unique_lock<mutex> lock(m_mtx);
            for (int i = 0; i < m_batchSize; i ++){
                m_jobQueue.emplace(move(jobs[i]));
            }
        }

        /* 唤醒所有堵塞的consumer */
        m_cv.notify_all();

        LOGV(BLUE"[producer]finished commits" CLEAR);
        return futures;
    }

    /* 
     * 消费者: consumer
     *  消费者是一直在启动着的
     *  只要jobQueue有数据，就处理job, 并更新job内部的promise
     *  可以多个consumer处理同一个jobQueue
    */
    void inference() {
        while(m_running){
            Job job;
            img result;

            /* 对jobQueue进行原子pop */
            {
                unique_lock<mutex> lock(m_mtx);

                /* 
                 * consumer不被阻塞只有两种情况：
                 *  1. jobQueue不是空，这个时候需要consumer去consume
                 *  2. model已经被析构了，所有的线程都需要停止
                 *
                 * 所以，相应的condition_variable的notify也需要有两个
                 *  1. 当jobQueue已经填好了数据的时候
                 *  2. 当model已经被析构的时候
                 */
                m_cv.wait(lock, [&](){
                    return !m_running || !m_jobQueue.empty();
                });

                /* 如果model已经被析构了，就跳出循环, 不再consume了*/
                if (!m_running) break;

                job = m_jobQueue.front();
                m_jobQueue.pop();
            }
            LOGV(DGREEN"[consumer] Consumer processing %s" CLEAR, job.path.c_str());

            /*
             * 对取出来的job数据进行处理，并更新promise
             * 这里面对应着batched inference之后的各个task的postprocess
             * 由于相比于GPU上的操作，这里的postprocess一般会在CPU上操作，所以会比较慢
             * 因此可以选择考虑多线程异步执行
             * 这里为了达到耗时效果，选择在CPU端做如下操作:
             *  1. letterbox resize
             *  2. bgr2rgb
             *  3. rgb2bgr
             */
            auto  image    = cv::imread(job.path.c_str());
            int   input_w  = image.cols;
            int   input_h  = image.rows;
            int   target_w = 800;
            int   target_h = 800;
            float scale    = min(float(target_w)/input_w, float(target_h)/input_h);
            int   new_w    = int(input_w * scale);
            int   new_h    = int(input_h * scale);
            
            cv::Mat tar(target_w, target_h, CV_8UC3, cv::Scalar(0, 0, 0));
            cv::Mat tmp;
            cv::resize(image, tmp, cv::Size(new_w, new_h));

            int x, y;
            x = (new_w < target_w) ? (target_w - new_w) / 2 : 0;
            y = (new_h < target_h) ? (target_h - new_h) / 2 : 0;

            cv::Rect roi(x, y, new_w, new_h);

            cv::Mat roiOfTar = tar(roi);
            tmp.copyTo(roiOfTar);

            cv::cvtColor(tar, tar, cv::COLOR_BGR2RGB);
            cv::cvtColor(tar, tar, cv::COLOR_RGB2BGR);

            result.path = changePath(job.path, "../results", ".png", "result");
            result.data = tar;

            job.tar->set_value(result);
            cv::imwrite(result.path, result.data);
            // this_thread::sleep_for(chrono::milliseconds(2000));
            LOGV(DGREEN"[consumer] Finished processing, save to %s" CLEAR, result.path.c_str());
        }
    }


private:
    string             m_listPath;
    vector<string>     m_imgPaths;
    vector<string>     m_batchedImgPaths;
    int                m_imgIndex{0};

    int                m_batchSize;
    queue<Job>         m_jobQueue;
    mutex              m_mtx;
    condition_variable m_cv;
    vector<thread>     m_workers;
    bool               m_running{false};
};

// RAII模式对实现类进行资源获取即初始化
std::shared_ptr<Model> create_model (std::string listPath, int batchSize){
    shared_ptr<ModelImpl> ins(new ModelImpl(listPath, batchSize));
    if (!ins->initialization())
        ins.reset(); //释放shared_ptr所拥有的对象
    return ins;
}

} //namespace model
