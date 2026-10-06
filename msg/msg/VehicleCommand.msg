uint64 timestamp # [us] steady clock, giong PX4 VehicleCommand (tap con)

uint32 VEHICLE_CMD_THACO_SAVE_DEFAULT_CONFIG = 44010    # sub: 0 all, 1 links, 2 camera, 3 network, 4 vision (param1)
uint32 VEHICLE_CMD_THACO_APPLY_CONFIG = 44011
uint32 VEHICLE_CMD_THACO_RESTORE_DEFAULT_CONFIG = 44012

float32 param1
float32 param2
float32 param3
float32 param4
float64 param5
float64 param6
float32 param7
uint32 command
uint8 target_system
uint8 target_component  # 0 = all components
uint8 source_system
uint16 source_component
uint8 confirmation
bool from_external
