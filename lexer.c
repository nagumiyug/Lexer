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

// 词法分析
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

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <inputfile> <outputfile> <outputtablefile>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
        printf("Failed to open file.\n");
        return 1;
    }

    FILE *out_fp = fopen(argv[2], "w");
    if (!out_fp) {
        printf("Failed to open output file.\n");
        fclose(fp);
        return 2;
    }

    FILE *out_fp_table = fopen(argv[3], "w");
    if (!out_fp) {
        printf("Failed to open output file.\n");
        fclose(out_fp);
        fclose(fp);
        return 2;
    }

    lexer(fp, out_fp, out_fp_table);

    fclose(fp);
    fclose(out_fp);
    fclose(out_fp_table);
    return 0;
}
