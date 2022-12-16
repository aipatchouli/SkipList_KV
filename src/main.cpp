#include "skiplist.hpp"
#include <iostream>
#define FILE_PATH "./store/dumpFile"

// 重载 < > 运算符
bool operator<(const std::string& a, const std::string& b) {
    return a.compare(b) < 0;
}

int main() {
    SkipList<std::string, std::string> skipList(1024);
    skipList.insertElement("Name", "Song Jiang");
    skipList.insertElement("University", "YunNan University");
    skipList.insertElement("Major", "Computer Science");
    skipList.insertElement("Seeking Job", "C++ Software Engineer");
    skipList.insertElement("Email", "jiangsonggwy@outlook.com");
    skipList.insertElement("Phone", "18888888888");

    std::cout << "skipList size: " << skipList.size() << std::endl;

    skipList.dumpFile();

    skipList.searchElement("Name");

    skipList.displayList();
    skipList.deleteElement("Phone");
    std::cout << "skipList size: " << skipList.size() << std::endl;
    skipList.displayList();

    return 0;
}