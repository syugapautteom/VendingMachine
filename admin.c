#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>

#include "util.h"
#include "admin.h"
#include "sha256.h"
#include "vending.h"

#ifdef _WIN32
    #define PLATFORM_NAME "Windows"
#elif __APPLE__
    #define PLATFORM_NAME "macOS"
#elif __linux__
    #define PLATFORM_NAME "Linux"
#else
    #define PLATFORM_NAME "Unknown"
#endif

#if defined(_WIN32) || defined(_WIN64)
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

// 관리자 메뉴 로그 기록
void LogAdminAction(const char *action) {
    FILE *logFile = fopen(ADMINLOG_FILE, "a");
    if (!logFile) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    fprintf(logFile, "[%04d-%02d-%02d %02d:%02d:%02d] (%s) %s\n",
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
        t->tm_hour, t->tm_min, t->tm_sec,
        PLATFORM_NAME, action);

    fclose(logFile);
}

// 관리자 모드 진입
void AdminMode(int *AdminPassword) {
    int temp;
    while ((temp = getchar()) != '\n' && temp != EOF);

    AccountProfile profile = ReadAccountProfile(PASSWORD_FILE);
    char storedHash[65];
    strcpy(storedHash, profile.passwordHash);
    
    LogAdminAction("관리자 모드 진입 성공");

    while (1) {
        printf("\n");
        printf("====================[ 자판기 관리자 메뉴 ]====================\n");
        printf("     1. 상품 보충 | 2. 상품 제거 | 3. 수익 회수 | 4. 가격 변경     \n");
        printf(" 5. 비밀번호 변경 | 6. 이용자 모드 | 7. 시스템 종료 | 8. 계정 초기화  \n");
        printf("=========================================================\n");
        printf("Select Index >> ");
        int AdminChoice;
        scanf("%d", &AdminChoice);

        if (AdminChoice == 1) {
            int ch;
            char input[32];
            int menu = 0, num = 0;

            printf("[ 상품 목록 ]\n");
            for (int i = 0; i < 4; i++) {
                printf("%d. %s (재고 : %d개)\n", i + 1, drinks[i].name, drinks[i].stock);
            }

            while ((ch = getchar()) != '\n' && ch != EOF); // 입력 버퍼 비움

            printf("보충할 상품 번호 선택 : ");
            fgets(input, sizeof(input), stdin);
            if (input[0] == '\n') {
                printf("⏪ 보충이 취소되었습니다.\n");
                continue;
            }
            menu = atoi(input);

            printf("보충할 수량 입력 (최대 10개) : ");
            fgets(input, sizeof(input), stdin);
            if (input[0] == '\n') {
                printf("⏪ 보충이 취소되었습니다.\n");
                continue;
            }
            num = atoi(input);

            if (num <= 0) {
                printf("❌ 음수 또는 0개는 보충할 수 없습니다!\n");
            } else if (num > 10) {
                printf("❌ 한 번에 최대 10개까지만 보충 가능합니다!\n");
            } else {
                drinks[menu - 1].stock += num;
                printf("✅ %s 재고가 %d개로 변경되었습니다.\n", drinks[menu - 1].name, drinks[menu - 1].stock);
                LogAdminAction("물건 보충 실행");
            }
        } else if (AdminChoice == 2) {
            int ch;
            char input[32];
            int menu = 0, num = 0;

            printf("[ 상품 목록 ]\n");
            for (int i = 0; i < 4; i++) {
                printf("%d. %s (재고 : %d개)\n", i + 1, drinks[i].name, drinks[i].stock);
            }

            while ((ch = getchar()) != '\n' && ch != EOF); // 버퍼 클리어

            printf("제거할 상품 번호 선택 : ");
            fgets(input, sizeof(input), stdin);
            if (input[0] == '\n') {
                printf("⏪ 제거가 취소되었습니다.\n");
                continue;
            }
            menu = atoi(input);

            printf("제거할 수량 입력 (최대 10개) : ");
            fgets(input, sizeof(input), stdin);
            if (input[0] == '\n') {
                printf("⏪ 제거가 취소되었습니다.\n");
                continue;
            }
            num = atoi(input);

            if (num <= 0) {
                printf("❌ 음수 또는 0개는 제거할 수 없습니다!\n");
            } else if (num > 10) {
                printf("❌ 한 번에 최대 10개까지만 제거 가능합니다!\n");
            } else if (drinks[menu - 1].stock >= num) {
                drinks[menu - 1].stock -= num;
                printf("✅ %s 재고가 %d개로 변경되었습니다.\n", drinks[menu - 1].name, drinks[menu - 1].stock);
                LogAdminAction("물건 제거 실행");
            } else {
                printf("❌ 제거할 수량이 재고보다 많습니다!\n");
            }
        } else if (AdminChoice == 3) {
            printf("총 수익 %d원이 회수되었습니다.\n", TotalProfit);
            TotalProfit = 0;
            LogAdminAction("수익 회수 실행");
        } else if (AdminChoice == 4) {
            int ch;
            char input[32];
            int drinkIndex = 0, newPrice = 0;

            printf("[ 음료 목록 ]\n");
            for (int i = 0; i < 4; i++) {
                printf("%d. %s (현재 가격 : %d원)\n", i + 1, drinks[i].name, drinks[i].price);
            }

            while ((ch = getchar()) != '\n' && ch != EOF); // 버퍼 클리어

            printf("가격을 변경할 음료 번호를 선택하세요 : ");
            fgets(input, sizeof(input), stdin);
            if (input[0] == '\n') {
                printf("⏪ 가격 변경이 취소되었습니다.\n");
                continue;
            }
            drinkIndex = atoi(input);

            if (drinkIndex < 1 || drinkIndex > 4) {
                printf("❌ 잘못된 번호입니다.\n");
                continue;
            }

            printf("새로운 가격을 입력하세요 (100 ~ 10000원) >> ");
            fgets(input, sizeof(input), stdin);
            if (input[0] == '\n') {
                printf("⏪ 가격 변경이 취소되었습니다.\n");
                return;
            }
            newPrice = atoi(input);

            if (newPrice < 100 || newPrice > 10000) {
                printf("❌ 유효하지 않은 가격입니다.\n");
                continue;
            }
            drinks[drinkIndex - 1].price = newPrice;
            printf("✅ %s의 가격이 %d원으로 변경되었습니다.\n", drinks[drinkIndex - 1].name, newPrice);
            LogAdminAction("음료 가격 변경 완료");
        } else if (AdminChoice == 5) {
            ChangeAdminPassword(storedHash);
        } else if (AdminChoice == 6) {
            LogAdminAction("일반 모드로 복귀");
            break;
        } else if (AdminChoice == 7) {
            LogAdminAction("시스템 종료");
            printf("시스템 종료 중...\n");
            exit(0);
        } else if (AdminChoice == 8) {
            while ((getchar()) != '\n' && getchar() != EOF);
            ResetAdminAccount();
        } else {
            printf("잘못된 선택입니다.\n");
        }
    }
}

