#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
using namespace std;

struct Node{
    char data;
    int freq;
    Node* left;
    Node* right;
    Node* next;
    Node(char c, int x){
        data = c;
        freq = x;
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
        void enqueue(Node* p);
        Node* dequeue();
        bool isEmpty();
};

void MinQueue :: enqueue(Node* p){
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

Node* MinQueue :: dequeue(){
    if(front == NULL) {
        return NULL;
    }
    Node* p = front;
    front = front->next;
    p->next = NULL;
    return p;
}

bool MinQueue ::  isEmpty(){
    return front == NULL;
}

class HuffmanCode{
    private:
        Node* root;
        int freq[256];
        string codes[256];
        int originalSize;
        
    public:
        HuffmanCode(){
            root = NULL;
            for(int i = 0; i < 256; i++){
                freq[i] = 0;
                codes[i] = "";
            }
            originalSize = 0;
        }
        ~HuffmanCode(){
            destroyTree(root);
        }
        void destroyTree(Node* p);
        void countFrequency(string filename);
        void buildTree();
        void generateCodes(Node* p, string str);
        void displayFrequency(string filename);
        void displayCodes(string filename);
        void showStats(int totalBits, string compressedFile);
        void save(string filename);
        bool retrieve(string filename);
        void compress(string filename);
        void decompress(string filename);
        void checkIntegrity(string filename);
};

void HuffmanCode :: destroyTree(Node* p){
    if(p == NULL){
        return;
    }
    destroyTree(p->left);
    destroyTree(p->right);
    delete p;
}

void HuffmanCode :: countFrequency(string filename){
    for(int i = 0; i < 256; i++){
        freq[i] = 0;
    }

    originalSize = 0;
    ifstream file(filename.c_str());

    if(!file){
        cout << "File not found!" << endl;
        system("pause");
        return;
    }
    char ch;
    while(file.get(ch)){
        int i = ch;
        if(i < 0){
            i += 256;
        }
        freq[i]++;
        originalSize++;
    }
    file.close();
}

void HuffmanCode :: buildTree(){
    destroyTree(root);
    root = NULL;
    MinQueue qQueue;
    for(int i = 0; i < 256; i++){
        if(freq[i] > 0){
            Node* p = new Node((char)i, freq[i]);
            qQueue.enqueue(p);
        }
    }

    if(qQueue.isEmpty()){
        cout << "File is empty, nothing to build a tree with!" << endl;
        system("pause");
        return;
    }

    Node* first = qQueue.dequeue();
    if(qQueue.isEmpty()){
        root = new Node('\0', first->freq);
        root->left = first;
    } else {
        qQueue.enqueue(first);
        while(true){
            Node* p = qQueue.dequeue();
            if(qQueue.isEmpty()){
                root = p;
                break;
            }
            Node* q = qQueue.dequeue();
            Node* parent = new Node('\0', p->freq + q->freq);
            parent->left = p;
            parent->right = q;
            qQueue.enqueue(parent);
        }
    }

    for(int i = 0; i < 256; i++){
        codes[i] = "";
    }
    generateCodes(root, "");
}

void HuffmanCode :: generateCodes(Node* p, string str){
    if(p == NULL){
        return;
    }
    if(p->left == NULL && p->right == NULL){
        int i = p->data;
        if(i < 0) {
            i += 256;
        }
        codes[i] = str;
    }
    generateCodes(p->left, str + "0");
    generateCodes(p->right, str + "1");
}

void HuffmanCode :: displayFrequency(string filename){
    countFrequency(filename);
    if(originalSize == 0){
        cout << "File is empty or not found.\n";
        return;
    }
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
            else{
                cout << (char)i;
            }
            cout << " : " << freq[i] << "\n";
        }
    }
}

void HuffmanCode :: displayCodes(string filename){
    countFrequency(filename);
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
            else{
                cout << (char)i;
            }
            cout << " : " << codes[i] << "\n";
        }
    }
}

