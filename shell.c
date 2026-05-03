#include "shell.h"
#include "kernel.h"
#include "keyboard.h"

int kstrcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int kstrncmp(const char* a, const char* b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] != b[i]) return a[i] - b[i];
    }
    return 0;
}

static void kstrcpy(char* dst, const char* src, int max) {
    int i = 0;
    while (src[i] && i < max - 1) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

void shell_print(const char* str, char* video, int* cursor_pos) {
    for (int i = 0; str[i] != '\0'; i++) {
        video[*cursor_pos] = str[i];
        video[*cursor_pos + 1] = 0x07;
        *cursor_pos += 2;
    }
}

static void shell_read_pass(char* video, int* pos, char* buf, int max) {
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
            } else if (idx < max - 1) {
                buf[idx++] = c;
                video[*pos] = '*';
                video[*pos + 1] = 0x07;
                *pos += 2;
            }
            update_cursor(*pos);
        }
    }
}

static int check_password(char* video, int* pos) {
    shell_print("Current password: ", video, pos);
    update_cursor(*pos);
    char entered[32];
    shell_read_pass(video, pos, entered, 32);
    *pos = ((*pos / 160) + 1) * 160;
    if (kstrcmp(entered, current_password) == 0) return 1;
    shell_print("Wrong password.", video, pos);
    return 0;
}

void interpretar_comando(char* input, char* video, int* cursor_pos) {
    *cursor_pos = ((*cursor_pos / 160) + 1) * 160;

    if (kstrcmp(input, "help") == 0) {
        shell_print("Commands: help, version, clear, info, whoami, echo, passwd, reboot", video, cursor_pos);
    }
    else if (kstrcmp(input, "version") == 0) {
        shell_print("RafaOS Kernel v1.0", video, cursor_pos);
    }
    else if (kstrcmp(input, "whoami") == 0) {
        shell_print("sudo", video, cursor_pos);
    }
    else if (kstrcmp(input, "info") == 0) {
        shell_print("Made by @ianderg, @binbash_0", video, cursor_pos);
    }
    else if (kstrncmp(input, "echo ", 5) == 0) {
        shell_print(input + 5, video, cursor_pos);
    }
    else if (kstrcmp(input, "passwd") == 0) {
        if (!check_password(video, cursor_pos)) return;

        shell_print("New password: ", video, cursor_pos);
        update_cursor(*cursor_pos);
        char new1[32];
        shell_read_pass(video, cursor_pos, new1, 32);
        *cursor_pos = ((*cursor_pos / 160) + 1) * 160;

        shell_print("Confirm password: ", video, cursor_pos);
        update_cursor(*cursor_pos);
        char new2[32];
        shell_read_pass(video, cursor_pos, new2, 32);
        *cursor_pos = ((*cursor_pos / 160) + 1) * 160;

        if (kstrcmp(new1, new2) == 0) {
            kstrcpy(current_password, new1, 32);
            shell_print("Password changed.", video, cursor_pos);
        } else {
            shell_print("Passwords do not match.", video, cursor_pos);
        }
    }
    else if (kstrcmp(input, "reboot") == 0) {
        if (!check_password(video, cursor_pos)) return;
        __asm__ volatile ("outb %0, %1" : : "a"((unsigned char)0xFE), "Nd"((unsigned short)0x64));
    }
    else if (kstrcmp(input, "clear") == 0) {
        for (int i = 0; i < 80 * 25 * 2; i += 2) {
            video[i] = ' ';
            video[i+1] = 0x07;
        }
        *cursor_pos = 0;
    }
    else if (input[0] != '\0') {
        shell_print("Command not found, type 'help'.", video, cursor_pos);
    }
}