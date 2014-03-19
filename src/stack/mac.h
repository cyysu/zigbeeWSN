#ifndef MAC_H
#define MAC_H

//���70,Page120
#define aBaseSlotDuration 60//��֡˳��Ϊ0ʱ,��֡ʱ϶����ʱ��Ϊ60��������
#define aNumSuperFrameSlots 16//�κγ�֡�а�����ʱ϶��Ϊ16
#define aBaseSuperFrameDuration (aBaseSlotDuration*aNumSuperFrameSlots)//��֡����Ϊ0ʱ����ɳ�֡�ķ�����
#define aMaxBE 5//csma-ca�㷨�У��˱�ָ�������ֵ
#define aMinBE 3//����ָ����Сֵ
#define aUnitBackoffPeriod 20   //�γ�CSMA_CA�㷨����ʹ�õĻ���ʱ��εķ�����Ϊ20
#define macMaxCSMABackoffs 4//mac�����˱ܴ���
#define aMaxBeaconOverhead 75//�ű�֡���ı���ͷ
#define aMaxBeaconPayloadLength (aMaxPHYPacketSize-aMaxBeaconOverhead)//�ű�֡������Ч�غɳ���,127-75=52
#define aMaxFrameOverhead 25//����֡ͷ����,3+20+2
#define aMaxFrameResponseTime 1220
//Ϊ�ȴ���������֡����Ӧ֡�����ű�ʹ��PAN��CAP��λ�����ֵ������ʱ�ڷ��ű�PAN�е������λ
#define aMaxFrameRetries LRWPAN_MAC_MAX_FRAME_RETRIES//�ڴ���ʧ��ʱ���������ش�����,3
#define aMaxLostBeacons 4//���ջ�����������ʧ�ű�������������������ʧȥͬ��
#define aMaxMACFrameSize (aMaxPHYPacketSize-aMaxFrameOverhead)//MAC֡����󳤶�,127-25=102
#define aResponseWaitTime (32*aBaseSuperFrameDuration)//�豸��������������ڵõ���Ӧ����֮ǰ��Ҫ�ȴ�����������
//32*60*16
#define aComScheduTime (16*T2CMPVAL*10)
//default timeout on network responses
#ifdef LRWPAN_DEBUG
//give longer due to debugging output
//������mac��������ȴ�ʱ��͹����豸�ȴ�ʱ��
#define MAC_GENERIC_WAIT_TIME      MSECS_TO_MACTICKS(100)//100*(62500/1000)=6250(MACTICKS)
#define MAC_ASSOC_WAIT_TIME        MAC_GENERIC_WAIT_TIME
#define MAC_ORPHAN_WAIT_TIME       MAC_GENERIC_WAIT_TIME
#else
#define MAC_GENERIC_WAIT_TIME      MSECS_TO_MACTICKS(20)//20*(62500/1000)=1250(MACTICKS)
#define MAC_ASSOC_WAIT_TIME        MAC_GENERIC_WAIT_TIME
#define MAC_ORPHAN_WAIT_TIME       MAC_GENERIC_WAIT_TIME
#endif

//������mac���ջ������Ĵ�С��mac���ձ��ĸ��������ֵ��4��+1
#define MAC_RXBUFF_SIZE LRWPAN_MAX_MAC_RX_PKTS+1//4+1

#define MAC_SPEC_LQI_MAX 0xFF
#define MAC_RADIO_CORR_MAX 110
#define MAC_RADIO_CORR_MIN 50

#define MAC_SCAN_SIZE 16
#define MAC_SPEC_ED_MAX 0xFF
#define MAC_RADIO_RECEIVER_SATURATION_DBM       10  //dBm
#define MAC_RADIO_RECEIVER_SENSITIVITY_DBM      -91 //dBm
#define MAC_SPEC_ED_MIN_DBM_ABOVE_RECEIVER_SENSITIVITY    10
#define MAC_RADIO_RSSI_OFFSET -45
#define ED_RF_POWER_MIN_DBM   (MAC_RADIO_RECEIVER_SENSITIVITY_DBM + MAC_SPEC_ED_MIN_DBM_ABOVE_RECEIVER_SENSITIVITY)
#define ED_RF_POWER_MAX_DBM   MAC_RADIO_RECEIVER_SATURATION_DBM

