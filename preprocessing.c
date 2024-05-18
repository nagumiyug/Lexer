#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define BUFFER_SIZE 1024

// 移除注释，并处理多余空格

void preprocessing(FILE *fp, FILE *out_fp) {
    char buffer[BUFFER_SIZE+1];
    bool in_comment = false; // 在多行注释中
    bool change = false; // 多行注释有换行

    while (fgets(buffer, BUFFER_SIZE, fp) != NULL) {
        int i = 0;
        char lastc = '\0';
        bool in_string = false; // 在字符串中
        bool is_head = true; // 一行开头
        bool is_space = false; // 有空格

        while (buffer[i]) {
            // 不在字符串中，不在多行注释中
            if (!in_string && !in_comment) {
                // 接下来是字符串
                if (buffer[i] == '\"') {
                    if (is_space) {
                        fprintf(out_fp, " ");
                        is_space = false;
                    }
                    in_string = true;
                    lastc = buffer[i++];
                    fprintf(out_fp, "%c", lastc);
                }
                // 接下来是多行注释 
                else if (buffer[i] == '/' && buffer[i+1] == '*') {
                    in_comment = true;
                    i = i + 2;
                }
                // 正常时
                else {
                    // 判断是否为一行的开头
                    if (is_head) {
                        // 找到第一个不为空格的字符
                        while (buffer[i] && isspace(buffer[i]))
                            i++;
                        // 未找到，说明一行全为空格，直接换行
                        if (!buffer[i]) {
                            fprintf(out_fp, "\n");
                            break;
                        }
                        // 有第一个不为空格的符号，则会重新循环
                        else
                            is_head = false;
                        continue;
                    }
                    // 如果当前符号是空格（非换行）
                    if (isspace(buffer[i]) && buffer[i] != '\n') {
                        is_space = true;
                        // 找到后面第一个非空格或到结尾
                        while (buffer[i] && isspace(buffer[i]))
                            i++;
                        // 后面未出现非空格则直接换行
                        if (!buffer[i]) {
                            fprintf(out_fp, "\n");
                            break;
                        }
                        else // 后面出现非空格则重新判断
                            continue;
                    }
                    // 是换行符，能到这说明前面不可能是空格
                    if (buffer[i] == '\n') {
                        fprintf(out_fp, "\n");
                        // 结束该行的处理
                        break;
                    }
                    //是单行注释，无论前面是否是空格，都要直接换行
                    if (buffer[i] == '/' && buffer[i+1] == '/') {
                        fprintf(out_fp, "\n");
                        // 结束该行的处理
                        break;
                    }
                    if (is_space) {
                        fprintf(out_fp, " ");
                        is_space = false;
                    }
                    lastc = buffer[i++];
                    fprintf(out_fp, "%c", lastc);
                }
            }
            // 在字符串中
            else if (in_string) {
                lastc = buffer[i++];
                fprintf(out_fp, "%c", lastc);
                if (lastc == '\"')
                    in_string = false;
            }
            // 在多行注释中
            else if (in_comment) {
                if (buffer[i+1]) {
                    // 多行注释结束
                    if (buffer[i] == '*' && buffer[i+1] == '/') {
                        // 多行注释中有换行
                        if (change) {
                            in_comment = false;
                            change = false;
                            // 找到下一个非空格字符
                            i = i + 2;
                            while (buffer[i] && isspace(buffer[i]))
                                i++;
                            // 说明后面没有非空格字符
                            if (!buffer[i]) {
                                fprintf(out_fp, "\n");
                                break;
                            }
                            else
                                continue;
                        }
                        // 多行注释中无换行
                        else {
                            i = i + 2;
                            // 多行注释结束时到行末
                            if (!buffer[i] || buffer[i] == '\n') {
                                fprintf(out_fp, "\n");
                                in_comment = false;
                                change = false;
                                break;
                            }
                            // 多行注释结束时未到行末
                            else {
                                // 前有空格，后有空格，直接忽视该多行注释即可
                                if (is_space && isspace(buffer[i])) {
                                    in_comment = false;
                                    change = false;
                                    continue;
                                }
                            }
                        }
                    }
                    // 多行注释未结束
                    else
                        i++;
                }
                else {
                    fprintf(out_fp, "\n");
                    change = true;
                    break;
                }
            }
        }
    }
}

int main() {
    /*
    if (argc < 3) {
        printf("Usage: %s <InputFile> <OutputFile>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        printf("Failed to open input file.\n");
        return 1;
    }

    FILE *out_fp = fopen(argv[2], "w");
    if (!out_fp) {
        printf("Failed to open Output file.\n");
        fclose(fp);
        return 2;
    }
    */
    FILE *fp = fopen("ex3.txt", "r");
    if (!fp) {
        printf("Failed to open input file.\n");
        return 1;
    }

    FILE *out_fp = fopen("p3.txt", "w");
    if (!out_fp) {
        printf("Failed to open Output file.\n");
        fclose(fp);
        return 2;
    }
    preprocessing(fp, out_fp);

    fclose(fp);
    fclose(out_fp);

    printf("Preprocessing complete.\n");
    return 0;
}
