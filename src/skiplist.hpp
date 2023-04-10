#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <mutex>
#include <random>
#include <type_traits>

#define STORE_FILE "store/dumpFile"

std::mutex mtx;
std::string delimiter = ":";

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dis(1, RAND_MAX);

template <typename K, typename V>
class Node {
public:
    Node() = default;
    Node(K key, V value, int level);
    K getKey() const;
    V getValue() const;
    void setValue(V value);

    ~Node() {
        delete[] forward;
    };

    Node<K, V>** forward;

    Node(const Node&) = delete;
    Node(Node&&) = delete;
    Node& operator=(const Node&) = delete;
    Node& operator=(Node&&) = delete;

private:
    K key;
    V value;
    int level{};
};

template <typename K, typename V>
Node<K, V>::Node(const K key, const V value, const int level)
    : key(key), value(value), level(level), forward(new Node<K, V>*[level + 1]) {
    std::memset(this->forward, 0, sizeof(Node<K, V>*) * (level + 1));
}

template <typename K, typename V>
K Node<K, V>::getKey() const {
    return key;
}

template <typename K, typename V>
V Node<K, V>::getValue() const {
    return value;
}

template <typename K, typename V>
void Node<K, V>::setValue(V value) {
    this->value = value;
}

template <typename K, typename V>
class SkipList {
public:
    explicit SkipList(int maxLevel)
        : MaxLevel(maxLevel), header(new Node<K, V>(K(), V(), maxLevel)) {
    }
    ~SkipList() {
        if (fileReader.is_open()) {
            fileReader.close();
        }
        if (fileWriter.is_open()) {
            fileWriter.close();
        }
        delete header;
    };

    SkipList(const SkipList&) = delete;
    SkipList(SkipList&&) = delete;
    SkipList& operator=(const SkipList&) = delete;
    SkipList& operator=(SkipList&&) = delete;

    Node<K, V>* createNode(K key, V value, int level);
    int randomLevel();
    // Insert a key, return 1 if the key is existed, return 0 successfully
    int insertElement(K /*key*/, V /*value*/);
    int deleteElement(K /*key*/);
    void displayList();
    bool searchElement(K /*key*/);
    void dumpFile();
    void loadFile();
    int size();

private:
    void getKeyValueFromString(const std::string& str, std::string* key, std::string* value);
    bool isValidString(const std::string& str);

    // max level for this skip list
    int MaxLevel;
    // current level of skip list
    int CurrentLevel = 0;
    // pointer to header node
    Node<K, V>* header;
    // element counter of skip list
    int sizeOfList = 0;

    // file operations
    std::ofstream fileWriter;
    std::ifstream fileReader;
};

template <typename K, typename V>
Node<K, V>* SkipList<K, V>::createNode(const K key, const V value, const int level) {
    auto* newNode = new Node<K, V>(key, value, level);
    return newNode;
}

// insert
template <typename K, typename V>
int SkipList<K, V>::insertElement(const K key, const V value) {
    mtx.lock();
    Node<K, V>* current = header;
    // create update array and initialize it
    Node<K, V>* update[MaxLevel + 1];
    std::memset(update, 0, sizeof(Node<K, V>*) * (MaxLevel + 1));

    /*    start from highest level of skip list
        move the current pointer forward while key is greater than key of node next to current
        otherwise inserted current in update and move one level down and continue search
    */
    for (int i = CurrentLevel; i >= 0; i--) {
        while (current->forward[i] != nullptr && current->forward[i]->getKey() < key) {
            current = current->forward[i];
        }
        update[i] = current;
    }
    current = current->forward[0];
    if (current != nullptr && current->getKey() == key) {
        current->setValue(value);
        std::cout << "Key " << key << " is already existed, update value" << std::endl;
        mtx.unlock();
        return 1;
    }
    if (current == nullptr || current->getKey() != key) {
        // skiplevel: set random height
        int skipListRandomLevel = randomLevel();
        // if random level is greater than list's current level
        if (skipListRandomLevel > CurrentLevel) {
            for (int i = CurrentLevel + 1; i < skipListRandomLevel + 1; i++) {
                update[i] = header;
            }
            // update the list's current level
            CurrentLevel = skipListRandomLevel;
        }
        // create new node with random level generated
        Node<K, V>* newNode = createNode(key, value, skipListRandomLevel);
        // insert node by rearranging pointers
        for (int i = 0; i <= skipListRandomLevel; i++) {
            newNode->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = newNode;
        }
        std::cout << "Successfully inserted key " << key << std::endl;
        sizeOfList++;
    }
    mtx.unlock();
    return 0;
}

