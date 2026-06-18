# Quiz App

Ứng dụng thi trắc nghiệm trên Windows viết bằng C++17 và Win32 API. Dữ liệu
tài khoản, câu hỏi, đề thi và kết quả được đồng bộ với Cloud Firestore.

## Yêu cầu

- Windows 10 hoặc Windows 11
- Git
- CMake 3.20 trở lên
- Visual Studio 2022 Build Tools với workload **Desktop development with C++**
- vcpkg
- VS Code và extension C/C++ (không bắt buộc)

## Chuẩn bị vcpkg

Chỉ cần thực hiện một lần:

```powershell
git clone https://github.com/microsoft/vcpkg.git C:\vcpkg
C:\vcpkg\bootstrap-vcpkg.bat
[Environment]::SetEnvironmentVariable("VCPKG_ROOT", "C:\vcpkg", "User")
```

Đóng và mở lại PowerShell hoặc VS Code, sau đó kiểm tra:

```powershell
$env:VCPKG_ROOT
```

## Tải và build dự án

```powershell
git clone https://github.com/PhuocPham200905/QuizApp.git
cd QuizApp

cmake -S . -B build `
  -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"

cmake --build build --config Release
```

Các thư viện `curl`, `nlohmann-json` và `zlib` sẽ được vcpkg cài tự động từ
file `vcpkg.json`.

File chạy được tạo tại một trong các vị trí sau, tùy trình biên dịch:

```text
build\Release\QuizApp.exe
build\QuizApp.exe
```

Trong VS Code, có thể nhấn `Ctrl+Shift+B` để build sau khi đã thiết lập
biến môi trường `VCPKG_ROOT`.

## Làm việc nhóm bằng Git

Mỗi thành viên nên làm việc trên một branch riêng:

```powershell
git switch main
git pull origin main
git switch -c feature/ten-chuc-nang
```

Sau khi sửa xong:

```powershell
git add .
git commit -m "Mô tả thay đổi"
git push -u origin feature/ten-chuc-nang
```

Sau đó tạo Pull Request trên GitHub để kiểm tra và gộp vào `main`. Trước khi
bắt đầu một phần việc mới, luôn cập nhật `main` bằng `git pull origin main`.

## Firebase

Ứng dụng C++ kết nối Firestore qua REST API. Các script trong
`firebase-seed` dùng Firebase Admin SDK để tạo dữ liệu mẫu.

Nếu cần chạy script seed:

```powershell
cd firebase-seed
npm.cmd install
node seed.js
```

Trước khi chạy, đặt service account của dự án Firebase tại:

```text
firebase-seed\serviceAccountKey.json
```

`serviceAccountKey.json` là thông tin bí mật và đã được `.gitignore` chặn.
Không commit, gửi qua GitHub hoặc đóng gói file này trong bản Release.

## Kiểm thử

Xem hướng dẫn tại [tests/README.md](tests/README.md). Báo cáo sinh ra trong
`test-results` không cần đưa lên GitHub.

## Đóng gói

Mã nguồn không lưu file `.exe`, `.dll`, thư mục `build` hoặc file `.zip`.
Bản ứng dụng dành cho người dùng nên được đăng tại mục **GitHub Releases**.
