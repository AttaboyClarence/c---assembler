#include "traditional_second_pass.h"
#include "../file.h"

static int patchOperand(char *arg, int position, AssemStrct *assem)
{
    Symbol *symbol;
    MachineCode *word;
    if (arg[0] == '#' || isRegister(arg) >= 0)
        return 1;
    /* Relative and external references are outside this benchmark's scope. */
    if (arg[0] == '%')
        return 0;
    symbol = assem->symbolTable[hashFunct(arg)];
    while (symbol != NULL && strcmp(symbol->name, arg) != 0)
        symbol = symbol->next;
    if (symbol == NULL || symbol->value == PRE_ENTRY ||
        symbol->attribute[EXTERNAL])
        return 0;
    word = &assem->instructionArray.item[position];
    fixConversion(word, symbol->value);
    word->ARE = 'R';
    return 1;
}

void traditionalSecondPass(const char *filename, AssemStrct *assem)
{
    FILE *fp = fopen(filename, "r");
    char line[MAX_LINE_LEN];
    char label[WORDSIZE];
    char mnemonic[WORDSIZE];
    char *rest, *end, *arg;
    const Operations *op;
    int ic = 100, lineNumber = 0, count;
    if (fp == NULL) {
        assem->ERROR = true;
        return;
    }
    while (getLine(fp, line)) {
        ++lineNumber;
        rest = skipspaces(line);
        if (endOfLineOrComment(rest))
            continue;
        if (isSymbol(rest, label))
            rest = getRestOfLine(rest);
        if (rest == NULL) {
            assem->ERROR = true;
            break;
        }
        getNextWord(rest, mnemonic);
        if (strcmp(mnemonic, ".data") == 0 ||
            strcmp(mnemonic, ".string") == 0 ||
            strcmp(mnemonic, ".entry") == 0 ||
            strcmp(mnemonic, ".extern") == 0)
            continue;
        op = isOp(assem, mnemonic, lineNumber);
        if (op == NULL || ic >= assem->IC) {
            assem->ERROR = true;
            break;
        }
        ++ic; /* One instruction word; every operand occupies one more word. */
        rest = getRestOfLine(rest);
        count = 0;
        while (rest != NULL && *rest != '\0') {
            arg = skipspaces(rest);
            end = arg;
            while (*end && *end != ',' && !isspace((unsigned char)*end))
                ++end;
            rest = end;
            if (*rest) {
                char separator = *rest;
                *rest++ = '\0';
                rest = skipspaces(rest);
                if (separator != ',' && *rest == ',')
                    rest = skipspaces(rest + 1);
            }
            if (*arg == '\0' || ++count > 2 || ic >= assem->IC ||
                !patchOperand(arg, ic, assem)) {
                assem->ERROR = true;
                break;
            }
            ++ic;
        }
        if (assem->ERROR)
            break;
    }
    if (ferror(fp) || ic != assem->IC)
        assem->ERROR = true;
    fclose(fp);
}
