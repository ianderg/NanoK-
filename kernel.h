#ifndef KERNEL_H
#define KERNEL_H

extern char current_password[32];

void print(char* video, int* pos, const char* text, unsigned char color);
void newline(int* pos);
void update_cursor(int pos);
void read_password(char* video, int* pos, char* buf, int max_len);

#endif