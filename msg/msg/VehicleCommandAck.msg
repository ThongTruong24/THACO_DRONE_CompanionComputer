uint64 timestamp # [us]

uint8 VEHICLE_CMD_RESULT_ACCEPTED = 0
uint8 VEHICLE_CMD_RESULT_TEMPORARILY_REJECTED = 1
uint8 VEHICLE_CMD_RESULT_DENIED = 2
uint8 VEHICLE_CMD_RESULT_UNSUPPORTED = 3
uint8 VEHICLE_CMD_RESULT_FAILED = 4
uint8 VEHICLE_CMD_RESULT_IN_PROGRESS = 5
uint8 VEHICLE_CMD_RESULT_CANCELLED = 6

uint32 command
uint8 result
uint8 result_param1
int32 result_param2
uint8 target_system
uint16 target_component
bool from_external
