#define NULL ((void*)0)
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define isdigit(c) (c >= '0') && (c <= '9')
#define tokget(cmp, tok, i) do { tok[++i] = str[; } while (cmp); ++i;

extern int printf(const char* fmt, ...);

static char* strcpy(char* dst, const char* src)
{
    while ((*dst++ = *src++));
    return --dst;
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

static char* op2(char* dst, const char* op)
{
    static long labelcount = 1;
    switch(*op) {
        case '+': 
            dst = strcpy(dst, "\tpop %rdx\n\tpop %rax\n\tadd %rdx, %rax\n");
            break;
        case '-': 
            dst = strcpy(dst, "\tpop %rdx\n\tpop %rax\n\tsub %rdx, %rax\n");
            break;
        case '/': 
            dst = strcpy(dst, "\tpop %rcx\n\tpop %rax\n\txor %rdx, %rdx\n\tidiv %rcx\n");
            break;
        case '*': 
            dst = strcpy(dst, "\tpop %rdx\n\tpop %rax\n\timul %rdx, %rax\n");
            break;
        case '%': 
            dst = strcpy(dst, "\tpop %rcx\n\tpop %rax\n\txor %rdx, %rdx\n\tidiv %rcx\n");
            break;
        case '^':
            dst = strcpy(dst, "\tpop %rdx\n\tpop %rax\n\txor %rdx, %rax\n");
            break;
        case '=':
            dst = strcpy(dst, "\tpop %rdx\n\tpop %rcx\n\txor %rax, %rax\n");
            dst = strcpy(dst, "\tcmp %rdx, %rcx\n\tsete %al\n");
            break;
        case '!':
            dst = strcpy(dst, "\tpop %rdx\n\tpop %rcx\n\txor %rax, %rax\n");
            dst = strcpy(dst, "\tcmp %rdx, %rcx\n\tsetne %al\n");
            break;
        case '>':
            if (*op != op[1]) { 
                dst = strcpy(dst, "\tpop %rdx\n\tpop %rcx\n\txor %rax, %rax\n");
                dst = strcpy(dst, "\tcmp %rdx, %rcx\n\tsetg");
                *dst++ = 'e' * (op[1] == '=');
                dst = strcpy(dst, " %al\n");
            }
            else dst = strcpy(dst, "\tpop %rcx\n\tpop %rax\n\tshr %cl, %rax\n");
            break;
        case '<':
            if (*op != op[1]) { 
                dst = strcpy(dst, "\tpop %rdx\n\tpop %rcx\n\txor %rax, %rax\n");
                dst = strcpy(dst, "\tcmp %rdx, %rcx\n\tsetl");
                *dst++ = 'e' * (op[1] == '=');
                dst = strcpy(dst, " %al\n");
            }
            else dst = strcpy(dst, "\tpop %rcx\n\tpop %rax\n\tshl %cl, %rax\n");
            break;
        case '&':
            if (*op == op[1]) {
                dst = strcpy(dst,"\tpop %rdx\n\tpop %rcx\n\txor %rax, %rax\n");
                dst = strcpy(dst, "\ttest %rcx, %rcx\n\tje L");
                *dst++ = labelcount + '0';
                dst = strcpy(dst, "\n\ttest %rdx, %rdx\n\tje L");
                *dst++ = labelcount + '0';
                dst = strcpy(dst, "\n\tinc %rax\nL");
                *dst++ = labelcount + '0';
                dst = strcpy(dst, ":\n");
                ++labelcount;
            }
            else dst = strcpy(dst, "\tpop %rdx\n\tpop %rax\n\tand %rdx, %rax\n");
            break;
        case '|':
            if (*op == op[1]) {
                dst = strcpy(dst, "\tpop %rdx\n\tpop %rcx\n\tmov $1, %rax\n");
                dst =strcpy(dst, "\ttest %rcx, %rcx\n\tjne L");
                *dst++ = labelcount + '0';
                dst = strcpy(dst, "\n\ttest %rdx, %rdx\n\tjne L");
                *dst++ = labelcount + '0';
                dst = strcpy(dst, "\n\tdec %rax\nL");
                *dst++ = labelcount + '0';
                dst = strcpy(dst, ":\n");
                ++labelcount;
            }
            else dst = strcpy(dst, "\tpop %rdx\n\tpop %rax\n\tor %rdx, %rax\n");
            break;
    }
    dst = strcpy(dst, "\tpush %r");
    *dst++ = 'a' + 3 * (*op == '%');
    return strcpy(dst, "x\n");
}

static int compile(char* dst, const char* src)
{
    long stackcount = 0, i = 0, j;
    char* tok = lex(src, &i), stack[0xff][4];
    
    dst = strcpy(dst, ".section __TEXT, __text\n\t.globl _main\n_main:\n");
    
    while (tok) {
        switch (*tok) {
            case 0:
                break;
            case '0' ... '9':
                dst = strcpy(dst, "\tmov $");
                dst = strcpy(dst, tok);
                dst = strcpy(dst, ", %rax\n\tpush %rax\n");
                break;
            case '(':
                strcpy(stack[stackcount++], tok);
                break;
            case ')':
                while (stackcount && *stack[stackcount - 1] != '(') {
                    dst = op2(dst, stack[--stackcount]);
                }
                stackcount = stackcount ? stackcount - 1: stackcount;
                break;
            default:
                /*printf("# '%s' -> ", tok);
                for (j = 0; j < stackcount; ++j) {
                    printf("'%s' ", stack[j]);    
                }
                printf("\n");*/
                while (stackcount && *stack[stackcount - 1] != '(' && 
                    oppres(tok) >= oppres(stack[stackcount - 1])) {
                    dst = op2(dst, stack[--stackcount]);
                }
                strcpy(stack[stackcount++], tok);
        }
        tok = lex(src, &i);
    }

    while (stackcount) {
        dst = op2(dst, stack[--stackcount]);
    }

    dst = strcpy(dst, "\tpop %rax\n\tret\n");

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
