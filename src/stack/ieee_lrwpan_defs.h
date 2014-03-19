#ifndef IEEE_LRWPAN_DEFS_H
#define IEEE_LRWPAN_DEFS_H

//IEEE 802.15.4 Frame definitions here

#define LRWPAN_BCAST_SADDR       0xFFFF //�̵Ĺ㲥��ַ����ʾ�����е�ǰ������ͨ���ŵ����豸�ľ���Ч
#define LRWPAN_BCAST_PANID       0xFFFF //�㲥���䷽ʽ����ʾ�����е�ǰ������ͨ���ŵ����豸�ľ���Ч
#define LRWPAN_SADDR_USE_LADDR   0xFFFE //û�з���̵�ַ��ֻ��ʹ����չ��ַ

#define LRWPAN_ACKFRAME_LENGTH 5  //ȷ��֡�ܳ���,see Page 110

#define LRWPAN_MAX_MACHDR_LENGTH 23 //MAC��֡ͷ��󳤶ȣ�see Page 102
#define LRWPAN_MAX_NETHDR_LENGTH 8  //�����֡ͷ��󳤶ȣ�see Page 195
#define LRWPAN_MAX_APSHDR_LENGTH 5  //Ӧ���Ӳ�֡ͷ��󳤶�

//���ݰ�����֡ͷ���ȣ�����Ӧ���Ӳ㣬������MAC��(�����������)��Ϊ23+8+5=36
#define LRWPAN_MAXHDR_LENGTH (LRWPAN_MAX_MACHDR_LENGTH+LRWPAN_MAX_NETHDR_LENGTH+LRWPAN_MAX_APSHDR_LENGTH)



#define LRWPAN_MAX_FRAME_SIZE 127

#define LRWPAN_FRAME_TYPE_BEACON 0  //֡����Ϊ�ű�֡
#define LRWPAN_FRAME_TYPE_DATA 1    //֡����Ϊ����֡
#define LRWPAN_FRAME_TYPE_ACK 2     //֡����Ϊȷ��֡
#define LRWPAN_FRAME_TYPE_MAC 3     //֡����ΪMAC����֡

//BYTE masks
#define LRWPAN_FCF_SECURITY_MASK 0x8    //��ȫ����λMASK
#define LRWPAN_FCF_FRAMEPEND_MASK 0x10  //֡δ������λMASK
#define LRWPAN_FCF_ACKREQ_MASK 0x20     //����ȷ�ϱ��λMASK
#define LRWPAN_FCF_INTRAPAN_MASK 0x40   //�ڲ�PAN���λMASK



#define LRWPAN_SET_FRAME_TYPE(x,f)     (x=x|f)    //����֡����
#define LRWPAN_GET_FRAME_TYPE(x)     (x&0x03)     //���֡����

//�ֱ𽫰�ȫ����λ��֡δ�����־λ������ȷ�ϱ�־λ���ڲ�PAN��־λ�� '1'
#define LRWPAN_SET_SECURITY_ENABLED(x) BITSET(x,3)
#define LRWPAN_SET_FRAME_PENDING(x)    BITSET(x,4)
#define LRWPAN_SET_ACK_REQUEST(x)      BITSET(x,5)
#define LRWPAN_SET_INTRAPAN(x)         BITSET(x,6)

//�ֱ𽫰�ȫ����λ��֡δ�����־λ������ȷ�ϱ�־λ���ڲ�PAN��־λ�� '0'
#define LRWPAN_CLR_SECURITY_ENABLED(x) BITCLR(x,3)
#define LRWPAN_CLR_FRAME_PENDING(x)    BITCLR(x,4)
#define LRWPAN_CLR_ACK_REQUEST(x)      BITCLR(x,5)
#define LRWPAN_CLR_INTRAPAN(x)         BITCLR(x,6)

//�ֱ��ȡ��ȫ����λ��֡δ�����־λ������ȷ�ϱ�־λ���ڲ�PAN��־λ
#define LRWPAN_GET_SECURITY_ENABLED(x) BITTST(x,3)
#define LRWPAN_GET_FRAME_PENDING(x)    BITTST(x,4)
#define LRWPAN_GET_ACK_REQUEST(x)      BITTST(x,5)
#define LRWPAN_GET_INTRAPAN(x)         BITTST(x,6)

//��ַģʽ
#define LRWPAN_ADDRMODE_NOADDR 0  //PAN��־���͵�ַ���򲻴���
#define LRWPAN_ADDRMODE_SADDR  2  //����16λ�̵�ַ����
#define LRWPAN_ADDRMODE_LADDR  3  //����64λ��չ��ַ����

