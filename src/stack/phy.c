#include "compiler.h"
#include "lrwpan_config.h"         //user configurations
#include "lrwpan_common_types.h"   //types common acrosss most files
#include "ieee_lrwpan_defs.h"
#include "hal.h"
#include "halStack.h"
#include "memalloc.h"
#include "phy.h"
#include "mac.h"
#include "evboard.h"
#ifdef SECURITY_GLOBALS
#include "security.h"
#endif
extern ACK_PENDING_TABLE AckPendingTable[MaxAckWindow];

PHY_PIB phy_pib;
PHY_SERVICE a_phy_service;
PHY_STATE_ENUM phyState;

//PHY_TRX_STATE_ENUM phyTRXSate;

//static tmp space for that is used by NET, APS, MAC layers
//since only one TX can be in progress at a time, there will be
//not contention for this.
//The current frame is built up in this space, in reverse transmit order.
BYTE tmpTxBuff[LRWPAN_MAX_FRAME_SIZE];//Size is 127������װ�����ݣ��Ӻ���ǰ

/*��������:������ʼ��*/
void phyInit(void ) {
  MemInit();  //��ʼ���洢��
  phyState = PHY_STATE_IDLE;
  phy_pib.flags.val = 0;//TX is idle,is unlocked
}
/*
void phySetTRXState(void)
{
	PHY_TRX_STATE_ENUM state;
	state = a_phy_service.args.set_trx.state;
	switch (state) {
		case RX_ON:
			if (phyGetTRXState()==RX_ON) {
				a_phy_service.status = LRWPAN_STATUS_PHY_RX_ON;
			} else if (phyTxBusy()) {
				a_phy_service.status = LRWPAN_STATUS_PHY_BUSY_TX;
			} else {
				phyStartRx();//�򿪽��ջ�
				a_phy_service.status = LRWPAN_STATUS_SUCCESS;
			}
		break;
		case TX_ON:
			if (phyGetTRXState()==TX_ON) {
				a_phy_service.status = LRWPAN_STATUS_PHY_TX_ON;
			} else if (phyRxBusy()) {
				a_phy_service.status = LRWPAN_STATUS_PHY_BUSY_RX;
			} else {
				phyStartTx();//�򿪷����
				a_phy_service.status = LRWPAN_STATUS_SUCCESS;
			}
		break;
		case TRX_OFF:
			if (phyGetTRXState()==TRX_OFF) {
				a_phy_service.status = LRWPAN_STATUS_PHY_TRX_OFF;
			} else if (phyTxBusy()) {
				a_phy_service.status = LRWPAN_STATUS_PHY_BUSY_TX;
			} else if (phyRxBusy()) {
				a_phy_service.status = LRWPAN_STATUS_PHY_BUSY_RX;
			} else {
				phyStopTRX();//�ر��շ���
				a_phy_service.status = LRWPAN_STATUS_SUCCESS;
			}
		break;
		case FORCE_TRX_OFF:
			phyStopTRX();//�ر��շ���
			a_phy_service.status = LRWPAN_STATUS_SUCCESS;
		break;
		default: break;
	}
}
*/
//call back from HAL to here, can be empty functions
//not needed in this stack
void phyRxCallback(void) {
}

/*��������:����㷢������ʱ����¼��ʱMACʱ�䣬���ڳ�ʱ����������*/
void phyTxStartCallBack(BYTE *ptr) {
  BYTE i;
  BYTE flen;
  BYTE *memptr;

  mac_pib.txStartTime = halGetMACTimer();//n*16us
  phy_pib.txStartTime = halGetMACTimer();
  if (!LRWPAN_GET_ACK_REQUEST(*(ptr+1))) {
    //mac_pib.flags.bits.ackPending = 1;  //we are requesting an ack for this packet
    //record the time of this packet
    //mac_pib.tx_start_time = halISRGetMACTimer();

    return;
  }
  for (i=0;i<MaxAckWindow;i++) {
    if (!AckPendingTable[i].options.bits.Used) {
      //DEBUG_STRING(DBG_INFO,"Apply a ack pending table.\n");
      mac_pib.macDSNIndex = i;
      AckPendingTable[i].options.bits.Used = 1;
      AckPendingTable[i].options.bits.ACKPending = 1;
      AckPendingTable[i].DSN = mac_pib.macDSN;
      AckPendingTable[i].currentAckRetries = mac_pib.macMaxAckRetries-1;
      AckPendingTable[i].TxStartTime = mac_pib.txStartTime;
      flen = *ptr;
      memptr = MemAlloc(flen);
      if (memptr==NULL) {
        AckPendingTable[i].options.bits.Used = 0;
        //DEBUG_STRING(DBG_ERR,"The mem has no room for applying for a AckPendingTable!\n");
        break;
      }
      AckPendingTable[i].Ptr = memptr;
      while (flen) {
        *(memptr) = *ptr;
        memptr++;
        ptr++;
        flen--;
      }
      break;
    }

  }

  /*
  //���û�пռ�����ڴ棬����ϵͳ�����Ժ�ᴦ��Ŀǰ����ķ�ʽ����֮������Ϊ������ACK����֡
  if (i==MaxAckWindow)
    conPrintROMString("The AckPendingTable has no room!\n");
*/
}

