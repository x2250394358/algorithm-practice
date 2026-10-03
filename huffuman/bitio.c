#include "bitio.h"

/*
 * 初始化位写入器
 * 把 writer 与给定的输出文件绑定，并清空缓冲区与位计数。
 * 调用后即可通过 writeBit 逐位写入。
 */
void bitWriterInit(BitWriter* writer, FILE* file){
    writer->file = file;
    writer->buffer = 0;
    writer->bitCount = 0;
}

/*
 * 初始化位读取器
 * 把 reader 与给定的输入文件绑定，并清空缓冲区与位计数。
 * 调用后即可通过 readBit 逐位读取。
 */
void bitReaderInit(BitReader* reader, FILE* file)
{
    reader->file = file;
    reader->buffer = 0;
    reader->bitCount = 0;
}

/*
 * 写入一个二进制位
 * 步骤：
 *   1. 把当前 buffer 整体左移 1 位，为新的位腾出最低位空间；
 *   2. 若待写入的位为 1，则把最低位置 1；
 *   3. bitCount 加 1；
 *   4. 当攒满 8 位时，把整个字节写进文件，并复位缓冲区和计数。
 * 这样多个"位"就被打包成一个"字节"，实现按位压缩存储。
 */
void writeBit(BitWriter* writer, int bit){
    writer->buffer <<= 1;
    if(bit == 1)
        writer->buffer |= 1;
    writer->bitCount++;
    if(writer->bitCount == 8)
    {
        fputc(writer->buffer, writer->file);
        writer->bitCount = 0;
        writer->buffer = 0;
    }
}

/*
 * 冲刷位写入器
 * 压缩数据的总位数往往不是 8 的整数倍，最后会剩下 1 ~ 7 个位。
 * 这里把这些剩余位左移到字节高位（低位自动补 0），作为一个字节写出，
 * 确保最后一个字节也不会丢失。解压时靠"原始字节数"来决定读到何时停止，
 * 末尾补的 0 位会被忽略，因此不会造成多余字符。
 */
void flushBits(BitWriter* writer)
{
    if(writer->bitCount > 0)
    {
        writer->buffer <<= (8 - writer->bitCount);
        fputc(writer->buffer, writer->file);
    }
    writer->buffer = 0;
    writer->bitCount = 0;
}

/*
 * 读出一个二进制位
 * 步骤：
 *   1. 若缓冲区已空（bitCount == 0），则从文件读入一个新字节，读失败
 *      （遇到 EOF）返回 -1 表示数据耗尽；
 *   2. 取 buffer 的最高位作为当前位（右移 7 位后与 1 相与）；
 *   3. buffer 左移 1 位，去掉已取出的最高位，bitCount 减 1。
 * 返回 0 或 1；文件提前结束返回 -1。
 */
int readBit(BitReader* reader)
{
    if(reader->bitCount == 0)
    {
        int value = fgetc(reader->file);
        if(value == EOF)
            return -1;
        reader->buffer = (unsigned char)value;
        reader->bitCount = 8;
    }
    int bit = (reader->buffer >> 7) & 1;
    reader->buffer <<= 1;
    reader->bitCount--;

    return bit;
}
