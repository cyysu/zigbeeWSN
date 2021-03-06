#include "hal.h"
#include "halStack.h"
#include "evboard.h"
#include "evbConfig.h"
/*******************************************************************************
Joystick
*******************************************************************************/
#define JOYSTICK_PUSH         P2_0   //定义操纵杆状态
#define JOYSTICK_PRESSED()    JOYSTICK_PUSH
//初始化寄存器P2DIR和P2INP,配置P2_0为输入和三态
#define INIT_JOYSTICK_PUSH() \
    do {                     \
        P2DIR &= ~0x01;      \
        P2INP |= 0x01;       \
    } while (0)

BOOL joystickPushed( void );

//定义操纵杆方向状态，枚举
typedef enum {
  CENTRED,
  LEFT,
  RIGHT,
  UP,
  DOWN
} JOYSTICK_DIRECTION;


#define JOYSTICK              P0_6  //设置操纵杆状态
#define INIT_JOYSTICK()       IO_DIR_PORT_PIN(0, 6, IO_IN) //初始化P0_6为输入
#define ADC_INPUT_JOYSTICK    0x06  //设置ADC的通道地址
//就是P0_6

//获取操纵杆方向状态
JOYSTICK_DIRECTION getJoystickDirection( void );

//开关状态
EVB_SW_STATE sw_state;

//获取操纵杆方向状态，判断两次

 JOYSTICK_DIRECTION getJoystickDirection( void ) {
    INT8 adcValue, i;
    JOYSTICK_DIRECTION direction[2];


    for(i = 0; i < 2; i++){
      //启动转换
       adcValue = halAdcSampleSingle(ADC_REF_AVDD, ADC_8_BIT, ADC_INPUT_JOYSTICK);
	  //1000 0000 , 000 0000 , 0000 0110
	  //ADC_REF_AVDD就是　　 20脚（AVDD_SOC）：为模拟电路连接2.0～3.6 V的电压�
	  //=AVDD5引脚参考电压,64抽样率,ANI6,就是P0_6
	  //到这里就获得了电压
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

//定义开关按键时间
#define SW_POLL_TIME   MSECS_TO_MACTICKS(100)

UINT32 last_switch_poll;
/*函数功能:每一100ms更新一次开关状态变量，如果定义 LRWPAN_ENABLE_SLOW_TIMER，
           则使用中断；否则采用查询方式*/
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

/*函数功能:开发底板初始化，包括单片机时钟、串口等，键盘和LED*/
void evbInit(void){
  halInit();
  INIT_JOYSTICK();
  sw_state.val = 0;
  INIT_LED1();
  INIT_LED2();
  INIT_LED3();
  INIT_LED4();
}

/*函数功能:设置LED1或LED2的开关状态*/
void evbLedSet(BYTE lednum, BOOL state) {
    switch(lednum) {
       case 1:    if (state) LED1_ON(); else LED1_OFF(); break;
       case 2:    if (state) LED2_ON(); else LED2_OFF(); break;
       case 3:    if (state) LED3_ON(); else LED3_OFF(); break;
       case 4:    if (state) LED4_ON(); else LED4_OFF(); break;
    }
}

/*函数功能:获得LED1或LED2的开关状态，TRUE 为 开，FALSE 为 关*/
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
/*函数功能:首先获取操纵杆的方向，再更新开关状态共用体的状态值*/
void evbIntCallback(void){
  JOYSTICK_DIRECTION x;
  x = getJoystickDirection(); //x为操纵杆方向
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

//以下是我写的部分
void controlU (void)
{
    INT8 i,adcValue;
      //启动转换
       adcValue = halAdcSampleSingle(ADC_REF_AVDD, ADC_8_BIT, ADC_INPUT_JOYSTICK);
	//电压以AVDD5为参考
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




