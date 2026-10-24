#include <iostream>
#include <fstream>
#include <string>
using namespace std;

struct Node{
    char data;
    int freq;
    Node* left;
    Node* right;
    Node* next;
    Node(char d, int f){
        data = d;
        freq = f;
        left = NULL;
        right = NULL;
        next = NULL;
    }
};

class MinQueue{
private:
    Node* front;
public:
    MinQueue(){
        front = NULL;
    }
    ~MinQueue(){
        while(front != NULL){
            Node* p = front;
            front = front->next;
            p->next = NULL;
        }
    }

    void enqueue(Node* p){
        p->next = NULL;
        if(front == NULL || front->freq > p->freq){
            p->next = front;
            front = p;
            return;
        }
        Node* q = front;
        while(q->next != NULL && q->next->freq <= p->freq){
            q = q->next;
        }
        p->next = q->next;
        q->next = p;
    }

    Node* dequeue(){
        if(front == NULL) {
            return NULL;
        }
        Node* p = front;
        front = front->next;
        p->next = NULL;
        return p;
    }

    bool isEmpty(){
        return front == NULL;
    }
};

class Huffman{
private:
    Node* root;
    int freq[256];
    string codes[256];
    string filename;
    int originalSize;

public:
    Huffman(string x){
        filename = x;
        root = NULL;
        for(int i = 0; i < 256; i++){
            freq[i] = 0;
            codes[i] = "";
        }
        originalSize = 0;
    }
    ~Huffman(){
        destroyTree(root);
    }

    void destroyTree(Node* p){
        if(p == NULL){
            return;
        }
        destroyTree(p->left);
        destroyTree(p->right);
        delete p;
    }

    void countFrequency(){
        for(int i = 0; i < 256; i++){
            freq[i] = 0;
        }

        originalSize = 0;
        ifstream file(filename.c_str());

        if(!file){
            return;
        }
        char ch;
        while(file.get(ch)){
            int i = ch;
            if(i < 0) i += 256;
            freq[i]++;
            originalSize++;
        }
        file.close();
    }

    void buildTree(){
        destroyTree(root);
        root = NULL;
        MinQueue queue;
        for(int i = 0; i < 256; i++){
            if(freq[i] > 0){
                Node* newNode = new Node((char)i, freq[i]);
                queue.enqueue(newNode);
            }
        }
        if(queue.isEmpty()) {
            return;
        }
        Node* first = queue.dequeue();
        if(queue.isEmpty()){
            root = new Node('\0', first->freq);
            root->left = first;
        } else {
            queue.enqueue(first);
            while(true){
                Node* p = queue.dequeue();
                if(queue.isEmpty()){
                    root = p;
                    break;
                }
                Node* q = queue.dequeue();
                Node* parent = new Node('\0', p->freq + q->freq);
                parent->left = p;
                parent->right = q;
                queue.enqueue(parent);
            }
        }
        generateCodes(root, "");
    }

    void generateCodes(Node* p, string str){
        if(p == NULL){
            return;
        }
        if(p->left == NULL && p->right == NULL){
            int i = p->data;
            if(i < 0) i += 256;
            codes[i] = str;
        }
        generateCodes(p->left, str + "0");
        generateCodes(p->right, str + "1");
    }

    void displayFrequency(){
        countFrequency();
        for(int i = 0; i < 256; i++){
            if(freq[i] > 0){
                if(i == '\n') {
                    cout << "Newline";
                }
                else if(i == ' ') {
                    cout << "Space";
                }
                else if(i == '\t') {
                    cout << "Tab";
                }
                else {
                    cout << (char)i;
                }
                cout << " : " << freq[i] << "\n";
            }
        }
    }

    void displayCodes(){
        countFrequency();
        if(originalSize == 0){
            cout << "File is empty or not found.\n";
            return;
        }
        buildTree();
        for(int i = 0; i < 256; i++){
            if(freq[i] > 0){
                if(i == '\n') {
                    cout << "Newline";
                }
                else if(i == ' ') {
                    cout << "Space";
                }
                else if(i == '\t') {
                    cout << "Tab";
                }
                else {
                    cout << (char)i;
                }
                cout << " : " << codes[i] << "\n";
            }
        }
    }

    void compress(){
        countFrequency();
        if(originalSize == 0){
            cout << "File is empty or not found.\n";
            return;
        }

        buildTree();
        ifstream currFile(filename.c_str());
        ofstream resFile("compressed.huf");

        for(int i = 0; i < 256; i++){
            resFile << freq[i] << " ";
        }

        resFile << "\n";
        char ch;
        int currSize = 0;

        while(currFile.get(ch)){
            int i = ch;
            if(i < 0) i += 256;
            string code = codes[i];
            resFile << code;
            currSize += code.length();
        }

        currFile.close();
        resFile.close();

        int newSize = currSize / 8;
        if(currSize % 8 != 0) {
            newSize++;
        }
        cout << "Original Size: " << originalSize << " bytes\n";
        cout << "Compressed Size (simulated): " << newSize << " bytes\n";
        double saved = 100.0 - (((double)newSize / originalSize) * 100.0);
        cout << "Space Saved: " << saved << "%\n";
    }

    void decompress(){
        ifstream currFile("compressed.txt");
        if(!currFile){
            cout << "Compressed file not found.\n";
            return;
        }

        for(int i = 0; i < 256; i++) {
            freq[i] = 0;
        }

        for(int i = 0; i < 256; i++){
            currFile >> freq[i];
        }

        char ch;
        buildTree();
        if(root == NULL){
            currFile.close();
            return;
        }

        ofstream resFile("decompressed.txt");
        Node* p = root;
        while(currFile.get(ch)){
            if(ch != '0' && ch != '1') {
                continue;
            }
            if(ch == '0') {
                p = p->left;
            }
            else if(ch == '1') {
                p = p->right;
            }
            if(p->left == NULL && p->right == NULL){
                resFile << p->data;
                p = root;
            }
        }

        currFile.close();
        resFile.close();
        cout << "File decompressed to decompressed.txt successfully.\n";
    }
};

int main(){
    string filename;
    cout << "Enter file to select: ";
    cin >> filename;
    Huffman h(filename);
    int choice;
    while(true){
        system("cls");
        cout << "===== HUFFMAN COMPRESSION =====\n";
        cout << "1. Character Frequency\n";
        cout << "2. Display Huffman Codes\n";
        cout << "3. Compress File\n";
        cout << "4. Decompress File\n";
        cout << "5. Exit\n";
        cout << "Input: ";
        cin >> choice;
        switch(choice){
            case 1:
                system("cls");
                h.displayFrequency();
                system("pause");
                break;
            case 2:
                system("cls");
                h.displayCodes();
                system("pause");
                break;
            case 3:
                system("cls");
                h.compress();
                system("pause");
                break;
            case 4:
                system("cls");
                h.decompress();
                system("pause");
                break;
            case 5:
                exit(0);
            default:
                cout << "Invalid Input.\n";
                system("pause");
                break;
        }
    }
    return 0;
}