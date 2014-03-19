#ifndef LRWPAN_COMMON_TYPES_H
#define LRWPAN_COMMON_TYPES_H

//types common across stack or multiple stack layers

//common macros
#define BITSET(var,bitno) ((var) |= (1 << (bitno)))
#define BITCLR(var,bitno) ((var) &= ~(1 << (bitno)))
#define BITTST(var,bitno) (var & (1 << (bitno)))


//�������ö��
typedef enum _LRWPAN_SVC_ENUM {
  LRWPAN_SVC_NONE,
  LRWPAN_SVC_PHY_INIT_RADIO,
  LRWPAN_SVC_PHY_TX_DATA,
  LRWPAN_SVC_PHY_ED,
  LRWPAN_SVC_PHY_SET_TRX,

  LRWPAN_SVC_MAC_GENERIC_TX,//һ���Է�������
  LRWPAN_SVC_MAC_RETRANSMIT,//�ش�
  LRWPAN_SVC_MAC_ASSOC_REQ,//��������
  LRWPAN_SVC_MAC_DISASSOC_REQ,//�Ͽ�����,add now
  LRWPAN_SVC_MAC_GTS_REQ,//GTS����
  LRWPAN_SVC_MAC_RESET_REQ,//MAC��λ
  //LRWPAN_SVC_MAC_RX_ENABLE,//ָ�����ջ�����ʱ��
  LRWPAN_SVC_MAC_SCAN_REQ,//�ŵ�ɨ��
  LRWPAN_SVC_MAC_START_REQ,//������֡����
  LRWPAN_SVC_MAC_SYNC_REQ,//��Э����ͬ��
  //LRWPAN_SVC_MAC_POLL_REQ,//����Э��������
  LRWPAN_SVC_MAC_BEACON_REQ,//�ű�����
  LRWPAN_SVC_MAC_ORPHAN_NOTIFY,//����ͨ��
  LRWPAN_SVC_MAC_ERROR,

  LRWPAN_SVC_NWK_GENERIC_TX,//����㷢������
  LRWPAN_SVC_NWK_DISC_NETWORK,//���緢��
  LRWPAN_SVC_NWK_FORM_NETWORK,//�γ�����
  LRWPAN_SVC_NWK_JOIN_NETWORK,//��������
  //LRWPAN_SVC_NWK_DIRE_JOIN_NETWORK,//ֱ�ӽ��豸ͬ��������
  LRWPAN_SVC_NWK_LEAVE_NETWORK,//�Ͽ�����,add now
  //LRWPAN_SVC_NWK_RESET,//���¸�λ�豸
  //LRWPAN_SVC_NWK_SYNC,//���ջ�ͬ��
  LRWPAN_SVC_NWK_GTS_REQ,

  LRWPAN_SVC_APS_GENERIC_TX,
  LRWPAN_SVC_APS_NWK_PASSTHRU,
  LRWPAN_SVC_APS_DO_ZEP_TX,
} LRWPAN_SVC_ENUM;


