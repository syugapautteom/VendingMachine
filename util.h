#ifndef UTIL_H
#define UTIL_H

#define RECEIPT_FILE "receipt.txt"
#define SALES_FILE "sales_log.txt"

//관리자 및 재고, 투입 금액, 수익 관련 전역 변수 선언
extern int TotalMoney;          // 자판기에 투입된 총 금액
extern int TotalProfit;         // 실제 벌어들인 수익
extern int menu, num;           // 관리자 물품 추가, 제거 변수

// TTS 스타일 구조체 선언
typedef enum {
    DEFAULT,
    CUTE,
    COOL,
    ROBOT,
    OFFICE
} TTSStyle;

// Drink 구조체 선언
typedef struct {
    char name[32];
    char ttsname[32];
    int price;
    int stock;
    int sales;
} Drink;

extern Drink drinks[4];
extern TTSStyle currentStyle;

// 화면 초기화
void ClearScreen(void);

// TTS 말투 스타일 선택
const char* GetStyleName(TTSStyle style);

// 구입시 TTS 출력
void SpeakMessage(const char* message);

// 스타일 선택
void SelectTTSStyle(void);
void SpeakDrinkMessage(const char* drinkTTSName);

// 1 sec 단위 Delay 명령
void HAL_Delay(int delayms);

// 현재 시각을 "YYYY-MM-DD HH:MM:SS" 형식으로 반환
void get_timestamp(char* buffer, size_t size);

// 판매 기록을 파일에 한 줄로 기록: 날짜 | 품목 | 누적 수량 | 비율
void record_sale(const char* item_name);

// 영수증 출력
void PrintReceipt(const char* drinkName, int price, int remainMoney);

typedef struct {
    char name[32];
    int count;
} ItemSale;

#endif // UTIL_H
