2021 고급자료구조 term_project [검색 엔진 구현]
컴퓨터공학과 20183085 이주원

구현한 검색엔진 프로그램의 이름은 search-engine 이다.
make 를 이용해 컴파일 후 search-engine 을 실행하면
3가지의 사용 메뉴가 출력된다.

1번을 누르면 문서를 삽입하는 메뉴가 실행되고,
문서들이 들어있는 Directory 이름을 입력한다.
예시로 실행파일이 있는 경로에 문서들이 들어있는 "doc" 이라는 이름의 Directory 가 있다고 하자.
"Enter the document path (based on the current path)) : " 라는 문자열이 출력되고,
"doc" 라고 입력하면 doc directory 에 있는 문서들을 하나씩 열어서
단어들을 분리시켜서 tree 에 저장시킨다.
"Indexing the entered document... Please wait" 라는 문자열이 출력되면서
단어들의 저장이 시작되고 수 초간의 시간을 기다리면 단어 저장이 완료되면서
"The document has been successfully inserted !" 라는 문자열이 출력된다.

2번을 누르면 단어를 검색할 수 있는 메뉴가 실행된다.
"Search ! Enter the word you want to find : " 라는 문자열이 출력되면서
찾고자 하는 문자열을 입력한다.
예시로 "아시아" 라는 문자열을 찾고자 한다면
"아시아" 라고 입력한다. 입력 후 "Searching.. Please wait" 라는 문자열이 출력되면서
검색을 시작하고 단어의 hash 값을 출력해준다.
검색이 끝나고 나서 단어가 존재한다면,
가중치를 계산하여 "The document in which the word '아시아' appears is 3035
and the frequency is 1 in that document." 와 같은 문자열이 출력된다.
어느 문서에서 아시아가 가중치를 가지고 출력됐는지가 출력되고,
빈도수도 함께 출력된다.
문서에 없는 단어를 입력한다면, "There are no documents with the word !" 이라는 문자열이 출력된다.

3번을 누르면 프로그램을 종료한다.