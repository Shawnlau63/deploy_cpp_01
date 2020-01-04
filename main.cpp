#include <iostream>
#include <opencv2/opencv.hpp>
#include <memory>
#include <time.h>
#include "windowsx.h"

#include "service.hpp"
#include "detector.hpp"

using namespace std;
using namespace cv;

struct MsgCallback : public Callback {

    Detector *lpDetector;

    MsgCallback() {
        lpDetector = new Detector("act_m.dll");
    }

    virtual void handle(const SOCKET &socket, const std::vector<char> &v_data) {

        cv::Mat image = cv::imdecode(cv::Mat(v_data), CV_LOAD_IMAGE_COLOR);
        torch::Tensor tensor_image = torch::from_blob(image.data, {1, image.rows, image.cols, 3}, torch::kByte);
        auto rst = lpDetector->forward(tensor_image);

        unsigned int n_magic = htonl(0x12345678);
        unsigned int magic_len = sizeof(n_magic);
        unsigned int data_len = sizeof(float) * rst.size(0) * rst.size(1);
        unsigned int n_data_len = htonl(data_len);
        unsigned int head_len = sizeof(n_magic) + sizeof(data_len);
        unsigned int buf_len = head_len + data_len;

        char *buf = new char[buf_len];
        memcpy(buf, &n_magic, sizeof(n_magic));
        memcpy(buf + magic_len, &n_data_len, sizeof(n_data_len));
        memcpy(buf + head_len, (char *) rst.data_ptr(), data_len);

        send(socket, buf, buf_len, 0);

        cout << rst << endl;
    }

    virtual ~MsgCallback() {
        delete lpDetector;
    }
};

int main(int, char **) {
    time_t timer;
    time(&timer);
    tm* t_tm = localtime(&timer);


    if(t_tm->tm_year>119){
        cout<<"Use expired"<<endl;
    }else {
        MsgCallback msgCallback;
        WinService winService(6000, msgCallback);
        winService.strat();
    }
    return 0;
}