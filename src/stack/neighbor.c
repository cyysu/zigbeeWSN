#include "compiler.h"
#include "lrwpan_config.h"         //user configurations
#include "lrwpan_common_types.h"   //types common acrosss most files
#include "ieee_lrwpan_defs.h"
#include "memalloc.h"
#include "hal.h"
#include "halStack.h"
#include "phy.h"
#include "mac.h"
#include "neighbor.h"

NAYBORENTRY mac_nbr_tbl[NTENTRIES];//17

/*��������:ͨ����չ��ַ���ھӱ���Ѱ�Ҷ�Ӧ������ֵ������ڷ���TRUE(����ֵΪ*index),
	   ���򷵻�FALSE*/
BOOL ntFindAddressByLADDR(LADDR *ptr, BYTE *index)
{
  BYTE j,i;
  BYTE *src,*dst;
  for (j=0;j<NTENTRIES;j++) {
    if (!mac_nbr_tbl[j].options.bits.used) //��ʾδʹ�ã�������¸�j++ѭ��
      continue;
    src = &ptr->bytes[0];
    dst = &mac_nbr_tbl[j].laddr[0];
    for (i=0;i<8;i++, src++, dst++) {
      if (*src != *dst)
        break;
    }
    if (i== 8) {//�ҵ�һ�µĵ�ַ��λ����Ϣ����INDEX
      *index = j;
      break;
    }
  }
  if (j != NTENTRIES)
    return(TRUE);//����TURE���ʾ��ַ�б�����һ�µĵ�ַ
  else
    return(FALSE);
}

/*��������:ͨ���̵�ַ���ھӱ���Ѱ�Ҷ�Ӧ������ֵ������ڷ���TRUE(����ֵΪ*index),���򷵻�FALSE*/
BOOL ntFindAddressBySADDR(SADDR saddr, BYTE *index)
{
  BYTE j;
  for (j=0;j<NTENTRIES;j++) {
    if (mac_nbr_tbl[j].options.bits.used) {
      if (mac_nbr_tbl[j].saddr == saddr) {
         *index = j;
         break;
      }
    }
  }
  if (j != NTENTRIES)
    return(TRUE);
  else
    return(FALSE);
}



/*��������:ͨ���̵�ַ��Ѱ�ҳ���ַ*/
void ntSaddrToLaddr(BYTE *laddr, SADDR saddr)
{
  BYTE i,j;
  for (i=0;i<NTENTRIES;i++) {
    if (mac_nbr_tbl[i].options.bits.used) {
      if (mac_nbr_tbl[i].saddr == saddr) {
        for (j=0;j<8;j++) {
          *laddr = mac_nbr_tbl[i].laddr[j];
          laddr++;
        }
      }
    }
  }
}

/*��������:������ھӱ����Ƿ����ƥ��Ķ̵�ַ����չ��ַ��
           ����ڷ���TRUE(����ֵΪ*index),���򷵻�FALSE*/
BOOL ntCheckAddressMapEntry(BYTE *laddr, SADDR saddr, BYTE *index)
{
  BYTE j,i;
  BYTE *src,*dst;
  for (j=0;j<NTENTRIES;j++) {
    if (!mac_nbr_tbl[j].options.bits.used)
      continue;
    if (mac_nbr_tbl[j].saddr != saddr)
      continue;
    src = laddr;
    dst = &mac_nbr_tbl[j].laddr[0];
    for (i=0;i<8;i++) {
      if (*src != *dst)
        break;
      src++;
      dst++;
    }
     if (i == 8) {// we have a match
       *index = j;
       return(TRUE);
    }
  }
  return(FALSE);
}


///////�����ھӱ�͵�ַ���б䶯�������Ժ����
/*��������:���ھӱ��������µĳ���ַ�Ͷ̵�ַ,���ȸöԵ�ַ�Ƿ��Ѿ����ڣ�
	   ���ڷ���mac_addr_tbl[j].saddr;�ٿ��Ƿ��пռ䣬���޷���LRWPAN_BCAST_SADDR��
	   ���ڿռ䣬����չ��ַ�Ͷ̵�ַ����mac_addr_tbl[j]������ 0   */
