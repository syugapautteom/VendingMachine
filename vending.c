#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include "vending.h"
#include "admin.h"
#include "util.h"

/* 자판기 구조체 Drink */
Drink drinks[4] = {
    {"🥤 콜라", "콜라", 0x8FC, 0x0A, 0x00},
    {"🫧 사이다", "사이다", 0x8FC, 0x0A, 0x00},
    {"🧋 카페라떼", "카페라떼", 0xA8C, 0x0A, 0x00},
    {"🍵 율무차", "율무차", 0x5DC, 0x0A, 0x00}
}; // {"품명", "TTS품명", 가격, 수량, 초기값}이 16진수 HEX CODE 형태로 구성됨

// 자판기 럭키 보너스 지급 관련 코드
void CheckLuckyBonus(int index) {
    double lucky = (double)rand() / RAND_MAX * 100.0;  // 0 ~ 100%

    if (lucky < BONUS_CHANCE_PERCENT) {
        if (drinks[index].stock > 0) {
            drinks[index].stock--;
            drinks[index].sales++;
            TotalProfit += drinks[index].price;

            printf("🎉 대박! 행운의 이벤트 당첨! %s 하나 더 나왔어요!\n", drinks[index].name);

            FILE *fp = fopen(SALES_FILE, "a");
            if (fp != NULL) {
                time_t now = time(NULL);
                struct tm *t = localtime(&now);
                fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d] (보너스) %s 지급\n",
                        t->tm_year+1900, t->tm_mon+1, t->tm_mday,
                        t->tm_hour, t->tm_min, t->tm_sec,
                        drinks[index].name);
                fclose(fp);
            }
        }
    }
}

// 음료 준비 중 진행 바
void ShowRandomProgressBar(const char* drinkName) {
    int percent = 0;
    int stage = 0;

    const char* stages[] = {
        "컵 꺼내는 중이에요~ 🥤",
        "살살살~ 얼음 넣는 중 ❄️",
        "음료 친구 부르는 중! 📣",
        "꼬물꼬물! 거품 만드는 중 🫧",
        "졸졸졸~ 따르고 있어요 🍯",
        "조금만 기다려줘요 >.< ⏳",
        "간 보는 중... 짭짤~ 🤤",
        "냄새 맡는 중... 킁킁~ 👃",
        "나올 준비하는 중이에요! 💃",
        "짠! 등장 직전!! ✨"
    };

    while (percent < 100) {
        if (percent < 70) {
            HAL_Delay(300 + rand() % 400);  // 70% 이전까지는 빠르게 진행
        } else {
            HAL_Delay(1000 + rand() % 600);  // 70% 이후로는 좀 더 오래
        }

        int step = 5 + rand() % 11;
        percent += step;
        if (percent > 100) percent = 100;

        ClearScreen();
        printf("🎞️ %s 추출 중...\n", drinkName);

        printf("%3d%% [", percent);
        int blocks = percent / 10;
        for (int i = 0; i < 10; i++) {
            printf(i < blocks ? "■" : " ");
        }
        printf("]  ");

        if (percent < 100) {
            stage = percent / 10;
            printf("%s\n", stages[stage]);
        }

        fflush(stdout);
    }

    printf("✅ %s가 나왔습니다!\n", drinkName);
}

// 음료 준비 및 잔액 품절 안내
bool handle_purchase(int index) {
    if (drinks[index].stock > 0 && TotalMoney >= drinks[index].price) {
        drinks[index].stock--;
        TotalMoney -= drinks[index].price;
        TotalProfit += drinks[index].price;
        printf("선택하신 음료 %s가 준비 중 입니다.\n", drinks[index].name);
        HAL_Delay(500);
        return true;  // ✅ 성공!
    } else if (drinks[index].stock <= 0) {
        printf("선택하신 음료 %s가 품절되었습니다.\n", drinks[index].name);
    } else {
        printf("잔액이 부족합니다.\n현재 잔액은 %d원입니다.\n", TotalMoney);
    }
    HAL_Delay(500);
    return false;  // ❌ 실패!
}

// 사용자로부터 초기 금액 투입을 받는 함수
static void MoneyInput(void) {
    char InputBuffer[32];
    int InMoney;
    
    while (1) {
        //printf("100원 이상 10,000원 이하로 금액을 투입해주세요 : ");
        fgets(InputBuffer, sizeof(InputBuffer), stdin);
        if (sscanf(InputBuffer, "%d", &InMoney) != 1) {
            printf("100원 이상 10,000원 이하로 금액을 투입해주세요 : ");
            //printf("숫자를 입력해주세요.\n");
            continue;
        }

        if (InMoney < 0) {
            printf("음수 금액은 투입할 수 없습니다. 다시 투입해주세요 : ");
            continue;
        }

        if (InMoney >= 100 && InMoney <= 10000) {
            TotalMoney += InMoney;
            printf("투입이 완료되었습니다 : %d원\n", InMoney);
            break;
        } else {
            printf("금액이 유효하지 않아 %d원을 반환합니다. 다시 투입해주세요 : ", InMoney);
            continue;
        }
    }
    HAL_Delay(1000);
}