void HuffmanCode :: showStats(int totalBits, string compressedFile){
    int finalBytes = totalBits / 8;
    if(totalBits % 8 != 0){
        finalBytes++;
    }
    
    cout << "Original Size: " << originalSize << " bytes\n";
    cout << "Compressed Size: " << finalBytes << " bytes\n";
    double saved = 100.0 - (((double)finalBytes / originalSize) * 100.0);
    cout << "Space Saved: " << saved << "%\n";
    cout << "Metadata and Compressed data saved to: " << compressedFile << "\n";
}

void HuffmanCode :: save(string filename){
    CreateDirectoryA("compressed", NULL);
    
    string compressedFile = "compressed/Compressed_" + filename;
    ofstream outCompressed(compressedFile.c_str(), ios::binary);

    outCompressed << originalSize << "\n";

    for(int i = 0; i < 256; i++){
        outCompressed << freq[i];
        if(i < 255){
            outCompressed << " ";
        }
    }
    outCompressed << "\n";
    
    ifstream inFile(filename.c_str());

    char ch;
    char bitBuffer = 0;
    int bitCount = 0;
    int totalBits = 0;
    
    while(inFile.get(ch)){
        int i = ch;
        if(i < 0) {
            i += 256;
        }
        string code = codes[i];
        for(int i = 0; i < code.length(); i++){
            if(code[i] == '1'){
                bitBuffer = (bitBuffer << 1) | 1;
            } else {
                bitBuffer = (bitBuffer << 1);
            }
            bitCount++;
            totalBits++;
            if(bitCount == 8){
                outCompressed.write(&bitBuffer, 1);
                bitBuffer = 0;
                bitCount = 0;
            }
        }
    }
    if(bitCount > 0){
        bitBuffer = bitBuffer << (8 - bitCount);
        outCompressed.write(&bitBuffer, 1);
    }
    
    inFile.close();
    outCompressed.close();
    
    showStats(totalBits, compressedFile);
}

bool HuffmanCode :: retrieve(string filename){
    string compressedFile = "compressed/Compressed_" + filename;
    ifstream inMeta(compressedFile.c_str());
    
    if(!inMeta){
        cout << "Compressed file not found in compressed/ folder.\n";
        return false;
    }
    
    inMeta >> originalSize;
    for(int i = 0; i < 256; i++) {
        freq[i] = 0;
    }
    for(int i = 0; i < 256; i++){
        inMeta >> freq[i];
    }
    inMeta.close();
    
    return true;
}

void HuffmanCode :: compress(string filename){
    countFrequency(filename);
    if(originalSize == 0){
        cout << "File is empty or not found.\n";
        return;
    }
    buildTree();
    save(filename);
}

void HuffmanCode :: decompress(string filename){
    if(!retrieve(filename)){
        return;
    }
    
    buildTree();
    if(root == NULL){
        return;
    }
    
    string compressedFile = "compressed/Compressed_" + filename;
    ifstream inBin(compressedFile.c_str(), ios::binary);
    if(!inBin){
        cout << "Compressed binary file not found in compressed/ folder.\n";
        return;
    }
    
    int dummySize;
    inBin >> dummySize;
    for(int i = 0; i < 256; i++){
        int dummyFreq;
        inBin >> dummyFreq;
    }
    
    char skipCh;
    while(inBin.get(skipCh)){
        if(skipCh == '\n'){
            break;
        }
    }
    
    CreateDirectoryA("decompressed", NULL);
    string outFileStr = "decompressed/Decompressed_" + filename;
    ofstream outFile(outFileStr.c_str());
    
    char byte;
    int charsWritten = 0;
    Node* p = root;
    
    while(inBin.read(&byte, 1) && charsWritten < originalSize){
        for(int i = 7; i >= 0; i--){
            int bit = (byte & (1 << i)) ? 1 : 0;
            if(bit == 0) {
                p = p->left;
            }
            else if(bit == 1) {
                p = p->right;
            }
            
            if(p->left == NULL && p->right == NULL){
                outFile << p->data;
                charsWritten++;
                p = root;
                if(charsWritten == originalSize) {
                    break;
                }
            }
        }
    }
    inBin.close();
    outFile.close();
    cout << "File decompressed to " << outFileStr << " successfully.\n";
}