SADDR ntNewAddressMapEntry(BYTE *laddr, SADDR saddr)
{
  BYTE j;
  if (ntCheckAddressMapEntry(laddr, saddr, &j)) {//entry is already in the table.
    return(mac_nbr_tbl[j].saddr);
  }
  //now find free entry in address map table
  for (j=0;j<NTENTRIES;j++) {
    if ((!mac_nbr_tbl[j].options.bits.used)||(mac_nbr_tbl[j].saddr == LRWPAN_BCAST_SADDR))
      break;
  }
  if(j==NTENTRIES)
    return(LRWPAN_BCAST_SADDR);//error, no room
  halUtilMemCopy(&mac_nbr_tbl[j].laddr[0], laddr, 8);
  mac_nbr_tbl[j].saddr = saddr;
  return(0);
}

/*
LRWPAN_MAX_DEPTH				=	5
LRWPAN_MAX_ROUTERS_PER_PARENT			=	4
LRWPAN_MAX_CHILDREN_PER_PARENT			=	17
LRWPAN_MAX_NON_ROUTER_CHILDREN			=	13
*/
/*��������:���������������depth�����Cskip(d), See Page 216 ��ʽ*/
UINT16 ntGetCskip(BYTE depth) {//����depth��ȡLRWPAN_CSKIP_X ,���ڼ���̵�ַ
  switch(depth){
  case 1: return(LRWPAN_CSKIP_1);//1446
  case 2: return(LRWPAN_CSKIP_2);//358
  case 3: return(LRWPAN_CSKIP_3);//86
  case 4: return(LRWPAN_CSKIP_4);//18
  case 5: return(LRWPAN_CSKIP_5);//0
  case 6: return(LRWPAN_CSKIP_6);//0
  case 7: return(LRWPAN_CSKIP_7);//0
  case 8: return(LRWPAN_CSKIP_8);//0
  case 9: return(LRWPAN_CSKIP_9);//0
  case 10: return(LRWPAN_CSKIP_10);//0
  }
  return(0);
}

/*��������:��ȡ��·�������豸���̵�ַ��Ӧ������������·���㷨*/
SADDR ntGetMaxSADDR(SADDR router_saddr,BYTE depth)//compute the maximum SADDR given the router_saddr and depth
{
  return(router_saddr + (ntGetCskip(depth)*(LRWPAN_MAX_ROUTERS_PER_PARENT)) + LRWPAN_MAX_NON_ROUTER_CHILDREN);
}

/*��������:��mac_nbr_tbl�У�������̵�ַ��ƥ����ھ������Ϣ*/
NAYBORENTRY *ntFindBySADDR (UINT16 saddr)
{
  NAYBORENTRY *nt_ptr;
  BYTE j;
  nt_ptr = &mac_nbr_tbl[0];
  for (j=0;j<NTENTRIES;j++,nt_ptr++) {
    if(nt_ptr->options.bits.used&&nt_ptr->saddr==saddr)
      return(nt_ptr);
  }
  return(NULL);
}

/*��������:��mac_nbr_tbl�У���������չ��ַ��ƥ����ھ������Ϣ*/
NAYBORENTRY *ntFindByLADDR (LADDR *ptr){
  NAYBORENTRY *nt_ptr;
  BYTE j,i;
  nt_ptr = &mac_nbr_tbl[0];
  for (j=0;j<NTENTRIES;j++,nt_ptr++) {
    if (!nt_ptr->options.bits.used) continue;
    for (i=0;i<8;i++) {
      if (nt_ptr->laddr[i] != ptr->bytes[i])
        break;
    }
    if (i == 8)
      return(nt_ptr);
  }
  return(NULL);
}




