/**
 * Crack an unknown Caesar cypher
 * Usage: ./crack_caesar dict_file < encrypted_message
 * Will print out the decrypted message.
 *
 * This file is available at https://github.com/adamyi/caesar_cracker
 *
 * Copyright (c) 2018 Adam Yi <z5231521@cse.unsw.edu.au>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the crack_caesar project.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUFFER_SIZE 10005

/**
 * Here we use a linguistic tree implementation similar to the Trie Tree.
 *
 * A more obvious approach to this problem is to count the frequency of each
 * alphabet characters appeared in the dict file and the encrypted text and try
 * to map them together.
 *
 * However, this approach is potentially better, given that we have a huge dictionary.
 * Because for other languages instead of English, it could be possible that the
 * character frequency features might not be as significant as the one shown in
 * English.
 */

typedef struct Node {
    int c[26];
    struct Node *children[26];
} Node;

// return alphabet number (a->0, b->1, ...)
// if c is not an alphabet letter, return -1
int parseChar(int c) {
    if (c > 64 && c < 91) // capitalized
        return c - 65;
    if (c > 96 && c < 123) // lower-case
        return c - 97;
    return -1; // non-capitalized
}

Node *newNode() {
    Node *node = malloc(sizeof(Node));
    memset(node->c, 0, sizeof(node->c));
    memset(node->children, 0, sizeof(node->children));
    return node;
}

int calcOffset(int c, int pc) {
    if (c >= pc)
        return c - pc;
    return c - pc + 26;
}

Node *buildDictTree(FILE *fp) {
    Node *root = newNode();
    Node *p = root;
    int c = fgetc(fp);
    int pc = -1;
    while (c != EOF) {
        c = parseChar(c);
        if (c == -1) {
            pc = -1;
        } else {
            if (pc != -1) {
                if (p->children[calcOffset(c, pc)] == NULL) {
                    p->children[calcOffset(c, pc)] = newNode();
                }
                p = p->children[calcOffset(c, pc)];
                p->c[c] = 1;
            } else
                p = root;
            pc = c;
        }
        c = fgetc(fp);
    }
    return root;
}

char buf[MAX_BUFFER_SIZE];

int guessOffset(Node *root) {
    int offset[26];
    memset(offset, 0, sizeof(offset));
    int i = -1;
    Node *p = root;
    int pc = -1;
    int skip = 1;
    while (buf[++i] != '\0') {
        int c = parseChar(buf[i]);
        if (c == -1) {
            if (pc > -1) {
                for (int i = 0; i < 26; i++) {
                    offset[calcOffset(i, pc)] += p->c[i];
                }
                pc = -1;
            }
            skip = 0;
        } else {
            if (skip)
                continue;
            if (pc > -1) {
                if (p->children[calcOffset(c, pc)] == NULL) {
                    pc = -2;
                    skip = 1;
                    continue;
                } else
                    p = p->children[calcOffset(c, pc)];
            } else
                p = root;
            pc = c;
        }
    }

    int max = offset[0], maxo = 0;
    for (int i = 1; i < 26; i++) {
        // printf("%d %d\n", i, offset[i]);
        if (offset[i] > max) {
            max = offset[i];
            maxo = i;
        }
    }
    return maxo;
}

void printDecypher(int offset) {
    int i = -1;
    while(buf[++i] != '\0') {
        int c = parseChar(buf[i]);
        if (c == -1)
            printf("%c", buf[i]);
        else
            if (c + offset > 25)
                printf("%c", buf[i] + offset - 26);
            else
                printf("%c", buf[i] + offset);
    }
}

int main(int argc, const char *argv[]) {
    if (argc < 2) {
        perror("You need to supply a dict!\n");
        return 1;
    }
    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Error loading dict file.\n");
        return 2;
    }

    Node *tree = buildDictTree(file);

    fread(buf, 1, MAX_BUFFER_SIZE, stdin);
    buf[MAX_BUFFER_SIZE - 1] = '\0'; // avoid buffer overflow exploitation

    //int off = guessOffset(tree);
    //printf("offset: %d\n", off);

    printDecypher(guessOffset(tree));

    return 0;
}
