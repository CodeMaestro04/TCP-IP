# 워드 크기에 맞는 ROL과 ROR 함수
def ROL(a, n, w):
    mask = (1 << w) - 1  # w 비트 마스크
    return ((a << n) & mask) | (a >> (w - n))

def ROR(a, n, w):
    mask = (1 << w) - 1  # w 비트 마스크
    return (a >> n) | ((a << (w - n)) & mask)

# 일반화된 CHAM 키 스케줄 함수
def CHAM_key_schedule(mk, k, w):
    # 키의 워드 개수
    num_word = k // w
    # rk 초기화
    rk = [0] * (2 * num_word)
    
    for i in range(num_word):
        rk[i] = mk[i] ^ ROL(mk[i], 1, w) ^ ROL(mk[i], 8, w)
        rk[(i + num_word) ^ 1] = mk[i] ^ ROL(mk[i], 1, w) ^ ROL(mk[i], 11, w)
    
    # 라운드 키 반환
    return rk

# 일반화된 CHAM 암호화 함수
def CHAM_Encryption(pt, rk, r, w):
    for i in range(0, r, 2):
        # 짝수 라운드
        pt[0] = pt[0] ^ i
        temp = pt[1]
        temp = ROL(temp, 1, w)
        temp = temp ^ rk[i % len(rk)]
        pt[0] = (pt[0] + temp) & ((1 << w) - 1)  # w 비트로 제한
        pt[0] = ROL(pt[0], 8 , w) 
        temp = pt[0]

        # 시프트
        pt[0], pt[1], pt[2], pt[3] = pt[1], pt[2], pt[3], temp

        # 홀수 라운드
        pt[0] = pt[0] ^ (i + 1)
        temp = pt[1]
        temp = ROL(temp, 8 , w)
        temp = temp ^ rk[(i + 1) % len(rk)]
        pt[0] = (pt[0] + temp) & ((1 << w) - 1)
        pt[0] = ROL(pt[0], 1, w)
        temp = pt[0]

        # 시프트
        pt[0], pt[1], pt[2], pt[3] = pt[1], pt[2], pt[3], temp

    return pt

# 일반화된 CHAM 복호화 함수
def CHAM_Decryption(ct, rk, r, w):
    for i in range(r - 1, 0, -2):
        # 홀수 라운드
        ct[3] = ROR(ct[3], 1, w)
        temp = ct[0]
        temp = ROL(temp, 8 , w)
        temp = temp ^ rk[i % len(rk)]
        ct[3] = (ct[3] - temp) & ((1 << w) - 1)
        ct[3] = ct[3] ^ i
        temp = ct[3]
        
        ct[3], ct[2], ct[1], ct[0] = ct[2], ct[1], ct[0], temp
        
        # 짝수 라운드
        ct[3] = ROR(ct[3], 8 , w)
        temp = ct[0]
        temp = ROL(temp, 1, w)
        temp = temp ^ rk[(i - 1) % len(rk)]
        ct[3] = (ct[3] - temp) & ((1 << w) - 1)
        ct[3] = ct[3] ^ (i - 1)
        temp = ct[3]
        
        ct[3], ct[2], ct[1], ct[0] = ct[2], ct[1], ct[0], temp

    return ct            

# CHAM CTR 모드
def CHAM_CTR_Encryption(pt, rk, pt_len, r, w):
    
    ctr = [0x00000000] * 4 if w == 32 else [0x0000] * 4  # 카운터 값 초기화
    ct_temp = [0] * 4                   # 함수 출력 저장 배열
    input_temp = [0] * 4                # 함수 입력 저장 배열
    ct = [0] * pt_len                   # 암호문 저장 배열
    pt_block = pt_len // 4              # 평문의 블록 개수
    pt_remain = pt_len % 4              # 평문의 블록외의 남은 word 수
    
    for i in range(pt_block):
        input_temp = ctr.copy()
        ct_temp = CHAM_Encryption(input_temp, rk, r, w)
        for j in range(4):
            ct[4*i+j] = ct_temp[j] ^ pt[4*i+j]
        ctr[0] += 1
        
    input_temp = ctr.copy()
    ct_temp = CHAM_Encryption(input_temp, rk, r, w)
    for j in range(pt_remain):
        ct[4*pt_block+j] = ct_temp[j] ^ pt[4*pt_block+j]
        
    return ct

# 파일 암호화======================================================================

# hex_to_int: 입력값을 w 비트 단위로 나누어 정수로 변환
def hex_to_int(input, input_len, w):
    output = []
    for i in range(0, input_len, w // 4):  # w 비트를 16진수 자릿수로 변환 (4비트 = 1헥사 자리)
        temp = int(input[i:i + (w // 4)], 16)
        output.append(temp)
    return output

# int 형식에서 bytes 형식으로 변경하는 함수
def int_to_bytes(input, w):
    output_byte = b''
    byte_len = w // 8  # w 비트를 바이트로 변환 (8비트 = 1바이트)
    for num in input:
        temp_byte = num.to_bytes(byte_len, byteorder='big')
        output_byte += temp_byte
    return output_byte

# bytes_to_int: 바이트 데이터를 w 비트 단위로 읽어서 정수로 변환
def bytes_to_int(input_bytes, w):
    int_list = []
    byte_len = w // 8  # w 비트를 바이트로 변환 (8비트 = 1바이트)
    for i in range(0, len(input_bytes), byte_len):
        int_value = int.from_bytes(input_bytes[i:i + byte_len], byteorder='big')
        int_list.append(int_value)
    return int_list
