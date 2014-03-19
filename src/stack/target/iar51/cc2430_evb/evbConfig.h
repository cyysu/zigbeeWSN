#ifndef EVBCONFIG_H
#define EVBCONFIG_H

//macros taken from Chipcon EVB header file

#define FOSC 32000000    //CPUƵ��32MHz

#define LED_OFF 0  //����LED��Ϊ1
#define LED_ON  1  //����LED��Ϊ0

//I could not get LED2, LED4 to work on my CC2430EB
#define LED1          P1_0 //����LED1��P1_0
#define LED2          P1_1 //����LED2��P1_2
#define LED3          P1_2 //����LED3��P1_3 ģ��
#define LED4          P1_3 //����LED4��P1_4 ���

#define INIT_LED1()   do { LED1 = LED_OFF; IO_DIR_PORT_PIN(1, 0, IO_OUT); P1SEL &= ~0x01;} while (0)
//���� LED1����P1_0Ϊ�����P1SEL_0='0'
#define INIT_LED2()   do { LED2 = LED_OFF; IO_DIR_PORT_PIN(1, 1, IO_OUT); P1SEL &= ~0x02;} while (0)
//���� LED1����P1_3Ϊ�����P1SEL_3='0'
#define INIT_LED3()   do { LED3 = LED_OFF; IO_DIR_PORT_PIN(1, 2, IO_OUT); P1SEL &= ~0x04;} while (0)
#define INIT_LED4()   do { LED4 = LED_OFF; IO_DIR_PORT_PIN(1, 3, IO_OUT); P1SEL &= ~0x08;} while (0)


#define LED1_ON()  (LED1 = LED_ON)  //���� LED�ƿ�
#define LED2_ON()  (LED2 = LED_ON)
#define LED3_ON()  (LED3 = LED_ON)
#define LED4_ON()  (LED4 = LED_ON)



#define LED1_OFF()  (LED1 = LED_OFF)  //���� LED����
#define LED2_OFF()  (LED2 = LED_OFF)
#define LED3_OFF()  (LED3 = LED_OFF)
#define LED4_OFF()  (LED4 = LED_OFF)

#define LED1_STATE() (LED1 == LED_ON) //���� LED��״̬��'1'Ϊ����'0'Ϊ��
#define LED2_STATE() (LED2 == LED_ON)




#endif







