#ifndef NWK_H
#define NWK_H

#define NWK_FRM_TYPE_DATA  0		//�����֡����������
#define NWK_FRM_TYPE_CMD   1		//�����֡����������
#define NWK_FRM_TYPE_MASK  3

#define NWK_IS_DATA(x) ((x & NWK_FRM_TYPE_MASK)==NWK_FRM_TYPE_DATA)		//�����֡����������
#define NWK_IS_CMD(x) ((x & NWK_FRM_TYPE_MASK)==NWK_FRM_TYPE_CMD)		//�����֡����������


#define NWK_SUPPRESS_ROUTE_DISCOVER  (0<<6)/*b7b6=00, Suppress route discovery*/
#define NWK_ENABLE_ROUTE_DISCOVER    (1<<6)/*b7b6=01, Enable route discovery*/
#define NWK_FORCE_ROUTE_DISCOVER     (2<<6)/*b7b6=10, Force route discovery*/

#define NWK_ROUTE_MASK         (3 << 6)
#define NWK_GET_ROUTE(x)       (x & NWK_ROUTE_MASK)//Get the value, which is Discover route field value


//this value of zero means that our packets will not
//be recognized as Zibee SPEC packets which is what we
//want.  Do not want these packets confused with packets
//from Zigbee compliant devices.
#define NWK_PROTOCOL           (0<<2)
#define NWK_PROTOCOL_MASK      (15 << 2)
#define NWK_GET_PROTOCOL(x)    ((x & NWK_PROTOCOL_MASK) >> 2)		//��ȡЭ��汾

#define NWK_SECURITY_MASK      (1 << 1)

#define NWK_GENERIC_RETRIES  3   //number of retries for NWK operations

#define NWK_RXBUFF_SIZE LRWPAN_MAX_NWK_RX_PKTS+1//4+1, LRWPAN_MAX_NWK_RX_PKTS=4

#define NWK_NETWORK_SIZE 16;

#define nwkIdle() (nwkState == NWK_STATE_IDLE)
#define nwkBusy() (nwkState != NWK_STATE_IDLE)

#define nwkSetLaddrNull(ptr) \
	*ptr = 0;\
	*(ptr+1) = 0;\
	*(ptr+2) = 0;\
	*(ptr+3) = 0;\
	*(ptr+4) = 0;\
	*(ptr+5) = 0;\
	*(ptr+6) = 0;\
	*(ptr+7) = 0;

#define nwkDoService() \
   a_nwk_service.status = LRWPAN_STATUS_NWK_INPROGRESS;\
   nwkState = NWK_STATE_COMMAND_START;\
   nwkFSM();

//������������Ϣ�ֶ�
typedef struct _NWK_NETWORK_DESCRIPTOR {
  UINT16 PANId;
  SADDR CoordShortAddress;
  LADDR CoordExtendedAddress;
  BYTE LogicalChannel;
  BYTE LinkQuality;
  BYTE StackProfile;
  BYTE ZigBeeVersion;
  BYTE BeaconOrder;
  BYTE SuperframeOrder;
  BYTE SecurityLevel;
  BOOL PermitJioning;
}NWK_NETWORK_DESCRIPTOR;

typedef struct _NWK_FWD_PKT {//ת�����ݵĽṹ��
   BYTE *data;  //points to top of original pkt as it sits in the heap
   BYTE nwkOffset;  //start of the nwkheader in this packet
}NWK_FWD_PKT;

typedef enum _NWK_RXSTATE_ENUM {
	NWK_RXSTATE_IDLE,	//����㷢��״̬����
	NWK_RXSTATE_START,	//����㷢��״̬��ʼ
	NWK_RXSTATE_APS_HANDOFF,//ת����Ӧ�ò㴦��
	NWK_RXSTATE_DOROUTE		//���ջ�����·��
} NWK_RXSTATE_ENUM;


typedef struct _NWK_PIB{
  union _NWK_PIB_FLAGS{
    BYTE val;
    struct {
      unsigned nwkFormed:1;//�����Ƿ��γ�
    }bits;
  }flags;
  BYTE nwkDSN;
  BYTE nwkNetworkCount;
#ifdef LRWPAN_FFD
  BYTE rxTail;  //tail pointer of rxBuff
  BYTE rxHead;  //head pointer of rxBuff
  //fifo for RX pkts, holds LRWPAN_MAX_NWK_RX_PKTS
  NWK_FWD_PKT  rxBuff[NWK_RXBUFF_SIZE];  //NWK_RXBUFF_SIZE=4+1,buffer for packets to be forwarded,4+1
#endif
}NWK_PIB;