/*��������:ͨ���̵�ַ��ɾ�����豸���ھӱ���Ϣ�͵�ַ��Ϣ*/
BOOL ntDelNbrBySADDR (UINT16 deviceaddr)
{
  NAYBORENTRY *nt_ptr;
  BYTE i;
  nt_ptr = &mac_nbr_tbl[0];
  for (i=0;i<NTENTRIES;i++,nt_ptr++){
    if ( nt_ptr->options.bits.used&&nt_ptr->saddr==deviceaddr){
      nt_ptr->options.bits.used = 0;
      nt_ptr->saddr = LRWPAN_BCAST_SADDR;
      break;
    }
  }
  if (i==NTENTRIES)
    return (FALSE);
  else
    return (TRUE);
}

/*��������:ͨ������ַ��ɾ�����豸���ھӱ���Ϣ�͵�ַ��Ϣ��add now*/
BOOL ntDelNbrByLADDR(LADDR * ptr)
{
  NAYBORENTRY *nt_ptr;
  BYTE i,j;
  nt_ptr = &mac_nbr_tbl[0];
  for (i=0;i<NTENTRIES;i++,nt_ptr++) {
    if (!nt_ptr->options.bits.used) continue;
    for (j=0;j<8;j++) {
      if (nt_ptr->laddr[j] != ptr->bytes[j])
        break;
    }
    if (j==8) {
      nt_ptr->saddr = LRWPAN_BCAST_SADDR;
      nt_ptr->options.bits.used = 0;
      break;
    }
  }	
  if (i==NTENTRIES)
    return (FALSE);
  else
    return (TRUE);
}

/*��������:��ʼ���ھӱ�������ʼ����ַ��*/
void ntInitTable(void)
{
  NAYBORENTRY *nt_ptr;
  BYTE j;
  nt_ptr = &mac_nbr_tbl[0];
  for (j=0;j<NTENTRIES;j++,nt_ptr++) {
    nt_ptr->options.val = 0;
    nt_ptr->saddr = LRWPAN_BCAST_SADDR;
  }
}
#ifdef LRWPAN_FFD
/*��������:��ַ�����ʼ��mac_pib.ChildRFDs��mac_pib.ChildRouters��mac_pib.nextChildRFD��mac_pib.nextChildRouter
            һ�����ڼ��������Ժ�֪���Լ���������ȣ����е���*/
void ntInitAddressAssignment(void)
{
  mac_pib.ChildRFDs = 0;
  mac_pib.ChildRouters = 0;
  mac_pib.nextChildRFD=macGetShortAddr()+1+ntGetCskip(mac_pib.macDepth+1)*(LRWPAN_MAX_ROUTERS_PER_PARENT);
  mac_pib.nextChildRouter = macGetShortAddr() + 1;
}

/*��������:���ھӱ�����һ�����е�Ԫ�����ޣ����ع㲥��ַ(LRWPAN_BCAST_SADDR)��
          ��������ھӣ����ݲ������г�ʼ��������һ���µĶ̵�ַ*/
