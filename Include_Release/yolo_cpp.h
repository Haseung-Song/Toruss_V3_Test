#pragma once
#ifdef _WIN32
#define YOLO_API __declspec(dllexport) // 해당 키워드가 없으면, 외부 프로그램에서 호출할 수 없음
#else
#define YOLO_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

	// YOLO 객체 생성
	YOLO_API void* YOLO_Create(const char* model_path);

	// YOLO 추론 (nms까지 한 bbox 정보를 out_boxes에 저장 tlwh foramt)
	YOLO_API int YOLO_Run(void* handle, const float* input_buffer, size_t input_size, float* out_boxes, int max_boxes, const float prob_thr);

	// YOLO 객체 제거
	YOLO_API void YOLO_Destroy(void* handle);
#ifdef __cplusplus
}
#endif