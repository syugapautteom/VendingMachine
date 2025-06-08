#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "util.h"
#include "vending.h"

/* OS별 CLEAR 동작 */
#ifdef _WIN32
#define CLEAR_COMMAND "cls"         //Windows 환경에서의 CLEAR
#else
#define CLEAR_COMMAND "clear"       //macOS, Linux 환경에서의 CLEAR
#endif

/* OS별 TTS 동작  */
#ifdef _WIN32
    #define OS_WINDOWS
#elif __APPLE__
    #define OS_MAC
#else
    #define OS_LINUX
#endif

// 관리자 물품 추가, 제거 변수
int menu = 0, num = 0;

// 투입 금액 관련 (기본 0 초기화 상태)
int TotalMoney = 0;
int TotalProfit = 0;

// 화면 클리어
void ClearScreen(void) {
    system(CLEAR_COMMAND);
}

// TTS 말투 스타일 선택
TTSStyle currentStyle = DEFAULT;
const char* GetStyleName(TTSStyle style) {
    switch (style) {
        case DEFAULT: return "기본";
        case CUTE: return "귀엽게";
        case COOL: return "시크하게";
        case ROBOT: return "로봇톤";
        case OFFICE: return "직장인톤";
        default: return "알 수 없음";
    }
}

// 구입시 TTS 출력
void SpeakMessage(const char* message) {
    char command[512];
#ifdef OS_WINDOWS
    const char* prefix = "powershell -Command \"Add-Type –AssemblyName System.Speech; (New-Object System.Speech.Synthesis.SpeechSynthesizer).Speak('";
    const char* suffix = "')\"";
    snprintf(command, sizeof(command), "%s%s%s", prefix, message, suffix);
#elif defined(OS_MAC)
    const char* voice;
    switch (currentStyle) {
        case CUTE:   voice = "Yuna"; break;
        case COOL:   voice = "Samantha"; break;
        case ROBOT:  voice = "Whisper"; break;
        case OFFICE: voice = "Alex"; break;
        default:     voice = "Yuna"; break;
    }
    snprintf(command, sizeof(command), "say -v %s \"%s\"", voice, message);
#elif defined(OS_LINUX)
    snprintf(command, sizeof(command), "espeak \"%s\"", message);
#else
    printf("TTS not supported on this OS\n");
    return;
#endif

    system(command);
}

// 스타일 선택
// 스타일 선택
void SelectTTSStyle(void) {
    ClearScreen();
    printf("====================[ 🎙️ 음성 스타일 메뉴 ]====================\n");
    printf("1. 기본  |  2. 귀엽게  |  3. 시크하게  |  4. 로봇톤  |  5. 직장인톤  \n");
    printf("===========================================================\n");

    // 💡 입력 준비
    char input[32];
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF); // 버퍼 정리

    printf("원하는 스타일을 선택하세요 (취소 키 Enter) : ");
    fgets(input, sizeof(input), stdin);

    // 🚫 취소 조건
    if (input[0] == '\n') {
        printf("⏪ 스타일 변경이 취소되었습니다. 메뉴로 돌아갑니다...\n");
        HAL_Delay(1000);
        ClearScreen();
        return;
    }

    int choice = atoi(input);
    if (choice >= 1 && choice <= 5) {
        currentStyle = (TTSStyle)(choice - 1);
        printf("✅ 스타일이 '%s'로 변경되었습니다!\n", GetStyleName(currentStyle));
    } else {
        printf("❌ 올바르지 않은 선택입니다.\n");
    }

    HAL_Delay(1000);
    ClearScreen();
}


