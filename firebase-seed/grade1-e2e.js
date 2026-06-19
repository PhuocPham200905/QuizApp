const { initializeApp, cert } = require("firebase-admin/app");
const { getFirestore } = require("firebase-admin/firestore");
const serviceAccount = require("./serviceAccountKey.json");

initializeApp({
  credential: cert(serviceAccount),
});

const db = getFirestore();
const subject = "Toán lớp 1 - Bộ 25 câu";
const examTitle = "Kiểm tra Toán lớp 1 - 25 câu";
const password = "LopMot@2026";

function studentId(email) {
  let id = "s_";
  for (const character of email) {
    id += /[a-z0-9]/i.test(character) ? character.toLowerCase() : "_";
  }
  return id;
}

async function seedStudents() {
  const batch = db.batch();
  const accounts = [];

  for (let index = 1; index <= 10; index++) {
    const number = String(index).padStart(2, "0");
    const email = `hocsinh${number}.lop1@quiz.local`;
    const id = studentId(email);
    const account = {
      id,
      name: `Học sinh lớp 1 - ${number}`,
      email,
      passwordHash: password,
      role: "student",
      className: "1",
    };
    batch.set(db.collection("students").doc(id), account);
    accounts.push(account);
  }

  await batch.commit();
  console.log(JSON.stringify({
    action: "seed-students",
    count: accounts.length,
    password,
    accounts: accounts.map(({ passwordHash, ...account }) => account),
  }));
}

async function resetContent() {
  const questionSnapshot = await db.collection("questions")
    .where("subject", "==", subject)
    .get();
  const examSnapshot = await db.collection("exams")
    .where("title", "==", examTitle)
    .get();
  const examIds = new Set(examSnapshot.docs.map((document) => document.id));
  const resultSnapshot = await db.collection("results").get();
  const sheetSnapshot = await db.collection("answerSheets").get();
  const sessionSnapshot = await db.collection("examSessions").get();

  const documents = [
    ...questionSnapshot.docs,
    ...examSnapshot.docs,
    ...resultSnapshot.docs.filter((document) => examIds.has(document.data().examId)),
    ...sheetSnapshot.docs.filter((document) => examIds.has(document.data().examId)),
    ...sessionSnapshot.docs.filter((document) => examIds.has(document.data().examId)),
  ];

  if (documents.length > 0) {
    const batch = db.batch();
    for (const document of documents) {
      batch.delete(document.ref);
    }
    await batch.commit();
  }

  console.log(JSON.stringify({
    action: "reset-content",
    deleted: documents.length,
  }));
}

async function getQuestionIds() {
  const snapshot = await db.collection("questions")
    .where("subject", "==", subject)
    .get();
  const questions = snapshot.docs
    .map((document) => ({ id: document.id, ...document.data() }))
    .sort((left, right) => {
      const leftNumber = Number(left.id.replace(/\D/g, ""));
      const rightNumber = Number(right.id.replace(/\D/g, ""));
      return leftNumber - rightNumber;
    });

  console.log(JSON.stringify({
    action: "get-question-ids",
    subject,
    count: questions.length,
    ids: questions.map((question) => question.id),
  }));
}

async function getExam() {
  const snapshot = await db.collection("exams")
    .where("title", "==", examTitle)
    .get();
  const exams = snapshot.docs.map((document) => ({
    id: document.id,
    ...document.data(),
  }));
  console.log(JSON.stringify({
    action: "get-exam",
    title: examTitle,
    count: exams.length,
    exams,
  }));
}

async function createDisposableExam() {
  const id = "e_delete_ui_test";
  await db.collection("exams").doc(id).set({
    title: "Đề tạm để kiểm thử xóa",
    durationMinutes: 10,
    questionIds: [],
    answerKey: ["A"],
    createdBy: "t1",
    startAt: "2026-01-01 00:00",
    closeAt: "2026-12-31 23:59",
    examPassword: "",
    attachmentPath: "",
    attachmentUrl: "",
    attemptLimit: 1,
    isOpen: false,
  });
  console.log(JSON.stringify({ action: "create-disposable-exam", id }));
}

async function checkDisposableExam() {
  const document = await db.collection("exams").doc("e_delete_ui_test").get();
  console.log(JSON.stringify({
    action: "check-disposable-exam",
    exists: document.exists,
  }));
}

async function deleteDisposableExam() {
  await db.collection("exams").doc("e_delete_ui_test").delete();
  console.log(JSON.stringify({
    action: "delete-disposable-exam",
    deleted: true,
  }));
}