// 관리자 계정 초기화
// 관리자 계정 초기화
void ResetAdminAccount(void) {
    printf("\n⚠️ 관리자 계정을 초기화하면 프로그램이 종료됩니다.\n");
    printf("정말 초기화하시겠습니까? (y/n) : ");
    char confirm = getchar();
    while (getchar() != '\n');

    if (confirm == 'y' || confirm == 'Y') {
        int result1 = remove(PASSWORD_FILE);
        int result2 = remove(SIGNATURE_FILE);

        if (result1 == 0 && result2 == 0) {
            printf("✅ 관리자 계정 및 무결성 인증 파일이 초기화되었습니다.\n");
        } else {
            printf("❌ 일부 파일 삭제에 실패했습니다.\n");
        }

        printf("📴 프로그램을 종료합니다.\n");
        exit(0);
    } else {
        printf("관리자 계정 초기화가 취소되었습니다. 다시 돌아갑니다.\n");
    }
}


// 관리자 계정 비밀번호 변경
void ChangeAdminPassword(const char *storedHash) {
    char pwBuffer[32], confirm[32], current[32];
    char currentHash[65], newHash[65];
    int temp;

    AccountProfile profile = ReadAccountProfile(PASSWORD_FILE);

    while ((temp = getchar()) != '\n' && temp != EOF);

    printf("현재 로그인된 관리자 ID : %s\n", profile.identity);
    printf("현재 관리자 비밀번호를 다시 입력하세요 : ");
    fgets(current, sizeof(current), stdin);
    current[strcspn(current, "\n")] = '\0';
    HashPassword_SHA256(current, currentHash);

    if (strcmp(currentHash, storedHash) != 0) {
        printf("❌ 비밀번호가 일치하지 않습니다. 변경을 취소합니다.\n");
        return;
    }

    printf("새 관리자 비밀번호를 입력하세요 : ");
    fgets(pwBuffer, sizeof(pwBuffer), stdin);
    pwBuffer[strcspn(pwBuffer, "\n")] = '\0';

    if (strlen(pwBuffer) == 0) {
        printf("⚠️ 비밀번호 입력이 취소되었습니다.\n");
        return;
    }

    printf("다시 입력하세요 : ");
    fgets(confirm, sizeof(confirm), stdin);
    confirm[strcspn(confirm, "\n")] = '\0';

    if (strlen(confirm) == 0) {
        printf("⚠️ 비밀번호 확인이 취소되었습니다.\n");
        return;
    }

    if (strcmp(pwBuffer, confirm) != 0) {
        printf("비밀번호가 일치하지 않습니다. 변경을 취소합니다.\n");
    } else {
        HashPassword_SHA256(pwBuffer, newHash);

        FILE *fp = fopen(PASSWORD_FILE, "w");
        if (!fp) {
            printf("❌ 계정 파일을 저장할 수 없습니다.\n");
            return;
        }

        fprintf(fp, "account_profile {\n");
        fprintf(fp, "    identity = %s\n", profile.identity);
        fprintf(fp, "    password = %s\n", newHash);
        fprintf(fp, "    account_level = %s\n", profile.level);
        fprintf(fp, "}\n");
        fclose(fp);

        printf("✅ 비밀번호가 성공적으로 변경되었습니다.\n");
        LogAdminAction("비밀번호 변경 완료");
    }
    
    // ⚠️ 무결성 인증 갱신
    struct stat attr;
    if (stat(PASSWORD_FILE, &attr) == 0) {
        char fileHash[65];
        HashPassword_SHA256(PASSWORD_FILE, fileHash);

        char combined[128];
        snprintf(combined, sizeof(combined), "%s|%ld", fileHash, attr.st_mtime);

        BYTE hash[SHA256_BLOCK_SIZE];
        SHA256_CTX ctx;
        sha256_init(&ctx);
        sha256_update(&ctx, (const BYTE *)combined, strlen(combined));
        sha256_final(&ctx, hash);

        FILE *sign = fopen(SIGNATURE_FILE, "w");
        if (sign) {
            for (int i = 0; i < SHA256_BLOCK_SIZE; i++) {
                fprintf(sign, "%02x", hash[i]);
            }
            fprintf(sign, "\n");
            fclose(sign);
        }
    }
}

