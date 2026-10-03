#include<stdlib.h>
#include<string.h>
#include "huffman.h"

/* 创建一个新的哈夫曼树节点：分配内存并初始化数据、频率和左右孩子，失败返回 NULL */
HuffmanNode *createNode(unsigned char data, uint64_t frequency)
{
    HuffmanNode *node = malloc(sizeof(HuffmanNode));
    if(node == NULL)
        return NULL;
    node->data = data;
    node->frequency = frequency;
    node->left = node->right = NULL;


    return node;
}

/* 统计字符频率：把 256 个频率清零后，按字节值作为下标对 data 每个字节计数 */
void countFrequency(const unsigned char* data, size_t size, uint64_t frequency[256])
{
    for(int i = 0; i < 256; i++)
    {
        frequency[i] = 0;
    }

    for(size_t i = 0; i < size; i++)
    {
        frequency[data[i]]++;
    }
}

/* 初始化最小堆：把元素个数清零 */
void heapInit(Minheap* heap)
{
    heap->size = 0;
}

/* 向最小堆插入一个节点：先放数组末尾，再不断与父节点比较、上浮，维持堆序 */
void heapInsert(Minheap* heap, HuffmanNode* node)
{
    int i = heap->size;
    heap->data[i] = node;
    heap->size++;
    while(i > 0)
    {
        int parent = (i - 1) / 2;
        if(heap->data[parent]->frequency <= heap->data[i]->frequency)
            break;
        HuffmanNode* tmp = heap->data[parent];
        heap->data[parent] = heap->data[i];
        heap->data[i] = tmp;

        i = parent;
    }
}

/* 取出堆顶（权重最小）节点：末尾元素补到堆顶后逐层下沉，恢复堆序；堆空返回 NULL */
HuffmanNode* extractMin(Minheap* heap)
{
    if(heap->size == 0)
        return NULL;
    HuffmanNode* min = heap->data[0];
    heap->size--;
    if(heap->size > 0)
    {
        heap->data[0] = heap->data[heap->size];
        int i = 0;
        while(1)
        {
            int l = i * 2 + 1;
            int r = l + 1;


            int smt = i;
            if(l < heap->size && heap->data[l]->frequency < heap->data[smt]->frequency)
            {
                smt = l;
            }
            if(r < heap->size && heap->data[r]->frequency < heap->data[smt]->frequency)
            {
                smt = r;
            }

            if(smt == i)
                break;
            HuffmanNode* tmp = heap->data[smt];
            heap->data[smt] = heap->data[i];
            heap->data[i] = tmp;

            i =smt;
        }
    }

    return min;
}

/* 构建哈夫曼树：把频率>0的字符作叶子入堆，反复取出两个最小节点合并为新父节点再入堆，直到只剩根 */
HuffmanNode* buildHuffmanTree(uint64_t frequency[256])
{
    Minheap *heap = malloc(sizeof(Minheap));
    heapInit(heap);
    for(int i = 0; i < 256; i++)
    {
        if(frequency[i] > 0)
        {
            HuffmanNode* node = createNode((unsigned char)i, frequency[i]);
            if(node == NULL)
                return NULL;
            heapInsert(heap, node);
        }
    }
    while(heap->size > 1)
    {
        HuffmanNode* l = extractMin(heap);
        HuffmanNode* r = extractMin(heap);
        HuffmanNode* p = createNode(0, l->frequency + r->frequency);
        if(!p)
            return NULL;
        p->left = l;
        p->right = r;

        heapInsert(heap, p);
    }
    HuffmanNode* root = extractMin(heap);
    free(heap);            /* 释放最小堆数组本身，避免每次构建泄漏约 2KB */
    return root;
}

/*
 * 递归生成哈夫曼编码（先序遍历）：向左走追加 '0'，向右走追加 '1'，
 * 到达叶子时把累积路径复制到 codes[该字符]。
 * 若整棵树只有一个叶子（只有一种字符，深度为 0），直接赋予编码 "0"。
 */
