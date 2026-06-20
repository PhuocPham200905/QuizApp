const { initializeApp, cert } = require("firebase-admin/app");
const { getFirestore } = require("firebase-admin/firestore");

const serviceAccount = require("./serviceAccountKey.json");

initializeApp({
  credential: cert(serviceAccount),
});

const db = getFirestore();

async function list() {
  const snapshot = await db.collection("exams").get();
  const questionSnapshot = await db.collection("questions").get();
  const exams = snapshot.docs.map((document) => ({
    id: document.id,
    ...document.data(),
  }));
  const questions = questionSnapshot.docs.map((document) => ({
    id: document.id,
    content: document.data().content || "",
    subject: document.data().subject || "",
  }));
  const references = {};
  for (const collection of ["results", "answerSheets", "examSessions"]) {
    const referenceSnapshot = await db.collection(collection)
      .where("examId", "==", "e2")
      .get();
    references[collection] = referenceSnapshot.docs.map((document) => ({
      id: document.id,
      ...document.data(),
    }));
  }
  console.log(JSON.stringify({ action: "list", exams, questions, references }));
}

async function repair() {
  const snapshot = await db.collection("exams").get();
  const questionSnapshot = await db.collection("questions").get();
  const exams = snapshot.docs.map((document) => ({
    id: document.id,
    ref: document.ref,
    data: document.data(),
  }));

  let bankExam = exams.find((exam) =>
    exam.data.title?.trim().toLowerCase() === "test demo"
  );
  const fileExam = exams.find((exam) =>
    exam.data.title?.trim().toLowerCase() === "test demo file docx"
  );

  if (!fileExam) {
    throw new Error("Không tìm thấy đề có tên 'Test demo file docx'.");
  }
  if (!bankExam) {
    bankExam = {
      id: "recovered-test-demo",
      data: {
        title: "Test DEMO",
        durationMinutes: 10,
        questionIds: ["q2", "q3", "q4", "q5"],
        answerKey: [],
        createdBy: fileExam.data.createdBy || "t1",
        startAt: "2026-06-19 23:15",
        closeAt: "2026-12-31 23:59",
        examPassword: "",
        attachmentPath: "",
        attachmentUrl: "",
        attemptLimit: 3,
        violationLimit: fileExam.data.violationLimit || 3,
        shuffleQuestions: false,
        shuffleAnswers: false,
        isOpen: true,
      },
    };
  }

  const questions = questionSnapshot.docs.map((document) => ({
    id: document.id,
    data: document.data(),
  }));
  const questionById = new Map(
    questions.map((question) => [question.id, question])
  );
  const numericId = (id) => Number.parseInt(id.replace(/^\D+/, ""), 10) || 0;
  const requestedIds = Array.isArray(bankExam.data.questionIds)
    ? bankExam.data.questionIds
    : [];
  const targetCount = requestedIds.length || 4;
  const usedIds = new Set();
  const repairedIds = requestedIds.map((questionId) => {
    if (!questionById.has(questionId) || usedIds.has(questionId)) {
      return null;
    }
    usedIds.add(questionId);
    return questionId;
  });
  const candidates = questions
    .filter((question) => !usedIds.has(question.id))
    .sort((left, right) => numericId(left.id) - numericId(right.id));

  for (let index = 0; index < repairedIds.length; index += 1) {
    if (repairedIds[index] === null) {
      const replacement = candidates.shift();
      if (!replacement) {
        throw new Error("Không đủ câu hỏi hợp lệ để sửa đề Test DEMO.");
      }
      repairedIds[index] = replacement.id;
      usedIds.add(replacement.id);
    }
  }
  while (repairedIds.length < targetCount) {
    const replacement = candidates.shift();
    if (!replacement) {
      throw new Error("Không đủ câu hỏi hợp lệ để bổ sung đề Test DEMO.");
    }
    repairedIds.push(replacement.id);
  }
  bankExam.data.questionIds = repairedIds;

  const targetE2 = db.collection("exams").doc("e2");
  const targetE3 = db.collection("exams").doc("e3");
  const existingE3 = await targetE3.get();
  if (existingE3.exists && existingE3.id !== fileExam.id) {
    throw new Error("Mã e3 đang được một đề khác sử dụng; không tự động ghi đè.");
  }

  const batch = db.batch();
  batch.set(targetE2, bankExam.data);
  batch.set(targetE3, fileExam.data);

  for (const exam of [bankExam, fileExam]) {
    if (exam.ref && exam.id !== "e2" && exam.id !== "e3") {
      batch.delete(exam.ref);
    }
  }

  await batch.commit();
  console.log(JSON.stringify({
    action: "repair",
    updated: [
      { id: "e2", title: bankExam.data.title },
      { id: "e3", title: fileExam.data.title },
    ],
    questionIds: bankExam.data.questionIds,
  }));
}

async function verify() {
  const [examSnapshot, questionSnapshot] = await Promise.all([
    db.collection("exams").get(),
    db.collection("questions").get(),
  ]);
  const questionIds = new Set(
    questionSnapshot.docs.map((document) => document.id)
  );
  const issues = [];

  for (const document of examSnapshot.docs) {
    const exam = document.data();
    const requestedIds = Array.isArray(exam.questionIds)
      ? exam.questionIds
      : [];
    if (requestedIds.length === 0) {
      continue;
    }

    const seen = new Set();
    const missing = [];
    const duplicated = [];
    for (const questionId of requestedIds) {
      if (!questionIds.has(questionId)) {
        missing.push(questionId);
      } else if (seen.has(questionId)) {
        duplicated.push(questionId);
      }
      seen.add(questionId);
    }
    if (missing.length > 0 || duplicated.length > 0) {
      issues.push({
        examId: document.id,
        title: exam.title || "",
        missing,
        duplicated,
      });
    }
  }

  if (issues.length > 0) {
    console.error(JSON.stringify({ action: "verify", ok: false, issues }));
    process.exitCode = 1;
    return;
  }
  console.log(JSON.stringify({
    action: "verify",
    ok: true,
    examCount: examSnapshot.size,
    questionCount: questionSnapshot.size,
  }));
}

const action = process.argv[2] || "list";
({ list, repair, verify }[action] || (() => {
  throw new Error("Usage: node repair-demo-exam-ids.js <list|repair|verify>");
}))().catch((error) => {
  console.error(error);
  process.exit(1);
});