#define macGetCurSlotTimes() (mac_pib.macCurrentSlotTimes)

#define macGetShortAddr()   (mac_pib.macShortAddress)
#define macSetDepth(x)      mac_pib.macDepth = x

#define macIdle() (macState == MAC_STATE_IDLE)//�ж�MAC��״̬�Ƿ�Ϊ��
#define macBusy() (macState != MAC_STATE_IDLE)//�ж�MAC��״̬�Ƿ�Ϊæ

#define macTXIdle() (!mac_pib.flags.bits.TxInProgress)
#define macTXBusy() (mac_pib.flags.bits.TxInProgress)
#define macSetTxBusy() mac_pib.flags.bits.TxInProgress = 1
#define macSetTxIdle() mac_pib.flags.bits.TxInProgress = 0

#define macDoService() \
	a_mac_service.status = LRWPAN_STATUS_MAC_INPROGRESS;\
	macState = MAC_STATE_COMMAND_START;\
	macFSM();\

//���ݷ��ͻ�����ͣ���Ҫ�ȴ�ACKʱ�������Ϣ��
//�Ժ����Ҫ����������Ϣ����Ŀ�ĵ�ַ���ŵ��ŵȣ������ϲ㴦��
#define MaxAckWindow 8
typedef struct _ACK_PENDING_TABLE {
  union {
    BYTE val;
    struct {
      unsigned Used:1;  //�����ж��Ƿ���
      unsigned ACKPending:1;  //�����ж��Ƿ���յ�ȷ��֡
    }bits;
  }options;
  BYTE currentAckRetries;//��¼������֡�Ѿ����ʹ���
  BYTE DSN;
  LRWPAN_STATUS_ENUM ErrorState;
  BYTE *Ptr;//ָ�������ݵ��׵�ַ���������·���
  UINT32 TxStartTime;//��¼�������ݵ�ʱ�䣬�Ա㳬ʱ����
}ACK_PENDING_TABLE;

//�����ŵ�ɨ���е������ͱ���ɨ��
typedef struct _SCAN_PAN_INFO {
  //BYTE CoordAddrMode;
  UINT16 CoordPANId;
  SADDR ShortAddress;
  LADDR ExtendedAddress;
  BYTE LogicalChannel;
  UINT16 SuperframeSpec;
  BYTE LinkQuality;//��ʵ��
  UINT32 TimeStamp;//��ʵ��
  BYTE ACLEntry;//��ʵ��
  BYTE StackProfile;//Ϊ�����������ӵ�
  BYTE ZigBeeVersion;//Ϊ�����������ӵ�
  BYTE Depth;//Ϊ�����������ӵ�
  union {//��ʵ��
    BYTE val;
    struct {
      unsigned GTSPermit:1;
      unsigned SecurityUse:1;
      unsigned AssociationPermit:1;
      unsigned PANCoordinator:1;
      unsigned BatteryLifeExtension:1;
      unsigned RouterRoom:1;//Ϊ'1'��ʾ�ýڵ��пռ��ROUTER��������
      unsigned RfdRoom:1;//Ϊ'1'��ʾ�ýڵ��пռ��RFD��������
    }bits;
  }options;
}SCAN_PAN_INFO;

//extern SCAN_PAN_INFO PANDesciptorList[MAC_SCAN_SIZE];


//�ŵ�ɨ������
typedef enum _SCAN_TYPE_ENUM {
  ENERGY_DETECT,
  ACTIVE,
  PASSIVE,
  ORPHAN
}SCAN_TYPE_ENUM;

