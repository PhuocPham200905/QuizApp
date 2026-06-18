# Báo cáo kiểm thử Quiz App

- Thời gian: 2026-06-18 11:09:00
- Tổng kiểm thử: 32
- Đạt: 32
- Không đạt: 0

| Vai trò | Chức năng | Kết quả | Chi tiết |
|---|---|---|---|
| Chung | File CSV mẫu | ĐẠT | header='content,optionA,optionB,optionC,optionD,correctAnswer' |
| Chung | CSV fixture tiếng Việt | ĐẠT | header='content,optionA,optionB,optionC,optionD,correctAnswer'; rows=3 |
| Admin | Đăng nhập và phân quyền | ĐẠT | dashboard='Bảng điều khiển Admin'; responding=True |
| Admin | Quản lý học sinh | ĐẠT | opened=True; responding=True; rows=1 |
| Admin | Quản lý giáo viên | ĐẠT | opened=True; responding=True; rows=1 |
| Admin | Tạo tài khoản giáo viên | ĐẠT | opened=True; responding=True |
| Admin | Ngân hàng câu hỏi | ĐẠT | opened=True; responding=True |
| Admin | Danh sách đề thi | ĐẠT | opened=True; responding=True; rows=5 |
| Admin | Mở/đóng đề thi | ĐẠT | opened=True; responding=True; rows=5 |
| Admin | Kết quả tất cả học sinh | ĐẠT | opened=True; responding=True; rows=1 |
| Admin | Hồ sơ và ba lớp nhập mật khẩu | ĐẠT | Đủ họ tên, email, mật khẩu hiện tại, mật khẩu mới và xác nhận. |
| Admin | Hướng dẫn sử dụng | ĐẠT | Màn hình hướng dẫn mở được. |
| Admin | Đăng xuất | ĐẠT | Quay về màn hình đăng nhập. |
| Giáo viên | Đăng nhập và phân quyền | ĐẠT | dashboard='Bảng điều khiển Giáo viên'; responding=True |
| Giáo viên | Ngân hàng câu hỏi | ĐẠT | opened=True; responding=True |
| Giáo viên | Thêm câu hỏi | ĐẠT | opened=True; responding=True |
| Giáo viên | Nhập câu hỏi | ĐẠT | opened=True; responding=True |
| Giáo viên | Xóa câu hỏi | ĐẠT | opened=True; responding=True |
| Giáo viên | Danh sách đề thi | ĐẠT | opened=True; responding=True; rows=5 |
| Giáo viên | Giáo viên tải đề thi | ĐẠT | opened=True; responding=True |
| Giáo viên | Mở/đóng đề thi | ĐẠT | opened=True; responding=True; rows=5 |
| Giáo viên | Kết quả tất cả học sinh | ĐẠT | opened=True; responding=True; rows=1 |
| Giáo viên | Hồ sơ và ba lớp nhập mật khẩu | ĐẠT | Đủ họ tên, email, mật khẩu hiện tại, mật khẩu mới và xác nhận. |
| Giáo viên | Hướng dẫn sử dụng | ĐẠT | Màn hình hướng dẫn mở được. |
| Giáo viên | Đăng xuất | ĐẠT | Quay về màn hình đăng nhập. |
| Học sinh | Đăng nhập và phân quyền | ĐẠT | dashboard='Bảng điều khiển Học sinh'; responding=True |
| Học sinh | Đề thi đang mở | ĐẠT | opened=True; responding=True; rows=5 |
| Học sinh | Làm bài thi | ĐẠT | opened=True; responding=True; rows=5 |
| Học sinh | Lịch sử điểm | ĐẠT | opened=True; responding=True; rows=1 |
| Học sinh | Hồ sơ và ba lớp nhập mật khẩu | ĐẠT | Đủ họ tên, email, mật khẩu hiện tại, mật khẩu mới và xác nhận. |
| Học sinh | Hướng dẫn sử dụng | ĐẠT | Màn hình hướng dẫn mở được. |
| Học sinh | Đăng xuất | ĐẠT | Quay về màn hình đăng nhập. |
