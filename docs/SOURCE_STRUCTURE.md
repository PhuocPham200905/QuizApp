# Cấu trúc source code

Ứng dụng được tách theo chức năng để nhiều người có thể làm việc song song và giảm xung đột Git.

| Khu vực | File chính | Nội dung |
|---|---|---|
| Model | `src/models/Models.h` | User, Student, Teacher, Admin, Question, Exam, Result |
| Firebase | `src/services/FirebaseService.h` | Gửi/nhận dữ liệu Firestore |
| Dữ liệu nghiệp vụ | `src/data/QuizData.h` | Đăng nhập, người dùng, câu hỏi, đề thi, kết quả, phiên thi |
| Khung Win32 | `src/ui/features/GuiCore.inl` | Cửa sổ, control, theme, message, phím tắt |
| Đăng nhập | `src/ui/features/AuthScreens.inl` | Đăng nhập, đăng ký, hồ sơ, hướng dẫn |
| Dashboard | `src/ui/features/DashboardScreens.inl` | Admin, giáo viên, học sinh, quản lý tài khoản |
| Câu hỏi | `src/ui/features/QuestionScreens.inl` | Ngân hàng, thêm, nhập CSV, xóa câu hỏi |
| Đề thi | `src/ui/features/ExamScreens.inl` | Tạo/sửa/xóa đề, làm bài, thời gian, chống gian lận |
| Kết quả | `src/ui/features/ResultScreens.inl` | Lịch sử điểm, thống kê, CSV, nhật ký chống gian lận |
| Tiện ích GUI | `src/ui/features/GuiUtilities.inl` | Kiểm tra dữ liệu, chuẩn hóa chuỗi, điều hướng |

## Quy ước làm việc nhóm

1. Mỗi tính năng làm trên một branch riêng, ví dụ `feature/question-import`.
2. Không định dạng lại toàn bộ file nếu chỉ sửa một chức năng nhỏ.
3. Chạy build và `tests/Run-QuizAppTests.ps1` trước khi tạo pull request.
4. Không commit file chứa khóa Firebase, service account hoặc mật khẩu thật.
5. Nếu thay đổi model hoặc `QuizData`, thông báo cho người đang sửa UI vì đây là các file dùng chung.

Các file `.inl` được include bên trong lớp `GuiApp`. Chúng vẫn là mã C++ bình thường nhưng cho phép giữ trạng thái GUI chung mà không dồn mọi màn hình vào một file lớn.