// display skip list
template <typename K, typename V>
void SkipList<K, V>::displayList() {
    std::cout << "\n*****Skip List*****\n"
              << std::endl;
    for (int i = 0; i <= CurrentLevel; i++) {
        Node<K, V>* node = header->forward[i];
        std::cout << "Level " << i << ": ";
        while (node != nullptr) {
            std::cout << node->getKey() << " : " << node->getValue() << ";";
            node = node->forward[i];
        }
        std::cout << std::endl;
    }
}

// dump data to file
template <typename K, typename V>
void SkipList<K, V>::dumpFile() {
    fileWriter.open(STORE_FILE);
    if (!fileWriter.is_open()) {
        std::cout << "Cannot open file" << std::endl;
        return;
    }
    for (int i = 0; i <= CurrentLevel; i++) {
        Node<K, V>* node = header->forward[i];
        while (node != nullptr) {
            fileWriter << node->getKey() << delimiter << node->getValue() << std::endl;
            node = node->forward[i];
        }
    }
    fileWriter.flush();
    fileWriter.close();
}

// load data from file
template <typename K, typename V>
void SkipList<K, V>::loadFile() {
    fileReader.open(STORE_FILE);
    if (!fileReader.is_open()) {
        std::cout << "Cannot open file" << std::endl;
        return;
    }
    std::string line;
    while (std::getline(fileReader, line)) {
        if (isValidString(line)) {
            std::string key;
            std::string value;
            getKeyValueFromString(&line, &key, &value);
            if (!key.empty() && !value.empty()) {
                insertElement(key, value);
                std::cout << "Load key: " << key << " value: " << value << std::endl;
            }
        }
    }
    fileReader.close();
}

// is valid string
template <typename K, typename V>
bool SkipList<K, V>::isValidString(const std::string& str) {
    if (str.empty()) {
        return false;
    }
    if (str.find(delimiter) == std::string::npos) {
        return false;
    }
    return true;
}

// get key and value from string
template <typename K, typename V>
void SkipList<K, V>::getKeyValueFromString(const std::string& str, std::string* key, std::string* value) {
    if (isValidString(str)) {
        size_t pos = str.find(delimiter);
        *key = str.substr(0, pos);
        *value = str.substr(pos + 1);
    }
}

// delete element
template <typename K, typename V>
int SkipList<K, V>::deleteElement(K key) {
    mtx.lock();
    Node<K, V>* current = header;
    // create update array and initialize it
    Node<K, V>* update[MaxLevel + 1];
    std::memset(update, 0, sizeof(Node<K, V>*) * (MaxLevel + 1));

    /*    start from highest level of skip list
        move the current pointer forward while key is greater than key of node next to current
        otherwise inserted current in update and move one level down and continue search
    */
    for (int i = CurrentLevel; i >= 0; i--) {
        while (current->forward[i] != nullptr && current->forward[i]->getKey() < key) {
            current = current->forward[i];
        }
        update[i] = current;
    }
    // reached level 0 and advance pointer to right, which is possibly our desired node
    current = current->forward[0];
    // if current node is target node
    if (current != nullptr && current->getKey() == key) {
        // start from lowest level and rearrange pointers just like we do in singly linked list
        // to remove target node
        for (int i = 0; i <= CurrentLevel; i++) {
            // if at level i, next node is not target node, break the loop, no need to process
            // further level
            if (update[i]->forward[i] != current) {
                break;
            }
            update[i]->forward[i] = current->forward[i];
        }
        // remove target node
        delete current;
        // reduce list level
        while (CurrentLevel > 0 && header->forward[CurrentLevel] == nullptr) {
            CurrentLevel--;
        }
        std::cout << "Successfully deleted key " << key << std::endl;
        sizeOfList--;
        mtx.unlock();
        return 0;
    }
    mtx.unlock();
    return 1;
}

// search element
template <typename K, typename V>
bool SkipList<K, V>::searchElement(K key) {
    mtx.lock();
    Node<K, V>* current = header;
    // start from highest level of skip list
    for (int i = CurrentLevel; i >= 0; i--) {
        while (current->forward[i] != nullptr && current->forward[i]->getKey() < key) {
            current = current->forward[i];
        }
    }
    // reached level 0 and advance pointer to right, which is possibly our desired node
    current = current->forward[0];
    // if current node is target node
    if (current != nullptr && current->getKey() == key) {
        std::cout << "Found key: " << key << ", Value" << current->getValue() << std::endl;
        mtx.unlock();
        return true;
    }
    mtx.unlock();
    return false;
}

// get skip list size
template <typename K, typename V>
int SkipList<K, V>::size() {
    return sizeOfList;
}

// random level generator
template <typename K, typename V>
int SkipList<K, V>::randomLevel() {    
    int k = 1;
    while ((dis(gen) % 2) != 0) {
        k++;
    }
    k = (k < MaxLevel) ? k : MaxLevel;
    return k;
}