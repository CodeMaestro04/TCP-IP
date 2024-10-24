from cham1 import *
import socket


# Create a socket object (소켓 생성)
s = socket.socket()
host = "127.0.0.1"
port = 5550

# Connect to the server (서버에 연결)
s.connect((host, port))
print("Connected to the server.")
print("Waiting for Seurity Setting.........")

while True:
    
    #  서버로부터 CHAM에 대한 정보 수신
    recv_data = s.recv(4096)
    CHAM = recv_data.decode('utf-8')


    #mk는 테스트 벡터로 구성
    if  CHAM == 'CHAM-64/128':
        mk = [0x0100, 0x0302, 0x0504, 0x0706, 0x0908, 0x0b0a, 0x0d0c, 0x0f0e]
        k = 128
        r = 88
        w = 16
        message = 'Success'
        Succeed_message = message.encode('utf-8')
        s.send(Succeed_message)
        break
    
    elif CHAM == 'CHAM-128/128':
        mk =[0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c] 
        k = 128
        r = 112
        w = 32
        message = 'Success'
        Succeed_message = message.encode('utf-8')
        s.send(Succeed_message)
        break
    
    elif CHAM == 'CHAM-128/256':
        mk = [0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c, 0xf3f2f1f0, 0xf7f6f5f4, 0xfbfaf9f8, 0xfffefdfc]
        k = 256 
        r = 120
        w = 32
        message = 'Success'
        Succeed_message = message.encode('utf-8')
        s.send(Succeed_message)
        break
    else:
        print("Wrong-direction from server.")
        message = 'Wrong Direction'
        failed_message = message.encode('utf-8')
        s.send(failed_message)

rk = CHAM_key_schedule(mk, k, w)
print("If you wish to end the chat, type '//quit'.")


while True:
    
    # Input a message (메시지 입력)
    message = input("Enter a message: ")
    
    #문자열을 UTF-8 바이트로 인코드 및 자료 형 변환
    message_bytes = message.encode('utf-8').hex()
    message_ints = hex_to_int(message_bytes, len(message_bytes),w)       

    if message == '//quit':
        print("Disconnected with server")
        break
    
    # 암호화 수행
    ct = CHAM_CTR_Encryption(message_ints  ,rk, len(message_ints), r, w)
    ct_bytes = int_to_bytes(ct,w)

    # Send the encrypted message (암호화된 메시지 서버로 전송)
    s.send(ct_bytes)
    
    #  서버로부터 데이터 수신
    recv_data = s.recv(4096)
    
    # 데이터가 NULL인 경우, 클라이언트의 연결 종료 처리
    if not recv_data:
        print('Disconnected with Client.')
        break
    
    #복호화 수행 및 데이터 형 변환
    recv_data_ints = bytes_to_int(recv_data,w)
    decrypted_data_ints = CHAM_CTR_Encryption(recv_data_ints,rk,len(recv_data_ints), r ,w)
    decrypted_bytes = int_to_bytes(decrypted_data_ints,w)

    #바이트를 문자열로
    recv_message_bytes = decrypted_bytes.decode('utf-8')
    print(f"Decrypted message: {recv_message_bytes}")



s.close()