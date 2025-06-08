/*─────────────────────────────────────────────
 * 📜 저작권 및 라이선스 고지 (Copyright & License)
 *
 * 본 소프트웨어는 "일등이조" 팀의 저작물로,
 * 대한민국 저작권법 및 국제 저작권 조약에 의해 보호받습니다.
 * 자세한 내용은 README.md 파일을 참조하십시오.
 *
 * ■ 팀 정보 (Team Info)
 * - 팀명: 일등이조 (22F76)
 * - 팀원: 이은정, 홍창기
 * - 교수: 김은지 교수님
 *
 * 🗓 최초 작성: 2024-03-19 / 최종 수정: 2025-05-25
 *─────────────────────────────────────────────*/

/* 외부 헤더파일 */
#include <stdio.h>      // 표준 입출력 함수 (printf, scanf 등)
#include <stdlib.h>     // 동적 메모리, 형 변환, 시스템 함수 (malloc, exit 등)
#include <stdint.h>     // 고정 크기 부호없는 정수 자료형 (uint8_t, int32_t 등)
#include <string.h>     // 문자열 처리 함수 (strcmp, strcpy, memset 등)
#include <unistd.h>     // 유닉스/리눅스 기반 시스템 함수 (sleep, access 등)
#include <time.h>       // 시간 관련 함수 (time, localtime, srand 등)

#include "util.h"       // 유틸리티 함수, 전역 변수 모음 (랜덤, 문자열 보조 등)
#include "admin.h"      // 관리자 모드 관련 함수 (메뉴, 재고 수정 등)
#include "vending.h"    // 자판기 기능 제어 (메뉴 표시, 구매 처리 등)

/* 메인 실행코드 */
int main(void) {
    srand((uint32_t)time(NULL));
    
    while (1) {
        char retry = 'y';       // 기본값 초기화
        int retry_count = 3;    // 로그인 기회 3번

        while (retry_count > 0) {
            if (AuthenticatePassword() == 0) {
                goto LOGIN_SUCCESS;  // 성공 시 바로 진행!
            }
            retry_count--;
            printf("❌ 로그인에 실패하였습니다. 로그인 시도 가능 횟수 %d회 남았습니다.\n", retry_count);

            if (retry_count == 0) break;  // 횟수 모두 소진 시 반복문 탈출

            printf("다시 시도하시겠습니까? (y/n) : ");
            retry = getchar();
            while (getchar() != '\n');  // 입력 버퍼 정리

            if (retry != 'y' && retry != 'Y') {
                printf("👋 프로그램을 종료합니다.\n");
                return 1;
            }
        }

        if (retry_count == 0) {
            printf("👋 로그인 시도 가능 횟수 초과로 프로그램이 종료됩니다.\n");
            return 1;
        }

        ClearScreen();
    }

LOGIN_SUCCESS:
    HAL_Delay(1500);
    printf("자판기 시작 중... 계속하려면 Enter 키를 누르세요...");
    ClearScreen();
    StartVendingMachine(NULL);
    ClearScreen();
    return 0;
}
