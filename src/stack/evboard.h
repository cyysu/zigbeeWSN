#ifndef EVBOARD_H
#define EVBOARD_H

//���忪��״̬�Ĺ�����
typedef union _EVB_SW_STATE {
  BYTE val;
  struct _SW_STATE_bits {
    unsigned s1_val:1;
    unsigned s1_last_val:1;
    unsigned s1_tgl:1;
    unsigned s2_val:1;
    unsigned s2_last_val:1;
    unsigned s2_tgl:1;
  }bits;
}EVB_SW_STATE;



extern EVB_SW_STATE sw_state;


#define EVB_LED1_ON()       evbLedSet(1,TRUE)   //����LED1�ƿ�
#define EVB_LED1_OFF()     evbLedSet(1,FALSE)   //����LED1����
#define EVB_LED2_ON()       evbLedSet(2,TRUE)	//����LED2�ƿ�
#define EVB_LED2_OFF()     evbLedSet(2,FALSE)	//����LED2����
#define EVB_LED3_ON()       evbLedSet(3,TRUE)
#define EVB_LED3_OFF()     evbLedSet(3,FALSE)
#define EVB_LED4_ON()       evbLedSet(4,TRUE)
#define EVB_LED4_OFF()     evbLedSet(4,FALSE)

#define EVB_LED1_STATE()  evbLedGet(1)  //�ж�LED�Ƶ�״̬����Ϊ'1'����Ϊ'0'
#define EVB_LED2_STATE()  evbLedGet(2)

//ͨ��sw_state.bits.s1_val״̬���жϿ���S1�Ƿ��»�ſ���'1'Ϊ���£�'0'Ϊ�ſ�
#define EVB_SW1_PRESSED()     (sw_state.bits.s1_val)
#define EVB_SW1_RELEASED()    (!sw_state.bits.s1_val)

//ͨ��sw_state.bits.s2_val״̬���жϿ���S2�Ƿ��»�ſ���'1'Ϊ���£�'0'Ϊ�ſ�
#define EVB_SW2_PRESSED()     (sw_state.bits.s2_val)
#define EVB_SW2_RELEASED()    (!sw_state.bits.s2_val)



#define EVB_SW1_TOGGLED()     (sw_state.bits.s1_tgl)
#define EVB_SW1_CLRTGL()      sw_state.bits.s1_tgl=0
#define EVB_SW2_TOGGLED()     (sw_state.bits.s2_tgl)
#define EVB_SW2_CLRTGL()      sw_state.bits.s2_tgl=0

//���¿���״̬�������״̬�����������ַ�ʽ��һ����ѯ�������жϡ�
void evbPoll(void);
void evbInit(void);

//LED�ƵĿ�������
void evbLedSet(BYTE lednum, BOOL state);
//��ȡLED�ƵĿ���״̬
BOOL evbLedGet(BYTE lednum);
void controlU (void);

#endif