// SHA-256 해시 함수 (파일 또는 문자열 자동 분기)
void HashPassword_SHA256(const char *input, char *outputHex) {
    if (strcmp(input, SIGNATURE_FILE) == 0) {
        // special transformation for license.sign
        FILE *f = fopen(input, "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            long size = ftell(f);
            fseek(f, 0, SEEK_SET);
            BYTE *data = malloc(size);
            fread(data, 1, size, f);
            fclose(f);

            // transform: invert and left shift each byte
            for (long i = 0; i < size; i++) {
                data[i] = (~data[i] & 0xFF) << 1 & 0xFF;
            }

            BYTE hash[SHA256_BLOCK_SIZE];
            SHA256_CTX ctx;
            sha256_init(&ctx);
            sha256_update(&ctx, data, size);
            sha256_final(&ctx, hash);
            free(data);

            for (int i = 0; i < SHA256_BLOCK_SIZE; i++) {
                sprintf(outputHex + (i * 2), "%02x", hash[i]);
            }
            outputHex[64] = '\0';
            return;
        }
    }

    // 파일 열기 시도
    FILE *fp = fopen(input, "rb");
    if (fp) {
        BYTE hash[SHA256_BLOCK_SIZE];
        SHA256_CTX ctx;
        sha256_init(&ctx);

        BYTE buffer[1024];
        size_t bytesRead;
        while ((bytesRead = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
            sha256_update(&ctx, buffer, bytesRead);
        }
        fclose(fp);
        sha256_final(&ctx, hash);

        for (int i = 0; i < SHA256_BLOCK_SIZE; i++) {
            sprintf(outputHex + (i * 2), "%02x", hash[i]);
        }
        outputHex[64] = '\0';
    } else {
        // 파일이 아니면 문자열로 간주
        BYTE hash[SHA256_BLOCK_SIZE];
        SHA256_CTX ctx;

        sha256_init(&ctx);
        sha256_update(&ctx, (const BYTE*)input, strlen(input));
        sha256_final(&ctx, hash);

        for (int i = 0; i < SHA256_BLOCK_SIZE; i++) {
            sprintf(outputHex + (i * 2), "%02x", hash[i]);
        }
        outputHex[64] = '\0';
    }
}

// 무결성 확인 함수
int VerifyAccountFileIntegrity(void) {
    FILE *signFile = fopen(SIGNATURE_FILE, "r");
    if (!signFile) {
        printf("❌ 무결성 서명 파일을 열 수 없습니다.\n");
        return 1;
    }

    char storedHash[65];
    if (!fgets(storedHash, sizeof(storedHash), signFile)) {
        fclose(signFile);
        printf("❌ 서명 파일을 읽을 수 없습니다.\n");
        return 1;
    }
    storedHash[strcspn(storedHash, "\n")] = '\0';
    fclose(signFile);

    struct stat attr;
    if (stat(PASSWORD_FILE, &attr) != 0) {
        printf("❌ 계정 파일 정보를 불러올 수 없습니다.\n");
        return 1;
    }

    char fileHash[65];
    BYTE hash[SHA256_BLOCK_SIZE];
    SHA256_CTX ctx;
    sha256_init(&ctx);

    FILE *fp = fopen(PASSWORD_FILE, "rb");
    if (!fp) return 1;

    BYTE buffer[1024];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        sha256_update(&ctx, buffer, bytesRead);
    }
    fclose(fp);
    HAL_Delay(1000);

    sha256_final(&ctx, hash);
    for (int i = 0; i < SHA256_BLOCK_SIZE; i++) {
        sprintf(fileHash + (i * 2), "%02x", hash[i]);
    }
    fileHash[64] = '\0';

    char combined[128];
    snprintf(combined, sizeof(combined), "%s|%ld", fileHash, attr.st_mtime);
    sha256_init(&ctx);
    sha256_update(&ctx, (const BYTE *)combined, strlen(combined));
    sha256_final(&ctx, hash);

    char result[65];
    for (int i = 0; i < SHA256_BLOCK_SIZE; i++) {
        sprintf(result + (i * 2), "%02x", hash[i]);
    }
    result[64] = '\0';

    // 🔍 디버깅용 해시 비교 출력
    //printf("📎 저장된 해시: %s\n", storedHash);
    //printf("🔍 계산된 해시: %s\n", result);

    if (strcmp(result, storedHash) != 0) {
        printf("🚫 관리자 계정 파일이 위조되었거나 변경되었습니다.\n");
        return 1;
    }

    return 0;
}

