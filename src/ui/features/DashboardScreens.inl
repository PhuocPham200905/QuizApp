    void showAdminDashboard() {
        clearControls();
        currentScreen = SCREEN_ADMIN_DASHBOARD;
        title("Bảng điều khiển Admin", "Xin chào " + currentUser->getName());
        addNav("Quản trị viên");
        navButton("Học sinh", 16, 116, 208, 38, ID_ADMIN_STUDENTS);
        navButton("Giáo viên", 16, 160, 208, 38, ID_ADMIN_TEACHERS);
        navButton("Tạo giáo viên", 16, 204, 208, 38, ID_ADMIN_CREATE_TEACHER);
        navButton("Câu hỏi", 16, 248, 208, 38, ID_ADMIN_QUESTIONS);
        navButton("Đề thi", 16, 292, 208, 38, ID_ADMIN_EXAMS);
        navButton("Mở/đóng đề", 16, 336, 208, 38, ID_ADMIN_TOGGLE_EXAM);
        navButton("Kết quả", 16, 380, 208, 38, ID_ADMIN_RESULTS);
        navButton("Chống gian lận", 16, 424, 208, 38, ID_ADMIN_AUDIT);

        cardPanel(280, 120, 880, 172);
        surfaceLabelText("Quản trị hệ thống", 304, 144, 360, 32, brandFont);
        surfaceLabelText("Theo dõi tài khoản, đề thi và kết quả trên toàn bộ lớp học.",
                         304, 184, 650, 26, smallFont, THEME_MUTED);
        defaultButton("Quản lý học sinh", 304, 230, 190, 40, ID_ADMIN_STUDENTS);
        button("Quản lý giáo viên", 510, 230, 190, 40, ID_ADMIN_TEACHERS);
        button("Xem kết quả", 716, 230, 150, 40, ID_ADMIN_RESULTS);

        cardPanel(280, 324, 270, 150);
        surfaceLabelText("Tài khoản", 304, 348, 180, 28, brandFont);
        surfaceLabelText("Học sinh và giáo viên", 304, 384, 210, 24, smallFont, THEME_MUTED);
        button("Mở danh sách", 304, 420, 150, 36, ID_ADMIN_STUDENTS);

        cardPanel(575, 324, 270, 150);
        surfaceLabelText("Đề thi", 599, 348, 180, 28, brandFont);
        surfaceLabelText("Lịch mở đề và trạng thái", 599, 384, 210, 24, smallFont, THEME_MUTED);
        button("Quản lý đề", 599, 420, 150, 36, ID_ADMIN_EXAMS);

        cardPanel(870, 324, 270, 150);
        surfaceLabelText("Giám sát", 894, 348, 180, 28, brandFont);
        surfaceLabelText("Kết quả và chống gian lận", 894, 384, 220, 24, smallFont, THEME_MUTED);
        button("Xem báo cáo", 894, 420, 150, 36, ID_ADMIN_RESULTS);
    }

    void showTeacherDashboard() {
        clearControls();
        currentScreen = SCREEN_TEACHER_DASHBOARD;
        title("Bảng điều khiển Giáo viên", "Xin chào " + currentUser->getName());
        addNav("Giáo viên");
        navButton("Câu hỏi", 16, 116, 208, 38, ID_TEACHER_QUESTIONS);
        navButton("Thêm câu hỏi", 16, 160, 208, 38, ID_TEACHER_ADD_QUESTION);
        navButton("Nhập câu hỏi", 16, 204, 208, 38, ID_TEACHER_IMPORT_QUESTIONS);
        navButton("Xóa câu hỏi", 16, 248, 208, 38, ID_TEACHER_DELETE_QUESTION);
        navButton("Đề thi", 16, 292, 208, 38, ID_TEACHER_EXAMS);
        navButton("Up đề thi", 16, 336, 208, 38, ID_TEACHER_CREATE_EXAM);
        navButton("Mở/đóng đề", 16, 380, 208, 38, ID_TEACHER_TOGGLE_EXAM);
        navButton("Kết quả", 16, 424, 208, 38, ID_TEACHER_RESULTS);
        navButton("Quản lý đề", 16, 468, 208, 38, ID_TEACHER_MANAGE_EXAMS);

        cardPanel(280, 120, 880, 172);
        surfaceLabelText("Tạo và theo dõi bài thi", 304, 144, 420, 32, brandFont);
        surfaceLabelText("Chuẩn bị ngân hàng câu hỏi, tải đề file gốc và xem kết quả học sinh.",
                         304, 184, 760, 26, smallFont, THEME_MUTED);
        defaultButton("Up đề thi", 304, 230, 150, 40, ID_TEACHER_CREATE_EXAM);
        button("Nhập câu hỏi", 470, 230, 150, 40, ID_TEACHER_IMPORT_QUESTIONS);
        button("Quản lý đề", 636, 230, 150, 40, ID_TEACHER_MANAGE_EXAMS);
        button("Tạo câu hỏi", 802, 230, 150, 40, ID_TEACHER_AI_QUESTIONS);

        cardPanel(280, 324, 270, 164);
        surfaceLabelText("1. Chuẩn bị", 304, 348, 180, 28, brandFont);
        surfaceLabelText("Nhập câu hỏi hoặc chọn file đề gốc.", 304, 386, 220, 48, smallFont, THEME_MUTED);
        button("Ngân hàng", 304, 436, 130, 36, ID_TEACHER_QUESTIONS);

        cardPanel(575, 324, 270, 164);
        surfaceLabelText("2. Tạo đề", 599, 348, 180, 28, brandFont);
        surfaceLabelText("Đặt thời gian, mật khẩu và số lượt làm.", 599, 386, 220, 48, smallFont, THEME_MUTED);
        button("Tải đề", 599, 436, 130, 36, ID_TEACHER_CREATE_EXAM);

        cardPanel(870, 324, 270, 164);
        surfaceLabelText("3. Theo dõi", 894, 348, 180, 28, brandFont);
        surfaceLabelText("Xem điểm, trạng thái và quản lý đề.", 894, 386, 220, 48, smallFont, THEME_MUTED);
        button("Kết quả", 894, 436, 130, 36, ID_TEACHER_RESULTS);
    }

    void showStudentDashboard() {
        clearControls();
        currentScreen = SCREEN_STUDENT_DASHBOARD;
        title("Bảng điều khiển Học sinh", "Xin chào " + currentUser->getName());
        addNav("Học sinh");
        navButton("Đề đang mở", 16, 116, 208, 38, ID_STUDENT_OPEN_EXAMS);
        navButton("Làm bài", 16, 160, 208, 38, ID_STUDENT_TAKE_EXAM);
        navButton("Lịch sử điểm", 16, 204, 208, 38, ID_STUDENT_RESULTS);

        cardPanel(280, 120, 880, 172);
        surfaceLabelText("Sẵn sàng làm bài", 304, 144, 360, 32, brandFont);
        surfaceLabelText("Xem đề đang mở, nhập mã đề và làm bài theo thời gian giáo viên đặt.",
                         304, 184, 760, 26, smallFont, THEME_MUTED);
        defaultButton("Làm bài ngay", 304, 230, 150, 40, ID_STUDENT_TAKE_EXAM);
        button("Xem đề mở", 470, 230, 150, 40, ID_STUDENT_OPEN_EXAMS);
        button("Xem điểm", 636, 230, 150, 40, ID_STUDENT_RESULTS);

        cardPanel(280, 324, 270, 164);
        surfaceLabelText("Đề đang mở", 304, 348, 180, 28, brandFont);
        surfaceLabelText("Kiểm tra danh sách đề có thể làm.", 304, 386, 220, 48, smallFont, THEME_MUTED);
        button("Xem đề", 304, 436, 130, 36, ID_STUDENT_OPEN_EXAMS);

        cardPanel(575, 324, 270, 164);
        surfaceLabelText("Trong giờ thi", 599, 348, 180, 28, brandFont);
        surfaceLabelText("Hệ thống tự nộp khi hết thời gian.", 599, 386, 220, 48, smallFont, THEME_MUTED);
        button("Bắt đầu", 599, 436, 130, 36, ID_STUDENT_TAKE_EXAM);

        cardPanel(870, 324, 270, 164);
        surfaceLabelText("Lịch sử điểm", 894, 348, 180, 28, brandFont);
        surfaceLabelText("Xem lại kết quả các bài đã nộp.", 894, 386, 220, 48, smallFont, THEME_MUTED);
        button("Xem điểm", 894, 436, 130, 36, ID_STUDENT_RESULTS);
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
