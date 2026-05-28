# makefile: 빌드, 테스트, 자동화를 편하게 하도록 만든 make파일

# 변수 선언
CC = g++					# c++ 컴파일러로 'g++' 사용.
CCFLAGS = -g -std=c++11		# -g : 에러를 찾기 쉽게 디버깅 정보를 포함, -std=c++11 : C++11 표준 문법을 사용. 
TYPE = -lncurses			# ncurses라는 터미널 UI 라이브러리 연결을 위한 옵션.

# run 명령어 선언
run: Stage.o ItemAndMission.o Main.o	# 명시된 파일 사용해서 명령어 실행한다는 의미.
	$(CC) $(CCFLAGS) -o run Stage.o ItemAndMission.o Main.o $(TYPE) # 실행파일 만드는 명령어
clean:
	rm -f *.o run # 오브젝트 파일들과 실행파일을 지운다.
%.o : %.cpp %.h
	$(CC) $(CCFLAGS) -c $< # .cpp파일과 .h파일을 사용해서 명령어 실행한다는 의미
%.o : %.cpp					# $<: 첫번째 파일(.cpp파일)을 의미한다. -> 오브젝트 파일 만듬.
	$(CC) $(CCFLAGS) -c $<	# .h파일이 없는 경우, .cpp만 사용해서 오브젝트 파일 만듬
% : %.cpp
	$(CC) $(CCFLAGS) -o $@ $<
