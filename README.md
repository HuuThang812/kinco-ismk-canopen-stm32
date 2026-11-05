# Kinco iSMK Integrated Servo CANopen (CiA 402) Driver for STM32

Bộ thư viện mã nguồn C điều khiển động cơ servo tích hợp **Kinco iSMK Series** (iSMK40 / iSMK60 / iSMK80) theo tiêu chuẩn công nghiệp **CANopen (CiA 402)** trên vi điều khiển **STM32** (STM32F1, F4, G4, H7...) sử dụng **STM32 HAL**.

---

## 📚 Mục Lục Tài Liệu Chi Tiết (Documentation)

Dự án được chia thành từng phần rõ ràng để tiện tra cứu và tích hợp:

* 📖 **[Part 1: Định nghĩa CANopen COB-ID & Cấu trúc dữ liệu Motor](docs/part1_canopen_definitions.md)**
  * Bảng tra cứu 10 hòm thư CANopen chuẩn CiA 301.
  * Bảng 16 thanh ghi Object Dictionary CiA 402.
  * Ý nghĩa struct đối tượng `Kinco_Motor_t`.
* ⏳ **Part 2: Khởi tạo mạng & Máy trạng thái CiA 402** *(Đang cập nhật tài liệu)*
* ⏳ **Part 3: Hướng dẫn điều khiển Góc quay (Position Mode)** *(Đang cập nhật tài liệu)*
* ⏳ **Part 4: Bộ giải mã phản hồi (Feedback) trong ngắt CAN** *(Đang cập nhật)*
* ⏳ **Part 5: Hướng dẫn cấu hình STM32CubeMX & Sơ đồ đấu dây** *(Đang cập nhật)*

---

## 🚀 Tiến Độ Dự Án (Project Roadmap)

- [x] **Part 1:** Khởi tạo từ điển thanh ghi & cấu trúc dữ liệu (`kinco_ismk_def.h`)
- [x] **Part 2:** Khởi tạo mạng NMT & Chuỗi kích hoạt máy trạng thái CiA 402 (`kinco_ismk_core.c`, `kinco_ismk.h`)
- [ ] **Part 3:** Hàm điều khiển Góc quay / Vị trí (Position Mode)
- [ ] **Part 4:** Bóc tách phản hồi góc, tốc độ thực tế từ ngắt CAN
- [ ] **Part 5:** Code mẫu hoàn chỉnh trên STM32CubeIDE

---

## 📄 Bản quyền (License)

Dự án được phát hành theo giấy phép [MIT License](LICENSE).
