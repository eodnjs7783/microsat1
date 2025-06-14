// uart.c
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdio.h>

#define UART_DEV "/dev/serial0"
#define BAUDRATE B115200

int uart_open(void) {
    int fd = open(UART_DEV, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0) {
        perror("UART open");
        return -1;
    }
    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        close(fd);
        return -1;
    }
    cfsetospeed(&tty, BAUDRATE);
    cfsetispeed(&tty, BAUDRATE);

    tty.c_cflag &= ~PARENB;            // 패리티 비트 없음
    tty.c_cflag &= ~CSTOPB;            // 스톱 비트 1개
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;                // 8비트 데이터
    tty.c_cflag |= CLOCAL | CREAD;     // 로컬 모드, 수신 활성화

    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // 논캐논 모드
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);         // 소프트웨어 흐름제어 비활성
    tty.c_oflag &= ~OPOST;                          // 출력 후처리 비활성

    // 변경사항 반영
    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        close(fd);
        return -1;
    }
    return fd;
}
