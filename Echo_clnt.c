/***** A.1 * File Include                       *******************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <error.h>
/***** B.1 * Definition of New Constants        *******************************/
/***** C.1 * Declaration of Variables(LOCAL)    *******************************/
/***** C.2 * Declaration of Variables(EXTERN)   *******************************/
/***** D.1 * Definition of Functions(LOCAL)     *******************************/
/***** D.2 * Definition of Functions(EXTERN)    *******************************/

int main(int argc, char* argv[])
{
    int sock; //클라이언트 소켓 선언
    struct sockaddr_in serv_addr; 
    //서버의 주소 정보를 저장할 IPv4 주소를 다루기 위한 보다 구체적인 구조체 선언
    char msg[1024] = { 0x00, }; //msg 버퍼를 0으로 초기화
    int str_len = 0; //송수신할 문자열의 길이를 0으로 초기화

    if (argc != 3) //명령줄에서 입력한 인수가 3개가 아니면
    {
        printf("Usage : %s <IP> <port>\n", argv[0]); 
        //사용 방법을 출력한다
        exit(1);
    }

    sock = socket(AF_INET, SOCK_STREAM, 0); //클라이언트 소켓 생성
   
    if (sock == -1)
        perror("socket()");
    //클라이언트 소켓 생성이 실패하면 perror 함수를 이용해서 오류 메세지를 출력

    memset(&serv_addr, 0, sizeof(serv_addr));
    //memset 함수로 서버주소 구조체를 0으로 초기화 시킨다.
    
    serv_addr.sin_family = AF_INET;
    //서버주소의 주소 체계를 IPv4로 선언
    
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    /*서버주소의 구조체 중 IP주소를 당담하는 필드체계에 명령줄에서 입력받은 첫번째 인수를
    IP주소로 사용 */
    
    serv_addr.sin_port = htons(atoi(argv[2]));
    /*서버주소에서 포트번호의 필드에다가 명령줄에서 
    입력받은 2번쨰 인수를 atoi함수를 통해 정수만을 뽑아내고, htons함수를 이용해서
    호스트 바이트 순서에서 네트워크 바이트 순서로 바꾸어서 저장 */

    if (connect(sock,(struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
        perror("connect()");
   /* 클라이언트 소켓, 서버 주소구조채의 포인터를 네트워크 주소를 표현하는 가장 일반화된 구조체
    struct sockaddr*로 표현, 그리고 서버주소의 사이즈를 매개변수로 받는 connect함수를 통해
   연결 요청을 서버에 보냄 */
   
    printf("Input message : ");
    //보낼 메세지를 출력하라는 메세지를 출력
    
    scanf("%s", msg); 
    //입력한 메세지를 msg 버퍼에 저장
    
    str_len = write(sock, msg, 1024);
    /* 클라이언트 소켓에 msg버퍼에 저장된 최대 1024비트를 쓴다. 
    그리고 이 msg의 길이를 str_len에 저장한다. */
    
    if (str_len == -1)
        perror("write()");
    // write 함수에 오류가 났다면 -1이 반환되어 오류 메시지가 출력된다.
    
    memset(msg, 0, 1024);
    //memset 함수를 통해 msg버퍼에 저장된 최대 1024비트를 0으로 초기화 한다.

    str_len = read(sock, msg, 1024);
    // read 함수를 통해 클라이언트 소켓으로부터 저장된 최대1024비트를 msg버퍼에 저장
   
    if (str_len == -1)
        perror("read()");
    /* read 함수에 반환된 값이 -1이면 오류가 났다는 뜻이므로 perror함수를 통해
    오류 메세지를 출력한다 */
    
    printf("Message from server : %s\n", msg);
    //msg에 저장된 문자열을 출력한다.
    
    close(sock); //클라이언트 소켓을 닫는다.
    return 0;
}