SADDR ntAddNeighbor(BYTE *ptr, BYTE capinfo)
{
  NAYBORENTRY *nt_ptr;
  BYTE j;
  BYTE *tmpptr;

  //First, find free entry in neighbor table
  nt_ptr = &mac_nbr_tbl[0];
  for (j=0;j<NTENTRIES;j++,nt_ptr++) {
    if (!nt_ptr->options.bits.used) {
      nt_ptr->options.bits.used = 1;
      nt_ptr->capinfo = capinfo;
      //nt_ptr->PANID = a_mac_rx_data.SrcPANID;
      nt_ptr->PANID = mac_pib.macPANID;
      nt_ptr->Relationship = LRWPAN_DEVRELATION_CHILD;
      nt_ptr->Depth = mac_pib.macDepth+1;
      nt_ptr->BeaconOrder = 15;
      nt_ptr->LQI = a_mac_rx_data.LQI;
      nt_ptr->LogicalChannel = phy_pib.phyCurrentChannel;
      if (LRWPAN_GET_CAPINFO_DEVTYPE(capinfo)) {//As a FFD
        nt_ptr->DeviceType = LRWPAN_DEVTYPE_ROUTER;
        if (LRWPAN_GET_CAPINFO_ALLOCADDR(capinfo)) {//����̵�ַ
          nt_ptr->saddr = mac_pib.nextChildRouter;
          mac_pib.nextChildRouter += ntGetCskip(mac_pib.macDepth+1);
        } else
          nt_ptr->saddr = LRWPAN_SADDR_USE_LADDR;
        mac_pib.ChildRouters++;
      } else {//As a RFD
        nt_ptr->DeviceType = LRWPAN_DEVTYPE_ENDDEVICE;
        if (LRWPAN_GET_CAPINFO_ALLOCADDR(capinfo)) {//����̵�ַ
     nt_ptr->saddr = mac_pib.nextChildRFD+*ptr-1;//mini
          //mini//nt_ptr->saddr = mac_pib.nextChildRFD;
          //minimac_pib.nextChildRFD++;
        } else
          nt_ptr->saddr = LRWPAN_SADDR_USE_LADDR;
        mac_pib.ChildRFDs++;
      }
      break;
    }
  }
  if (j== NTENTRIES)
    return(LRWPAN_BCAST_SADDR);//error, no room

  //now copy long addr
  tmpptr = &nt_ptr->laddr[0];
  for(j=0;j<8;j++) {
    *tmpptr = *ptr;
    tmpptr++;
    ptr++;
  }
  return(nt_ptr->saddr);
}
#endif

/*��������:���ݲ���dstSADDR����ȷ����һ��Ŀ�ĵ�ַ�����·�ɹ��ܣ���Ӧ�����������硣
	   ��dstSADDR�Ǳ��豸�Ķ̵�ַ���������ع㲥��ַ(LRWPAN_BCAST_SADDR)
	   ��dstSADDR��0����ʾ��Э�����̵�ַ����dstSADDR���ڸ������ڣ�����Э�����̵�ַ
	   ��dstSADDR�ڣ��򷵻���һ��Ŀ�ĵ�ַ�����޷�·���򷵻ع㲥��ַ(LRWPAN_BCAST_SADDR)*/
SADDR ntFindNewDst(SADDR dstSADDR)
{
  SADDR tmpSADDR;
  NAYBORENTRY *nt_ptr;
  BYTE j;
  if (dstSADDR == macGetShortAddr()) {//trying to send to myself, this is an error
    return(0xFFFF);
  }
  //if destination is coordinator, has to go to our parent
  if (dstSADDR == 0)
    return(mac_pib.macCoordShortAddress);
  // See if this destination is within my routing range
  // if not, then have to send it to my parent
#ifndef LRWPAN_COORDINATOR
  //do not check this for coordinator, as all nodes in range of coordinator.
  tmpSADDR = ntGetMaxSADDR(macGetShortAddr(),mac_pib.macDepth+1);
  if (!((dstSADDR > macGetShortAddr()) && (dstSADDR <= tmpSADDR))) { //not in my range, must go to parent.
    return(mac_pib.macCoordShortAddress);
  }
#endif
  //goes to one of my children, check out each one.	
  nt_ptr = &mac_nbr_tbl[0];
  for (j=0;j<NTENTRIES;j++,nt_ptr++) {
    if (!nt_ptr->options.bits.used)
      continue;
    if (LRWPAN_GET_CAPINFO_DEVTYPE(nt_ptr->capinfo)) {
      //router. check its range, the range is mac_pib.depth+2 because we need
      //the depth of the my child's child (grandchild).
      tmpSADDR = ntGetMaxSADDR(nt_ptr->saddr,mac_pib.macDepth+2);
      if ((dstSADDR >= nt_ptr->saddr) && (dstSADDR <= tmpSADDR)) {
        //either for my child router or one of its children.
        return(nt_ptr->saddr);
      }
    }else {
      //if for a non-router child, return
      if (dstSADDR == nt_ptr->saddr) return(nt_ptr->saddr);
    }
  }
  return(0xFFFF);//if get here, then packet is undeliverable
}


