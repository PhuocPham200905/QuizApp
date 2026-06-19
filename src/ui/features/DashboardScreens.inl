    void showAdminDashboard() {
        clearControls();
        currentScreen = SCREEN_ADMIN_DASHBOARD;
        title("Bảng điều khiển Admin", "Xin chào " + currentUser->getName());
        addNav("Quản trị viên");
        button("Học sinh", 24, 120, 190, 40, ID_ADMIN_STUDENTS);
        button("Giáo viên", 24, 170, 190, 40, ID_ADMIN_TEACHERS);
        button("Tạo giáo viên", 24, 220, 190, 40, ID_ADMIN_CREATE_TEACHER);
        button("Câu hỏi", 24, 270, 190, 40, ID_ADMIN_QUESTIONS);
        button("Đề thi", 24, 320, 190, 40, ID_ADMIN_EXAMS);
        button("Mở/đóng đề", 24, 370, 190, 40, ID_ADMIN_TOGGLE_EXAM);
        button("Kết quả", 24, 420, 190, 40, ID_ADMIN_RESULTS);
        button("Chống gian lận", 24, 470, 190, 40, ID_ADMIN_AUDIT);

        HWND heading = label("Quản trị hệ thống", 280, 125, 360, 32);
        SendMessageW(heading, WM_SETFONT, (WPARAM)brandFont, TRUE);
        label("Theo dõi tài khoản, đề thi và kết quả trên toàn bộ lớp học.", 280, 168, 650, 26);
        defaultButton("Quản lý học sinh", 280, 225, 200, 48, ID_ADMIN_STUDENTS);
        button("Quản lý giáo viên", 500, 225, 200, 48, ID_ADMIN_TEACHERS);
        button("Xem kết quả", 720, 225, 170, 48, ID_ADMIN_RESULTS);

        HWND statusTitle = label("Tổng quan hệ thống", 280, 330, 260, 30);
        SendMessageW(statusTitle, WM_SETFONT, (WPARAM)brandFont, TRUE);
        edit("Quản lý học sinh, giáo viên, ngân hàng câu hỏi, đề thi và kết quả tại một nơi.",
             280, 375, 660, 90, 0, false, true, true);
    }

    void showTeacherDashboard() {
        clearControls();
        currentScreen = SCREEN_TEACHER_DASHBOARD;
        title("Bảng điều khiển Giáo viên", "Xin chào " + currentUser->getName());
        addNav("Giáo viên");
        button("Câu hỏi", 24, 120, 190, 40, ID_TEACHER_QUESTIONS);
        button("Thêm câu hỏi", 24, 170, 190, 40, ID_TEACHER_ADD_QUESTION);
        button("Nhập câu hỏi", 24, 220, 190, 40, ID_TEACHER_IMPORT_QUESTIONS);
        button("Xóa câu hỏi", 24, 270, 190, 40, ID_TEACHER_DELETE_QUESTION);
        button("Đề thi", 24, 320, 190, 40, ID_TEACHER_EXAMS);
        button("Up đề thi", 24, 370, 190, 40, ID_TEACHER_CREATE_EXAM);
        button("Mở/đóng đề", 24, 420, 190, 40, ID_TEACHER_TOGGLE_EXAM);
        button("Kết quả", 24, 450, 190, 40, ID_TEACHER_RESULTS);
        button("Quản lý đề", 24, 495, 190, 40, ID_TEACHER_MANAGE_EXAMS);

        HWND heading = label("Tạo và theo dõi bài thi", 280, 125, 420, 32);
        SendMessageW(heading, WM_SETFONT, (WPARAM)brandFont, TRUE);
        label("Chuẩn bị ngân hàng câu hỏi, up đề file gốc và xem kết quả học sinh.", 280, 168, 760, 26);
        defaultButton("Up đề thi", 280, 225, 170, 48, ID_TEACHER_CREATE_EXAM);
        button("Nhập câu hỏi", 470, 225, 170, 48, ID_TEACHER_IMPORT_QUESTIONS);
        button("Quản lý đề", 660, 225, 170, 48, ID_TEACHER_MANAGE_EXAMS);
        button("Tạo bằng AI", 850, 225, 170, 48, ID_TEACHER_AI_QUESTIONS);

        HWND statusTitle = label("Gợi ý quy trình", 280, 330, 260, 30);
        SendMessageW(statusTitle, WM_SETFONT, (WPARAM)brandFont, TRUE);
        edit("1. Nhập câu hỏi hoặc chọn file đề gốc.\r\n"
             "2. Tạo đề thi, đặt giờ mở/đóng và số lượt làm.\r\n"
             "3. Học sinh làm bài, giáo viên xem kết quả.",
             280, 375, 660, 140, 0, false, true, true);
    }

    void showStudentDashboard() {
        clearControls();
        currentScreen = SCREEN_STUDENT_DASHBOARD;
        title("Bảng điều khiển Học sinh", "Xin chào " + currentUser->getName());
        addNav("Học sinh");
        button("Đề đang mở", 24, 120, 190, 40, ID_STUDENT_OPEN_EXAMS);
        button("Làm bài", 24, 170, 190, 40, ID_STUDENT_TAKE_EXAM);
        button("Lịch sử điểm", 24, 220, 190, 40, ID_STUDENT_RESULTS);

        HWND heading = label("Sẵn sàng làm bài", 280, 125, 360, 32);
        SendMessageW(heading, WM_SETFONT, (WPARAM)brandFont, TRUE);
        label("Xem đề đang mở, nhập mã đề và làm bài theo thời gian giáo viên đặt.", 280, 168, 760, 26);
        defaultButton("Làm bài ngay", 280, 225, 170, 48, ID_STUDENT_TAKE_EXAM);
        button("Xem đề mở", 470, 225, 170, 48, ID_STUDENT_OPEN_EXAMS);
        button("Xem điểm", 660, 225, 170, 48, ID_STUDENT_RESULTS);

        HWND statusTitle = label("Lưu ý khi làm bài", 280, 330, 260, 30);
        SendMessageW(statusTitle, WM_SETFONT, (WPARAM)brandFont, TRUE);
        edit("• Chỉ bắt đầu được khi đã tới giờ mở đề.\r\n"
             "• Nếu đề có mật khẩu, nhập mật khẩu giáo viên cung cấp.\r\n"
             "• Khi hết giờ, hệ thống tự nộp bài.",
             280, 375, 660, 140, 0, false, true, true);
    }

    string usersText(string role) {
        stringstream ss;
        if (role == "student") {
            map<string, vector<shared_ptr<User>>> studentsByClass;
            for (shared_ptr<User> user : data.getUsers()) {
                if (user->getRole() == "student") {
                    studentsByClass[user->getClassName()].push_back(user);
                }
            }

            for (const auto& group : studentsByClass) {
                ss << "LOP: " << group.first << "\r\n";
                ss << "ID     Họ tên                 Email\r\n";
                ss << "------------------------------------------------------------\r\n";
                for (shared_ptr<User> user : group.second) {
                    ss << left << setw(7) << user->getUserId()
                       << setw(22) << user->getName()
                       << user->getEmail() << "\r\n";
                }
                ss << "\r\n";
            }

            return ss.str();
        }

        ss << "ID     Vai trò    Họ tên                 Email                       Lớp/Phụ trách\r\n";
        ss << "--------------------------------------------------------------------------------\r\n";
        for (shared_ptr<User> user : data.getUsers()) {
            if (user->getRole() == role) {
                ss << left << setw(7) << user->getUserId()
                   << setw(10) << user->getRole()
                   << setw(22) << user->getName()
                   << setw(28) << user->getEmail()
                   << user->getClassName() << "\r\n";
            }
        }
        return ss.str();
    }

    void showStudents(string keyword = "") {
        clearControls();
        currentScreen = SCREEN_VIEW;
        data.loadUsersFromFirebase();
        title("Quản lý học sinh");
        addNav("Quản trị viên");
        label("Tìm kiếm", 250, 88, 90, 26);
        edit(keyword, 350, 84, 260, 30, 6103);
        button("Tim", 625, 84, 80, 32, ID_STUDENT_SEARCH);
        userListView(250, 128, 760, 315, "student", keyword);
        label("ID học sinh", 270, 455, 110, 26);
        edit("", 390, 451, 170, 30, 6101);
        label("Lớp mới", 580, 455, 80, 26);
        edit("", 665, 451, 130, 30, 6102);
        button("Chuyển lớp", 390, 500, 130, 38, ID_STUDENT_CHANGE_CLASS);
        button("Xóa học sinh", 535, 500, 130, 38, ID_STUDENT_DELETE);
        button("Về dashboard", 680, 500, 140, 38, ID_DASHBOARD);
        label("Mật khẩu mới", 270, 555, 115, 26);
        edit("", 390, 551, 170, 30, 6104, true);
        button("Đặt lại MK", 580, 548, 130, 38, ID_STUDENT_RESET_PASSWORD);
        setFocusTo(6101);
    }

    void showTeachers(string keyword = "") {
        clearControls();
        currentScreen = SCREEN_VIEW;
        data.loadUsersFromFirebase();
        title("Quản lý giáo viên");
        addNav("Quản trị viên");
        label("Tìm kiếm", 250, 88, 90, 26);
        edit(keyword, 350, 84, 260, 30, 6203);
        button("Tim", 625, 84, 80, 32, ID_TEACHER_SEARCH);
        userListView(250, 128, 760, 315, "teacher", keyword);
        label("ID giáo viên", 270, 455, 110, 26);
        edit("", 390, 451, 170, 30, 6201);
        label("Phụ trách mới", 580, 455, 110, 26);
        edit("", 690, 451, 130, 30, 6202);
        button("Đổi phụ trách", 390, 500, 130, 38, ID_TEACHER_CHANGE_CLASS);
        button("Xóa giáo viên", 535, 500, 130, 38, ID_TEACHER_DELETE);
        button("Về dashboard", 680, 500, 140, 38, ID_DASHBOARD);
        label("Mật khẩu mới", 270, 555, 115, 26);
        edit("", 390, 551, 170, 30, 6204, true);
        button("Đặt lại MK", 580, 548, 130, 38, ID_TEACHER_RESET_PASSWORD);
        setFocusTo(6201);
    }

    void submitChangeManagedUserClass(string role) {
        int idField = role == "student" ? 6101 : 6201;
        int classField = role == "student" ? 6102 : 6202;
        string userId = getText(idField);
        string newClassName = normalizeClassName(getText(classField));

        if (userId.empty() || newClassName.empty()) {
            error("Vui lòng nhập ID và lớp/phụ trách mới.");
            return;
        }

        if (!data.updateUserClass(userId, role, newClassName)) {
            error("Cập nhật thất bại. Vui lòng thử lại.");
            return;
        }

        message(role == "student" ? "Đã chuyển lớp học sinh." : "Đã đổi lớp/môn phụ trách.");
        if (role == "student") showStudents();
        else showTeachers();
    }

    void submitDeleteManagedUser(string role) {
        int idField = role == "student" ? 6101 : 6201;
        string userId = getText(idField);

        if (userId.empty()) {
            error("Vui lòng nhập ID cần xóa.");
            return;
        }

        if (!data.deleteUser(userId, role)) {
            error("Xóa thất bại. Vui lòng thử lại.");
            return;
        }

        message(role == "student" ? "Đã xóa học sinh." : "Đã xóa giáo viên.");
        if (role == "student") showStudents();
        else showTeachers();
    }

    void submitResetManagedUserPassword(string role) {
        int idField = role == "student" ? 6101 : 6201;
        int passwordField = role == "student" ? 6104 : 6204;
        string userId = getText(idField);
        string newPassword = getText(passwordField);

        if (userId.empty() || newPassword.empty()) {
            error("Vui lòng nhập ID và mật khẩu mới.");
            return;
        }

        if (containsRoleKeyword(newPassword)) {
            error("Mật khẩu mới không được chứa từ khóa: admin, teacher, student.");
            return;
        }

        if (!data.resetUserPassword(userId, role, newPassword)) {
            error("Đặt lại mật khẩu thất bại. Vui lòng thử lại.");
            return;
        }

        message(role == "student" ? "Đã đặt lại mật khẩu học sinh." : "Đã đặt lại mật khẩu giáo viên.");
        if (role == "student") showStudents();
        else showTeachers();
    }

    void showCreateTeacher() {
        clearControls();
        currentScreen = SCREEN_CREATE_TEACHER;
        title("Tạo tài khoản giáo viên");
        label("Họ tên", 300, 125, 140, 26);
        edit("", 470, 121, 280, 30, 3001);
        label("Email", 300, 170, 140, 26);
        edit("", 470, 166, 280, 30, 3002);
        label("Mật khẩu", 300, 215, 140, 26);
        edit("", 470, 211, 280, 30, 3003, true);
        label("Lớp/môn phụ trách", 300, 260, 150, 26);
        edit("", 470, 256, 280, 30, 3004);
        defaultButton("Tạo giáo viên", 470, 315, 140, 38, ID_CREATE_TEACHER_SUBMIT);
        button("Về dashboard", 620, 315, 140, 38, ID_DASHBOARD);
        setFocusTo(3001);
    }

    void submitCreateTeacher() {
        string name = getText(3001);
        string email = getText(3002);
        string password = getText(3003);
        string className = normalizeClassName(getText(3004));
        if (name.empty() || email.empty() || password.empty() || className.empty()) {
            error("Vui lòng nhập đầy đủ thông tin.");
            return;
        }
        if (containsRoleKeyword(name) || containsRoleKeyword(password)) {
            error("Tên và mật khẩu không được chứa từ khóa: admin, teacher, student.");
            return;
        }

        if (!data.loadUsersFromFirebase()) {
            error("Không thể tải dữ liệu. Vui lòng kiểm tra Internet rồi thử lại.");
            return;
        }

        if (data.findUserByEmail(email) != nullptr) {
            error("Email này đã tồn tại.");
            return;
        }

        if (!data.addTeacher(name, email, password, className)) {
            error("Tạo giáo viên thất bại. Vui lòng thử lại.");
            return;
        }

        message("Đã tạo tài khoản giáo viên.");
        showAdminDashboard();
    }
