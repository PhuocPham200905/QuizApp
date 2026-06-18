const { initializeApp, cert } = require("firebase-admin/app");
const { getFirestore } = require("firebase-admin/firestore");

const serviceAccount = require("./serviceAccountKey.json");

initializeApp({
  credential: cert(serviceAccount),
});

const db = getFirestore();

async function seed() {
  await db.collection("admins").doc("a1").set({
    name: "Admin Demo",
    email: "admin@quiz.local",
    passwordHash: "123456",
    role: "admin",
    className: "Admin",
  });

  await db.collection("teachers").doc("t1").set({
    name: "Teacher Demo",
    email: "teacher@quiz.local",
    passwordHash: "123456",
    role: "teacher",
    className: "12A1",
  });

  await db.collection("students").doc("s1").set({
    name: "Nguyen An",
    email: "an@quiz.local",
    passwordHash: "123456",
    role: "student",
    className: "12A1",
  });

  await db.collection("questions").doc("q1").set({
    content: "C++ la ngon ngu lap trinh thuoc nhom nao?",
    optionA: "Huong doi tuong",
    optionB: "Chi HTML",
    optionC: "He dieu hanh",
    optionD: "Co so du lieu",
    correctAnswer: "A",
    subject: "C++",
  });

  await db.collection("questions").doc("q2").set({
    content: "Tu khoa nao dung de tao class trong C++?",
    optionA: "structs",
    optionB: "class",
    optionC: "object",
    optionD: "newclass",
    correctAnswer: "B",
    subject: "C++",
  });

  await db.collection("questions").doc("q3").set({
    content: "Tinh dong goi trong OOP giup dieu gi?",
    optionA: "An chi tiet ben trong",
    optionB: "Tang kich thuoc file",
    optionC: "Xoa bien",
    optionD: "Tat chuong trinh",
    correctAnswer: "A",
    subject: "OOP",
  });

  await db.collection("questions").doc("q4").set({
    content: "Vector trong C++ nam trong thu vien nao?",
    optionA: "<map>",
    optionB: "<array>",
    optionC: "<vector>",
    optionD: "<string>",
    correctAnswer: "C",
    subject: "C++",
  });

  await db.collection("questions").doc("q5").set({
    content: "Ke thua trong OOP duoc dung de lam gi?",
    optionA: "Tai su dung va mo rong class",
    optionB: "Ma hoa mat khau",
    optionC: "Ket noi Wi-Fi",
    optionD: "Ve giao dien",
    correctAnswer: "A",
    subject: "OOP",
  });

  await db.collection("exams").doc("e1").set({
    title: "De mau C++ OOP",
    durationMinutes: 30,
    startAt: "2026-01-01 00:00",
    closeAt: "2026-12-31 23:59",
    examPassword: "oop123",
    createdBy: "t1",
    isOpen: true,
    questionIds: ["q1", "q2", "q3", "q4", "q5"],
  });

  await db.collection("answerSheets").doc("s1").set({
    studentId: "s1",
    examId: "e1",
    answers: {
      q1: "A",
      q2: "B",
      q3: "A",
      q4: "C",
      q5: "A",
    },
    submittedAt: "2026-06-16 20:00:00",
    score: 5,
  });

  await db.collection("results").doc("r1").set({
    studentId: "s1",
    examId: "e1",
    score: 5,
    totalQuestions: 5,
    submittedAt: "2026-06-16 20:00:00",
  });

  console.log("Seed data thanh cong!");
}

seed().catch((error) => {
  console.error("Seed data that bai:");
  console.error(error);
  process.exit(1);
});
