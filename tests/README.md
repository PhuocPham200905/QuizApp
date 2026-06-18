# Kiểm thử Quiz App

Chạy trong PowerShell tại thư mục dự án:

```powershell
.\tests\Run-QuizAppTests.ps1 `
  -AdminPassword "<mật-khẩu-admin>" `
  -TeacherPassword "<mật-khẩu-giáo-viên>" `
  -StudentPassword "<mật-khẩu-học-sinh>"
```

Script chỉ kiểm tra các luồng không làm thay đổi dữ liệu thật:

- Đăng nhập và phân quyền Admin/Giáo viên/Học sinh.
- Điều hướng toàn bộ màn hình chính.
- Số dòng trong các bảng ListView.
- Mở các form tạo, nhập, xóa và quản lý.
- File CSV mẫu và CSV fixture tiếng Việt.
- Validation đổi mật khẩu.
- Đăng xuất và độ ổn định của tiến trình.

Kết quả được ghi vào `test-results/quiz-app-test-report.json` và
`test-results/quiz-app-test-report.md`. Mật khẩu không được ghi vào báo cáo.


