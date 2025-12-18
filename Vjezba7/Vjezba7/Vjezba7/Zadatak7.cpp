#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>

// Structure that represents a directory node in a tree (child-sibling representation)
struct Dir {
    std::string name;
    Dir* parent;
    Dir* child;   // first child directory
    Dir* sibling; // next sibling directory

    Dir(const std::string& n, Dir* p = nullptr)
        : name(n), parent(p), child(nullptr), sibling(nullptr) {}
};

// Creates and returns the root directory.
Dir* createRoot();

// Finds and returns a pointer to a child directory with the given name under current, or nullptr if not found.
Dir* findChild(Dir* current, const std::string& name);

// Creates a new directory with the given name under current if it does not already exist.
void makeDirectory(Dir* current, const std::string& name);

// Changes current to the named child directory. Returns true on success, false if not found.
bool changeDirectory(Dir*& current, const std::string& name);

// Moves current to its parent directory. Returns true on success, false if already at root.
bool goUp(Dir*& current);

// Lists (prints) the immediate subdirectories of current.
void listDirectory(Dir* current);

// Returns a string representing the full path from root to current.
std::string getPath(Dir* current);

// Recursively frees memory for the subtree rooted at node.
void freeTree(Dir* node);

// Trim whitespace from both ends of a string and return the result.
std::string trim(const std::string& s);

// Split input into command (first token) and argument (rest of the line trimmed).
void splitCommand(const std::string& input, std::string& cmd, std::string& arg);

int main() {
    Dir* root = createRoot();
    Dir* current = root;

    while (true) {
        std::cout << "\nCurrent directory: " << getPath(current) << "\n";
        std::cout << "Enter a command:\n";
        std::cout << "  md [name]    - create directory (you may type 'md name' or 'md' then enter name)\n";
        std::cout << "  cd <name>    - change to subdirectory\n";
        std::cout << "  cd..         - go to parent directory\n";
        std::cout << "  dir          - list contents\n";
        std::cout << "  exit         - exit program\n";
        std::cout << "> ";

        std::string line;
        if (!std::getline(std::cin, line)) break;
        line = trim(line);
        if (line.empty()) continue;

        // Accept direct token "cd.." only (removed support for "c..")
        if (line == "cd..") {
            if (!goUp(current)) {
                std::cout << "Already at root directory.\n";
            }
            continue;
        }

        std::string cmd, arg;
        splitCommand(line, cmd, arg);

        // normalize command to lowercase
        std::transform(cmd.begin(), cmd.end(), cmd.begin(), [](unsigned char c){ return std::tolower(c); });

        if (cmd == "exit") {
            break;
        } else if (cmd == "dir") {
            listDirectory(current);
        } else if (cmd == "md") {
            std::string name = arg;
            if (name.empty()) {
                std::cout << "Enter directory name to create: ";
                if (!std::getline(std::cin, name)) break;
                name = trim(name);
            }
            if (name.empty()) {
                std::cout << "Name cannot be empty.\n";
                continue;
            }
            makeDirectory(current, name);
        } else if (cmd == "cd") {
            if (arg.empty()) {
                std::cout << "Enter directory name to change into: ";
                if (!std::getline(std::cin, arg)) break;
                arg = trim(arg);
            }
            if (arg.empty()) {
                std::cout << "Name cannot be empty.\n";
                continue;
            }
            if (arg == "..") {
                if (!goUp(current)) {
                    std::cout << "Already at root directory.\n";
                }
            } else {
                if (!changeDirectory(current, arg)) {
                    std::cout << "Directory \"" << arg << "\" not found.\n";
                }
            }
        } else {
            std::cout << "Unknown command. Try again.\n";
        }
    }

    freeTree(root);
    return 0;
}

// Creates and returns the root directory.
Dir* createRoot() {
    return new Dir("root", nullptr);
}

// Finds and returns a pointer to a child directory with the given name under current, or nullptr if not found.
Dir* findChild(Dir* current, const std::string& name) {
    if (!current) return nullptr;
    Dir* it = current->child;
    while (it) {
        if (it->name == name) return it;
        it = it->sibling;
    }
    return nullptr;
}

// Creates a new directory with the given name under current if it does not already exist.
void makeDirectory(Dir* current, const std::string& name) {
    if (!current) return;
    if (findChild(current, name)) {
        std::cout << "Directory \"" << name << "\" already exists.\n";
        return;
    }

    Dir* node = new Dir(name, current);
    // Insert as first child for simplicity
    node->sibling = current->child;
    current->child = node;
    std::cout << "Directory \"" << name << "\" created.\n";
}

// Changes current to the named child directory. Returns true on success, false if not found.
bool changeDirectory(Dir*& current, const std::string& name) {
    Dir* child = findChild(current, name);
    if (!child) return false;
    current = child;
    return true;
}

// Moves current to its parent directory. Returns true on success, false if already at root.
bool goUp(Dir*& current) {
    if (!current || !current->parent) return false;
    current = current->parent;
    return true;
}

// Lists (prints) the immediate subdirectories of current.
void listDirectory(Dir* current) {
    if (!current) return;
    std::cout << "Contents of " << current->name << ":\n";
    Dir* it = current->child;
    if (!it) {
        std::cout << "  (no subdirectories)\n";
        return;
    }
    while (it) {
        std::cout << "  " << it->name << "\n";
        it = it->sibling;
    }
}

// Returns a string representing the full path from root to current.
std::string getPath(Dir* current) {
    if (!current) return "";
    std::string path;
    Dir* it = current;
    while (it) {
        path = "/" + it->name + path;
        it = it->parent;
    }
    return path;
}

// Recursively frees memory for the subtree rooted at node.
void freeTree(Dir* node) {
    if (!node) return;
    // free children
    Dir* child = node->child;
    while (child) {
        Dir* next = child->sibling;
        freeTree(child);
        child = next;
    }
    delete node;
}

// Trim whitespace from both ends of a string and return the result.
std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    if (start == s.size()) return "";
    size_t end = s.size() - 1;
    while (end > start && std::isspace(static_cast<unsigned char>(s[end]))) --end;
    return s.substr(start, end - start + 1);
}

// Split input into command (first token) and argument (rest of the line trimmed).
void splitCommand(const std::string& input, std::string& cmd, std::string& arg) {
    std::istringstream iss(input);
    if (!(iss >> cmd)) {
        cmd.clear();
        arg.clear();
        return;
    }
    std::string rest;
    std::getline(iss, rest);
    arg = trim(rest);
}