//mac���PAN��Ϣ�⣬�����71
typedef struct _MAC_PIB {
  UINT32 macAckWaitDuration;//����֡�����Ӧ��֡����Ҫ�ȴ���ʱ��54��Ĭ�ϣ�-120
  union _MAC_PIB_flags {
    UINT32 val;
    struct {
      unsigned macAssociationPermit:1;//�Ƿ������豸��������
      unsigned macAutoRequest:1;//��ַ�������ű�֡�У��Ƿ��Զ�����������������
      unsigned macBattLifeExt:1;//�ű�֡��CAP�׶Σ��Ƿ񽵵ͽ��ջ�����ʱ��
      unsigned macGTSPermit:1;//�Ƿ�����GTS����
      unsigned macPromiscousMode:1;//����ģʽ���Ƿ��������������������б���
      unsigned macPanCoordinator:1;//���豸�Ƿ�ΪЭ����	
      unsigned ackPending:1;//�Ƿ���ACKδ����
      unsigned TxInProgress:1;//������Ƿ��ڹ�����
      unsigned GotBeaconResponse:1;//�Ƿ�õ��ű���Ӧ��set to a '1' when get Beacon Response
      unsigned WaitingForBeaconResponse:1; //�Ƿ��ڵȵ��ű���Ӧ��set to '1' when waiting for Response
      unsigned macPending:1;//�ڽ��ջ�������Ƿ���δ�����MAC������
      unsigned macIsAssociated:1;//�γ����磨coordinator����������磨FFD��RFD��ǰ������'0',��ʾ��������ĳ�Ա���ɹ�������'1',��ʾ������ĳ�Ա��������㴦��
      unsigned WaitingForAssocResponse:1;//�ڷ�����������ǰ������'1'�����յ���Ӧ������'0',�Է����յ������Ӧ����MAC�㴦��
      unsigned GotOrphanResponse:1;//�Ƿ�õ�������Ӧ
      unsigned WaitingForOrphanResponse:1;//�Ƿ��ڵȴ�������Ӧ
      unsigned WaitingForSyncRep:1;
      unsigned TransmittingSyncRep:1;
      unsigned TransmittingSyncReq:1;
     // unsigned TransmittingGTSReq:1;
    }bits;
  }flags;
  LADDR macCoordExtendedAddress;//Э������64λ����ַ
  SADDR macCoordShortAddress;//Э������16λ�̵�ַ
  LADDR macParentExtendedAddress;//���豸��64λ����ַ
  SADDR macParentShortAddress;//���豸��16λ�̵�ַ
  UINT16 macShortAddress;//���豸�Ķ̵�ַ
  LADDR macExtendedAddress;//���豸��64λ����ַ

  BYTE finalSlot,GTSDescriptCount,GTSDirection,macCurrentSlot;
  BYTE pendingSaddrNumber,pendingLaddrNumber;

  BOOL corGTSPermit,corAssociationPermit;
  BYTE localGTSSlot,localGTSLength;
  BOOL localdirection;

  UINT16 macPANID;//16λPAN ID
  BYTE macBeaconOrder;
  BYTE macSuperframeOrder;
  UINT32 macCurrentSlotTimes;
  BYTE macScanNodeCount;
  BYTE macDSN;//MAC������������֡���к�
  BYTE macDSNIndex;//MAC������������֡�洢��������
  BYTE macBSN;
  BYTE macDepth;//���豸�������е����
  BYTE macCapInfo;//�������������е�������Ϣ
  BYTE macMaxAckRetries;//Ӧ���ش���������
  struct  {
    unsigned maxMaxCSMABackoffs:3;//���˴���λ3�� 4
    unsigned macMinBE:2;//��С�Ļ���ָ��Ϊ2�� 0-3
  }misc;//����CSMA-CA�㷨
  UINT32 txStartTime;    //time that packet was sent
  UINT32 last_data_rx_time;//time that last data rx packet was received that was accepted by this node

  BYTE bcnDepth;//?
  SADDR bcnSADDR;//?
  UINT16 bcnPANID;//?
  BYTE bcnRSSI;//?

  BYTE currentAckRetries;//��ǰӦ���ش�����
  BYTE rxTail;             //tail pointer of rxBuff
  BYTE rxHead;             //head pointer of rxBuff
  //fifo for RX pkts, holds LRWPAN_MAX_MAC_RX_PKTS
  MACPKT  rxBuff[MAC_RXBUFF_SIZE];  //buffer for packets not yet processed��4+1

#ifdef LRWPAN_FFD
  //neighbor info
  UINT16 nextChildRFD;//��һ�����豸�Ķ̵�ַ�����ڶ̵�ַ����
  UINT16 nextChildRouter;//��һ����·�����Ķ̵�ַ�����ڶ̵�ַ����
  BYTE   ChildRFDs;         //number of neighbor RFDs
  BYTE   ChildRouters;      //number of neighbor Routers
#endif

}MAC_PIB;