//��ȡ������Ŀ�ĵ�ַ��Դ��ַģʽ��xΪ֡��������ֽ�
#define LRWPAN_GET_DST_ADDR(x) ((x>>2)&0x3)
#define LRWPAN_GET_SRC_ADDR(x) ((x>>6)&0x3)
#define LRWPAN_SET_DST_ADDR(x,f) (x=x|(f<<2))
#define LRWPAN_SET_SRC_ADDR(x,f) (x=x|(f<<6))

#define LRWPAN_FCF_DSTMODE_MASK   (0x03<<2)
#define LRWPAN_FCF_DSTMODE_NOADDR (LRWPAN_ADDRMODE_NOADDR<<2)
#define LRWPAN_FCF_DSTMODE_SADDR (LRWPAN_ADDRMODE_SADDR<<2)
#define LRWPAN_FCF_DSTMODE_LADDR (LRWPAN_ADDRMODE_LADDR<<2)

#define LRWPAN_FCF_SRCMODE_MASK   (0x03<<6)
#define LRWPAN_FCF_SRCMODE_NOADDR (LRWPAN_ADDRMODE_NOADDR<<6)
#define LRWPAN_FCF_SRCMODE_SADDR (LRWPAN_ADDRMODE_SADDR<<6)
#define LRWPAN_FCF_SRCMODE_LADDR (LRWPAN_ADDRMODE_LADDR<<6)

#define LRWPAN_IS_ACK(x) (LRWPAN_GET_FRAME_TYPE(x) == LRWPAN_FRAME_TYPE_ACK)	//�ж�֡���ͣ��Ƿ�Ϊȷ��֡
#define LRWPAN_IS_BCN(x) (LRWPAN_GET_FRAME_TYPE(x) == LRWPAN_FRAME_TYPE_BEACON)	//�ж�֡���ͣ��Ƿ�Ϊ�ű�֡
#define LRWPAN_IS_MAC(x) (LRWPAN_GET_FRAME_TYPE(x) == LRWPAN_FRAME_TYPE_MAC)	//�ж�֡���ͣ��Ƿ�ΪMAC����
#define LRWPAN_IS_DATA(x) (LRWPAN_GET_FRAME_TYPE(x) == LRWPAN_FRAME_TYPE_DATA)	//�ж�֡���ͣ��Ƿ�Ϊ����֡

//The ASSOC Req and Rsp are not 802 compatible as more information is
//added to these packets than is in the spec.

#ifdef IEEE_802_COMPLY
#define LRWPAN_MACCMD_ASSOC_REQ       0x01			//������������
#define LRWPAN_MACCMD_ASSOC_RSP       0x02			//������Ӧ����
#else
#define LRWPAN_MACCMD_ASSOC_REQ       0x81			//������������
#define LRWPAN_MACCMD_ASSOC_RSP       0x82			//������Ӧ����
#endif

#define LRWPAN_MACCMD_DISASSOC        0x03			//�Ͽ�����ͨ������
#define LRWPAN_MACCMD_DATA_REQ        0x04			//������������
#define LRWPAN_MACCMD_PAN_CONFLICT    0x05			//PAN ID��ͻͨ������
#define LRWPAN_MACCMD_ORPHAN          0x06			//����ͨ������
#define LRWPAN_MACCMD_BCN_REQ         0x07			//�ű���������
#define LRWPAN_MACCMD_COORD_REALIGN   0x08			//Э��������ͬ������
#define LRWPAN_MACCMD_GTS_REQ         0x09			//GTS����ͽ������
#define LRWPAN_MACCMD_SYNC_REQ 0X0A
#define LRWPAN_MACCMD_SYNC_RSP 0X0B

#define LRWPAN_MACCMD_ASSOC_REQ_PAYLOAD_LEN 2   //��������������Ч���صĳ���
#define LRWPAN_MACCMD_ASSOC_RSP_PAYLOAD_LEN 4   //������Ӧ������Ч���صĳ���

/*
//˵����һЩ������Ч���ȣ��ű�֡(�����)����
#ifdef IEEE_802_COMPLY
#define LRWPAN_MACCMD_ASSOC_REQ_PAYLOAD_LEN 2   //��������������Ч���صĳ���
#define LRWPAN_MACCMD_ASSOC_RSP_PAYLOAD_LEN 4   //������Ӧ������Ч���صĳ���
#else
#define LRWPAN_MACCMD_ASSOC_REQ_PAYLOAD_LEN 6  //has four extra bytes in it, 'magic number'
#define LRWPAN_MACCMD_ASSOC_RSP_PAYLOAD_LEN 7  //has three extra bytes, shortAddr & depth of parent
#endif
*/

