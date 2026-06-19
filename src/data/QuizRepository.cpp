#include "QuizRepository.h"

string QuizRepository::getLastError() const { return firebase.getLastError(); }
bool QuizRepository::getCollection(string collectionName, json& output) { return firebase.getCollection(collectionName, output); }
bool QuizRepository::getDocument(string collectionName, string documentId, json& output) { return firebase.getDocument(collectionName, documentId, output); }
bool QuizRepository::setDocument(string collectionName, string documentId, const json& fields) { return firebase.setDocument(collectionName, documentId, fields); }
bool QuizRepository::createDocument(string collectionName, string documentId, const json& fields) { return firebase.createDocument(collectionName, documentId, fields); }
bool QuizRepository::deleteDocument(string collectionName, string documentId) { return firebase.deleteDocument(collectionName, documentId); }
bool QuizRepository::getRawUrl(string url, string& output) { return firebase.getRawUrl(url, output); }
string QuizRepository::documentIdFromName(string name) const { return firebase.documentIdFromName(name); }
string QuizRepository::stringField(const json& fields, string key, string defaultValue) const { return firebase.stringField(fields, key, defaultValue); }
int QuizRepository::intField(const json& fields, string key, int defaultValue) const { return firebase.intField(fields, key, defaultValue); }
bool QuizRepository::boolField(const json& fields, string key, bool defaultValue) const { return firebase.boolField(fields, key, defaultValue); }
vector<string> QuizRepository::stringArrayField(const json& fields, string key) const { return firebase.stringArrayField(fields, key); }
json QuizRepository::stringValue(string value) const { return firebase.stringValue(value); }
json QuizRepository::intValue(int value) const { return firebase.intValue(value); }
json QuizRepository::boolValue(bool value) const { return firebase.boolValue(value); }
json QuizRepository::stringArrayValue(vector<string> values) const { return firebase.stringArrayValue(values); }
json QuizRepository::answerMapValue(map<string, char> answers) const { return firebase.answerMapValue(answers); }
