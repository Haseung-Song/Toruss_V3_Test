#pragma once

#ifdef _WIN32
#define BYTE_API __declspec(dllexport)
#else
#define BYTE_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

    // tracker 생성
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

    // tracker 제거
    BYTE_API void BYTE_Destroy(void* handle);

#ifdef __cplusplus
}
#endif
