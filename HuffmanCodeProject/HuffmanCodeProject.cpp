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
        int freq[256] = {0};
        string codes[256];
        int originalSize;
        
    public:
        HuffmanCode(){
            root = NULL;
            originalSize = 0;
        }
        ~HuffmanCode(){
            destroyTree(root);
        }
        void destroyTree(Node* p);
        bool countFrequency(string filename);
        void buildTree();
        void generateCodes(Node* p, string str);
        void displayChar(int i);
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

bool HuffmanCode :: countFrequency(string filename){
    for(int i = 0; i < 256; i++){
        freq[i] = 0;
    }

    originalSize = 0;
    ifstream file(filename);

    if(file){
        char ch;
        while(file.get(ch)){
            freq[ch]++;
            originalSize++;
        }
        file.close();
    }
    
    if(originalSize == 0){
        cout << "File is empty or not found.\n";
        return false;
    }
    return true;
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
        codes[p->data] = str;
    }
    generateCodes(p->left, str + "0");
    generateCodes(p->right, str + "1");
}

void HuffmanCode :: displayChar(int i){
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
}

void HuffmanCode :: displayFrequency(string filename){
    if(!countFrequency(filename)){
        return;
    }
    for(int i = 0; i < 256; i++){
        if(freq[i] > 0){
            displayChar(i);
            cout << " : " << freq[i] << "\n";
        }
    }
}

void HuffmanCode :: displayCodes(string filename){
    if(!countFrequency(filename)){
        return;
    }
    buildTree();
    for(int i = 0; i < 256; i++){
        if(freq[i] > 0){
            displayChar(i);
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
    ofstream outCompressed(compressedFile, ios::binary);

    outCompressed << originalSize << "\n";

    for(int i = 0; i < 256; i++){
        outCompressed << freq[i];
        if(i < 255){
            outCompressed << " ";
        }
    }
    outCompressed << "\n";
    
    ifstream inFile(filename);

    char ch;
    char bitBuffer = 0;
    int bitCount = 0;
    int totalBits = 0;
    
    while(inFile.get(ch)){
        string code = codes[ch];
        for(int i = 0; i < code.length(); i++){
            bitBuffer = bitBuffer << 1;
            if(code[i] == '1'){
                bitBuffer = bitBuffer | 1;
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
    ifstream inMeta(compressedFile);
    
    if(!inMeta){
        cout << "Compressed file not found in compressed/ folder.\n";
        return false;
    }
    
    inMeta >> originalSize;
    for(int i = 0; i < 256; i++){
        inMeta >> freq[i];
    }
    inMeta.close();
    
    return true;
}

void HuffmanCode :: compress(string filename){
    if(!countFrequency(filename)){
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
    ifstream inBin(compressedFile, ios::binary);
    
    string header;
    getline(inBin, header);
    getline(inBin, header);
    
    CreateDirectoryA("decompressed", NULL);
    string outFileStr = "decompressed/Decompressed_" + filename;
    ofstream outFile(outFileStr);
    
    char byte;
    int charsWritten = 0;
    Node* p = root;
    
    while(inBin.read(&byte, 1) && charsWritten < originalSize){
        for(int i = 7; i >= 0; i--){
            if(byte & (1 << i)){
                p = p->right;
            }
            else {
                p = p->left;
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
    int freqDecomp[256] = {0};
    int decompSize = 0;
    bool sameOrder = true;
    bool freqMatch = true;
    
    if(!countFrequency(filename)){
        return;
    }
    
    string decompFile = "decompressed/Decompressed_" + filename;
    ifstream fileOrig(filename);
    ifstream fileDecomp(decompFile);
    
    if(!fileDecomp){
        cout << "Decompressed file not found. Please decompress first.\n";
        return;
    }
    
    char ch1, ch2;
    while(fileDecomp.get(ch2)){
        freqDecomp[ch2]++;
        decompSize++;
        if(!fileOrig.get(ch1) || ch1 != ch2){
            sameOrder = false;
        }
    }
    
    fileOrig.close();
    fileDecomp.close();
    
    cout << "========================================\n";
    cout << "Char\tOriginal\tDecompressed\n";
    cout << "========================================\n";
    
    for(int i = 0; i < 256; i++){
        if(freq[i] > 0 || freqDecomp[i] > 0){
            displayChar(i);
            cout << "\t" << freq[i] << "\t\t" << freqDecomp[i] << "\n";
            if(freq[i] != freqDecomp[i]){
                freqMatch = false;
            }
        }
    }
    
    cout << "========================================\n";
    if(originalSize == decompSize && freqMatch && sameOrder){
        cout << "Result: Integrity Check Passed! Files are identical.\n";
    }
    else {
        cout << "Result: Integrity Check Failed! Files do not match.\n";
        if(originalSize != decompSize){
            cout << "Error: Files are not the same size.\n";
        }
        if(!freqMatch){
            cout << "Error: Character frequencies do not match.\n";
        }
        else if(!sameOrder){
            cout << "Error: Same character frequency but different arrangement found.\n";
        }
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
        
        system("cls");
        switch(choice){
            case 1:
                cout << "Enter file to analyze: ";
                cin >> file1;
                h.displayFrequency(file1);
                break;
            case 2:
                cout << "Enter file for Huffman codes: ";
                cin >> file1;
                h.displayCodes(file1);
                break;
            case 3:
                cout << "Enter file to compress: ";
                cin >> file1;
                cout << "\n";
                h.compress(file1);
                break;
            case 4:
                cout << "Enter original filename to decompress: ";
                cin >> file1;
                cout << "\n";
                h.decompress(file1);
                break;
            case 5:
                cout << "Enter original filename to verify integrity: ";
                cin >> file1;
                cout << "\n";
                h.checkIntegrity(file1);
                break;
            case 6:
                exit(0);
            default:
                cout << "Invalid Input.\n";
                break;
        }
        system("pause");
    }
}