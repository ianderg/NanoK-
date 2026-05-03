static inline unsigned char inb(unsigned short port) {
    unsigned char ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static int shift_pressed = 0;
static int caps_lock = 0;

unsigned char kbd_map[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
    'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
};

unsigned char kbd_map_shift[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|',
    'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' '
};

unsigned char kbd_getchar() {
    if (!(inb(0x64) & 0x01))
        return 0;

    unsigned char scancode = inb(0x60);

    if (scancode == 0x2A || scancode == 0x36) {
        shift_pressed = 1;
        return 0;
    }
    if (scancode == 0xAA || scancode == 0xB6) {
        shift_pressed = 0;
        return 0;
    }
    if (scancode == 0x3A) {
        caps_lock = !caps_lock;
        return 0;
    }

    if (scancode & 0x80)
        return 0;

    if (scancode >= 128)
        return 0;

    unsigned char c = kbd_map[scancode];

    if (caps_lock && c >= 'a' && c <= 'z')
        return kbd_map_shift[scancode];

    if (shift_pressed)
        return kbd_map_shift[scancode];

    return c;
}