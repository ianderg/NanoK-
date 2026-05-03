#include "keyboard.h"
#include "shell.h"
#include "kernel.h"

__attribute__((section(".multiboot")))
const unsigned int multiboot_header[] = {
    0x1BADB002,
    0x00,
    -(0x1BADB002)
};

char current_password[32] = "1234";

void update_cursor(int pos) {
    unsigned short p = pos / 2;
    __asm__ volatile ("outb %0, %1" : : "a"((unsigned char)0x0F), "Nd"((unsigned short)0x3D4));
    __asm__ volatile ("outb %0, %1" : : "a"((unsigned char)(p & 0xFF)), "Nd"((unsigned short)0x3D5));
    __asm__ volatile ("outb %0, %1" : : "a"((unsigned char)0x0E), "Nd"((unsigned short)0x3D4));
    __asm__ volatile ("outb %0, %1" : : "a"((unsigned char)((p >> 8) & 0xFF)), "Nd"((unsigned short)0x3D5));
}

void print(char* video, int* pos, const char* text, unsigned char color) {
    for (int i = 0; text[i] != '\0'; i++) {
        video[*pos] = text[i];
        video[*pos + 1] = color;
        *pos += 2;
    }
}

void newline(int* pos) {
    *pos = ((*pos / 160) + 1) * 160;
}

static int kstrcmp_k(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void read_password(char* video, int* pos, char* buf, int max_len) {
    int idx = 0;
    while (1) {
        char c = kbd_getchar();
        if (c > 0) {
            if (c == '\n') {
                buf[idx] = '\0';
                return;
            } else if (c == '\b') {
                if (idx > 0) {
                    idx--;
                    *pos -= 2;
                    video[*pos] = ' ';
                    video[*pos + 1] = 0x07;
                }
            } else if (idx < max_len - 1) {
                buf[idx++] = c;
                video[*pos] = '*';
                video[*pos + 1] = 0x07;
                *pos += 2;
            }
            update_cursor(*pos);
        }
    }
}

void _start() {
    char* video = (char*) 0xb8000;

    for (int i = 0; i < 80 * 25 * 2; i += 2) {
        video[i] = ' ';
        video[i+1] = 0x07;
    }

    int cursor_pos = 0;
    print(video, &cursor_pos, "RafaOS", 0x0F);
    newline(&cursor_pos);

    while (1) {
        print(video, &cursor_pos, "Password: ", 0x0E);
        update_cursor(cursor_pos);
        char entered[32];
        read_password(video, &cursor_pos, entered, 32);
        newline(&cursor_pos);
        if (kstrcmp_k(entered, current_password) == 0) {
            print(video, &cursor_pos, "Access granted.", 0x0A);
            newline(&cursor_pos);
            break;
        } else {
            print(video, &cursor_pos, "Wrong password.", 0x04);
            newline(&cursor_pos);
        }
    }

    char cmd_buffer[100];
    int cmd_idx = 0;

    print(video, &cursor_pos, "> ", 0x0A);
    update_cursor(cursor_pos);

    while (1) {
        char c = kbd_getchar();

        if (c > 0) {
            if (c == '\n') {
                cmd_buffer[cmd_idx] = '\0';
                interpretar_comando(cmd_buffer, video, &cursor_pos);
                cmd_idx = 0;

                if (cursor_pos > 0) {
                    newline(&cursor_pos);
                }
                print(video, &cursor_pos, "> ", 0x0A);
            }
            else if (c == '\b') {
                if (cmd_idx > 0) {
                    cmd_idx--;
                    cursor_pos -= 2;
                    video[cursor_pos] = ' ';
                    video[cursor_pos + 1] = 0x07;
                }
            }
            else {
                if (cmd_idx < 99) {
                    cmd_buffer[cmd_idx++] = c;
                    video[cursor_pos] = c;
                    video[cursor_pos + 1] = 0x0F;
                    cursor_pos += 2;
                }
            }
            update_cursor(cursor_pos);
        }
    }
}