async function setExamAttemptLimit() {
  const limit = Number(process.argv[3]);
  if (!Number.isInteger(limit) || limit <= 0) {
    throw new Error("Attempt limit must be a positive integer.");
  }
  const snapshot = await db.collection("exams")
    .where("title", "==", examTitle)
    .limit(1)
    .get();
  if (snapshot.empty) {
    throw new Error("Grade 1 exam was not found.");
  }
  await snapshot.docs[0].ref.update({ attemptLimit: limit });
  console.log(JSON.stringify({
    action: "set-exam-attempt-limit",
    id: snapshot.docs[0].id,
    attemptLimit: limit,
  }));
}

async function setExamSchedule() {
  const startAt = process.argv[3];
  const closeAt = process.argv[4];
  if (!startAt || !closeAt) {
    throw new Error("Start and close times are required.");
  }
  const snapshot = await db.collection("exams")
    .where("title", "==", examTitle)
    .limit(1)
    .get();
  if (snapshot.empty) {
    throw new Error("Grade 1 exam was not found.");
  }
  await snapshot.docs[0].ref.update({ startAt, closeAt });
  console.log(JSON.stringify({
    action: "set-exam-schedule",
    id: snapshot.docs[0].id,
    startAt,
    closeAt,
  }));
}

async function getSessions() {
  const snapshot = await db.collection("examSessions").get();
  const sessions = snapshot.docs.map((document) => ({
    id: document.id,
    ...document.data(),
  }));
  console.log(JSON.stringify({
    action: "get-sessions",
    count: sessions.length,
    sessions,
  }));
}

async function deleteSession() {
  const id = process.argv[3];
  if (!id) throw new Error("Session ID is required.");
  await db.collection("examSessions").doc(id).delete();
  console.log(JSON.stringify({ action: "delete-session", id }));
}

async function getAuditSummary() {
  const snapshot = await db.collection("auditLogs").get();
  const actionCounts = {};
  for (const document of snapshot.docs) {
    const action = document.data().action || "UNKNOWN";
    actionCounts[action] = (actionCounts[action] || 0) + 1;
  }
  console.log(JSON.stringify({
    action: "get-audit-summary",
    count: snapshot.size,
    actionCounts,
  }));
}

async function cleanupSecondAttempt() {
  const studentId = "s_hocsinh01_lop1_quiz_local";
  const examId = "e6";
  const ids = [
    ["examSessions", `session_${studentId}_${examId}_2`],
    ["answerSheets", `sheet_${studentId}_${examId}_2`],
    ["results", `result_${studentId}_${examId}_2`],
  ];
  await Promise.all(ids.map(([collection, id]) =>
    db.collection(collection).doc(id).delete()
  ));
  console.log(JSON.stringify({ action: "cleanup-second-attempt", deleted: ids.length }));
}

async function verify() {
  const studentEmails = new Set(
    Array.from({ length: 10 }, (_, index) =>
      `hocsinh${String(index + 1).padStart(2, "0")}.lop1@quiz.local`
    )
  );
  const studentSnapshot = await db.collection("students").get();
  const students = studentSnapshot.docs.filter((document) =>
    studentEmails.has(document.data().email)
  );

  const examSnapshot = await db.collection("exams")
    .where("title", "==", examTitle)
    .get();
  const examIds = new Set(examSnapshot.docs.map((document) => document.id));
  const studentIds = new Set(students.map((document) => document.id));

  const resultSnapshot = await db.collection("results").get();
  const results = resultSnapshot.docs
    .map((document) => ({ id: document.id, ...document.data() }))
    .filter((result) =>
      examIds.has(result.examId) && studentIds.has(result.studentId)
    );

  console.log(JSON.stringify({
    action: "verify",
    students: students.length,
    exams: examSnapshot.size,
    results: results.length,
    scores: results.map((result) => ({
      studentId: result.studentId,
      score: result.score,
      totalQuestions: result.totalQuestions,
    })),
  }));
}

const actions = {
  "seed-students": seedStudents,
  "reset-content": resetContent,
  "get-question-ids": getQuestionIds,
  "get-exam": getExam,
  "create-disposable-exam": createDisposableExam,
  "check-disposable-exam": checkDisposableExam,
  "delete-disposable-exam": deleteDisposableExam,
  "set-exam-attempt-limit": setExamAttemptLimit,
  "set-exam-schedule": setExamSchedule,
  "get-sessions": getSessions,
  "delete-session": deleteSession,
  "get-audit-summary": getAuditSummary,
  "cleanup-second-attempt": cleanupSecondAttempt,
  verify,
};

const action = process.argv[2];
if (!actions[action]) {
  console.error("Usage: node grade1-e2e.js <seed-students|reset-content|get-question-ids|get-exam|create-disposable-exam|check-disposable-exam|delete-disposable-exam|set-exam-attempt-limit|set-exam-schedule|get-sessions|delete-session|get-audit-summary|cleanup-second-attempt|verify>");
  process.exit(1);
}

actions[action]().catch((error) => {
  console.error(error);
  process.exit(1);
});
