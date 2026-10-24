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
        if(front == NULL) return NULL;
        Node* p = front;
        front = front->next;
        p->next = NULL;
        return p;
    }
    bool isEmpty(){
        return front == NULL;
    }
};

class Compressor{
private:
    Node* root;
    int freq[256];
    string codes[256];
    int originalSize;
public:
    Compressor(){
        root = NULL;
        for(int i = 0; i < 256; i++){
            freq[i] = 0;
            codes[i] = "";
        }
        originalSize = 0;
    }
    ~Compressor(){
        destroyTree(root);
    }
    void destroyTree(Node* p){
        if(p == NULL) return;
        destroyTree(p->left);
        destroyTree(p->right);
        delete p;
    }
    void countFrequency(string filename){
        for(int i = 0; i < 256; i++) freq[i] = 0;
        originalSize = 0;
        ifstream file(filename.c_str());
        if(!file) return;
        char ch;
        while(file.get(ch)){
            int idx = ch;
            if(idx < 0) idx += 256;
            freq[idx]++;
            originalSize++;
        }
        file.close();
    }
    void buildTree(){
        destroyTree(root);
        root = NULL;
        MinQueue qQueue;
        for(int i = 0; i < 256; i++){
            if(freq[i] > 0){
                Node* p = new Node((char)i, freq[i]);
                qQueue.enqueue(p);
            }
        }
        if(qQueue.isEmpty()) return;
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
        for(int i = 0; i < 256; i++) codes[i] = "";
        generateCodes(root, "");
    }
    void generateCodes(Node* p, string str){
        if(p == NULL) return;
        if(p->left == NULL && p->right == NULL){
            int idx = p->data;
            if(idx < 0) idx += 256;
            codes[idx] = str;
        }
        generateCodes(p->left, str + "0");
        generateCodes(p->right, str + "1");
    }
    void displayFrequency(string filename){
        countFrequency(filename);
        if(originalSize == 0){
            cout << "File is empty or not found.\n";
            return;
        }
        for(int i = 0; i < 256; i++){
            if(freq[i] > 0){
                if(i == '\n') cout << "Newline";
                else if(i == ' ') cout << "Space";
                else if(i == '\t') cout << "Tab";
                else cout << (char)i;
                cout << " : " << freq[i] << "\n";
            }
        }
    }
    void displayCodes(string filename){
        countFrequency(filename);
        if(originalSize == 0){
            cout << "File is empty or not found.\n";
            return;
        }
        buildTree();
        for(int i = 0; i < 256; i++){
            if(freq[i] > 0){
                if(i == '\n') cout << "Newline";
                else if(i == ' ') cout << "Space";
                else if(i == '\t') cout << "Tab";
                else cout << (char)i;
                cout << " : " << codes[i] << "\n";
            }
        }
    }
    void compress(string filename){
        countFrequency(filename);
        if(originalSize == 0){
            cout << "File is empty or not found.\n";
            return;
        }
        buildTree();
        
        CreateDirectoryA("huffmancodes", NULL);
        CreateDirectoryA("compressed", NULL);
        
        string metaFile = "huffmancodes/HuffmanCodes_" + filename;
        string binFile = "compressed/Compressed_" + filename;
        
        ofstream outMeta(metaFile.c_str());
        outMeta << originalSize << "\n";
        for(int i = 0; i < 256; i++){
            outMeta << freq[i] << " ";
        }
        outMeta << "\n";
        outMeta.close();
        
        ifstream inFile(filename.c_str());
        ofstream outBin(binFile.c_str(), ios::binary);
        char ch;
        char bitBuffer = 0;
        int bitCount = 0;
        int totalBits = 0;
        
        while(inFile.get(ch)){
            int idx = ch;
            if(idx < 0) idx += 256;
            string code = codes[idx];
            for(int i = 0; i < code.length(); i++){
                if(code[i] == '1'){
                    bitBuffer = (bitBuffer << 1) | 1;
                } else {
                    bitBuffer = (bitBuffer << 1);
                }
                bitCount++;
                totalBits++;
                if(bitCount == 8){
                    outBin.write(&bitBuffer, 1);
                    bitBuffer = 0;
                    bitCount = 0;
                }
            }
        }
        if(bitCount > 0){
            bitBuffer = bitBuffer << (8 - bitCount);
            outBin.write(&bitBuffer, 1);
        }
        
        inFile.close();
        outBin.close();
        
        int finalBytes = totalBits / 8;
        if(totalBits % 8 != 0) finalBytes++;
        
        cout << "Original Size: " << originalSize << " bytes\n";
        cout << "Compressed Size: " << finalBytes << " bytes\n";
        double saved = 100.0 - (((double)finalBytes / originalSize) * 100.0);
        cout << "Space Saved: " << saved << "%\n";
        cout << "Metadata saved to: " << metaFile << "\n";
        cout << "Compressed data saved to: " << binFile << "\n";
    }
    void decompress(string filename){
        string metaFile = "huffmancodes/HuffmanCodes_" + filename;
        string binFile = "compressed/Compressed_" + filename;
        
        ifstream inMeta(metaFile.c_str());
        if(!inMeta){
            cout << "Metadata file not found in huffmancodes/ folder.\n";
            return;
        }
        inMeta >> originalSize;
        for(int i = 0; i < 256; i++) freq[i] = 0;
        for(int i = 0; i < 256; i++){
            inMeta >> freq[i];
        }
        inMeta.close();
        
        buildTree();
        if(root == NULL){
            return;
        }
        
        ifstream inBin(binFile.c_str(), ios::binary);
        if(!inBin){
            cout << "Compressed binary file not found in compressed/ folder.\n";
            return;
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
                if(bit == 0) p = p->left;
                else if(bit == 1) p = p->right;
                
                if(p->left == NULL && p->right == NULL){
                    outFile << p->data;
                    charsWritten++;
                    p = root;
                    if(charsWritten == originalSize) break;
                }
            }
        }
        inBin.close();
        outFile.close();
        cout << "File decompressed to " << outFileStr << " successfully.\n";
    }
};

int main(){
    Compressor compressor;
    int choice;
    string file1;
    
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
                cout << "Enter file to analyze: ";
                cin >> file1;
                compressor.displayFrequency(file1);
                system("pause");
                break;
            case 2:
                system("cls");
                cout << "Enter file for Huffman codes: ";
                cin >> file1;
                compressor.displayCodes(file1);
                system("pause");
                break;
            case 3:
                system("cls");
                cout << "Enter file to compress: ";
                cin >> file1;
                cout << "\n";
                compressor.compress(file1);
                system("pause");
                break;
            case 4:
                system("cls");
                cout << "Enter original filename to decompress: ";
                cin >> file1;
                cout << "\n";
                compressor.decompress(file1);
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
