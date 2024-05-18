#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <malloc.h>
#include <string.h>
#include <ctype.h>

#define TOKEN_NUM 42
#define BUFFER_SIZE 1024

// 常数表
typedef struct Constant {
    int id;
    int value;
    struct Constant *next; 
} Constant;
// 标识符表
typedef struct Identifier {
    int id;
    char *value;
    struct Identifier *next; 
} Identifier;
// 字符常量表
typedef struct Character {
    int id;
    char value;
    struct Character *next; 
} Character;
// 字符串字面量表
typedef struct String {
    int id;
    char *value;
    struct String *next; 
} String;
// 单词表
char TokenList[][TOKEN_NUM] = {
    "void", "int", "char",
    "if", "else", "while", "for", "main",
    "break", "continue", "return", "NULL", "", "", "", "",
    ";", ",", "(", ")", "[", "]", "{", "}", "+",
    "-", "*", "/", "%", "=", "<", ">", "!", "+=", "-=",
    "*=", "/=", "%=", "==", "<=", ">=", "!="
};

// 词法分析函数声明
void lexer(FILE *, FILE *, FILE *);
// 预处理函数声明
void preprocessing(FILE *, FILE *);

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <SourceFile>\n", argv[0]);
        return 1;
    }
    char mid_file[256] = "mid_", result_file[256] = "result_", table_file[256] = "table_";
    strcat(mid_file, argv[1]);
    strcat(result_file, argv[1]);
    strcat(table_file, argv[1]);
    // 预处理
    FILE *fp_r_source = fopen(argv[1], "r");
    if (!fp_r_source) {
        printf("Failed to open %s.\n", argv[1]);
        return 1;
    }
    FILE *fp_w_mid = fopen(mid_file, "w");
    if (!fp_w_mid) {
        printf("Failed to open %s.\n", mid_file);
        fclose(fp_r_source);
        return 1;
    }
    preprocessing(fp_r_source, fp_w_mid);
    printf("Preprocessing complete.\n");
    fclose(fp_r_source);
    fclose(fp_w_mid);
    // 词法分析
    FILE *fp_r_mid = fopen(mid_file, "r");
    if (!fp_r_mid) {
        printf("Failed to open %s.\n", mid_file);
        return 1;
    }
    FILE *fp_w_result = fopen(result_file, "w");
    if (!fp_w_result) {
        printf("Failed to open %s.\n", result_file);
        fclose(fp_r_mid);
        return 1;
    }
    FILE *fp_w_table = fopen(table_file, "w");
    if (!fp_w_table) {
        printf("Failed to open %s.\n", table_file);
        fclose(fp_r_mid);
        fclose(fp_w_result);
        return 1;
    }
    lexer(fp_r_mid, fp_w_result, fp_w_table);
    printf("Lexical analysis complete.\n");
    return 0;
}

// 预处理函数
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

