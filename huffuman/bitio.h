#ifndef BITIO_H        /* 防止头文件被重复包含 */
#define BITIO_H

#include <stdio.h>

/*
 * 位级写入器结构体
 * 由于哈夫曼编码是按"位"(bit)进行压缩的，而标准库的 fwrite/fputc
 * 只能按整字节写入，因此需要自己维护一个缓冲区，先把位攒够 8 个
 * 再作为一个字节写出。
 * 字段含义：
 *   file     - 目标输出文件指针（按二进制写模式打开）
 *   buffer   - 正在累积的字节缓冲区（低位逐位移入）
 *   bitCount - 缓冲区中已累积的位数（0 ~ 7，满 8 时触发写出）
 */
typedef struct {
    FILE* file;
    unsigned char buffer;
    int bitCount;
}BitWriter;

/*
 * 位级读取器结构体
 * 与 BitWriter 对称，用于按位读取压缩文件中的数据。
 * 每次从文件中读入一个字节到 buffer，然后逐位向外取出。
 * 字段含义：
 *   file     - 源输入文件指针（按二进制读模式打开）
 *   buffer   - 当前缓冲的字节数据
 *   bitCount - 缓冲字节中尚未取出的位数（8 表示已装好一个新字节）
 */
typedef struct {
    FILE* file;
    unsigned char buffer;
    int bitCount;
}BitReader;

/* 初始化位写入器：关联文件，清空缓冲区和位计数 */
void bitWriterInit(BitWriter* writer, FILE* file);
/* 初始化位读取器：关联文件，清空缓冲区和位计数 */
void bitReaderInit(BitReader* reader, FILE * file);
/* 向写入器写入一个二进制位（bit 为 0 或 1） */
void writeBit(BitWriter* writer, int bit);
/* 冲刷写入器：把缓冲区中剩余的不足一字节的位补零后写出 */
void flushBits(BitWriter* writer);
/* 从读取器读出一个二进制位，返回 0 或 1；文件读完时返回 -1 */
int readBit(BitReader* reader);


#endif