// 파일 존재 여부 확인
int FileExists(const char *path) {
    FILE *file = fopen(path, "r");
    if (file) {
        fclose(file);
        return 1;
    }
    return 0;
}

// 계정 정보 불러오기
AccountProfile ReadAccountProfile(const char *path) {
    AccountProfile profile = {0};
    FILE *fp = fopen(path, "r");

    if (!fp) {
        printf("❌ 계정 파일을 열 수 없습니다.\n");
        exit(1);
    }

    char line[128];
    bool hasId = false, hasPw = false, hasLevel = false;

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, "identity") != NULL) {
            sscanf(line, " identity = %s", profile.identity);
            hasId = true;
        } else if (strstr(line, "password") != NULL) {
            sscanf(line, " password = %s", profile.passwordHash);
            hasPw = true;
        } else if (strstr(line, "account_level") != NULL) {
            sscanf(line, " account_level = %s", profile.level);
            hasLevel = true;
        }
    }

    fclose(fp);
    HAL_Delay(1000);
    
    if (!hasId || !hasPw || !hasLevel) {
        printf("🚫 계정 파일 형식이 잘못되었습니다. 보안상 프로그램을 종료합니다.\n");
        exit(1);
    }

    if (strlen(profile.identity) == 0 || strlen(profile.passwordHash) != 64 || strcmp(profile.level, "management") != 0) {
        printf("🚫 계정 정보가 손상되었거나 위조되었습니다. 프로그램을 종료합니다.\n");
        exit(1);
    }

    return profile;
}

