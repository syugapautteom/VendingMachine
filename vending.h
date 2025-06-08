#ifndef VENDING_H
#define VENDING_H

// 자판기 럭키 보너스 지급 관련 코드
#define BONUS_CHANCE_PERCENT 0.02  // 🎯 확률 설정 (0.02%), 확률 변경시 꼭 소수 첫째자리까지 표현할 것(예시로 50% -> 50.0% 처럼)
void CheckLuckyBonus(int index);

// 음료 준비 중 진행 바
void ShowRandomProgressBar(const char* drinkName);

// 음료 준비 및 잔액 품절 안내
bool handle_purchase(int index);

// 사용자로부터 초기 금액 투입을 받는 함수
//static void MoneyInput(void);

// 상품 선택 Switch-Case 처리
void HandleDrinkSelection(int index);

// 자판기 사용자 메뉴 루프 처리
//static void VendingLoop(int *AdminPassword);

// 자판기 기능을 시작 (돈 투입, 관리자 모드 선택 및 사용자 모드 루프 호출)
void StartVendingMachine(int *AdminPassword);

#endif // VENDING_H