//���繤��״̬ö��
typedef enum _LRWPAN_STATUSENUM {
  LRWPAN_STATUS_SUCCESS = 0,
  LRWPAN_STATUS_TX_LOCKED,
  LRWPAN_STATUS_INVALID_REQUEST,//������Ч��add now
  LRWPAN_STATUS_INVALID_PARAMETER,//������Ч��add now
  LRWPAN_STATUS_UNKNOWN_DEVICE,//δ֪�豸��add now
  LRWPAN_STATUS_STARTUP_FAILURE,//��������ʧ��
  LRWPAN_STATUS_SEND_OVERTIME,//�����ط���ʱ

  LRWPAN_STATUS_PHY_FAILED,
  LRWPAN_STATUS_PHY_INPROGRESS,  //still working for splitphase operations
  LRWPAN_STATUS_PHY_RADIO_INIT_FAILED,
  LRWPAN_STATUS_PHY_TX_PKT_TOO_BIG,
  LRWPAN_STATUS_PHY_TX_START_FAILED,
  LRWPAN_STATUS_PHY_TX_FINISH_FAILED,
  LRWPAN_STATUS_PHY_CHANNEL_BUSY,
  LRWPAN_STATUS_PHY_TRX_OFF,
  LRWPAN_STATUS_PHY_TX_ON,
  LRWPAN_STATUS_PHY_RX_ON,
  LRWPAN_STATUS_PHY_BUSY_TX,
  LRWPAN_STATUS_PHY_BUSY_RX,

  LRWPAN_STATUS_MAC_FAILED,
  LRWPAN_STATUS_MAC_NO_ACK,
  LRWPAN_STATUS_MAC_NO_DATA,
  LRWPAN_STATUS_MAC_NOT_ASSOCIATED,
  LRWPAN_STATUS_MAC_NOT_DISASSOCIATED,
  LRWPAN_STATUS_MAC_DISABLE_TRX_FAILURE,
  LRWPAN_STATUS_MAC_INPROGRESS,  //still working for splitphase operations
  LRWPAN_STATUS_MAC_MAX_RETRIES_EXCEEDED,  //exceeded max retries
  LRWPAN_STATUS_MAC_TX_FAILED,    //MAC Tx Failed, retry count exceeded
  LRWPAN_STATUS_MAC_ASSOCIATION_TIMEOUT,  //association request timedout
  LRWPAN_STATUS_MAC_ASSOCIATION_DENY,
  LRWPAN_STATUS_MAC_ORPHAN_TIMEOUT,       //ophan notify timedout
  LRWPAN_STATUS_MAC_NO_BEACON,
  LRWPAN_STATUS_MAC_NO_SHORT_ADDRESS,

  LRWPAN_STATUS_NWK_INPROGRESS,
  LRWPAN_STATUS_NWK_JOIN_TIMEOUT,
  LRWPAN_STATUS_NWK_PACKET_UNROUTABLE,
  LRWPAN_STATUS_NWK_RADIUS_EXCEEDED,
  LRWPAN_STATUS_NWK_JOIN_NOT_PERMITTED,

  LRWPAN_STATUS_APS_INPROGRESS,
  LRWPAN_STATUS_APS_MAX_RETRIES_EXCEEDED,
  LRWPAN_STATUS_APS_ILLEGAL_ENDPOINT,
  LRWPAN_STATUS_APS_MAX_ENDPOINTS_EXCEEDED,
  LRWPAN_STATUS_INDIRECT_BUFFER_FULL,
  LRPAN_STATUS_ZEP_FAILED,
  LRPAN_STATUS_ZEPCALLBACK_FAILED,
  LRPAN_STATUS_USRCALLBACK_FAILED,
  LRWPAN_STATUS_HEAPFULL
}LRWPAN_STATUS_ENUM;


//�̵�ַ���ͣ�16λ
typedef UINT16 SADDR;
//typedef UINT16 PANID;

//these bytes ALWAYS stored in little-endian order
//����ַ���ͣ�64λ
typedef struct _LADDR {
  BYTE bytes[8];
}LADDR;

//only used to store IEEE Long Address or PAN short address
//�豸��ַ������,����ַ�Ͷ̵�ַ
typedef union _LADDR_UNION {
  LADDR laddr;
  SADDR saddr;
}LADDR_UNION;

//ZigBeeƵ�ʹ�����Χ
typedef enum _PHY_FREQ_ENUM {
  PHY_FREQ_868M=0,  //868---868.6MHz
  PHY_FREQ_RSV,
  PHY_FREQ_915M,    //902---928MHz
  PHY_FREQ_2405M    //2400---2483.5MHz
}PHY_FREQ_ENUM;

//ZigBee�豸����
typedef enum _NODE_TYPE_ENUM {
  NODE_TYPE_COORD=0,  //Э����
  NODE_TYPE_ROUTER,   //·����
  NODE_TYPE_ENDDEVICE //�豸
}NODE_TYPE_ENUM;

//used for radio initialization
/*�����ŵ�������*/
typedef union _RADIO_FLAGS {
	BYTE val;
	struct _RADIO_FLAGS_bits {
        //if true, then put radio in listen mode, which is non-auto ack, no address decoding
        unsigned listen_mode:1;
        unsigned pan_coordinator:1;   //set the pan coordinator bit
	}bits;
 }RADIO_FLAGS;

typedef struct _MACPKT {
	BYTE *data;
	INT8 rssi;
        BYTE corr;
}MACPKT;

typedef struct{
	UINT32 sec;
	UINT32 us;
}sTime;
//ʱ��� add by weimin 20071024.

#ifdef LRWPAN_COMPILER_BIG_ENDIAN
/*���ֽڴ洢��ʽ������ֵ�����λ�洢�ڵ�ַ�Ŀ�ʼ��*/
#define UINT32_LOWORD_LSB 3
#define UINT32_LOWORD_MSB 2
#define UINT32_HIWORD_LSB 1
#define UINT32_HIWORD_MSB 0
#define UINT16_LSB 1
#define UINT16_MSB 0
#else
/*С�ֽڴ洢��ʽ������ֵ�����λ�洢�ڵ�ַ��ĩβ��*/
#define UINT32_LOWORD_LSB 0
#define UINT32_LOWORD_MSB 1
#define UINT32_HIWORD_LSB 2
#define UINT32_HIWORD_MSB 3
#define UINT16_LSB 0
#define UINT16_MSB 1
#endif


#endif