// 비밀번호 인증 함수
int AuthenticatePassword(void) {
    AccountProfile profile = {0};
    char inputId[32];
    char inputPw[64];
    char inputHash[65];

    if (!FileExists(PASSWORD_FILE)) {
        printf("\n🛠️ 최초 실행: 관리자 계정을 설정합니다.\n");

        printf("🧑‍💼 새 관리자 ID를 입력하세요 : ");
        fgets(inputId, sizeof(inputId), stdin);
        inputId[strcspn(inputId, "\n")] = '\0';

        printf("🔐 새 관리자 비밀번호를 입력하세요 : ");
        fgets(inputPw, sizeof(inputPw), stdin);
        inputPw[strcspn(inputPw, "\n")] = '\0';

        HashPassword_SHA256(inputPw, inputHash);

        FILE *fp = fopen(PASSWORD_FILE, "w");
        if (!fp) {
            printf("❌ 계정 파일을 저장할 수 없습니다.\n");
            return 1;
        }
        fprintf(fp, "account_profile {\n");
        fprintf(fp, "    identity = %s\n", inputId);
        fprintf(fp, "    password = %s\n", inputHash);
        fprintf(fp, "    account_level = management\n");
        fprintf(fp, "}\n");
        fclose(fp);
        HAL_Delay(1000);

        // 🔐 무결성 서명 파일 생성
        struct stat attr;
        if (stat(PASSWORD_FILE, &attr) == 0) {
            char fileHash[65];
            HashPassword_SHA256(PASSWORD_FILE, fileHash);

            char combined[128];
            snprintf(combined, sizeof(combined), "%s|%ld", fileHash, attr.st_mtime);

            BYTE hash[SHA256_BLOCK_SIZE];
            SHA256_CTX ctx;
            sha256_init(&ctx);
            sha256_update(&ctx, (const BYTE *)combined, strlen(combined));
            sha256_final(&ctx, hash);

            FILE *sign = fopen(SIGNATURE_FILE, "w");
            if (sign) {
                for (int i = 0; i < SHA256_BLOCK_SIZE; i++) {
                    fprintf(sign, "%02x", hash[i]);
                }
                fprintf(sign, "\n");
                fclose(sign);
            }
        }

        printf("✅ 계정 등록 완료! 자동 로그인됩니다.\n");
        return 0;
    }

    if (VerifyAccountFileIntegrity() != 0) {
        return 1;
    }

    profile = ReadAccountProfile(PASSWORD_FILE);

    printf("🧑‍💼 관리자 ID를 입력하세요 : ");
    fgets(inputId, sizeof(inputId), stdin);
    inputId[strcspn(inputId, "\n")] = '\0';

    if (strcmp(inputId, profile.identity) != 0) {
        printf("❌ ID가 일치하지 않습니다.\n");
        return 1;
    }

    printf("🔐 비밀번호를 입력하세요 : ");
    fgets(inputPw, sizeof(inputPw), stdin);
    inputPw[strcspn(inputPw, "\n")] = '\0';

    HashPassword_SHA256(inputPw, inputHash);
    if (strcmp(inputHash, profile.passwordHash) == 0) {
        printf("✅ 로그인 성공! 권한: %s\n", profile.level);
        return 0;
    } else {
        printf("❌ 비밀번호가 일치하지 않습니다.\n");
        return 1;
    }
}
