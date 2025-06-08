#ifndef ADMIN_H
#define ADMIN_H

// 파일명 정의
#define PASSWORD_FILE "account.db"      // 관리자 계정 정보 파일
#define SIGNATURE_FILE "license.sign"   // 관리자 계정 무결성 확인 파일
#define ADMINLOG_FILE "admin_log.txt"   // 관리자 설정 로그 저장 파일

// 관리자 메뉴 로그 기록
void LogAdminAction(const char *action);

// 관리자 비밀번호 외부 암호화 파일 저장 설정
void AdminMode(int *AdminPassword);

// 관리자 계정 초기화
void ResetAdminAccount(void);

// 관리자 계정 비밀번호 변경
void ChangeAdminPassword(const char *storedHash);

// SHA-256 해시 함수
void HashPassword_SHA256(const char *input, char *outputHex);

// 무결성 확인 함수
int VerifyAccountFileIntegrity(void);

// 파일 존재 여부 확인
int FileExists(const char *path);

// 로그인 ID/PW 인증 구조체
typedef struct {
    char identity[32];
    char passwordHash[65];
    char level[16];
} AccountProfile;

// 계정 정보 불러오기
AccountProfile ReadAccountProfile(const char *path);

// 비밀번호 인증 함수
int AuthenticatePassword(void);

#endif // ADMIN_H
