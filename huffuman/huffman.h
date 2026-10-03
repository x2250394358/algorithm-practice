#ifndef HUFFMAN_H       /* 防止头文件被重复包含 */
#define HUFFMAN_H
#include "bitio.h"      /* 位级读写，编码/解码时需要 */
#include<stdint.h>      /* uint64_t 等定长整数类型 */
#include<stdio.h>       /* FILE 等标准输入输出 */
#define MAX_SIZE 256    /* 字节取值范围 0~255，共 256 个 */

/*
 * 哈夫曼树节点结构体
 * 每个节点要么是叶子节点（保存一个实际字符），要么是内部节点
 * （由两个子树合并而来，data 无意义，通常置 0）。
 * 字段含义：
 *   data      - 叶子节点对应的实际字节值（0~255）
 *   frequency - 该节点代表的字符（或子树字符集）的出现次数/权重
 *   left      - 左孩子指针（编码中对应位 '0'）
 *   right     - 右孩子指针（编码中对应位 '1'）
 */
typedef struct HuffmanNode{
    unsigned char data;
    uint64_t frequency;
    struct  HuffmanNode *left;
    struct  HuffmanNode *right;

    
} HuffmanNode;

/*
 * 最小堆结构体（用数组实现的二叉堆）
 * 用于存放哈夫曼节点，按 frequency（权重）从小到大排列，
 * 方便每次取出两个权重最小的节点来合并构建哈夫曼树。
 * 字段含义：
 *   data - 指针数组，保存堆中的各个节点
 *   size - 当前堆中元素个数
 */
typedef struct{
    HuffmanNode* data[MAX_SIZE];
    int size;
}Minheap;


/*
 * 压缩文件头结构体
 * 压缩文件开头需要保存足够的信息，以便解压时重建哈夫曼树。
 * 布局（按此顺序写入文件）：
 *   magic        - 4 字节魔数 "HUF!"，用于校验该文件是否为合法压缩文件
 *   originalSize - 原始文件字节数，解压时据此判断解码到何时停止
 *   frequency    - 256 个字节各自的出现次数，解压时据此重建哈夫曼树
 */
typedef struct{
    char magic[4];
    uint64_t originalSize;
    uint64_t frequency[256];
}HuffmanHeader;

/* 创建一个新的哈夫曼树节点并返回；分配失败返回 NULL */
HuffmanNode *createNode(unsigned char data, uint64_t frequency);

/* 统计缓冲区 data（长度 size）中各字节的出现次数，存入 frequency 数组 */
void countFrequency(const unsigned char* data, size_t size, uint64_t frequenct[256]);
/* 初始化最小堆：把元素个数清零 */
void heapInit(Minheap* headp);
/* 向最小堆插入一个节点，自动上浮保持堆序 */
void heapInsert(Minheap* heap, HuffmanNode* node);
/* 从最小堆取出权重最小的节点（堆顶），并调整堆维持堆序 */
HuffmanNode* extractMin(Minheap* heap);
/* 根据各字节频率构建哈夫曼树，返回根节点 */
HuffmanNode* buildHuffmanTree(uint64_t frequency[256]);
/* 遍历哈夫曼树，为每个叶子字符生成对应的二进制编码，存入 codes */
void generateCodes(HuffmanNode* root, char codes[256][256]);
/* 按编码表把原始数据逐位写入位写入器（压缩核心步骤） */
void encodeData(const unsigned char* data, size_t size, char codes[256][256], BitWriter* writer);
/* 把文件头 header 写入文件（魔数、原始大小、频率表） */
void writeHeader(FILE* file, const HuffmanHeader* header);
/* 从文件读入并校验文件头，成功返回 1，失败返回 0 */
int readHeader(FILE* file, HuffmanHeader* header);
/* 从压缩文件逐位解码出 originalSize 个字节写入 output，成功返回 1，失败返回 0 */
int decodeData(FILE* file, HuffmanNode* root, uint64_t originalSize, FILE* output);
/* 递归释放整棵哈夫曼树，防止内存泄漏 */
void freeHuffmanTree(HuffmanNode* root);
/* 以二进制方式读取整个文件内容到堆内存，成功返回 1，失败返回 0 */
int readFile(const char* filename, unsigned char** data, size_t *fileSize);
/* 压缩：读取输入文件，构建哈夫曼树并写出压缩文件，成功返回 1 */
int compressFile(const char* inputFilename, const char* outputFilename);
/* 解压：读取压缩文件，重建哈夫曼树并还原原始文件，成功返回 1 */
int decompressFile(const char* inputFilename, const char* outputFilename);

#endif
