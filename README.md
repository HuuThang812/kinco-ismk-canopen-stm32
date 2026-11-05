# Kinco iSMK Integrated Servo CANopen (CiA 402) Driver for STM32

Bộ thư viện mã nguồn C điều khiển động cơ servo tích hợp Kinco iSMK Series (iSMK40 / iSMK60 / iSMK80) theo tiêu chuẩn công nghiệp CANopen (CiA 402) trên vi điều khiển STM32 (STM32F1, F4, G4, H7...) sử dụng STM32 HAL.

---

## Mục Lục Tài Liệu Chi Tiết (Documentation)

Dự án được chia thành từng phần rõ ràng để tiện tra cứu và tích hợp:

* [Part 1: Định nghĩa CANopen COB-ID và Cấu trúc dữ liệu Motor](docs/part1_canopen_definitions.md)
  * Bảng tra cứu hòm thư CANopen chuẩn CiA 301.
  * Bảng thanh ghi Object Dictionary CiA 402.
  * Cấu trúc dữ liệu Kinco_Motor_t.
* [Part 2: Quản lý mạng NMT và Máy trạng thái CiA 402](docs/part2_nmt_state_machine.md)
  * Lệnh đánh thức NMT Start Node.
  * Chuỗi kích hoạt cấp nguồn, khóa trục và mở phanh cơ.
  * Trình tự khởi động chuẩn trong hàm main().
* Part 3: Hướng dẫn điều khiển Góc quay (Position Mode) *(Chờ duyệt)*
* Part 4: Bộ giải mã phản hồi (Feedback) trong ngắt CAN *(Đang cập nhật)*
* Part 5: Hướng dẫn cấu hình STM32CubeMX và Sơ đồ đấu dây *(Đang cập nhật)*

---

## Tiến Độ Dự Án (Project Roadmap)

- [x] Part 1: Khởi tạo từ điển thanh ghi và cấu trúc dữ liệu (kinco_ismk_def.h)
- [x] Part 2: Khởi tạo mạng NMT và Chuỗi kích hoạt máy trạng thái CiA 402 (kinco_ismk_core.c, kinco_ismk.h)
- [ ] Part 3: Hàm điều khiển Góc quay / Vị trí (Position Mode) *(Chờ duyệt)*
- [ ] Part 4: Bóc tách phản hồi góc, tốc độ thực tế từ ngắt CAN
- [ ] Part 5: Code mẫu hoàn chỉnh trên STM32CubeIDE

---

## Bản quyền (License)

Dự án được phát hành theo giấy phép [MIT License](LICENSE).