void HuffmanCode :: checkIntegrity(string filename){
    int freqOrig[256];
    int freqDecomp[256];
    int sizeOrig = 0;
    int sizeDecomp = 0;
    
    for(int i = 0; i < 256; i++){
        freqOrig[i] = 0;
        freqDecomp[i] = 0;
    }
    
    ifstream fileOrig(filename.c_str());
    if(!fileOrig){
        cout << "Original file not found for integrity check.\n";
        return;
    }
    char ch;
    while(fileOrig.get(ch)){
        int i = ch;
        if(i < 0){
            i += 256;
        }
        freqOrig[i]++;
        sizeOrig++;
    }
    fileOrig.close();
    
    string decompFile = "decompressed/Decompressed_" + filename;
    ifstream fileDecomp(decompFile.c_str());
    if(!fileDecomp){
        cout << "Decompressed file not found. Please decompress first.\n";
        return;
    }
    while(fileDecomp.get(ch)){
        int i = ch;
        if(i < 0){
            i += 256;
        }
        freqDecomp[i]++;
        sizeDecomp++;
    }
    fileDecomp.close();
    
    cout << "========================================\n";
    cout << "Char\tOriginal\tDecompressed\n";
    cout << "========================================\n";
    
    bool isIdentical = true;
    if(sizeOrig != sizeDecomp){
        isIdentical = false;
    }
    
    for(int i = 0; i < 256; i++){
        if(freqOrig[i] > 0 || freqDecomp[i] > 0){
            if(freqOrig[i] != freqDecomp[i]){
                isIdentical = false;
            }
            
            if(i == '\n'){
                cout << "NewLine\t";
            }
            else if(i == ' '){
                cout << "Space\t";
            }
            else if(i == '\t'){
                cout << "Tab\t";
            }
            else {
                cout << (char)i << "\t";
            }
            cout << freqOrig[i] << "\t\t" << freqDecomp[i] << "\n";
        }
    }
    
    cout << "========================================\n";
    if(isIdentical){
        cout << "Result: Integrity Check Passed! Files are identical.\n";
    }
    else {
        cout << "Result: Integrity Check Failed! Files do not match.\n";
    }
}

int main(){
    HuffmanCode h;
    int choice;
    string file1;
    
    while(true){
        system("cls");
        cout << "===== HUFFMAN COMPRESSION =====\n";
        cout << "1. Character Frequency\n";
        cout << "2. Display Huffman Codes\n";
        cout << "3. Compress File\n";
        cout << "4. Decompress File\n";
        cout << "5. Check Integrity\n";
        cout << "6. Exit\n";
        cout << "Input: ";
        cin >> choice;
        
        switch(choice){
            case 1:
                system("cls");
                cout << "Enter file to analyze: ";
                cin >> file1;
                h.displayFrequency(file1);
                system("pause");
                break;
            case 2:
                system("cls");
                cout << "Enter file for Huffman codes: ";
                cin >> file1;
                h.displayCodes(file1);
                system("pause");
                break;
            case 3:
                system("cls");
                cout << "Enter file to compress: ";
                cin >> file1;
                cout << "\n";
                h.compress(file1);
                system("pause");
                break;
            case 4:
                system("cls");
                cout << "Enter original filename to decompress: ";
                cin >> file1;
                cout << "\n";
                h.decompress(file1);
                system("pause");
                break;
            case 5:
                system("cls");
                cout << "Enter original filename to verify integrity: ";
                cin >> file1;
                cout << "\n";
                h.checkIntegrity(file1);
                system("pause");
                break;
            case 6:
                exit(0);
            default:
                cout << "Invalid Input.\n";
                system("pause");
                break;
        }
    }
    return 0;
}