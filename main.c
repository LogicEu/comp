#define NULL ((void*)0)
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define isdigit(c) (c >= '0') && (c <= '9')
#define tokget(cmp, tok, i) do { tok[++i] = str[; } while (cmp); ++i;

extern int printf(const char* fmt, ...);

static long strlen(const char* str)
{
    long i = 0;
    while (str[i]) { ++i; }
    return i;
}

static char* strcpy(char* dst, const char* src)
{
    while ((*dst++ = *src++));
    return dst;
}

static char* strcat(char* dst, const char* src)
{
    while (*dst) { ++dst; }
    while ((*dst++ = *src++));
    return dst;
}

static char* lex(const char* str, long* iter)
{
    static char token[0xff];
    long i = *iter, j = 0;
    switch(str[i]) {
        case 0: 
            return NULL;
        case ' ':
        case '\n':
        case '\t':
        case '\r':
            ++(*iter);
            return lex(str, iter);
        case '0' ... '9':
            while (isdigit(str[i + j])) {
                token[j] = str[i + j];
                ++j;
            }
            break;
        case '(':
        case ')':
            token[j++] = str[i];
            break;
        case '!':
        case '=':
            token[j++] = str[i];
            token[j] = '=' * (str[i + 1] == '=');
            j += !!token[j];
            break;
        default:
            token[j++] = str[i];
            if (str[i + 1] == str[i] || str[i + 1] == '=') {        
                token[j++] = str[i + 1];
            }
    }
    token[j] = 0;
    *iter += j;
    return token;
}

static int oppres(const char* op)
{
    switch (op[0]) {
        case ')':
        case '(': return 0;
        case '!': return 3 + 7 * (op[1] == '=');
        case '*':
        case '/':
        case '%': return 5;
        case '+':
        case '-': return 3 + 3 * (op[1] != op[0]);
        case '<':
        case '>': return 9 - 2 * (op[1] == op[0]);
        case '=': return 10;
        case '^': return 12;
        case '&': return 11 + 3 * (op[1] == op[0]);
        case '|': return 13 + 2 * (op[1] == op[0]);
    }
    return 16;
}

