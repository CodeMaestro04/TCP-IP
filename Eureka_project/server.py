from cham1 import *
import socket
import os

# Create a socket object (소켓 생성)
s = socket.socket()
host = "127.0.0.1"
port = 5550

s.bind((host, port))
s.listen(1)
print("Server is listening for incoming connections")

cs, addr = s.accept()
print(f"Connection from: {addr}")

while True:
    # Input a message (메시지 입력)
    message = input("Choose CHAM-64/128, CHAM-128/128, CHAM-128/256\n")
    direction = message.encode('utf-8')
    cs.send(direction)
    
    # client로부터 데이터 받기
    recv_data = cs.recv(4096)
    whether = recv_data.decode('utf-8')
    
    if whether == 'Success':
        print("Security is setted")
        direction = direction.decode('utf-8')
        if direction == 'CHAM-64/128':
            mk = [0x0100, 0x0302, 0x0504, 0x0706, 0x0908, 0x0b0a, 0x0d0c, 0x0f0e]
            k =128
            r = 88
            w = 16
            break
    
        elif direction == 'CHAM-128/128':
            mk =[0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c] 
            k = 128
            r = 112
            w = 32
            break
    
        elif direction == 'CHAM-128/256':
            mk = [0x03020100, 0x07060504, 0x0b0a0908, 0x0f0e0d0c, 0xf3f2f1f0, 0xf7f6f5f4, 0xfbfaf9f8, 0xfffefdfc]
            k = 256 
            r = 120
            w = 32
            break
    else:
        print("Setting Failed. Retry.\n")
        
rk = CHAM_key_schedule(mk, k, w)
print("If you wish to end the chat, type '//quit'.")

while True:

    # client로부터 데이터 받기
    recv_data = cs.recv(4096)
    
    # 데이터가 NULL인 경우, 클라이언트의 연결 종료 처리
    if not recv_data:
        print('Disconnected with Client.')
        break

    #데이터 자료형 변환
    recv_data_ints = bytes_to_int(recv_data,w)    
 
    # 복호화 수행 (암호문 복호화) 
    decrypted_data_ints = CHAM_CTR_Encryption(recv_data_ints,rk,len(recv_data_ints),r,w)
    
    # 복호화된 데이터 자료형 변환
    decrypted_bytes = int_to_bytes(decrypted_data_ints,w)
    recv_message_bytes = decrypted_bytes.decode('utf-8')
    
    #받은 데이터 출력
    print(f"Decrypted message: {recv_message_bytes}")
    
    
    # Input a message (메시지 입력)
    message = input("Enter a message: ")
    
    if message == '//quit':
        print('Disconnected with client')
        break
    
    #문자열을 UTF-8 바이트로 인코드 및 자료 형 변환
    message_bytes = message.encode('utf-8').hex()
    message_ints = hex_to_int(message_bytes, len(message_bytes),w)       

    # 암호화 수행
    ct = CHAM_CTR_Encryption(message_ints  ,rk,len(message_ints),r,w)
    ct_bytes = int_to_bytes(ct,w)

    # Send the encrypted message (암호화된 메시지 서버로 전송)
    cs.send(ct_bytes)
    

cs.close()