// 상품 선택 Switch-Case 처리
void HandleDrinkSelection(int index){
    if (handle_purchase(index)) {
        ShowRandomProgressBar(drinks[index].name);
        record_sale(drinks[index].name);
        CheckLuckyBonus(index);
        SpeakDrinkMessage(drinks[index].ttsname);
        PrintReceipt(drinks[index].ttsname, drinks[index].price, TotalMoney);
        
        // 상황에 따라 말풍선으로 TTS 메시지 출력하고 싶을 때 사용!
        //char voiceMessage[64];
        //snprintf(voiceMessage, sizeof(voiceMessage), "%s가 나왔습니다", drinks[index].ttsname);
        //SpeakMessage(voiceMessage);
    }
}

// 자판기 사용자 메뉴 루프 처리
static void VendingLoop(int *AdminPassword) {
    int SelectMenu;
    
    while (1) {
        printf("\n");
        printf("                                         현재 잔액 : %d원\n", TotalMoney);
        printf("====================[ 밍밍이 자판기 메뉴 ]====================\n");
        for (int i = 0; i < 4; i++) {
            printf("%d. %s (%d원, 재고 %s, %d개)\n",
                   i + 1,
                   drinks[i].name,
                   drinks[i].price,
                   (drinks[i].stock > 0) ? "있음" : "품절",
                   drinks[i].stock);
        }
        printf("====================[ 자판기 사용자 메뉴 ]====================\n");
        printf("              5. 금액 투입하기 |  7. 잔액 반환하기              \n");
        printf("====================[ 자판기 관리자 메뉴 ]====================\n");
        printf("              6. 관리자 모드  |  8. 음성/말투 변경하기           \n");
        printf("=========================================================\n");
        printf("원하는 메뉴를 선택하세요 : ");

        
        scanf("%d", &SelectMenu);
        
        switch (SelectMenu) {
            case 1:
                HandleDrinkSelection(0);
                HAL_Delay(500);
                ClearScreen();
                break;
            case 2:
                HandleDrinkSelection(1);
                HAL_Delay(500);
                ClearScreen();
                break;
            case 3:
                HandleDrinkSelection(2);
                HAL_Delay(500);
                ClearScreen();
                break;
            case 4:
                HandleDrinkSelection(3);
                HAL_Delay(500);
                ClearScreen();
                break;
            case 5:
                char input[32];
                int addMoney = 0;
                
                int ch; while ((ch = getchar()) != '\n' && ch != EOF);
                printf("💸 금액을 투입해주세요 : ");
                fgets(input, sizeof(input), stdin);

                // 엔터만 누르면 input[0] == '\n'
                if (input[0] == '\n') {
                    printf("⏪ 금액 투입이 취소되었습니다. 메뉴로 돌아갑니다...\n");
                    HAL_Delay(1000);
                    ClearScreen();
                    break;
                }

                addMoney = atoi(input);
                if (addMoney >= 100 && addMoney <= 10000) {
                    TotalMoney += addMoney;
                    printf("✅ 현재 잔액은 %d원입니다.\n", TotalMoney);
                } else {
                    printf("⚠️ 100원 이상 10,000원 이하만 투입 가능합니다.\n");
                }

                HAL_Delay(1000);
                ClearScreen();
                break;
            case 6:
                printf("⚙️ 관리자 모드 진입을 위한 보안 인증을 진행 중이니 잠시만 기다려주세요...\n");
                HAL_Delay(2500);    //보안지연시간
                ClearScreen();

                int flush;
                while ((flush = getchar()) != '\n' && flush != EOF);

                if (AuthenticatePassword() == 0) {
                    printf("관리자 인증 성공! 계속하려면 Enter 키를 누르세요...");
                    //while ((flush = getchar()) != '\n' && flush != EOF);
                    AdminMode(AdminPassword);
                }
                break;
            case 7:
                if (TotalMoney > 0) {
                    printf("남은 잔액 %d원을 반환합니다.\n이용해주셔서 감사합니다!\n", TotalMoney);
                    TotalMoney = 0;
                } else {
                    printf("반환할 잔액이 없습니다.\n이용해주셔서 감사합니다!\n");
                }
                HAL_Delay(1000);
                ClearScreen();
                break;
            case 8:
                SelectTTSStyle();
                HAL_Delay(1000);
                ClearScreen();
                break;
            default:
                printf("🐱 냥뇽이 : 오잉? 지금 이 칸은 어떻게 들어온고양?\n");
                HAL_Delay(1000);
                ClearScreen();
                break;
        }
    }
}

// 자판기 기능 전체를 시작하는 공개 함수
void StartVendingMachine(int *AdminPassword) {
    MoneyInput();
    ClearScreen();
 
    // 사용자 메뉴 루프 진입
    VendingLoop(AdminPassword);
}