static void generateCodesRecursive(HuffmanNode* root, char codes[256][256], char code[], int depth)
{
    if(root == NULL)
    {
        return ;
    }
    if(root->left == NULL && root->right == NULL)
    {
        if(depth == 0){
            codes[root->data][0] = '0';
            codes[root->data][1] = '\0';
            return;
        }
        code[depth] = '\0';
        for(int i = 0; i <= depth; i++)
        {
            codes[root->data][i] = code[i];
        }
        return ;
    }
    code[depth] = '0';
    generateCodesRecursive(root->left, codes, code, depth + 1);

    code[depth] = '1';
    generateCodesRecursive(root->right, codes, code, depth + 1);


}

/* 生成编码表：先清空 codes 全部字符串，再递归填充；codes[c] 即字符 c 的二进制编码 */
void generateCodes(HuffmanNode* root, char codes[256][256])
{
    char code[256];
    for(int i = 0; i < 256; i++)
    {
        codes[i][0] = '\0';
    }
    generateCodesRecursive(root, codes, code, 0);
}

/* 编码：遍历原始数据每个字节，查编码表逐位 writeBit 写入位写入器 */
void encodeData(const unsigned char* data, size_t size, char codes[256][256], BitWriter* writer)
{
    for(size_t i = 0; i < size; i++)
    {
        unsigned char c = data[i];
        const char* code = codes[c];
        for(size_t j = 0; code[j] != '\0'; j++)
        {
            if(code[j] == '0')
            {
                writeBit(writer, 0);
            }
            else
            {
                writeBit(writer, 1);
            }
        }
    }
}

/* 写入文件头：依次写出 4 字节魔数、8 字节原始大小、256 个频率值 */
void writeHeader(FILE* file, const HuffmanHeader* header)
{
    fwrite(header->magic, sizeof(header->magic), 1, file);
    fwrite(&header->originalSize, sizeof(header->originalSize), 1, file);
    fwrite(header->frequency, sizeof(header->frequency[0]), 256, file);

}

/* 读取并校验文件头：魔数必须是 "HUF!"，任一读取失败或魔数不符返回 0，全部成功返回 1 */
int readHeader(FILE* file, HuffmanHeader* header)
{
    if(fread(header->magic ,sizeof(header->magic), 1, file) != 1)
        return 0;
    if(memcmp(header->magic, "HUF!", 4) !=0 )
        return 0;
    if(fread(&header->originalSize, sizeof(header->originalSize), 1, file) != 1)
        return 0;
    if(fread(header->frequency, sizeof(header->frequency[0]), 256, file) != 256)
        return 0;
    
    return 1;

}

/*
 * 解码：只有一种字符时直接连续写出 originalSize 个该字符；
 * 否则从根节点出发，位 0 走左、位 1 走右，到达叶子即写回该字符并回到根节点，
 * 直到解出 originalSize 个字符。读到文件末尾或走到空节点视为失败返回 0。
 */
int decodeData(FILE* file, HuffmanNode* root, uint64_t originalSize, FILE * output)
{

    //只有一种字符
    if(root->left == NULL && root->right == NULL)
    {
        for(uint64_t i = 0; i < originalSize; i++)
        {
            fputc(root->data, output);
        }
        return 1;
    } 


    BitReader reader;
    bitReaderInit(&reader, file);

    HuffmanNode* cur = root;
    uint64_t decodeSize = 0;
    while(decodeSize < originalSize)
    {
        int bit = readBit(&reader);
        if(bit == -1)
            return 0;
        if(bit == 0)
        {
            cur = cur->left;
        }
        else{
            cur = cur->right;
        }
        if(cur == NULL)
            return 0;
        if(cur->left == NULL && cur->right == NULL)
        {
            fputc(cur->data, output);
            decodeSize++;
            cur = root; //解码需每一次都回到根节点
        }

    }
    return 1;
}

/* 递归释放整棵哈夫曼树：先释放左右子树再释放当前节点，防止内存泄漏 */
void freeHuffmanTree(HuffmanNode* root)
{
    if(root == NULL)
        return;
    freeHuffmanTree(root->left);
    freeHuffmanTree(root->right);
    free(root);
}

