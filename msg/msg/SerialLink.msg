# Mot link serial, theo CC_SERIAL_LINK (42015). Dinh danh link la ten nguoi dung dat, khong phai vai tro cung.
uint64 timestamp # [us]
float32 rx_rate    # B/s
float32 tx_rate    # B/s
float32 rx_loss    # %
uint32 rx_bytes
uint32 tx_bytes
uint32 rx_errors
uint32 baudrate
uint8 link_index
uint8 link_count
uint8 status
string<=16 name
string<=16 port
