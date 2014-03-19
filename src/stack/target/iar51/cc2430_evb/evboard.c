#include "hal.h"
#include "halStack.h"
#include "evboard.h"
#include "evbConfig.h"
/*******************************************************************************
Joystick
*******************************************************************************/
#define JOYSTICK_PUSH         P2_0   //������ݸ�״̬
#define JOYSTICK_PRESSED()    JOYSTICK_PUSH
//��ʼ���Ĵ���P2DIR��P2INP,����P2_0Ϊ�������̬
#define INIT_JOYSTICK_PUSH() \
    do {                     \
        P2DIR &= ~0x01;      \
        P2INP |= 0x01;       \
    } while (0)

BOOL joystickPushed( void );

//������ݸ˷���״̬��ö��
typedef enum {
  CENTRED,
  LEFT,
  RIGHT,
  UP,
  DOWN
} JOYSTICK_DIRECTION;


#define JOYSTICK              P0_6  //���ò��ݸ�״̬
#define INIT_JOYSTICK()       IO_DIR_PORT_PIN(0, 6, IO_IN) //��ʼ��P0_6Ϊ����
#define ADC_INPUT_JOYSTICK    0x06  //����ADC��ͨ����ַ
//����P0_6

//��ȡ���ݸ˷���״̬
JOYSTICK_DIRECTION getJoystickDirection( void );

//����״̬
EVB_SW_STATE sw_state;

//��ȡ���ݸ˷���״̬���ж�����

 JOYSTICK_DIRECTION getJoystickDirection( void ) {
    INT8 adcValue, i;
    JOYSTICK_DIRECTION direction[2];


    for(i = 0; i < 2; i++){
      //����ת��
       adcValue = halAdcSampleSingle(ADC_REF_AVDD, ADC_8_BIT, ADC_INPUT_JOYSTICK);
	  //1000 0000 , 000 0000 , 0000 0110
	  //ADC_REF_AVDD���ǡ��� 20�ţ�AVDD_SOC����Ϊģ���·����2.0��3.6 V�ĵ�ѹ�
	  //=AVDD5���Ųο���ѹ,64������,ANI6,����P0_6
	  //������ͻ���˵�ѹ
       if (adcValue < 0x29) {
          direction[i] = DOWN;  // Measured 0x01
       } else if (adcValue < 0x50) {
          direction[i] = LEFT;  // Measured 0x30
       } else if (adcValue < 0x45) {
          direction[i] = CENTRED;  // Measured 0x40
       } else if (adcValue < 0x35) {
          direction[i] = RIGHT; // Measured 0x4D
       } else if (adcValue < 0x20) {
          direction[i] = UP;    // Measured 0x5C
       } else {
          direction[i] = CENTRED; // Measured 0x69
       }
    }

    if(direction[0] == direction[1]){
       return direction[0];
    }
    else{
       return CENTRED;
    }
}

//���忪�ذ���ʱ��
#define SW_POLL_TIME   MSECS_TO_MACTICKS(100)

UINT32 last_switch_poll;
/*��������:ÿһ100ms����һ�ο���״̬������������� LRWPAN_ENABLE_SLOW_TIMER��
           ��ʹ���жϣ�������ò�ѯ��ʽ*/
/*only do this if the slow timer not enabled as reading the joystick takes a while.
If the slowtimer is enabled, then that interrupt is handing polling*/
void evbPoll(void){
#ifndef LRWPAN_ENABLE_SLOW_TIMER
  if ( halMACTimerNowDelta(last_switch_poll) > SW_POLL_TIME) {
   evbIntCallback();
   last_switch_poll = halGetMACTimer();
  }
#endif

}

/*��������:�����װ��ʼ����������Ƭ��ʱ�ӡ����ڵȣ����̺�LED*/
void evbInit(void){
  halInit();
  INIT_JOYSTICK();
  sw_state.val = 0;
  INIT_LED1();
  INIT_LED2();
  INIT_LED3();
  INIT_LED4();
}

/*��������:����LED1��LED2�Ŀ���״̬*/
void evbLedSet(BYTE lednum, BOOL state) {
    switch(lednum) {
       case 1:    if (state) LED1_ON(); else LED1_OFF(); break;
       case 2:    if (state) LED2_ON(); else LED2_OFF(); break;
       case 3:    if (state) LED3_ON(); else LED3_OFF(); break;
       case 4:    if (state) LED4_ON(); else LED4_OFF(); break;
    }
}

/*��������:���LED1��LED2�Ŀ���״̬��TRUE Ϊ ����FALSE Ϊ ��*/
BOOL evbLedGet(BYTE lednum){
  switch(lednum) {
       case 1:    return(LED1_STATE());
       case 2:    return(LED2_STATE());
    }
  return(FALSE);
}


/*if joystick pushed up, consider this a S1 button press,
if joystick pushed down, consider this a S2 button press,
does not allow for both buttons to be pressed at once,
tgl bits are set if the state bits become different*/
/*��������:���Ȼ�ȡ���ݸ˵ķ����ٸ��¿���״̬�������״ֵ̬*/
void evbIntCallback(void){
  JOYSTICK_DIRECTION x;
  x = getJoystickDirection(); //xΪ���ݸ˷���
  if (x == CENTRED) {
    sw_state.bits.s1_val = 0;
    sw_state.bits.s2_val = 0;
  }
  else  if (x == UP) sw_state.bits.s1_val = 1;
  else if (x == DOWN) sw_state.bits.s2_val = 1;
  if (sw_state.bits.s1_val != sw_state.bits.s1_last_val) sw_state.bits.s1_tgl = 1;
  if (sw_state.bits.s2_val != sw_state.bits.s2_last_val) sw_state.bits.s2_tgl = 1;
  sw_state.bits.s1_last_val = sw_state.bits.s1_val;
  sw_state.bits.s2_last_val = sw_state.bits.s2_val;
}

//general interrupt callback , when this is called depends on the HAL layer.
void usrIntCallback(void)
{
}

//��������д�Ĳ���
void controlU (void)
{
    INT8 i,adcValue;
      //����ת��
       adcValue = halAdcSampleSingle(ADC_REF_AVDD, ADC_8_BIT, ADC_INPUT_JOYSTICK);
	//��ѹ��AVDD5Ϊ�ο�
       if (adcValue < 0x29) {//0010 1001
          LED1_ON();
	   LED2_OFF();
	   LED3_ON();
	   LED4_OFF();
       } else {
         LED1_OFF();
	  LED2_ON();
	  LED3_OFF();
	  LED4_ON();
       }
}




