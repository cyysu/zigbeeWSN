#ifndef PHY_H
#define PHY_H

#include "compiler.h"
#include "halStack.h"

//���18
#define aMaxPHYPacketSize 127//�����������ݵ�Ԫ��PSDU�����������Ϊ127
#define aTurnaroundTime 12//RX��TX״̬ת������ʱ��Ϊ12����������

#define MAX_TX_TRANSMIT_TIME (SYMBOLS_TO_MACTICKS(300))  //300 MACTICKS = 300*16 us
//���Ĵ���ʱ�䣬��������㴫�䳬ʱ��

//��Ƶ״̬ö�٣��رա����տ������Ϳ������շ��Ϳ���
typedef enum _RADIO_STATUS_ENUM {
  RADIO_STATUS_OFF,
  RADIO_STATUS_RX_ON,
  RADIO_STATUS_TX_ON,
  RADIO_STATUS_RXTX_ON
}RADIO_STATUS_ENUM;

//�����PAN��Ϣ�⣺����㵱ǰʹ��Ƶ�ʡ�����㵱ǰ�ŵ�(0~26)��
//�����֧�ֵ��ŵ�������㴫�书�ʡ������CCAģʽ����������ݱ�־λ��
//���俪ʼʱ�䡢��ǰ�����ָ֡��ͳ��ȡ�
//��������ݱ�־λ��һ���ֽڵı�������һ��bits����������ʹ��仺��������
typedef struct _PHY_PIB {
  PHY_FREQ_ENUM phyCurrentFrequency;        //current frequency in KHz (2405000 = 2.405 GHz)
  BYTE phyCurrentChannel;
  UINT32 phyChannelsSupported;
  BYTE phyTransmitPower;
  BYTE phyCCAMode;
  union _PHY_DATA_flags {
    BYTE val;
    struct {
     unsigned txFinished:1;    //indicates if TX at PHY level is finished...
	 unsigned txBuffLock:1;    //lock the TX buffer.
    }bits;
  }flags;
  UINT32 txStartTime;
  unsigned char *currentTxFrm;   //current frame,�׵�ַָ��֡������
  BYTE currentTxFlen;   //current TX frame length,������FCS��
}PHY_PIB;

/*
//�շ���״̬ö��
typedef enum _PHY_TRX_STATE_ENUM {
	TRX_OFF,
	RX_ON,
	TX_ON,
	FORCE_TRX_OFF
} PHY_TRX_STATE_ENUM;
*/
//������ʼ����Ƶ�ı�ǣ�LRWPAN_COMMON_TYPES_H��������Ƶ��־ RADIO_FLAGS��
typedef union _PHY_ARGS {
  struct _PHY_INIT_RADIO {
    RADIO_FLAGS radio_flags;
  }phy_init_radio_args;
  struct {
  	INT8 EnergyLevel;
  	}ed;
  /*struct {
  	PHY_TRX_STATE_ENUM state;
  	}set_trx;*/
}PHY_ARGS;


//��������LRPAN���������ARGS��LRPAN״̬��
typedef struct _PHY_SERVICE {
  LRWPAN_SVC_ENUM cmd;
  PHY_ARGS args;
  LRWPAN_STATUS_ENUM status;
}PHY_SERVICE;



//�����״̬�����С����ʼ������ȴ���
typedef enum _PHY_STATE_ENUM {
  PHY_STATE_IDLE,
  PHY_STATE_COMMAND_START,
  PHY_STATE_TX_WAIT
 } PHY_STATE_ENUM;
//�����ⲿ�����������״̬���������Ϣ�⡢��������
//��ʱ����Ļ����СΪ127
extern PHY_STATE_ENUM phyState;//��ӳ�����״̬������״̬��
extern PHY_PIB phy_pib;
extern PHY_SERVICE a_phy_service;
extern BYTE tmpTxBuff[LRWPAN_MAX_FRAME_SIZE];//Size is 127������װ�����ݣ��Ӻ���ǰ
//extern PHY_TRX_STATE_ENUM phyTRXSate;

//prototypes
void phyFSM(void);
//void phyDoService(PHY_SERVICE *ps);
void phyInit(void );

#define phyIdle() (phyState == PHY_STATE_IDLE)
#define phyBusy() (phyState != PHY_STATE_IDLE)

#define phyTxLocked()   (phy_pib.flags.bits.txBuffLock == 1)
#define phyTxUnLocked()   (phy_pib.flags.bits.txBuffLock == 0)
#define phyGrabTxLock()	phy_pib.flags.bits.txBuffLock = 1
#define phyReleaseTxLock() phy_pib.flags.bits.txBuffLock = 0

/*
#define RFSTATUS_SFD_MASK	0x02
#define phyTxBusy()		(RFSTATUS&RFSTATUS_SFD_MASK)
*/
#define phyTxBusy()		(phy_pib.flags.bits.txFinished == 0)
#define phyTxIdle()		(phy_pib.flags.bits.txFinished == 1)
#define phySetTxBusy()		phy_pib.flags.bits.txFinished = 0
#define phySetTxIdle()		phy_pib.flags.bits.txFinished = 1

/*
#define RFSTATUS_SFD_MASK	0x02
#define phyRxBusy()		(RFSTATUS&RFSTATUS_SFD_MASK)


#define phyGetTRXState()	phyTRXSate

#define phyStartTx() \
	ISTXCALN;\
	ISTXON;\
	phyTRXSate = TX_ON;

//#define phyStartTx() (ISTXONCCA)

#define phyStartRx()\
	ISRXON;\
	phyTRXSate = RX_ON;

#define phyStopTRX()\
	ISRFOFF;\
	phyTRXSate = TRX_OFF;

*/





//cannot overlap services
//make this a macro to reduce stack depth
#define phyDoService() \
  a_phy_service.status = LRWPAN_STATUS_PHY_INPROGRESS;\
  phyState = PHY_STATE_COMMAND_START;\
  phyFSM();
#endif

