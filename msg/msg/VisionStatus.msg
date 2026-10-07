uint64 timestamp # [us]
float32 confidence_thresh
float32 inference_fps
uint16 input_width
uint16 input_height
uint8 video_fps
uint8 detections_count
uint8 status_flags # bit 0: running, bit 1: depth enabled
string<=24 model_name
string<=12 input_source