static void op2(char* dst, const char* op)
{
    static long labelcount = 1;
    switch(*op) {
        case '+': 
            strcat(dst, "\tpop %rdx\n\tpop %rax\n\tadd %rdx, %rax\n");
            break;
        case '-': 
            strcat(dst, "\tpop %rdx\n\tpop %rax\n\tsub %rdx, %rax\n");
            break;
        case '/': 
            strcat(dst, "\tpop %rcx\n\tpop %rax\n\txor %rdx, %rdx\n\tidiv %rcx\n");
            break;
        case '*': 
            strcat(dst, "\tpop %rdx\n\tpop %rax\n\timul %rdx, %rax\n");
            break;
        case '%': 
            strcat(dst, "\tpop %rcx\n\tpop %rax\n\txor %rdx, %rdx\n\tidiv %rcx\n");
            break;
        case '^':
            strcat(dst, "\tpop %rdx\n\tpop %rax\n\txor %rdx, %rax\n");
            break;
        case '=':
            strcat(dst, "\tpop %rdx\n\tpop %rcx\n\txor %rax, %rax\n");
            strcat(dst, "\tcmp %rdx, %rcx\n\tsete %al\n");
            break;
        case '!':
            strcat(dst, "\tpop %rdx\n\tpop %rcx\n\txor %rax, %rax\n");
            strcat(dst, "\tcmp %rdx, %rcx\n\tsetne %al\n");
            break;
        case '>':
            if (*op == op[1]) { 
                strcat(dst, "\tpop %rcx\n\tpop %rax\n\tshr %cl, %rax\n");
            }
            else {
                strcat(dst, "\tpop %rdx\n\tpop %rcx\n\txor %rax, %rax\n");
                strcat(dst, "\tcmp %rdx, %rcx\n\tsetg");
                dst[strlen(dst)] = 'e' * (op[1] == '=');
                strcat(dst, " %al\n");
            }
            break;
        case '<':
            if (*op == op[1]) { 
                strcat(dst, "\tpop %rcx\n\tpop %rax\n\tshl %cl, %rax\n");
            }
            else {
                strcat(dst, "\tpop %rdx\n\tpop %rcx\n\txor %rax, %rax\n");
                strcat(dst, "\tcmp %rdx, %rcx\n\tsetl");
                dst[strlen(dst)] = 'e' * (op[1] == '=');
                strcat(dst, " %al\n");
            }
            break;
        case '&':
            if (*op == op[1]) {
                strcat(
                    dst,
                    "\tpop %rdx\n\tpop %rcx\n\txor %rax, %rax\n\ttest %rcx, %rcx\n\tje L"
                );
                dst[strlen(dst)] = labelcount + '0';
                strcat(dst, "\n\ttest %rdx, %rdx\n\tje L");
                dst[strlen(dst)] = labelcount + '0';
                strcat(dst, "\n\tinc %rax\nL");
                dst[strlen(dst)] = labelcount + '0';
                strcat(dst, ":\n");
                ++labelcount;
            }
            else strcat(dst, "\tpop %rdx\n\tpop %rax\n\tand %rdx, %rax\n");
            break;
        case '|':
            if (*op == op[1]) {
                strcat(
                    dst,
                    "\tpop %rdx\n\tpop %rcx\n\tmov $1, %rax\n\ttest %rcx, %rcx\n\tjne L"
                );
                dst[strlen(dst)] = labelcount + '0';
                strcat(dst, "\n\ttest %rdx, %rdx\n\tjne L");
                dst[strlen(dst)] = labelcount + '0';
                strcat(dst, "\n\tdec %rax\nL");
                dst[strlen(dst)] = labelcount + '0';
                strcat(dst, ":\n");
                ++labelcount;
            }
            else strcat(dst, "\tpop %rdx\n\tpop %rax\n\tor %rdx, %rax\n");
            break;
    }
    strcat(dst, "\tpush %r");
    dst[strlen(dst)] = *op == '%' ? 'd' : 'a';
    strcat(dst, "x\n");
}

static int compile(char* dst, const char* src)
{
    long stackcount = 0, i = 0, j;
    char* tok = lex(src, &i), stack[0xff][4];
    
    strcpy(dst, ".section __TEXT, __text\n\t.globl _main\n_main:\n");
    
    while (tok) {
        switch (*tok) {
            case 0:
                break;
            case '0' ... '9':
                strcat(dst, "\tmov $");
                strcat(dst, tok);
                strcat(dst, ", %rax\n\tpush %rax\n");
                break;
            case '(':
                strcpy(stack[stackcount++], tok);
                break;
            case ')':
                while (stackcount && *stack[stackcount - 1] != '(') {
                    op2(dst, stack[--stackcount]);
                }
                stackcount = stackcount ? stackcount - 1: stackcount;
                break;
            default:
                printf("# '%s' -> ", tok);
                for (j = 0; j < stackcount; ++j) {
                    printf("'%s' ", stack[j]);    
                }
                printf("\n");
                while (stackcount && *stack[stackcount - 1] != '(' && 
                    oppres(tok) >= oppres(stack[stackcount - 1])) {
                    op2(dst, stack[--stackcount]);
                }
                strcpy(stack[stackcount++], tok);
        }
        tok = lex(src, &i);
    }

    while (stackcount) {
        op2(dst, stack[--stackcount]);
    }

    strcat(dst, "\tpop %rax\n\tret\n");

    return EXIT_SUCCESS;
}

int main(const int argc, const char** argv)
{
    char buffer[0xffff];
    if (argc < 2) {
        printf("Usage:\n$ comp '1 + (2 - 3) * 4 / 5'\n");
        return EXIT_FAILURE;
    }
    
    if (compile(buffer, argv[1])) {
        printf("Compilation failed!\n");
        return EXIT_FAILURE;
    }

    printf("%s\n", buffer);
    return EXIT_SUCCESS;
}
