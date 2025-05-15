#include <stdio.h>          // 표준 입출력 함수 (printf, perror 등)
#include <string.h>         // 문자열 처리 함수 (strcpy 등)
#include <unistd.h>         // POSIX 표준 함수 (read, write, close 등)
#include <net/if.h>         // 네트워크 인터페이스 구조체 ifreq
#include <sys/ioctl.h>      // ioctl 시스템 호출
#include <sys/socket.h>     // 소켓 함수 (socket, bind 등)
#include <linux/can.h>      // CAN 프레임 구조체 정의
#include <linux/can/raw.h>  // RAW CAN 소켓 옵션 정의

int main(void) {
    int s;                          // CAN 소켓의 파일 디스크립터를 저장할 변수
    struct sockaddr_can addr;       // CAN 소켓 주소 정보를 담는 구조체
    struct ifreq ifr;               // 네트워크 인터페이스 조회용 구조체
    struct can_frame frame;         // CAN 프레임 데이터를 담는 구조체
    char c;                         // 사용자 입력(문자)을 받을 변수

    // 1) 소켓 생성: PF_CAN 프로토콜, RAW 타입, CAN_RAW 프로토콜
    s = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (s < 0) {                    // socket() 반환값이 음수면 실패
        perror("socket");           // 실패 원인 출력
        return 1;                   // 비정상 종료
    }

    // 2) "can0" 인터페이스 이름 → 인터페이스 인덱스 조회
    strcpy(ifr.ifr_name, "can0");   // ifr_name에 문자열 "can0" 복사
    ioctl(s, SIOCGIFINDEX, &ifr);   // 소켓에 ioctl 요청하여 ifr.ifr_ifindex에 인덱스 저장

    // 3) 바인드(소켓과 인터페이스 연결)
    addr.can_family  = AF_CAN;            // 주소 패밀리 설정 (AF_CAN)
    addr.can_ifindex = ifr.ifr_ifindex;   // 조회된 인터페이스 인덱스 설정
    bind(s, (struct sockaddr *)&addr, sizeof(addr)); // 소켓에 주소 할당

    // 사용자 안내 메시지 출력
    printf("Pi5 CAN 송신기 (ID=0x123), 1=LED ON, 0=LED OFF, q=종료\n");

    while (1) {
        // 사용자 입력 프롬프트
        printf("> ");
        fflush(stdout);                   // 버퍼에 남은 출력 강제 전송

        // 1바이트 문자 입력
        if ((c = getchar()) == EOF)       // EOF(입력 종료) 검사
            break;                        // 입력 종료 시 루프 탈출
        if (c == '\n')                    // 엔터만 입력된 경우
            continue;                     // 다시 입력 대기
        if (c == 'q')                     // 'q'를 누르면 종료
            break;
        if (c != '1' && c != '0') {       // 1 또는 0이 아니면
            printf("입력 오류\n");        // 오류 메시지
            while (getchar() != '\n');   // 버퍼에 남은 입력 모두 비우기
            continue;                     // 다시 입력 대기
        }

        // CAN 프레임 작성
        frame.can_id  = 0x123;            // CAN ID 설정 (예: 0x123)
        frame.can_dlc = 1;                // 데이터 길이(DLC) 설정 (1바이트)
        frame.data[0] = (c == '1') ? 1 : 0; // 데이터 바이트에 1 또는 0 저장

        // 프레임 전송
        if (write(s, &frame, sizeof(frame)) != sizeof(frame))
            perror("write");              // 전송 실패 시 오류 출력
        else
            printf("Sent: LED %s\n", c=='1'?"ON":"OFF"); // 전송 성공 메시지

        // 남은 입력(엔터) 버퍼 비우기
        while (getchar() != '\n');
    }

    // 소켓 닫기
    close(s);
    return 0;  // 정상 종료
}

