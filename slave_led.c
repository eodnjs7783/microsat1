#include <stdio.h>          // 표준 입출력 함수
#include <string.h>         // 문자열 처리 함수
#include <unistd.h>         // POSIX 함수 (read, close 등)
#include <net/if.h>         // 네트워크 인터페이스 구조체
#include <sys/ioctl.h>      // ioctl 시스템 호출
#include <sys/socket.h>     // 소켓 함수
#include <linux/can.h>      // CAN 프레임 구조체
#include <linux/can/raw.h>  // RAW CAN 옵션
#include <wiringPi.h>       // GPIO 제어 라이브러리

#define LED_PIN 17         // BCM 모드에서 사용한 LED 핀 번호 (GPIO17)

int main(void) {
    int sockfd;                    // CAN 소켓 디스크립터
    struct sockaddr_can addr;      // CAN 주소 구조체
    struct ifreq ifr;              // 인터페이스 조회 구조체
    struct can_frame frame;        // 수신할 CAN 프레임 구조체

    // 1) GPIO 초기화 (WiringPi + BCM 모드)
    wiringPiSetupGpio();           // BCM 핀 넘버링 사용 설정
    pinMode(LED_PIN, OUTPUT);      // LED_PIN을 출력으로 설정
    digitalWrite(LED_PIN, LOW);    // LED 초기 상태 OFF

    // 2) CAN RAW 소켓 생성
    sockfd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (sockfd < 0) {
        perror("socket");          // 소켓 생성 실패 시
        return 1;                  // 비정상 종료
    }

    // 3) 인터페이스 인덱스 조회 ("can0")
    strcpy(ifr.ifr_name, "can0");   // ifr_name에 "can0" 복사
    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) < 0) {
        perror("ioctl SIOCGIFINDEX"); // ioctl 실패 시
        close(sockfd);                 // 소켓 닫기
        return 1;                      // 종료
    }

    // 4) 소켓 바인드
    addr.can_family  = AF_CAN;            // CAN 주소 패밀리
    addr.can_ifindex = ifr.ifr_ifindex;   // 조회된 인덱스 설정
    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");                   // bind 실패 시
        close(sockfd);                    // 소켓 닫기
        return 1;                         // 종료
    }

    printf("PiZero CAN 수신기 + LED 제어 준비 완료\n");

    // 5) 무한 수신 루프
    while (1) {
        int n = read(sockfd, &frame, sizeof(frame)); // CAN 프레임 읽기
        if (n < 0) {                                 // read 실패 검사
            perror("read");
            break;                                  // 루프 탈출
        }

        // (선택) 올바른 ID와 데이터 길이 체크
        if (frame.can_id == 0x123 && frame.can_dlc == 1) {
            if (frame.data[0] == 1) {               // 데이터가 1이면
                digitalWrite(LED_PIN, HIGH);        // LED ON
                printf("LED ON\n");
            } else {                                // 데이터가 0이면
                digitalWrite(LED_PIN, LOW);         // LED OFF
                printf("LED OFF\n");
            }
        }
    }

    // 소켓 닫기
    close(sockfd);
    return 0;  // 정상 종료
}

