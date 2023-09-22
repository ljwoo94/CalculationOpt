#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>

#define DEBUG
#define SOFTMAX_OUV_INV_SCALE (255)

const int tab32[32] = {
     0,  9,  1, 10, 13, 21,  2, 29,
    11, 14, 16, 18, 22, 25,  3, 30,
     8, 12, 20, 28, 15, 17, 24,  7,
    19, 27, 23,  6, 26,  5,  4, 31};

int log2_32 (uint32_t value)
{
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    return tab32[(uint32_t)(value*0x07C4ACDD) >> 27];
}

uint8_t leading_one_detect(int num)
{
    uint8_t pos = 0;
    for (int j = 31; j >= 0; --j)
    {
        if (((num >> j) && 0x01) == 0x01)
            pos += 1;
        else
            break;
    }
    return pos;
}

uint8_t ref_div(uint8_t X, int divisor, int divisor_round)
{
    uint8_t result;
    uint32_t dividend = X * SOFTMAX_OUV_INV_SCALE + divisor_round;
    result = dividend / divisor;

#ifdef DEBUG
    printf("REF_DIVISION: \n");
    printf("    dividend: %d, divisor: %d, divisor_round: %d\n\n", dividend, divisor, divisor_round);
#endif

    return result;
}

uint8_t reciprocal_div(uint8_t X, int divisor, int divisor_round)
{
    uint8_t result = 0;
    int _x = 16;
    uint8_t RSHAMT = _x + log2_32(divisor);
    uint32_t MULT = (((long long)1 << RSHAMT)) / divisor;
    uint32_t dividend = X * SOFTMAX_OUV_INV_SCALE + divisor_round;
    result = ((dividend * (long long)MULT)) >> RSHAMT;

#ifdef DEBUG
    printf("RECIPROCAL_DIVISION: \n");
    printf("    dividend: %d, divisor: %d, divisor_round: %d\n", dividend, divisor, divisor_round);
    printf("    MULT: %d, RSHAMT: %d\n", MULT, RSHAMT);
    printf("    LOG2: %d\n", log2_32(divisor));
    printf("    LOD: %d\n\n", leading_one_detect(divisor));
#endif
    return result;
}

uint8_t my_reciprocal_div(uint8_t X, int divisor, int divisor_round)
{
    uint8_t result = 0;
    uint8_t RSHAMT = 31 - leading_one_detect(divisor);
    uint32_t MULT = (((long long)1 << RSHAMT)) / divisor;
    uint32_t dividend = X * SOFTMAX_OUV_INV_SCALE + divisor_round;
    result = ((dividend * (long long)MULT)) >> RSHAMT;

#ifdef DEBUG
    printf("RECIPROCAL_DIVISION: \n");
    printf("    dividend: %d, divisor: %d, divisor_round: %d\n", dividend, divisor, divisor_round);
    printf("    MULT: %d, RSHAMT: %d\n", MULT, RSHAMT);
    printf("    LOG2: %d\n", log2_32(divisor));
    printf("    LOD: %d\n\n", leading_one_detect(divisor));
#endif
    return result;
}

void compare_ref_rec(uint8_t X, int divisor, int divisor_round)
{
    uint8_t ref = ref_div(X, divisor, divisor_round);
    uint8_t out = reciprocal_div(X, divisor, divisor_round);
    printf("Ref: %d, Out: %d\n\n", ref, out);
}

void random_div_tests(int cnt)
{
    srand(time(NULL));   // Initialization, should only be called once.
    int to_div, divisor, divisor_round;
    uint8_t ref, out;
    int fails = 0;
    int num_tcs = cnt;
    while(num_tcs != 0)
    {
        to_div = rand() % 255;
        divisor = rand() % 100000;
        divisor_round = (divisor >> 1) + 1;
        int validate = ((to_div*SOFTMAX_OUV_INV_SCALE)+divisor_round) / divisor;
        if (validate > UINT8_MAX)
            continue;

        ref = ref_div(to_div, divisor, divisor_round);
        out = reciprocal_div(to_div, divisor, divisor_round);
        // out = my_reciprocal_div(to_div, divisor, divisor_round);
        if (ref != out)
        {   
#ifdef DEBUG
            printf("[DEBUG] : FAILED\n");
#endif
            printf("to_div: %d, divisor: %d\n", to_div, divisor);
            printf("REF: %d, OUT: %d\n", ref, out);
            fails++;
        }
        --num_tcs;
    }
    printf("(%d / %d) Failed\n", fails, cnt);
}

int main(void)
{
    // TARGET CASE
    int sum = 90781;
    int sum_round = sum >> 1;
    uint8_t X = 178;
    compare_ref_rec(X, sum, sum_round);

    sum = 12345;
    sum_round = sum >> 1;
    X = 123;
    compare_ref_rec(X, sum, sum_round);

    sum = 6748;
    sum_round = sum >> 1;
    X = 78;
    compare_ref_rec(X, sum, sum_round);


    sum = 4571;
    sum_round = sum >> 1;
    X = 245;
    compare_ref_rec(X, sum, sum_round);

    random_div_tests(100000);
    
    return 0;
}