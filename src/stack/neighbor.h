#ifndef NEIGHBOR_H
#define NEIGHBOR_H

#define LRWPAN_DEVTYPE_COORDINATOR 0x00
#define LRWPAN_DEVTYPE_ROUTER 0x01
#define LRWPAN_DEVTYPE_ENDDEVICE 0x02

#define LRWPAN_DEVRELATION_PARENT 0x00
#define LRWPAN_DEVRELATION_CHILD 0x01
#define LRWPAN_DEVRELATION_SIBING 0x02

//�����ھӱ�ṹ��,see page 214,Э���������豸�����豸����Ϣ��mac����Ϣ��
typedef struct _NAYBORENTRY {
  UINT16 saddr;//16λ�̵�ַ	
  BYTE laddr[8];//64λ����ַ
  BYTE capinfo;//�ڵ����ܵ���Ϣ
  UINT16 PANID;
  BYTE DeviceType;
  BYTE Relationship;
  BYTE Depth;
  BYTE BeaconOrder;
  BYTE LQI;//link quality indictor//LQIֵ�Ǽ��������ھӽ����·�ĳɱ�?	
  BYTE LogicalChannel;
  union {
    BYTE val;
    struct {
      unsigned used: 1;  //true if used//����Ϊ1���ʾʹ�ø�ָ��
      unsigned RxOnWhenIdle: 1;
      unsigned PermitJoining: 1;
      unsigned PotentialParent: 1;
    }bits;
  }options;
}NAYBORENTRY;

#define NTENTRIES (LRWPAN_MAX_CHILDREN_PER_PARENT)//17,holds the neighbor table entries.
extern NAYBORENTRY mac_nbr_tbl[NTENTRIES];//�����ⲿ����MAC�ھӱ�

UINT16 ntGetCskip(BYTE depth);
SADDR ntGetMaxSADDR(SADDR router_saddr,BYTE depth);//������Ķ̵�ַ
SADDR ntNewAddressMapEntry(BYTE *laddr, SADDR saddr);//�µĵ�ַӳ�����
BOOL ntFindAddressBySADDR(SADDR saddr, BYTE *index);//ͨ���̵�ַѰַ,�ҵ��ھӱ��е�����ֵ
BOOL ntFindAddressByLADDR(LADDR *ptr, BYTE *index);//ͨ������ַѰַ,�ҵ��ھӱ��е�����ֵ
void ntSaddrToLaddr(BYTE *laddr, SADDR saddr);//ͨ���̵�ַ��Ѱ�ҳ���ַ
NAYBORENTRY *ntFindBySADDR (UINT16 saddr);//����̵�ַѰַָ��
NAYBORENTRY *ntFindByLADDR (LADDR *ptr);//���峤��ַѰַָ��
BOOL ntDelNbrBySADDR (UINT16 saddr);//ͨ���̵�ַ��ɾ�����豸���ھӱ���Ϣ�͵�ַ��Ϣ
BOOL ntDelNbrByLADDR (LADDR *ptr);//ͨ������ַ��ɾ�����豸���ھӱ���Ϣ�͵�ַ��Ϣ
void ntInitTable(void);//��ʼ���ھӱ�
SADDR ntFindNewDst(SADDR dstSADDR);//Ѱ���µ�Ŀ���ַ
#ifdef LRWPAN_FFD
void ntInitAddressAssignment(void);//��ʼ����ַ����
SADDR ntAddNeighbor(BYTE *ptr, BYTE capinfo);//����ھ�
#endif

#endif