/* 以二进制读取整个文件：定位末尾得大小->回到开头->分配缓冲->读入全部；空文件也视为成功 */
int readFile(const char* filename, unsigned char **data, size_t *fileSize)
{
    FILE* file = fopen(filename, "rb");
    if(file == NULL)
    {
        return 0;
    }
    if(fseek(file, 0, SEEK_END) != 0)
    {
        fclose(file);
        return 0;
    }
    long size = ftell(file);
    if(size < 0)
    {
        fclose(file);
        return 0;
    }

    rewind(file);

    if(size == 0)
    {
        *data = NULL;
        *fileSize = 0;
        fclose(file);
        return 1;
    }

    unsigned char* buffer = malloc((size_t)size);
    if(!buffer)
        return 0;
    size_t readSize = fread(buffer, 1, (size_t)size, file);
    if(readSize != (size_t)size)
    {
        free(buffer);
        fclose(file);

        return 0;
    }

    fclose(file);
    *data = buffer;
    *fileSize = (size_t)size;



    return 1;
}

/* 压缩主流程：读文件->统计频率->建哈夫曼树->生成编码表->写文件头->逐位编码写出->释放资源 */
int compressFile(const char* inputFilename, const char* outputFilename)
{
    unsigned char* data = NULL;
    size_t fileSize = 0;
    if(!readFile(inputFilename, &data, &fileSize))
    {
        printf("fail to open file: %s\n", inputFilename);
        return 0;
    }

    uint64_t frequency[256];
    countFrequency(data, fileSize, frequency);
    HuffmanNode* root = NULL;
    if(fileSize > 0)
    {
        root = buildHuffmanTree(frequency);
        if(root == NULL)
        {
            free(data);
            return 0;
        }
    }
    char codes[256][256];
    if(root != NULL)
    {
        generateCodes(root, codes);
    }

    FILE* output = fopen(outputFilename, "wb");
    if(output == NULL)
    {
        printf("can not to create output file: %s\n", outputFilename);
        freeHuffmanTree(root);
        free(data);

        return 0;
    }
    HuffmanHeader header;
    memcpy(header.magic, "HUF!", 4);
    header.originalSize = fileSize;
    for(int i = 0; i < 256; i++)
    {
        header.frequency[i] = frequency[i];
    }
    writeHeader(output, &header);

    if(fileSize > 0)
    {
        BitWriter writer;
        bitWriterInit(&writer, output);

        encodeData(data, fileSize, codes, &writer);
        flushBits(&writer);
    }
    fclose(output);

    freeHuffmanTree(root);
    free(data);
    printf("compressed: %s -> %s\n", inputFilename, outputFilename);

    return 1;
}

/* 解压主流程：读文件头->校验->（空文件直接建空输出）->重建哈夫曼树->逐位解码->释放资源 */
int decompressFile(const char* inputFilename, const char* outputFilename)
{
    FILE* input = fopen(inputFilename, "rb");
    if(input == NULL){
        printf("can not open file: %s\n", inputFilename);
        return 0;
    }
    HuffmanHeader header;

    if(!readHeader(input, &header))
    {
        printf("fail to read file\n");
        fclose(input);
        return 0;
    }

    if(header.originalSize == 0)
    {
        FILE* output = fopen(outputFilename, "wb");
        if(output == NULL)
        {
            printf("can not create file: %s", outputFilename);
            fclose(input);
            return 0;
        }

        fclose(input);
        fclose(output);

        printf("decompressed: %s -> %s\n", inputFilename, outputFilename);


        return 1;
    }

    HuffmanNode* root = buildHuffmanTree(header.frequency);
    if(root == NULL)
    {
        printf("can not build huffmanTree\n");
        fclose(input);

        return 0;
    }
    FILE* output = fopen(outputFilename, "wb");
    if(output == NULL)
    {
        printf("can not create outputFile: %s\n", outputFilename);

        freeHuffmanTree(root);
        fclose(input);

        return 0;
    }
    int success = decodeData(input, root, header.originalSize, output);


    if(!success)
    {
        printf("fail to decompress\n");
        fclose(input);
        fclose(output);
        freeHuffmanTree(root);

        return 0;
    }

    fclose(input);
    fclose(output);
    freeHuffmanTree(root);

    printf("decompressed: %s -> %s\n", inputFilename, outputFilename);

    return 1;
}