// 词法分析函数
void lexer(FILE *fp, FILE *out_fp, FILE *out_fp_table) {
    // 初始化各链表
    Constant *head_constant = (Constant *)malloc(sizeof(Constant));
    head_constant->id = -1;
    head_constant->value = 0;
    head_constant->next = NULL;
    Character *head_character = (Character *)malloc(sizeof(Character));
    head_character->id = -1;
    head_character->value = '\0';
    head_character->next = NULL;
    String *head_string = (String *)malloc(sizeof(String));
    head_string->id = -1;
    head_string->value = NULL;
    head_string->next = NULL;
    Identifier *head_identifier = (Identifier *)malloc(sizeof(Identifier));
    head_identifier->id = -1;
    head_identifier->value = NULL;
    head_identifier->next = NULL;

    char buffer[BUFFER_SIZE+1];
    int line = 1; // 当前行数
    bool end_flag = false; // 结束标志

    while (fgets(buffer, BUFFER_SIZE, fp) != NULL) {
        int i = 0;
        while (buffer[i]) {
            // 跳过空格，处理换行
            if (isspace(buffer[i])) {
                if (buffer[i] == '\n')
                    line++;
                i++;
                continue;
            }
            // 常数处理
            if (buffer[i] <= '9' && buffer[i] >= '0') {
                int tmp = buffer[i++] - '0';
                // 记录并计算该常数值，直到第一个非数字字符
                while (buffer[i] <= '9' && buffer[i] >= '0')
                    tmp = tmp * 10 + (buffer[i++] - '0');
                // 查询常数表
                Constant *p = head_constant;
                while (p->next) {
                    if (p->next->value == tmp) {
                        fprintf(out_fp, "<13, Constant, %d>\n", p->next->id);
                        break;
                    }
                    p = p->next;
                }
                // 未在常数表中查询到结果，在表末新建该常数记录
                if (p->next == NULL) {
                    Constant *q = (Constant *)malloc(sizeof(Constant));
                    q->id = p->id + 1;
                    q->value = tmp;
                    q->next = NULL;
                    p->next = q;
                    fprintf(out_fp, "<13, Constant, %d>\n", p->next->id);
                }
            } 
            // 字符常量处理
            else if (buffer[i] == '\'') {
                i++;
                // 判断是否为正常的字符常量
                if (buffer[i] && buffer[i+1] == '\'') { 
                    char ch = buffer[i];
                    // 查询字符常量表
                    Character *p = head_character;
                    while (p->next) {
                        if (p->next->value == ch) {
                            fprintf(out_fp, "<15, Constant, %d>\n", p->next->id);
                            break;
                        }
                        p = p->next;
                    }
                    // 未在字符常量表中查询到结果，在表末新建该字符常量记录
                    if (p->next == NULL) {
                        Character *q = (Character *)malloc(sizeof(Character));
                        q->id = p->id + 1;
                        q->value = ch;
                        q->next = NULL;
                        p->next = q;
                        fprintf(out_fp, "<15, Constant, %d>\n", p->next->id);
                    }
                    i = i + 2;
                } 
                // 字符常量的错误记录
                else {
                    fprintf(out_fp, "--------------------Character error in line %d.\n", line);
                    // 直到下一个空格或结尾
                    while (buffer[i] && !isspace(buffer[i]))
                        i++;
                }
            } 
            // 字符串字面量处理
            else if (buffer[i] == '\"') {
                i++;
                int len = 0;
                while (buffer[i + len] && buffer[i + len] != '\"')
                    len++;
                // 判断字符串是否正确闭合
                if (buffer[i + len]) {
                    // 记录字符串字面量
                    char *pc = (char *)malloc(sizeof(char)*(len+1));
                    for (int j = 0; j < len; j++)
                        pc[j] = buffer[i + j];
                    pc[len] = '\0';
                    // 查询字符串字面量表
                    String *p = head_string;
                    while (p->next) {
                        if (!strcmp(p->next->value, pc)) {
                            fprintf(out_fp, "<14, Constant, %d>\n", p->next->id);
                            free(pc);
                            break;
                        }
                        p = p->next;
                    }
                    // 未在字符串字面量表中查询到结果，在表末新建记录
                    if (p->next == NULL) {
                        String *q = (String *)malloc(sizeof(String));
                        q->id = p->id + 1;
                        q->next = NULL;
                        q->value = pc;
                        p->next = q;
                        fprintf(out_fp, "<14, Constant, %d>\n", p->next->id);
                    }
                    i = i + len + 1;
                }
                // 字符串未正确闭合的错误记录
                else {
                    fprintf(out_fp, "--------------------String error in line %d.\n", line);
                    i = i + len;
                }
            } 
            // 标识符处理
            else if ((buffer[i] <= 'Z' && buffer[i] >= 'A') || (buffer[i] <= 'z' && buffer[i] >= 'a') || (buffer[i] == '_')) {
                int len = 1;
                while ((buffer[i+len] <= 'Z' && buffer[i+len] >= 'A') || (buffer[i+len] <= 'z' && buffer[i+len] >= 'a') || (buffer[i+len] == '_') 
                || (buffer[i+len] <= '9' && buffer[i+len] >= '0'))
                    len++;
                // 记录标识符
                char *pc = (char *)malloc(sizeof(char)*(len+1));
                for (int j = 0; j < len; j++)
                    pc[j] = buffer[i+j];
                pc[len] = '\0';
                // 查询是否为关键字
                int idx_kevword;
                for (idx_kevword = 0; idx_kevword <= 11; idx_kevword++) {
                    if (!strcmp(TokenList[idx_kevword], pc)) {
                        fprintf(out_fp, "<%d, Keyword, %s>\n", idx_kevword, TokenList[idx_kevword]);
                        free(pc);
                        break;
                    }
                }
                if (idx_kevword <= 11) {
                    i = i + len;
                    continue;
                }
                // 查询标识符表
                Identifier *p = head_identifier;
                while (p->next) {
                    if (!strcmp(p->next->value, pc)) {
                        fprintf(out_fp, "<12, Identifier, %d>\n", p->next->id);
                        free(pc);
                        break;
                    }
                    p = p->next;
                }
                // 未在标识符表中查询到结果，在表末新建记录
                if (p->next == NULL) {
                    Identifier *q = (Identifier *)malloc(sizeof(Identifier));
                    q->next = NULL;
                    q->id = p->id + 1;
                    q->value = pc;
                    p->next = q;
                    fprintf(out_fp, "<12, Identifier, %d>\n", p->next->id);
                }
                i = i + len;
            } 
            // 符号处理
            else { 
                // 查询是否为界限符
                char ch = buffer[i];
                int idx_delimiter;
                for (idx_delimiter = 16; idx_delimiter <= 23; idx_delimiter++) {
                    if (TokenList[idx_delimiter][0] == ch) {
                        fprintf(out_fp, "<%d, Delimiter, %s>\n", idx_delimiter, TokenList[idx_delimiter]);
                        break;
                    }
                }
                if (idx_delimiter <= 23) {
                    i++;
                    continue;
                }
                // 查询是否为运算符
                int idx_operator;
                int both_flag = 0;
                for (idx_operator = 24; idx_operator <= 32; idx_operator++) {
                    if (TokenList[idx_operator][0] == ch) {
                        if (buffer[i+1] == '=') {
                            fprintf(out_fp, "<%d, Operator, %s>\n", idx_operator+9, TokenList[idx_operator+9]);
                            both_flag = 1;
                        } else
                            fprintf(out_fp, "<%d, Operator, %s>\n", idx_operator, TokenList[idx_operator]);
                        break;
                    }
                }
                if (idx_operator <= 32) {
                    i = i + 1 + both_flag;
                    continue;
                }
                // 查询是否为结束符号
                if (ch == '#') {
                    end_flag = true;
                    break;
                }
                // 未定义符号的错误记录
                fprintf(out_fp, "--------------------Undefined error in line %d.\n", line);
                i++;
                while (buffer[i] && !isspace(buffer[i]))
                    i++;
            }
        }
        if (end_flag)
            break;
    }

    // 打印常数表，字符常量表，字符串表，标识符表，同时销毁
    // 打印标识符表
    fprintf(out_fp_table, "<Identifier>\n-id, value-\n");
    Identifier *p_ide = head_identifier->next, *q_ide = NULL;
    while (p_ide) {
        q_ide = p_ide;
        p_ide = p_ide->next;
        q_ide->next = NULL;
        fprintf(out_fp_table, "%d, %s\n", q_ide->id, q_ide->value);
        free(q_ide->value);
        free(q_ide);
        q_ide = NULL;
    }
    free(head_identifier);
    fprintf(out_fp_table, "\n");
    // 打印字符常量表
    fprintf(out_fp_table, "<Constant-Character>\n-id, value-\n");
    Character *p_cha = head_character->next, *q_cha = NULL;
    while (p_cha) {
        q_cha = p_cha;
        p_cha = p_cha->next;
        q_cha->next = NULL;
        fprintf(out_fp_table, "%d, %c\n", q_cha->id, q_cha->value);
        free(q_cha);
        q_cha = NULL;
    }
    free(head_character);
    fprintf(out_fp_table, "\n");
    // 打印字符串表
    fprintf(out_fp_table, "<Constant-String>\n-id, value-\n");
    String *p_str = head_string->next, *q_str = NULL;
    while (p_str) {
        q_str = p_str;
        p_str = p_str->next;
        q_str->next = NULL;
        fprintf(out_fp_table, "%d, %s\n", q_str->id, q_str->value);
        free(q_str->value);
        free(q_str);
        q_str = NULL;
    }
    free(head_string);
    fprintf(out_fp_table, "\n");
    // 打印常数表
    fprintf(out_fp_table, "<Constant-Constant>\n-id, value-\n");
    Constant *p_con = head_constant->next, *q_con = NULL;
    while (p_con) {
        q_con = p_con;
        p_con = p_con->next;
        q_con->next = NULL;
        fprintf(out_fp_table, "%d, %d\n", q_con->id, q_con->value);
        free(q_con);
        q_con = NULL;
    }
    free(head_constant);
    fprintf(out_fp_table, "\n");
}