//used for parsing of RX data�������յ�������
typedef struct _MAC_RX_DATA {
  MACPKT *orgpkt;//original packet
  BYTE fcflsb;//֡������ǰ��λ
  BYTE fcfmsb;//֡��������λ
  UINT16 DestPANID;//Ŀ��PAN��
  LADDR_UNION DestAddr; //dst address, either short or long
  UINT16 SrcPANID;//ԴPAN��
  LADDR_UNION SrcAddr;  //src address, either short or long
  BYTE LQI;//���ݵײ�corrֵ�������
  //BYTE ED;//���ݵײ�rssiֵ�������
  BYTE pload_offset;    //start of payload
}MAC_RX_DATA;

//���͵�����
typedef struct _MAX_TX_DATA {
	UINT16 DestPANID;//Ŀ��PAN��
	LADDR_UNION DestAddr;//dst address, either short or long
	UINT16 SrcPANID;//ԴPAN��
	SADDR SrcAddr;         //src address, either short or long, this holds short address version
	                       //if long is needed, then get this from HAL layer
	BYTE fcflsb;//frame control bits specify header bits
	BYTE fcfmsb;
	union  {
		BYTE val;
		struct _MAC_TX_DATA_OPTIONS_bits {
			unsigned gts:1;//ͨ��GTS����
			unsigned indirect:1;//��Ӵ���
		}bits;
	}options;		
}MAC_TX_DATA;

typedef struct _GTS_Allocate{
	SADDR address1;
	BYTE GTSInformation;
	BYTE gtsDirection;
}GTSAllocate;

//�����ϲ���²㴫�ݲ������Ժ�����
typedef union _MAC_ARGS {
  struct {
    LADDR DeviceAddress;
    BYTE DisassociateReason;
    BOOL SecurityEnable;
  }disassoc_req;
  union {
    struct {
      SCAN_TYPE_ENUM ScanType;
      UINT32 ScanChannels;
      BYTE ScanDuration;
    }request;
    struct {
      SCAN_TYPE_ENUM ScanType;
      UINT32 UnscanChannels;
      BYTE ResultListSize;
    }confirm;
  }scan;
  struct {
    union  {
      BYTE val;
      struct {
        unsigned PANCoordinator:1;//�Ƿ���Ϊһ���µ�PAN��Э����
        unsigned BatteryLifeExtension:1;//
        unsigned CoordRealignment:1;//
        unsigned SecurityEnable:1;//
      }bits;
    }options;
    UINT16 PANID;
    BYTE LogicalChannel;
    //UINT32 StartTime;
    BYTE BeaconOrder;
    BYTE SuperframeOrder;
  }start;
  union {
    struct {
      BYTE LogicalChannel;
      BYTE CoordAddrMode;
      UINT16 CoordPANID;
      LADDR_UNION CoordAddress;	
      BYTE SecurityEnable;
      BYTE CapabilityInformation;
    }request;
    struct {
      SADDR AssocShortAddress;

    }confirm;
  }associate;
  struct {
    BOOL SetDefaultPIB;
  }reset;
  struct {
    BYTE LogicalChannel;
  }beacon_req;
  struct {
    LRWPAN_STATUS_ENUM status;
  }error;//����״̬
  struct {
    SADDR saddr;
  }ping_node;
  struct{
    BYTE requestLength;
    BOOL requestDirection;
    BOOL requestCharacter;
  }GTSRequest;
}MAC_ARGS;



//MAC״̬�����У����ʼ��һ���Ĵ���ȴ���һ���Ĵ���ȴ��������������豸֪ͨ�������豸�ȴ���
//��������ȴ��������ű���Ӧ�����͹�����Ӧ
typedef enum _MAC_STATE_ENUM {
  MAC_STATE_IDLE,
  MAC_STATE_COMMAND_START
 } MAC_STATE_ENUM;

