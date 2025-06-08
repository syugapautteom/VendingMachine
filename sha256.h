#ifndef SHA256_H
#define SHA256_H

// size_t 정의용
#include <stddef.h>

// SHA256의 출력값 크기 (32바이트 = 256비트)
#define SHA256_BLOCK_SIZE 32

typedef unsigned char BYTE;    // 8비트 바이트
typedef unsigned int  WORD;    // 32비트 워드 (16비트 시스템에서는 long으로 바꿔야 할 수도 있음)

// SHA256 연산을 위한 구조체
typedef struct {
    BYTE data[64];               // 입력 데이터 블록 (최대 512비트)
    WORD datalen;                // 현재 data 배열에 들어있는 바이트 수
    unsigned long long bitlen;  // 지금까지 처리한 총 비트 수
    WORD state[8];               // 해시 중간 상태값 (최종 해시 결과는 여기에 누적됨)
} SHA256_CTX;

// SHA256 초기화 함수
void sha256_init(SHA256_CTX *ctx);

// 데이터 입력 함수 (여러 번 호출 가능)
void sha256_update(SHA256_CTX *ctx, const BYTE data[], size_t len);

// 해시 최종 결과 생성 함수
void sha256_final(SHA256_CTX *ctx, BYTE hash[]);

#endif // SHA256_H
