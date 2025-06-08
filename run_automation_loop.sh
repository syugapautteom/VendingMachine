#!/bin/bash

# 자동화용 입력 파일 생성
INPUT_FILE="auto_input.txt"
cat << EOF > $INPUT_FILE
daelim
tujalnam

10000
1
7
8
2
6
daelim
tujalnam

1
1
3
2
1
2
3
4
1
3000
5
tujalnam
tujalnam
tujalnam
8
n
7
EOF

# 반복 횟수
LOOP_COUNT=100
EXECUTABLE="./VendingMachine"

# 실행파일 체크
if [ ! -x "$EXECUTABLE" ]; then
  echo "❌ 실행 파일이 없거나 권한이 없습니다: $EXECUTABLE"
  exit 1
fi

# 자동 실행 루프
for ((i=1; i<=LOOP_COUNT; i++)); do
  echo ""
  echo "🚀 자동 테스트 $i/$LOOP_COUNT 차수 실행 중..."
  $EXECUTABLE < $INPUT_FILE
  echo "✅ 완료: $i회차"
  sleep 1
  echo "-------------------------------"
done

rm $INPUT_FILE

echo "🎉 모든 자동 테스트 완료!"