//MAC��������壺�������ơ�ARGS��״̬
typedef struct _MAC_SERVICE {
  LRWPAN_SVC_ENUM cmd;//����ӿ�
  MAC_ARGS args;//�����ӿ�
  LRWPAN_STATUS_ENUM status;//����״̬�ӿ�
}MAC_SERVICE;

typedef struct _MAC_SYNC_TIME{
	UINT32 ReceiveTime;UINT32 ReceiveTime_1;
	UINT32 TS1;UINT32 TS1_1;
	UINT32 TM1;UINT32 TM1_1;
	UINT32 TM2;UINT32 TM2_1;
	UINT32 TS2;UINT32 TS2_1;
	UINT32 Offset;UINT32 Offset_1;
}MAC_SYNC_TIME;

//MAC�����״̬ö��
typedef enum _MAC_RXSTATE_ENUM {
  MAC_RXSTATE_IDLE,
  MAC_RXSTATE_NWK_HANDOFF,
  MAC_RXSTATE_CMD_PENDING
} MAC_RXSTATE_ENUM;

extern MAC_SYNC_TIME mac_sync_time;
extern MAC_PIB mac_pib;
extern MAC_SERVICE a_mac_service;
extern MAC_STATE_ENUM macState;
extern MAC_TX_DATA a_mac_tx_data;
extern MAC_RX_DATA a_mac_rx_data;


void macInitAckPendingTable(void);
void macResetAckPendingTable(void);
void macInitRxBuffer(void);
void macResetRxBuffer(void);
void macInit(void);
void macFSM(void);

LRWPAN_STATUS_ENUM macInitRadio(BYTE channel, UINT16 panid);
LRWPAN_STATUS_ENUM macWarmStartRadio(void);
void macSetPANID(UINT16 panid);
void macSetChannel(BYTE channel);
void macSetShortAddr(UINT16 saddr);
//local functions
void macApplyAckTable(BYTE *ptr, BYTE flen);
void macTxData(void);
void macTxFSM(void);
static void macParseHdr(void);
void macRxFSM(void);
BOOL macRxBuffFull(void);
BOOL macRxBuffEmpty(void);
MACPKT *macGetRxPacket(void);
void macFreeRxPacket(BOOL freemem);
static BOOL macCheckLaddrSame(LADDR *laddr1,LADDR *laddr2);

static void macParseBeacon(void);

static void macFormatDiassociateRequest(void);
void macSendDisassociateRequest(void);
static BOOL macCheckDataRejection(void);
void macFormatOrphanNotify(void);

void macFormatBeaconRequest(void);
void macSendBeaconRequest(void);

//��������������
//#ifndef LRWPAN_COORDINATOR
void macFormatAssociateRequest(void);
void macSendAssociateRequest(void);
void macNetworkAssociateRequest(void);
//#endif
//���͹���ͨ������
void macSendOrfhanNotify(void);

UINT8 macRadioComputeED(INT8 rssiDbm);
BYTE macRadioComputeLQI(UINT8 corr);
void macScan(void);
BOOL macGetPANInfo(BYTE logicalchannel,BYTE beacomnum);
BOOL macGetCoordRealignInfo(void);

void macPibReset(void);

//#ifndef LRWPAN_COORDINATOR
static void macParseOrphanResponse(void);
//#endif


#ifdef LRWPAN_FFD
/*//mini
#ifdef LRWPAN_COORDINATOR
BOOL laddr_permit (LADDR *ptr);
#endif
//mini*/
void macFormatBeacon(void);
void macSendBeacon(void);
void macNetworkAssociateResponse(void);
void macFormatAssociateResponse(void);
void macSendAssociateResponse(void);
void macFormatCoordRealign(SADDR orphan_saddr);
void macSendCoordRealign(SADDR orphan_saddr);
void macOrphanNotifyResponse(void);
void macStartNewSuperframe(void);
#endif

SADDR macParseAssocResponse(void);



#endif

