/***** A.1 * File Include                       *******************************/

//함수 호출에 대한 구간

#include <stdio.h> // 표준 입출력 함수 호출
#include <stdlib.h> //일반적인 유틸리티 함수 호출
#include <string.h> // 문자열 처리 함수 호출
#include <unistd.h> // 다양한 POSIX 운영 체제 API 함수 호출
#include <arpa/inet.h> //인터넷 주소 변환 함수 및 소켓 구조체 정의 함수 호출
#include <sys/socket.h> //소켓 관련 함수 및 상수 정의 함수 호출
#include <error.h> //오류 번호 정의 및 처리 함수
/***** B.1 * Definition of New Constants        *******************************/
//새로운 상수 정의에 대한 구간

/***** C.1 * Declaration of Variables(LOCAL)    *******************************/
//지역 변수 선언 구간

/***** C.2 * Declaration of Variables(EXTERN)   *******************************/
//외부 변수 선언 구간

/***** D.1 * Definition of Functions(LOCAL)     *******************************/
//지역 함수 선언 구간

/***** D.2 * Definition of Functions(EXTERN)    *******************************/
//외부 함수 선언 구간


int main(int argc, char* argv[]) /*메인 함수의 진입점(메인은 보통 변수를 안받는다)
변수로 argc(argument count),
argv(argument variable)를 받는다. 전자는 메인 함수에 전달되는 데이터의 개수를 의미하고
후자는 문자열의 주소를 저장하는 포인터 배열이다.
예를 들어 이 함수에 ./tiny 8000 aaa이라는 입력을 준다면, argc는 2개, argv[0]에는 실행
경로인 ./tiny가 들어가고, argv[1]에는 8000 문자열의 시작 주소가 들어가고, 
argv[2]에는 aa의 문자열 시작 주소가 들어간다. 각 인자는 띄어쓰기로 구분된다.

{
    int serv_sock; //서버 소켓 디스크립터를 저장할 변수 정의
    int clnt_sock; // 클라이언트 소켓 디스크립터를 저장할 변수 정의
    int data_len = 0; //전송되거나 수신된 데이터 길이를 저장할 변수 정의 및 초기화

    struct sockaddr_in serv_addr; 
    //서버의 주소 정보를 저장할 IPv4 주소를 다루기 위한 보다 구체적인 구조체 선언
    
    struct sockaddr_in clnt_addr; 
    //클라이언트의 주소정보를 저장할 IPv4 주소를 다루기 위한 보다 구체적인 구조체 선언
    
    socklen_t clnt_addr_size = 0; 
    //클라이언트 주소 구조체의 크기를 저장할 변수 선언

    char msg[1024] = { 0x00, }; 
    //전송 및 수신할 메세지를 저장할 버퍼의 최대크기 정의
    // 0으로 초기화

    if (argc != 2)// 명령줄 인수가 2개가 아닌 경우만 실행(프로그램 이름과 포트번호)
    {
        printf("Usage : %s <port>\n", argv[0]); //argv[0]는 프로그램의 이름
        exit(1); 
        //종료
    }

    serv_sock = socket(AF_INET, SOCK_STREAM, 0); 
    //TCP소켓을 생성해서 serv_sock으로 정의
    // 차례로 주소 체계, 소켓 유형, 프로토콜을 의미한다.
   
    if (serv_sock == -1) 
        perror("socket()"); 
        //소켓 생성에 실패하면 "perror" 함수를 사용하여 오류 메세지 출력

    memset(&serv_addr, 0, sizeof(serv_addr)); 
	  /*memset함수를 이용하여 구조체 변수serv_addr의 메모리 주소의 내용을 0으로 초기화*/
    
    serv_addr.sin_family = AF_INET; 
    //서버의 주소 체계 IPV4로 설정(.sin_family는 주소 체계)
    
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    /*'서버 주소의 구조체 중 IP 주소를 저장하는 필드에서 실제 IP주소를 저장된 곳'에 
    htonl를 통해서 호스트 바이트 순서로 '32비트 정수인 INADDR_ANY'를 변환하는 것 */
    
    /*INADDR_ANY는 상수로 0.0.0.0을 나타내며 이는 여러주소에 들어오는 데이터를 모두
    수신 가능하게 해준다. */
    
    serv_addr.sin_port = htons(atoi(argv[1]));
    //명령줄에서 전달된 포트 번호를 서버 주소에 포트 번호에다가 대입
    //htons 함수 이용

    if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        perror("bind() : "); 
    /* 서버소켓과 서버 주소구조채의 포인터를 네트워크 주소를 표현하는 가장 일반화된 구조체
    struct sockaddr*로 표현 그리고 서버주소의 크기를 매개변수로 받아 바인딩 */
    /*바인딩이 실패하면 'perror'함수를 이용 오류 메세지 출력*/
    
    
    if (listen(serv_sock, 5) == -1)
        perror("listen() : ");
    //서버 소켓을 수신대기 상태로 전환, 대기열의 크기는 5
    //대기 상태 전환이 실패하면 'perror'함수를 이용하여 오류 메세지 출력

    clnt_addr_size = sizeof(clnt_addr); 
    //클라이언트 주소의 크기를 대입
    
    clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
    //'accept' 함수는 클라이언트의 연결 요청을 수락, 새로운 소켓 객체 반환
    // 새로운 소켓 객체가 'clnt_addr'에 저장
    
    if (clnt_sock == -1)
        perror("accept() : ");
    //'perror'함수를 사용하여 accept 실패시 오류 메세지 출력

    data_len = read(clnt_sock, msg, 1024);
    //클라이언트로부터 소켓으로부터 메세지를 읽어 'msg'버퍼에 저장(read 함수 사용)
    
    if (data_len == -1) 
    {
      	perror("read() : ");   
    }
    //만역 읽기작업이 실패하면 'perror'함수를 사용하여 오류 메세지를 출력

    printf("Echo message : %s\n", msg);
    //읽은 메세지를 에코로 다시 보낸다는 뜻에서 'msg'버퍼에서 출력
    
    write(clnt_sock, msg, 1024);
    //클라이언트 소켓에 msg 버퍼에 저장된 메세지 작성('write'함수 이용) 
    //최대 크기인 1024바이트를 전송
    
    close(clnt_sock);
    close(serv_sock);
    // 클라이언트 소켓과 서버소켓을 닫아 연결 종료
    
    return 0;
}
