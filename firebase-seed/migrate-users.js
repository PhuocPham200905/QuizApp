const { initializeApp, cert } = require("firebase-admin/app");
const { getFirestore } = require("firebase-admin/firestore");

const serviceAccount = require("./serviceAccountKey.json");

initializeApp({
  credential: cert(serviceAccount),
});

const db = getFirestore();

const roleToCollection = {
  admin: "admins",
  teacher: "teachers",
  student: "students",
};

const roleToPrefix = {
  admin: "a",
  teacher: "t",
  student: "s",
};

function getNextId(existingIds, prefix) {
  let maxNumber = 0;

  for (const id of existingIds) {
    if (!id.startsWith(prefix)) {
      continue;
    }

    const numberPart = Number(id.slice(prefix.length));
    if (!Number.isNaN(numberPart)) {
      maxNumber = Math.max(maxNumber, numberPart);
    }
  }

  return `${prefix}${maxNumber + 1}`;
}

async function hasSameEmail(collectionName, email) {
  if (!email) {
    return false;
  }

  const snapshot = await db
    .collection(collectionName)
    .where("email", "==", email)
    .limit(1)
    .get();

  return !snapshot.empty;
}

async function migrateUsers() {
  const usersSnapshot = await db.collection("users").get();

  if (usersSnapshot.empty) {
    console.log("Khong co du lieu trong collection users.");
    return;
  }

  const existingIds = {};
  for (const collectionName of Object.values(roleToCollection)) {
    const snapshot = await db.collection(collectionName).get();
    existingIds[collectionName] = new Set(snapshot.docs.map((doc) => doc.id));
  }

  let migratedCount = 0;
  let skippedCount = 0;

  for (const userDoc of usersSnapshot.docs) {
    const data = userDoc.data();
    const role = data.role;
    const collectionName = roleToCollection[role];
    const idPrefix = roleToPrefix[role];

    if (!collectionName || !idPrefix) {
      console.log(`Bo qua users/${userDoc.id}: role khong hop le (${role}).`);
      skippedCount++;
      continue;
    }

    if (await hasSameEmail(collectionName, data.email)) {
      console.log(`Bo qua users/${userDoc.id}: email da ton tai trong ${collectionName}.`);
      skippedCount++;
      continue;
    }

    const newId = getNextId(existingIds[collectionName], idPrefix);
    existingIds[collectionName].add(newId);

    await db.collection(collectionName).doc(newId).set(data);
    console.log(`Da copy users/${userDoc.id} -> ${collectionName}/${newId}`);
    migratedCount++;
  }

  console.log(`Hoan tat. Da copy: ${migratedCount}, bo qua: ${skippedCount}.`);
  console.log("Luu y: script chi copy du lieu, khong xoa collection users cu.");
}

migrateUsers().catch((error) => {
  console.error("Migrate that bai:");
  console.error(error);
  process.exit(1);
});
