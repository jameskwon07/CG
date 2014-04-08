Line 1-11 : Mac OS에서 빌드를 하기 위해 ifdef 구문 추가
Line 58-59 : Fig.3 구현을 하면서 Mouse 클릭에 대해 정상구동을 위해 현재 사이즈를 저장
Line 60 : Iteration 숫자가 반복이 많이 되어 글로벌 변수로 따로 저장
Line 61 : PA1 과제 Fig. 3 구현을 위해 Delta 값을 글로벌 변수로 따로 저장
Line 251-255 : Fig. 3 구현을 위해 주석처리.

display 함수 구현 변경 점
1. 기존 for 구문 시작에 있던 delta 값을 글로벌 변수로 따로 저장.

reshape 함수 구현 변경 점
1. 기존은 width와 height 변수 값을 직접 변경을 했는 데 width, height는 display에서 사용되며 Fig.3 구현을 위해 고정을 시켜야 함.
따라서 기존 width, height를 사용을 위해 current_width, current_height 변수 값을 변경하도록 변경(mouse 함수에서 사용됨)