/*��������:��־λtxFinished�� 1 ����ʾ�������*/
void phyTxEndCallBack(void) {
//TX is finished.
  phySetTxIdle();
}


/*��������:������״̬ת��*/
void phyFSM(void) {
	
	
  //do evbpolling here
  evbPoll();			//ÿ100ms,����һ�ο���״̬������

  //check background tasks here

  switch (phyState) {
  case PHY_STATE_IDLE:
    halIdle();  //Hal Layer might want to do something in idle state
    break;
  case PHY_STATE_COMMAND_START:
    switch(a_phy_service.cmd) {
    case LRWPAN_SVC_PHY_INIT_RADIO: //not split phase
      a_phy_service.status = halInitRadio(phy_pib.phyCurrentFrequency,
                                                  phy_pib.phyCurrentChannel,
                                                  a_phy_service.args.phy_init_radio_args.radio_flags
                                                    );
	   phyState = PHY_STATE_IDLE;
       break;
    case LRWPAN_SVC_PHY_TX_DATA:
        phySetTxBusy();
        a_phy_service.status =
           halSendPacket(phy_pib.currentTxFlen,
                         phy_pib.currentTxFrm);
        phyReleaseTxLock();//ԭ��Э��ջ�У����ڷ��͵ȴ����ͷŷ��ͻ����˴�����������ͷ�

        //phyState = PHY_STATE_IDLE;
        phyState = PHY_STATE_TX_WAIT;
        break;
    case LRWPAN_SVC_PHY_ED:
      /*if (phyGetTRXState()==TX_ON) {
      a_phy_service.status = LRWPAN_STATUS_PHY_TX_ON;
      break;
    }
      if (phyGetTRXState()==TRX_OFF) {
      a_phy_service.status = LRWPAN_STATUS_PHY_TRX_OFF;
      break;
    }*/
      //a_phy_service.args.ed.EnergyLevel = 0x00;//Ӧ�õõ�����ˮƽ
      a_phy_service.args.ed.EnergyLevel = RSSI;      //CC2530(RSSIL>RSSI)
      if (a_phy_service.args.ed.EnergyLevel == -128)
        a_phy_service.status = LRWPAN_STATUS_PHY_FAILED;
      else
        a_phy_service.status = LRWPAN_STATUS_SUCCESS;
      //conPrintUINT8(a_phy_service.args.ed.EnergyLevel);
      //a_phy_service.args.ed.EnergyLevel += 0x80;
      phyState = PHY_STATE_IDLE;
      break;
    case LRWPAN_SVC_PHY_SET_TRX:
      //phySetTRXState();	
      phyState = PHY_STATE_IDLE;
      break;
    default: break;
    }//end switch cmd
    break;
  case PHY_STATE_TX_WAIT:  //wait for TX out of radio to complete or timeout
    if (phyTxIdle()){
        phyState = PHY_STATE_IDLE;
     }
    else if  (halMACTimerNowDelta(phy_pib.txStartTime) > MAX_TX_TRANSMIT_TIME){
      //should not happen, indicate an error to console
      //DEBUG_STRING(DBG_ERR,"PHY: MAX_TX_TRANSMIT_TIME timeout\n");
      a_phy_service.status = LRWPAN_STATUS_PHY_TX_FINISH_FAILED;
      //no action for now, will see if this happens
      phySetTxIdle();
      phyState = PHY_STATE_IDLE;
    }
    break;
  default: break;
  }
}

