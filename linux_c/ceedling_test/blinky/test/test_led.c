#ifdef TEST

#include "unity.h"

#include "led.h"
#include "mock_test_hw_memmap.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_just_for_test(void)
{
    GPIOPinWrite_Expect(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
    led_turn_on();

    led_put(3);
}

void test_led_NeedToImplement(void)
{
    /** TEST_IGNORE_MESSAGE("Need to Implement led"); */
}

#endif // TEST
