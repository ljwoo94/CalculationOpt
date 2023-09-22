import math, random

tab32 = [
     0,  9,  1, 10, 13, 21,  2, 29,
    11, 14, 16, 18, 22, 25,  3, 30,
     8, 12, 20, 28, 15, 17, 24,  7,
    19, 27, 23,  6, 26,  5,  4, 31]

def simulation_log2(value:int) -> int:
    value |= value >> 1
    value |= value >> 2
    value |= value >> 4
    value |= value >> 8
    value |= value >> 16
    idx = (int(value * 0x07C4ACDD) & 0xFFFF_FFFF) >> 27
    return tab32[idx]

def _zerofind(value:int) -> int:
    ret = 0
    rev = list(range(32))[::-1]
    for i in rev:
        if (value >> i) & 0x0000_0001 == 0x0000_0000:
            ret += 1
        else:
            break
    return ret

def simulation_div(x:int, D:int, N:int) -> int:
    # k = x + math.ceil(simulation_log2(D))
    k = x + (x - _zerofind(D-1)) # ceil(log2(D)) = N - LDZ(D-1)
    # a = math.ceil((2**k)/D) - (2**x)
    a = (((1 << k) + ((D>>1)+1)) // D) - (1 << x)

    # b = math.floor((N * a) / (2**x))
    b = (N * a) >> x
    # print(k, N, a, b)

    # result = math.floor((math.floor((N-b)/2) + b) / (2**(k-x-1)))
    result = (((N-b)>>1) + b) >> (k-x-1)

    return result

def random_test(tc:int) -> None:
    tc_num = tc
    fails = 0
    while tc_num:
        
        to_div = random.randint(0,255)
        divisor = random.randint(0,100000)
        divisor_round = divisor >> 1
        validate = ((to_div*255)+divisor_round) // divisor
        if (validate > 255):
            continue

        ref = ((to_div*255)+divisor_round) // divisor
        out = simulation_div(32, divisor, to_div)

        if ref != out:
            fails += 1

        tc_num -= 1
    print(f"{fails} / {tc} Failed")


if __name__ == "__main__":
    
    # for unsigned 32 bits
    x = 32
    D = 90781
    N = 178 * 255 + (D>>1)
    # D = 883
    # N = 30022
    result = simulation_div(x, D, N)
    print(result)

    random_test(100)



