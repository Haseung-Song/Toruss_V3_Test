// MOTWrapper.h
#pragma once
#ifdef MOTWRAPPER_EXPORTS
#define MOT_API __declspec(dllexport)
#else
#define MOT_API __declspec(dllimport)
#endif
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <cstring>
const int MAX_DETECTIONS = 100;

struct LetterboxInfo {
    float scale;
    int pad_w;
    int pad_h;
};

struct MOT_API TrackingResult {
    int id;             // track id
    cv::Rect2f box;     // bounding box
    int cam_ch;         // channel
    int cls;
    float prob;
};

class MOT_API MOTWrapper {
public:
    MOTWrapper();
    ~MOTWrapper();

    // gpu_id(int)를 인자로 받아 해당 번호의 gpu를 이용하도록 dll 수정이 필요함
    bool initDetector(
        const std::string& onnx_path,
        int input_size
    );
    /*
    전체 채널의 개수
    카메라의 프레임(당장은 30프레임으로 통일 될 거라 기대, 향후 배열 등으로 확장성 고려한 패치 필요)
    트랙 잔존 시간
    low & high 트랙 임계값
    신규 트랙 추가 임계값
    비용 계산 임계값(높을수록 여유 있게 1-match_thr)
    */
    bool initTracker(
        int num_channels,
        int fps,
        int keep_time_sec,
        float track_thr,
        float high_thr,
        float match_thr
    );

    // 입력 Mat, 채널 번호
    std::vector<TrackingResult>detect_and_track(const cv::Mat& frame,int cam_ch);

    void release();

private:
    // DLL handles
    void* m_yolo;
    std::vector<void*> m_trackers;

    int m_num_channels;
    int m_input_size;

    // 내부 버퍼 (재사용)
    std::vector<float> m_input_tensor;
    std::vector<float> m_det_out;

private:
    // 내부 유틸
    LetterboxInfo apply_letterbox(const cv::Mat& src, cv::Mat& dst, int target_size);
};