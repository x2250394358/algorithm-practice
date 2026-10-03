#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "huffman.h"
#include "bitio.h"


/*
 * 主函数：命令行入口，解析参数并分发到压缩/解压流程
 * 用法：
 *   压缩：huffman -c input.txt output.huf
 *   解压：huffman -d input.huf output.txt
 * 参数：
 *   argc - 参数个数（含程序名），应为 4
 *   args - 参数数组：
 *          args[0] 程序名
 *          args[1] 模式开关（"-c" 压缩 / "-d" 解压）
 *          args[2] 输入文件名
 *          args[3] 输出文件名
 * 返回：成功 0，用法错误或执行失败返回 1
 */
int main(int argc, char* args[])
{
   /* Windows 控制台默认 GBK，切换到 UTF-8 以正确显示中文输出 */
#ifdef _WIN32
   SetConsoleOutputCP(65001);
   SetConsoleCP(65001);
#endif
   /* 参数个数必须是 4（程序名 + 模式 + 输入 + 输出），否则打印用法 */
   if(argc != 4)
   {
    printf("用法：\n");
    printf("压缩：huffman -c input.txt output.huf\n");
    printf("解压：huffman -d input.huf output.txt\n");
    return 1;
   }
   /* 根据第二个参数选择压缩还是解压 */
   if(strcmp(args[1], "-c") == 0)
   {
        return compressFile(args[2], args[3]);
   }
   else if(strcmp(args[1], "-d") == 0)
   {
        return decompressFile(args[2], args[3]);
   }
   else
   {
    
    return 1;
   }

    return 0;
}