#define LRWPAN_MACCMD_DISASSOC_PAYLOAD_LEN 2    //�Ͽ�����ͨ��������Ч���صĳ���

#define LRWPAN_MACCMD_BCN_REQ_PAYLOAD_LEN  1

#define DEVICE_DISASSOC_WITH_NETWORK	2//�豸ϣ����PAN�Ͽ�
#define FORCE_DEVICE_OUTOF_NETWORK	1//Э����ϣ���豸��PAN�Ͽ�

#define LRWPAN_MACCMD_COORD_REALIGN_PAYLOAD_LEN 8   //Э��������ͬ��������Ч���صĳ���


//this is only for our beacons
#ifdef LRWPAN_ZIGBEE_BEACON_COMPLY
#define LRWPAN_NWK_BEACON_SIZE (9+4)    //9 byte payload, 4 byte header
#else
#define LRWPAN_NWK_BEACON_SIZE (9+4+4) //add in an extra four-byte magic number
#endif


//�������������е�������Ϣ�����ʽ��see Page113
#define LRWPAN_ASSOC_CAPINFO_ALTPAN       0x01
#define LRWPAN_ASSOC_CAPINFO_DEVTYPE      0x02
#define LRWPAN_ASSOC_CAPINFO_PWRSRC       0x04
#define LRWPAN_ASSOC_CAPINFO_RONIDLE      0x08
#define LRWPAN_ASSOC_CAPINFO_SECURITY     0x40
#define LRWPAN_ASSOC_CAPINFO_ALLOCADDR    0x80

#define LRWPAN_GET_CAPINFO_ALTPAN(x)       BITTST(x,0)
#define LRWPAN_GET_CAPINFO_DEVTYPE(x)      BITTST(x,1)
#define LRWPAN_GET_CAPINFO_PWRSRC(x)       BITTST(x,2)
#define LRWPAN_GET_CAPINFO_RONIDLE(x)      BITTST(x,3)
#define LRWPAN_GET_CAPINFO_SECURITY(x)     BITTST(x,6)
#define LRWPAN_GET_CAPINFO_ALLOCADDR(x)    BITTST(x,7)

#define LRWPAN_SET_CAPINFO_ALTPAN(x)       BITSET(x,0)
#define LRWPAN_SET_CAPINFO_DEVTYPE(x)      BITSET(x,1)
#define LRWPAN_SET_CAPINFO_PWRSRC(x)       BITSET(x,2)
#define LRWPAN_SET_CAPINFO_RONIDLE(x)      BITSET(x,3)
#define LRWPAN_SET_CAPINFO_SECURITY(x)     BITSET(x,6)
#define LRWPAN_SET_CAPINFO_ALLOCADDR(x)    BITSET(x,7)

#define LRWPAN_CLR_CAPINFO_ALTPAN(x)       BITCLR(x,0)
#define LRWPAN_CLR_CAPINFO_DEVTYPE(x)      BITCLR(x,1)
#define LRWPAN_CLR_CAPINFO_PWRSRC(x)       BITCLR(x,2)
#define LRWPAN_CLR_CAPINFO_RONIDLE(x)      BITCLR(x,3)
#define LRWPAN_CLR_CAPINFO_SECURITY(x)     BITCLR(x,6)
#define LRWPAN_CLR_CAPINFO_ALLOCADDR(x)    BITCLR(x,7)

//BEACON defs
#define LRWPAN_BEACON_SF_ASSOC_PERMIT_MASK (1<<7)
#define LRWPAN_BEACON_SF_PAN_COORD_MASK    (1<<6)
#define LRWPAN_BEACON_SF_BATTLIFE_EXTEN_PERMIT_MASK   (1<<4)

#define LRWPAN_GET_BEACON_SF_ASSOC_PERMIT(x) ( (x) & (LRWPAN_BEACON_SF_ASSOC_PERMIT_MASK))

#define LRWPAN_BEACON_GF_GTS_PERMIT_MASK (0x80)
#define LRWPAN_BEACON_GF_GTS_NUMBER_MASK (0x07)


//Association status������״̬
#define LRWPAN_ASSOC_STATUS_SUCCESS 0
#define LRWPAN_ASSOC_STATUS_NOROOM  1
#define LRWPAN_ASSOC_STATUS_DENIED  2
#define LRWPAN_ASSOC_STATUS_MASK    3

#define LRWPAN_GET_ASSOC_STATUS(x) ((x)&LRWPAN_ASSOC_STATUS_MASK)//LRWPAN_ASSOC_STATUS_MASK=0x03





#endif