// 스타일 선택
void SpeakDrinkMessage(const char* drinkTTSName) {
    const char* message;

    // 각 스타일별 멘트 배열
    const char* defaultMsgs[] = {
        "%s가 나왔습니다.",
        "%s 준비됐습니다.",
        "선택하신 %s, 여기 있어요!",
        "%s 추출 완료되었습니다.",
        "이용해주셔서 감사합니다. %s 나왔어요!"
    };
    const char* cuteMsgs[] = {
        "%s 나왔다냥~ 냥냥!",
        "%s 완성! 한 입만 주세용~ >3<",
        "%s 도착! 냥뇽이가 직접 만든 거예요!",
        "짜잔~ %s 등장했어요! ✨",
        "%s 드링킹타임~ 꿀꺽꿀꺽!"
    };
    const char* coolMsgs[] = {
        "%s 나왔어. 챙겨가.",
        "%s 준비 완료. 그럼 이만.",
        "%s 도착. 다음.",
        "끝. %s 나왔어.",
        "%s 나왔습니다. 처리하세요."
    };
    const char* robotMsgs[] = {
        "%s. 이. 배. 송. 완. 료.",
        "음료. %s. 출. 고. 됐. 습. 니. 다.",
        "시스템: %s 나왔습니다.",
        "프로세스 완료. %s 추출됨.",
        "✅ %s 제공 완료. 다음 명령 대기 중."
    };
    const char* officeMsgs[] = {
        "고객님, %s가 나왔습니다.",
        "선택하신 %s 음료 추출이 완료되었습니다.",
        "%s 준비되었습니다. 이용해 주셔서 감사합니다.",
        "%s가 나왔습니다. 맛있게 드세요.",
        "감사합니다. %s 음료 제공이 완료되었습니다."
    };

    srand((unsigned int)time(NULL));
    int randIndex = rand() % 5;

    // 스타일에 따라 멘트 선택
    switch (currentStyle) {
        case CUTE:
            message = cuteMsgs[randIndex]; break;
        case COOL:
            message = coolMsgs[randIndex]; break;
        case ROBOT:
            message = robotMsgs[randIndex]; break;
        case OFFICE:
            message = officeMsgs[randIndex]; break;
        default:
            message = defaultMsgs[randIndex]; break;
    }

    char finalMessage[128];
    snprintf(finalMessage, sizeof(finalMessage), message, drinkTTSName);
    printf("🗣️ %s\n", finalMessage);
    SpeakMessage(finalMessage);
}

// 1 sec 단위 Delay 명령
void HAL_Delay(int delayms){
    usleep(delayms * 1000);
}

int total_sales = 0;

// 현재 시각을 "YYYY-MM-DD HH:MM:SS" 형식으로 반환
void get_timestamp(char* buffer, size_t size) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

// 판매 기록을 파일에 한 줄로 기록: 날짜 | 품목 | 누적 수량 | 비율
void record_sale(const char* item_name) {
    char timestamp[32];
    get_timestamp(timestamp, sizeof(timestamp));

    for (int i = 0; i < 4; i++) {
        if (strcmp(drinks[i].name, item_name) == 0) {
            drinks[i].sales++;  // 누적 수 증가

            FILE* file = fopen(SALES_FILE, "a");
            if (!file) {
                perror("🚫 파일 열기 실패");
                return;
            }

            fprintf(file, "%s | %s | 누적 %d개 판매됨\n",
                timestamp, item_name, drinks[i].sales);
            fclose(file);

            //printf("✅ 판매 기록 완료!\n");
            return;
        }
    }

    printf("🚫 해당 품목을 찾을 수 없습니다 : %s\n", item_name);
}


// 영수증 출력
void PrintReceipt(const char* drinkName, int price, int remainMoney) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    const char* thanksMessages[] = {
        "🥤 감사합니다! 다음에도 또 만나요~!",
        "🧋 오늘도 달콤한 하루 되세요!",
        "🍵 당신의 선택, 정말 멋져요!",
        "😽 냥뇽이가 감동했어요~ 고마워요!",
        "💖 자판기는 당신을 사랑합니다(?)",
        "🫧 시원하게 드시고 힘내세요!",
        "🔥 당신의 선택, 불꽃처럼 멋져요!",
        "✨ 반짝이는 하루가 되길 바랄게요~",
        "🎁 또 오면 깜짝 선물이 있을지도!?",
        "😸 행복한 음료 생활 되세요~!"
    };

    srand((unsigned int)time(NULL));
    int msgIndex = rand() % 10;

    FILE *fp = fopen(RECEIPT_FILE, "a");
    if (fp != NULL) {
        fprintf(fp,"=======================[ 📄 영수증 ]=======================\n");
        fprintf(fp, "구매 음료  : %s\n", drinkName);
        fprintf(fp, "가격      : %d원\n", price);
        fprintf(fp, "시간      : %04d-%02d-%02d %02d:%02d:%02d\n",
                t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                t->tm_hour, t->tm_min, t->tm_sec);
        fprintf(fp, "잔액      : %d원\n", remainMoney);
        fprintf(fp,"=========================================================\n");
        fprintf(fp, "%s\n", thanksMessages[msgIndex]);
        fclose(fp);
    }

    // 콘솔에도 출력
    printf("\n");   // 한 줄 띄어쓰기 진행(가독성 향상)
    printf("=======================[ 📄 영수증 ]=======================\n");
    printf("구매 음료  : %s\n", drinkName);
    printf("가격      : %d원\n", price);
    printf("시간      : %04d-%02d-%02d %02d:%02d:%02d\n",
           t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
           t->tm_hour, t->tm_min, t->tm_sec);
    printf("잔액      : %d원\n", remainMoney);
    printf("=========================================================\n");
    printf("%s\n", thanksMessages[msgIndex]);
    
    HAL_Delay(1500);
}
