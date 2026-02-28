// ProjectTwo.cpp : Jose Bustamante
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

// ---------------------- Course definition ----------------------

struct Course {
    string courseNumber;
    string name;
    vector<string> prerequisites;
};

// ---------------------- String helpers -------------------------

string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

string normalize(const string& text) {
    string t = trim(text);
    transform(t.begin(), t.end(), t.begin(), ::toupper);
    return t;
}

// Split a CSV line into trimmed tokens
vector<string> splitCSV(const string& line) {
    vector<string> tokens;
    string token;
    bool inQuotes = false;
    string current;

    for (char c : line) {
        if (c == '"') {
            inQuotes = !inQuotes;
            current.push_back(c);
        }
        else if (c == ',' && !inQuotes) {
            tokens.push_back(trim(current));
            current.clear();
        }
        else {
            current.push_back(c);
        }
    }
    if (!current.empty()) {
        tokens.push_back(trim(current));
    }
    return tokens;
}

// ---------------------- BST implementation ---------------------

struct Node {
    Course course;
    Node* left;
    Node* right;

    Node(const Course& c) : course(c), left(nullptr), right(nullptr) {}
};

class CourseBST {
private:
    Node* root;

    void insert(Node*& node, const Course& course) {
        if (node == nullptr) {
            node = new Node(course);
            return;
        }
        string keyNew = normalize(course.courseNumber);
        string keyNode = normalize(node->course.courseNumber);

        if (keyNew < keyNode) {
            insert(node->left, course);
        }
        else if (keyNew > keyNode) {
            insert(node->right, course);
        }
        else {
            // Duplicate key: ignore or update
        }
    }

    Course* search(Node* node, const string& key) const {
        if (node == nullptr) return nullptr;

        string keyNode = normalize(node->course.courseNumber);
        if (key == keyNode) {
            return &node->course;
        }
        else if (key < keyNode) {
            return search(node->left, key);
        }
        else {
            return search(node->right, key);
        }
    }

    void inOrder(Node* node) const {
        if (node == nullptr) return;
        inOrder(node->left);
        cout << node->course.courseNumber << ", " << node->course.name << endl;
        inOrder(node->right);
    }

    void destroy(Node* node) {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

public:
    CourseBST() : root(nullptr) {}
    ~CourseBST() { destroy(root); }

    void insert(const Course& course) {
        insert(root, course);
    }

    Course* search(const string& courseNumber) const {
        string key = normalize(courseNumber);
        return search(root, key);
    }

    void printInOrder() const {
        inOrder(root);
    }

    bool isEmpty() const {
        return root == nullptr;
    }
};

// ---------------------- Loading courses ------------------------

bool loadCoursesFromFile(const string& fileName, CourseBST& tree) {
    ifstream file(fileName);
    if (!file.is_open()) {
        cout << "Error: Could not open file '" << fileName << "'." << endl;
        return false;
    }

    string line;
    int lineNo = 0;
    vector<string> seenCourseNumbers;

    while (getline(file, line)) {
        lineNo++;

        string trimmed = trim(line);
        if (trimmed.empty() || trimmed[0] == '#') {
            continue; // skip blank/comment lines
        }

        vector<string> tokens = splitCSV(line);
        if (tokens.size() < 2) {
            cout << "Error: missing course number or title (line " << lineNo << ")." << endl;
            continue;
        }

        string courseNumber = normalize(tokens[0]);
        string title = trim(tokens[1]);

        if (courseNumber.empty()) {
            cout << "Error: invalid/empty course number (line " << lineNo << ")." << endl;
            continue;
        }
        if (title.empty()) {
            cout << "Error: empty course title (line " << lineNo << ")." << endl;
            continue;
        }

        // Check duplicate course number in this load
        if (find(seenCourseNumbers.begin(), seenCourseNumbers.end(), courseNumber) != seenCourseNumbers.end()) {
            cout << "Error: duplicate course number '" << courseNumber << "' (line " << lineNo << ")." << endl;
            continue;
        }
        seenCourseNumbers.push_back(courseNumber);

        Course course;
        course.courseNumber = courseNumber;
        course.name = title;

        // Prerequisites
        for (size_t i = 2; i < tokens.size(); ++i) {
            string prereq = normalize(tokens[i]);
            if (prereq.empty()) {
                cout << "Warning: blank prerequisite ignored (line " << lineNo << ")." << endl;
                continue;
            }
            course.prerequisites.push_back(prereq);
        }

        tree.insert(course);
    }

    file.close();
    cout << "Courses loaded successfully from '" << fileName << "'." << endl;
    return true;
}

// ---------------------- Printing course info -------------------

void printCourseInformation(const CourseBST& tree, const string& courseNumber) {
    Course* course = tree.search(courseNumber);
    if (course == nullptr) {
        cout << "Course not found." << endl;
        return;
    }

    cout << course->courseNumber << ", " << course->name << endl;

    if (course->prerequisites.empty()) {
        cout << "No prerequisites listed." << endl;
        return;
    }

    cout << "Prerequisites: ";

    bool first = true;
    for (const string& prereqCode : course->prerequisites) {
        Course* prereqCourse = tree.search(prereqCode);
        if (!first) {
            cout << ", ";
        }
        if (prereqCourse != nullptr) {
            cout << prereqCourse->courseNumber;
        }
        else {
            // Still show the code If not found
            cout << prereqCode << " (not found)";
        }
        first = false;
    }
    cout << endl;
}

// ---------------------- Menu loop ------------------------------

void printMenu() {
    cout << endl;
    cout << "1. Load Data Structure." << endl;
    cout << "2. Print Course List." << endl;
    cout << "3. Print Course." << endl;
    cout << "9. Exit" << endl;
    cout << endl;
    cout << "What would you like to do? ";
}

int main() {
    CourseBST tree;
    bool dataLoaded = false;

    cout << "Welcome to the course planner." << endl;

    int choice = 0;
    while (true) {
        printMenu();
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please enter a number." << endl;
            continue;
        }

        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // clear rest of line

        if (choice == 1) {
            string fileName;
            cout << "Enter the file name to load: ";
            getline(cin, fileName);
            // Reset the tree safely
            tree = CourseBST();
            if (loadCoursesFromFile(fileName, tree)) {
                dataLoaded = true;
            }
        }
        else if (choice == 2) {
            if (!dataLoaded || tree.isEmpty()) {
                cout << "No data loaded. Please load the data structure first (option 1)." << endl;
            }
            else {
                cout << "Here is a sample schedule:" << endl << endl;
                tree.printInOrder();
            }
        }
        else if (choice == 3) {
            if (!dataLoaded || tree.isEmpty()) {
                cout << "No data loaded. Please load the data structure first (option 1)." << endl;
            }
            else {
                string courseNumber;
                cout << "What course do you want to know about? ";
                getline(cin, courseNumber);
                courseNumber = normalize(courseNumber);
                printCourseInformation(tree, courseNumber);
            }
        }
        else if (choice == 9) {
            cout << "Thank you for using the course planner!" << endl;
            break;
        }
        else {
            cout << choice << " is not a valid option." << endl;
        }
    }

    return 0;
}