typedef struct _NWK_RX_DATA {
	MACPKT orgpkt;
	BYTE nwkOffset;
	//parse these out of the packet for reference
	SADDR dstSADDR;
   	SADDR srcSADDR;
}NWK_RX_DATA;


typedef struct _NWK_TX_DATA {
	SADDR dstSADDR;
	BYTE  *dstLADDR;
   	SADDR srcSADDR;
	BYTE radius;
	BYTE fcflsb;
	BYTE fcfmsb;	
}NWK_TX_DATA;

typedef union _NWK_ARGS {
  union {
    struct {
      UINT32 ScanChannels;
      BYTE ScanDuration;
    }request;
    struct {
      BYTE NetworkCount;
    }confirm;
  }discovery_network;
  struct {
    UINT32 ScanChannels;
    BYTE ScanDuration;
    BYTE BeaconOrder;
    BYTE SuperframeOrder;
    UINT16 PANID;
    BOOL BatteryLifeExtension;
  }form_network;
  struct {
    UINT16 PANID;
    UINT32 ScanChannels;
    BYTE ScanDuration;
    union {
      BYTE val;
      struct {
        unsigned JoinAsRouter:1;
        unsigned RejoinNetwork:1;
        unsigned PowerSource:1;
        unsigned RxOnWhenIdle:1;
        unsigned MACSecurity:1;
      }bits;
    }options;
  }join_network;
  struct {
    LADDR DeviceAddress;//Ӧ���ǳ���ַ�Ͷ̵�ַ���У��Ժ�Ľ�,add now
  }leave_network;
  struct{			
    BYTE requestLength;
    BOOL requestDirection;
    BOOL requestCharacter;
  }GTSRequest;
}NWK_ARGS;

typedef enum _NWK_STATE_ENUM {
  NWK_STATE_IDLE,
  NWK_STATE_COMMAND_START,
  NWK_STATE_GENERIC_TX_WAIT,
  NWK_STATE_FORM_NETWORK_START,
  NWK_STATE_JOIN_NETWORK_START,
  NWK_STATE_JOIN_NETWORK_START_WAIT,
  NWK_STATE_REJOIN_NETWORK_START,
  NWK_STATE_REJOIN_WAIT,
  NWK_STATE_JOIN_SEND_BEACON_REQ,
  NWK_STATE_JOIN_NWK_WAIT1_BREQ,
  NWK_STATE_JOIN_NWK_WAIT2_BREQ,
  NWK_STATE_JOIN_MAC_ASSOC_CHANSELECT,
  NWK_STATE_JOIN_MAC_ASSOC,
  NWK_STATE_JOIN_MAC_ASSOC_WAIT,
  NWK_STATE_FWD_WAIT
} NWK_STATE_ENUM;


typedef struct _NWK_SERVICE {
  LRWPAN_SVC_ENUM cmd;
  NWK_ARGS args;
  LRWPAN_STATUS_ENUM status;
}NWK_SERVICE;

extern NWK_SERVICE a_nwk_service;
extern NWK_TX_DATA a_nwk_tx_data;
extern NWK_STATE_ENUM nwkState;
extern NWK_RX_DATA a_nwk_rx_data;
extern NWK_PIB nwk_pib;


void nwkParseHdr(BYTE *ptr);

UINT16 nwkGetHopsToDest(SADDR dstSADDR);

BOOL nwkRxBusy(void);
void nwkRxHandoff(void);



void nwkFSM(void);
void nwkInit(void);
void nwkInitNetworkDescriptorList(void);


BOOL nwkCheckLaddrNull(LADDR *ptr);
static void nwkRxFSM(void);		//��������״̬��
#ifdef LRWPAN_FFD			//�����ȫ�����豸
void nwkCopyFwdPkt(void);	//����㸴��ת�����ݰ�
static BOOL nwkRxBuffFull(void);//�������ջ������Ƿ�Ϊ�� 	
static BOOL nwkRxBuffEmpty(void);//�������ջ������Ƿ�Ϊ��
static NWK_FWD_PKT *nwkGetRxPacket(void);//�ú�������һ��ת�������������
static void nwkFreeRxPacket(BOOL freemem);//�ͷŽ��յ������ݰ�
#endif

#ifdef LRWPAN_COORDINATOR
void nwkFormNetwork(void);//�γ�����
#endif

void nwkDiscoveryNetwork(void);//��������

#ifndef LRWPAN_COORDINATOR
void nwkJoinNetwork(void);//��������
#endif

void nwkLeaveNetwork(void);//�Ͽ�����

void nwkTxData(BOOL fwdFlag);

#endif


