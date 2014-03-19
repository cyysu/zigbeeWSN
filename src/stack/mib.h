/*

*2007/09/26

*/
//************************************************************
// #ifndef MIB_H
// #define MIB_H


#define MIB_BASE_OBJID_DEVDES           (2)
#define ZjgBee_DEFAULT_ANN_INTERVAL   (5000)
#define ZigBee_RESERVED_VALUE         (0x20)
#define ZigBee_DEFAULT_DEVID1         "cqupt_zigbee_001                "
#define ZigBee_DEFAULT_DEVID2	      "cqupt_zigbee_002                "
#define ZigBee_DEFAULT_DEVID3	      "cqupt_zigbee_003                "
#define ZigBee_DEFAULT_DEVID4	      "cqupt_zigbee_004                "
#define ZigBee_DEFAULT_DEVID5	      "cqupt_zigbee_005"
#define ZigBee_DEV_VER                0x01  //��ʾ����
#define ZigBee_DEFAULT_PDTAG          "AI_CONTROL                      "
#define ZigBee_DEV_TYPE               (0x01)//��ʪ��
//#define ZigBee_DEV_TYPE               (0x01)//����
//#define ZigBee_DEV_TYPE               (0x01)//����
//#define ZigBee_DEV_TYPE               (0x01)//����
//#define ZigBee_DEV_TYPE               (0x01)//����

/*------------------------------------------------------------------------------*
 *- EPA Mib const definition
 *------------------------------------------------------------------------------*/

#define MIB_NUM_FBAPP                   (5)
#define MIB_NUM_DOMAINAPP               (5)
#define MIB_NUM_LINKOBJ                 (5)

#define MIB_BASE_OBJID_MIBHDR           (1)
#define MIB_BASE_OBJID_DEVDES           (2)
#define MIB_BASE_OBJID_CLKSYNC          (3)
#define MIB_BASE_OBJID_MAXRSPTIME       (4)
#define MIB_BASE_OBJID_COMSCHEDULE      (5)
#define MIB_BASE_OBJID_DEVAPP           (6)
#define MIB_BASE_OBJID_FBAPPHDR         (7)
#define MIB_BASE_OBJID_FBAPP            (2000)
#define MIB_BASE_OBJID_DOMAINHDR        (9)
#define MIB_BASE_OBJID_DOMAINAPP        (4000)
#define MIB_BASE_OBJID_LINKOBJHDR       (8)
#define MIB_BASE_OBJID_LINKOBJ          (5000)
#define ZigBee_PIB		(10)

#define PHY_AIB_ID (1)
#define MAC_AIB_ID (2)
#define NWK_AIB_ID (3)
#define APS_AIB_ID (4)
#define All_Attribute_ID     (0xe1)
#define NODE_DESC_CLASS 10
#define NODE_POWER_DESC_CLASS 11
#define SIMPLE_CLASS 12
#define localtypeID (0x00)
#define flags_valID (0x01)
#define APS_flagsID 0x02
#define FrequencybandID 0x03
#define MACcapabilityflagsID 0x04
#define ManufacturercodeID 0x05
#define MaximumbuffersizeID 0x06
#define MaximumtransfersizeID 0x07
#define mode_valID 0x10
#define CurrentpowermodeID 0x11
#define AvailablepowersourcesID 0x12
#define source_valID 0x13
#define CurrentpowersourceID 0x14
#define CurrentpowersourcelevelID 0x15
#define endpointID 0x20
#define ApplicationprofileidentifierID 0x21
#define ApplicationdeviceidentifierID 0x22
#define S_flags_valID 0x23
#define ApplicationdeviceversionID 0x24
#define ApplicationflagsID 0x25

typedef struct {
	UINT16	obj_id;
	UINT8	res[2];
	BYTE	dev_id[32];                          // �豸��ʶ
	BYTE   pd_tag[32];                          // �豸λ��
	UINT16	 act_ShortAddress;               //��ǰ�ɲ����Ķ̵�ַ
	LADDR     act_ExtendedAddress;           // ��ǰ�ɲ���IP��ַ
	UINT8          dev_type;                        // �豸����
	UINT8          status;                          // �豸����״̬
	UINT16         dev_ver;                         // �豸�汾��
	UINT16         ann_interval;                    // �豸�������͵�ʱ����
	UINT16         ann_ver;                         // �����������İ汾��
	UINT8          dev_r_state;                     // �豸����״̬
	UINT8          dev_r_num;                       // �豸�����
	UINT16         lan_r_port;                      // �豸������Ϣ����˿�
	UINT16         max_r_num;                       // �豸����������
	BOOL	dup_tag_detected;                // �豸������λ���Ƿ��������������豸�ظ�
	UINT8          zigbeeID;						//�����豸ID
} DevDes, *PDevDes;


void  DevDes_Init(void) ;
void  MIBInit(void);
void MIBRead();
void ZigBeePIB_read(void);
void MIBWRITE();
void ZigBeePIB_WRITE();
void h2n16(UINT16 src, UINT8 dst[]) ;
void h2n32(UINT32 src, UINT8 dst[]);
void n2h32(UINT32* dst,UINT8 src[]) ;
void n2h16(UINT16* dst,UINT8 src[]) ;




