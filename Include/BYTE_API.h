#pragma once

#ifdef _WIN32
#define BYTE_API __declspec(dllexport)
#else
#define BYTE_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

    /*
    tracker 생성
    track_thresh: 고득점 탐지(Detections)와 저득점 탐지(Detections_low)를 가르는 기준점.
    high_thresh: 새로운 트랙을 생성하기 위해 필요한 최소 점수 (Step 4에서 사용).
    track_thr <= high_thr
    match_thresh: IoU 매칭 시 허용되는 최대 거리 (유사도 기준).
    max_time_lost: 트랙이 'Lost' 상태로 머물 수 있는 최대 프레임 수 (fps * keep_time_sec).
    */ 
    BYTE_API void* BYTE_Create(int frame_rate, int keep_time_sec, const float track_thr, const float high_thr, const float match_thr);

    // tracker update
    // objects: [x, y, w, h, score, cls] * num_objects
    // out_tracks: [x, y, w, h, track_id, cls, prob] * max_tracks
    BYTE_API int BYTE_Update(
        void* handle,
        const float* objects,
        int num_objects,
        float* out_tracks,
        int max_tracks
    );
    // 팬틸트 변동량에 따라 STrack의 mean, covariance 수정
    BYTE_API void BYTE_ApplyMotion(void* handle, float hfov, float d_pan, float d_tilt);
    // 테스트 function
    BYTE_API int BYTE_GetTrack(void* handle, float* out_tracks, int max_tracks);
    // tracker 제거
    BYTE_API void BYTE_Destroy(void* handle);

#ifdef __cplusplus
}
#endif
