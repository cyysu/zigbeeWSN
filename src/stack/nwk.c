#include "compiler.h"
#include "lrwpan_config.h"         //user configurations
#include "lrwpan_common_types.h"   //types common acrosss most files
#include "ieee_lrwpan_defs.h"
#include "memalloc.h"
#include "neighbor.h"
#include "hal.h"
#include "halStack.h"
#include "phy.h"
#include "mac.h"
#include "nwk.h"
#include "aps.h"
#include "neighbor.h"
#ifdef SECURITY_GLOBALS
#include "security.h"
#endif

NWK_RXSTATE_ENUM nwkRxState;
NWK_NETWORK_DESCRIPTOR NetworkDescriptorList[16];
NWK_SERVICE a_nwk_service;
NWK_STATE_ENUM nwkState;
NWK_RX_DATA a_nwk_rx_data;
NWK_TX_DATA a_nwk_tx_data;
BYTE nwkDSN;
NWK_PIB nwk_pib;

extern BYTE EnergyDetectList[MAC_SCAN_SIZE];
extern SCAN_PAN_INFO PANDescriptorList[MAC_SCAN_SIZE];



/*
//locals
#ifndef LRWPAN_COORDINATOR		//�������Э������ִ������Ĵ���
static UINT32 nwk_utility_timer;   //utility timer
static UINT8 nwk_retries;       //utility retry counter
#endif
*/


/*��������:�������ز�����ʼ��*/
void nwkInit(void){	
  nwkDSN = 0;		
  nwk_pib.flags.val = 0;
  nwkState = NWK_STATE_IDLE;
  nwkRxState= NWK_RXSTATE_IDLE;
#ifdef LRWPAN_FFD
  nwk_pib.rxTail = 0;	//��ȫ�����豸�Ľ��նѵĳ�ʼ��
  nwk_pib.rxHead = 0;
#endif
  nwkInitNetworkDescriptorList();
}

void nwkInitNetworkDescriptorList(void)
{
  BYTE i;
  nwk_pib.nwkNetworkCount = 0;
  for (i=0;i<16;i++) {
    NetworkDescriptorList[i].LinkQuality = 0;
  }
}

BOOL nwkCheckLaddrNull(LADDR *ptr)
{
  BYTE i;
  for (i=0;i<8;i++) {
    if (ptr->bytes[i]!=0)
      break;
  }
  if (i==8)
    return TRUE;
  else
    return FALSE;
}

/*��������:�����״̬������*/
void nwkFSM(void){

  macFSM();//����mac״̬��
  nwkRxFSM();//���ý���״̬��

  switch (nwkState) {
  case NWK_STATE_IDLE://see if we have packets to forward and can grab the TX buffer
#ifdef LRWPAN_FFD
    if (!nwkRxBuffEmpty() && phyTxUnLocked()) { //grab the lock and forward the packet
      phyGrabTxLock();
      nwkCopyFwdPkt();//transmit it
      nwkTxData(TRUE); //use TRUE as this is a forwarded packet
    }
#endif
    break;
  case NWK_STATE_COMMAND_START:

    switch(a_nwk_service.cmd) {
    case LRWPAN_SVC_NWK_GENERIC_TX:
      nwkTxData(FALSE);	//����ת���������Լ�������һ�����籨��
      nwkState = NWK_STATE_IDLE;
      break;
    case LRWPAN_SVC_NWK_GTS_REQ:
      a_mac_service.cmd=LRWPAN_SVC_MAC_GTS_REQ;
      a_mac_service.args.GTSRequest.requestCharacter=a_nwk_service.args.GTSRequest.requestCharacter;
      a_mac_service.args.GTSRequest.requestDirection=a_nwk_service.args.GTSRequest.requestDirection;
      a_mac_service.args.GTSRequest.requestLength=a_nwk_service.args.GTSRequest.requestLength;
      nwkState = NWK_STATE_IDLE;
      macDoService();
      break;
    case LRWPAN_SVC_NWK_DISC_NETWORK:
      nwkDiscoveryNetwork();
      nwkState = NWK_STATE_IDLE;
      break;
#ifdef LRWPAN_COORDINATOR
    case LRWPAN_SVC_NWK_FORM_NETWORK:
      //ֻ��Э�������������Ĺ��ܣ����򷵻�"LRWPAN_STATUS_INVALID_REQ"��break
      if (!mac_pib.flags.bits.macPanCoordinator) {
        a_nwk_service.status = LRWPAN_STATUS_INVALID_REQUEST;
        //DEBUG_STRING(DBG_ERR, "Network formed error, it is not a PAN Coordinator!\n");
        nwkState = NWK_STATE_IDLE;
        break;
      }
      a_nwk_service.status = LRWPAN_STATUS_SUCCESS;
      //DEBUG_STRING(DBG_INFO, "Network formed, start to form network!\n");
      nwkFormNetwork();
      nwkState = NWK_STATE_IDLE;
      break;
#else
    case LRWPAN_SVC_NWK_JOIN_NETWORK:
      if (mac_pib.flags.bits.macIsAssociated) {
        a_nwk_service.status = LRWPAN_STATUS_INVALID_REQUEST;
        //DEBUG_STRING(DBG_ERR, "Network joined error, it has been associated!\n");
        nwkState = NWK_STATE_IDLE;
        break;
      }
      //DEBUG_STRING(DBG_INFO,"Network joined, start to join the network!\n");
      nwkJoinNetwork();
      nwkState = NWK_STATE_IDLE;	
      break;
		
#endif
    case LRWPAN_SVC_NWK_LEAVE_NETWORK:
      if (!mac_pib.flags.bits.macIsAssociated) {
        a_nwk_service.status = LRWPAN_STATUS_INVALID_REQUEST;
        //DEBUG_STRING(DBG_ERR, "Network leaved error, it is not associated!\n");
        nwkState = NWK_STATE_IDLE;
        break;
      }
      //DEBUG_STRING(DBG_INFO, "Network leaved, start to leave the network!\n");
      nwkLeaveNetwork();
      nwkState = NWK_STATE_IDLE;
      break;
    default: break;
    }
  default:  break;
  }
}

//Add the NWK header, then send it to MAC
//if fwdFlag is true, then packet is being forwarded, so nwk header
//is already in place, and assume that currentTxFrm and currentTxPLen
//are correct as well, and that the radius byte has been decremented.
/*��������:����㷢�����ݣ����豸δ���������㲥�뾶Ϊ0����������MAC����*/
void nwkTxData(BOOL fwdFlag) {
  //if we are not associated, don't bother sending NWK packet
  if (!mac_pib.flags.bits.macIsAssociated) {
    //call a dummy service that just returns an error code
    //have to do it this way since the caller is expecting a mac service call
    a_mac_service.args.error.status = LRWPAN_STATUS_MAC_NOT_ASSOCIATED;
    a_mac_service.cmd = LRWPAN_SVC_MAC_ERROR;
    goto nwkTxData_sendit;
  }
  if (a_nwk_tx_data.radius == 0) {//�㲥�뾶Ϊ0������
    //DEBUG_STRING(DBG_ERR,"Nwk Radius is zero, discarding packet.\n");
    //can no longer forward this packet.
    a_mac_service.args.error.status =  LRWPAN_STATUS_NWK_RADIUS_EXCEEDED;
    a_mac_service.cmd = LRWPAN_SVC_MAC_ERROR;
    goto nwkTxData_sendit;
  }
  if (fwdFlag)
    goto nwkTxData_addmac;//fwdFlagΪ1����ʾҪת���ñ���
  //sequence number
  --phy_pib.currentTxFrm;
  *phy_pib.currentTxFrm = nwkDSN;
  nwkDSN++;
  //radius, decrement before sending, receiver will get a value that is one less than this node.
  --phy_pib.currentTxFrm;
  *phy_pib.currentTxFrm = (--a_nwk_tx_data.radius);
  //src address
  --phy_pib.currentTxFrm;
  *phy_pib.currentTxFrm = (BYTE) (a_nwk_tx_data.srcSADDR >> 8);
  --phy_pib.currentTxFrm;
  *phy_pib.currentTxFrm = (BYTE) (a_nwk_tx_data.srcSADDR);
  //dst address
  --phy_pib.currentTxFrm;
  *phy_pib.currentTxFrm = (BYTE) (a_nwk_tx_data.dstSADDR >> 8);
  --phy_pib.currentTxFrm;
  *phy_pib.currentTxFrm = (BYTE) (a_nwk_tx_data.dstSADDR);

  //frame control
  --phy_pib.currentTxFrm;
  *phy_pib.currentTxFrm = a_nwk_tx_data.fcfmsb;
  --phy_pib.currentTxFrm;
  *phy_pib.currentTxFrm = a_nwk_tx_data.fcflsb;
  //network header is fixed size
  phy_pib.currentTxFlen +=  8;

nwkTxData_addmac:
  //Ϊ�˷��㣬���������޸���һ�㣬�Ժ������˽�
  /*
  //fill in the MAC fields. For now, we don't support inter-PAN,so the PANID has to be our mac PANID	
  //��֧�ֿ����δ��ͣ�Ŀ�ĺ�Դ����ID�����Լ�������ID
  a_mac_tx_data.DestPANID = mac_pib.macPANID;
  a_mac_tx_data.SrcPANID = mac_pib.macPANID;
  if (a_nwk_tx_data.dstSADDR == LRWPAN_SADDR_USE_LADDR ){//������͵�Ŀ�ĵ�ַΪ0xFFFE
    //long address is specified from above.  We assume they know where they are going no routing necessary
    //a_mac_tx_data.fcfmsb = LRWPAN_FCF_DSTMODE_LADDR|LRWPAN_FCF_SRCMODE_LADDR;
    //��fcfmsb�ֽ���Ϊ11001100��Ŀ�ĵ�ַ��Դ��ַ��Ϊ����ַ
    //copy in the long address
    //fill it in.�Ժ����
    if (macGetShortAddr()==LRWPAN_SADDR_USE_LADDR)
      a_mac_tx_data.fcfmsb = LRWPAN_FCF_DSTMODE_LADDR|LRWPAN_FCF_SRCMODE_LADDR;
    else
      a_mac_tx_data.fcfmsb = LRWPAN_FCF_DSTMODE_LADDR|LRWPAN_FCF_SRCMODE_SADDR;
    halUtilMemCopy(&a_mac_tx_data.DestAddr.laddr.bytes[0], a_nwk_tx_data.dstLADDR, 8);
  } else {//������͵�Ŀ�ĵ�ַ��Ϊ0xFFFE
    //lets do some routing��Ҫ·��
#ifdef LRWPAN_RFD	//����Ǿ������豸��ִ�����µĲ���
    //RFD's are easy. Always send to parent, our SRC address is always long
    //so that parent can confirm that the RFD is still in their neighbor table
    //will use the parent short address
    //fcfmsb�ֽ���Ϊ11001000��Ŀ�ĵ�ַΪ�̵�ַ��Դ��ַΪ����ַ
    //a_mac_tx_data.fcfmsb = LRWPAN_FCF_DSTMODE_SADDR|LRWPAN_FCF_SRCMODE_LADDR;
    //fill it in.�Ժ����
    if (macGetShortAddr()==LRWPAN_SADDR_USE_LADDR)
      a_mac_tx_data.fcfmsb = LRWPAN_FCF_DSTMODE_SADDR|LRWPAN_FCF_SRCMODE_LADDR;
    else
      a_mac_tx_data.fcfmsb = LRWPAN_FCF_DSTMODE_SADDR|LRWPAN_FCF_SRCMODE_SADDR;
    //16λĿ�ĵ�ַ��Э�����Ķ̵�ַ
    a_mac_tx_data.DestAddr.saddr = mac_pib.macCoordShortAddress;
#else		//�����ȫ�����豸
    {
      SADDR newDstSADDR;
      //this is router. need to determine the new dstSADDR
      newDstSADDR = a_nwk_tx_data.dstSADDR; //default
      DEBUG_STRING(DBG_INFO,"Routing pkt to: ");
      DEBUG_UINT16(DBG_INFO,newDstSADDR);
      if (a_nwk_tx_data.dstSADDR != LRWPAN_BCAST_SADDR) {
        //not broadcast address
        newDstSADDR = ntFindNewDst(a_nwk_tx_data.dstSADDR);
        DEBUG_STRING(DBG_INFO," through: ");
        DEBUG_UINT16(DBG_INFO,newDstSADDR);
        if (newDstSADDR == LRWPAN_BCAST_SADDR) {
          DEBUG_STRING(DBG_INFO,", UNROUTABLE, error!\n ");
          //error indicator. An unroutable packet from here.
          a_mac_service.args.error.status = LRWPAN_STATUS_NWK_PACKET_UNROUTABLE;
          a_mac_service.cmd = LRWPAN_SVC_MAC_ERROR;
          goto nwkTxData_sendit;
        }
        DEBUG_STRING(DBG_INFO,"\n");
      }
      //fill it in.�Ժ����
      if (macGetShortAddr()==LRWPAN_SADDR_USE_LADDR)
        a_mac_tx_data.fcfmsb = LRWPAN_FCF_DSTMODE_SADDR|LRWPAN_FCF_SRCMODE_LADDR;
      else
        a_mac_tx_data.fcfmsb = LRWPAN_FCF_DSTMODE_SADDR|LRWPAN_FCF_SRCMODE_SADDR;
      a_mac_tx_data.DestAddr.saddr = newDstSADDR;//Ŀ�ĵ�ַ
    }
#endif
  }
  //for data frames, we want a MAC level ACK, unless it is a broadcast.
  //���˹㲥���������������֡���Ƕ�ϣ���ܵõ�һ��mac���ȷ��֡
  if ( ((LRWPAN_GET_DST_ADDR(a_mac_tx_data.fcfmsb)) == LRWPAN_ADDRMODE_SADDR) &&
      a_mac_tx_data.DestAddr.saddr == LRWPAN_BCAST_SADDR) {
        //no MAC ACK
        a_mac_tx_data.fcflsb = LRWPAN_FRAME_TYPE_DATA|LRWPAN_FCF_INTRAPAN_MASK;
      }else {
        a_mac_tx_data.fcflsb = LRWPAN_FRAME_TYPE_DATA|LRWPAN_FCF_INTRAPAN_MASK |LRWPAN_FCF_ACKREQ_MASK;
      }
  //send it.
  a_mac_service.cmd = LRWPAN_SVC_MAC_GENERIC_TX;
  */
  ////////////////////////////////////////////////////////////////////////////////////����Ϊ�¼ӵ�
  a_mac_tx_data.DestPANID = mac_pib.macPANID;
  a_mac_tx_data.SrcPANID = mac_pib.macPANID;
  if (a_nwk_tx_data.dstSADDR == LRWPAN_SADDR_USE_LADDR ){
    if (macGetShortAddr()==LRWPAN_SADDR_USE_LADDR) {
      a_mac_tx_data.fcfmsb = LRWPAN_FCF_DSTMODE_LADDR|LRWPAN_FCF_SRCMODE_LADDR;
    } else {
      a_mac_tx_data.fcfmsb = LRWPAN_FCF_DSTMODE_LADDR|LRWPAN_FCF_SRCMODE_SADDR;
    }
    halUtilMemCopy(&a_mac_tx_data.DestAddr.laddr.bytes[0], a_nwk_tx_data.dstLADDR, 8);
  } else {
    if (macGetShortAddr()==LRWPAN_SADDR_USE_LADDR)
      a_mac_tx_data.fcfmsb = LRWPAN_FCF_DSTMODE_SADDR|LRWPAN_FCF_SRCMODE_LADDR;
    else
      a_mac_tx_data.fcfmsb = LRWPAN_FCF_DSTMODE_SADDR|LRWPAN_FCF_SRCMODE_SADDR;
   // a_mac_tx_data.DestAddr = mac_pib.macParentShortAddress;//delete by weimin,for the mac_destaddress should equel to nwk_tx_desaddress
 //   a_mac_tx_data.DestAddr.laddr = a_nwk_tx_data.dstLADDR;
 a_mac_tx_data.DestAddr.saddr=a_nwk_tx_data.dstSADDR; //add by weimin
/*//mini
 #ifdef LRWPAN_ROUTER
 a_mac_tx_data.DestAddr.saddr=a_nwk_tx_data.dstSADDR;
 #else
  a_mac_tx_data.DestAddr.saddr=0x0001;
 #endif
//mini*/

  }
  a_mac_tx_data.SrcAddr = macGetShortAddr();
  if ( ((LRWPAN_GET_DST_ADDR(a_mac_tx_data.fcfmsb)) == LRWPAN_ADDRMODE_SADDR) &&
      a_mac_tx_data.DestAddr.saddr == LRWPAN_BCAST_SADDR) {
        a_mac_tx_data.fcflsb = LRWPAN_FRAME_TYPE_DATA|LRWPAN_FCF_INTRAPAN_MASK;
      }else {
        a_mac_tx_data.fcflsb = LRWPAN_FRAME_TYPE_DATA|LRWPAN_FCF_INTRAPAN_MASK |LRWPAN_FCF_ACKREQ_MASK;
      }
  a_mac_service.cmd = LRWPAN_SVC_MAC_GENERIC_TX;
  ////////////////////////////////////////////////////////////////////////////////////����Ϊ�¼ӵ�
nwkTxData_sendit:
  macDoService();//����mac��ķ���
}


/*��������:��������״̬����������nwkRxStateΪNWK_RXSTATE_IDLE��ֱ�ӷ��أ�
            ��nwkRxStateΪNWK_RXSTATE_START������֡���ͣ����д���������֡������
            ������֡�����ϲ�����ת������nwkRxStateΪNWK_RXSTATE_APS_HANDOFF�����������Ϣ��
            ��nwkRxStateΪNWK_RXSTATE_DOROUTE�����㲥�뾶Ϊ0��ת��������û�пռ䣬��������֡���������ת��*/
static void nwkRxFSM(void) {
  BYTE *ptr;	
nwkRxFSM_start://����״̬����ʼλ
  switch(nwkRxState) {
  case NWK_RXSTATE_IDLE:
    break;
  case NWK_RXSTATE_START://we have a packet, lets check it out.
    ptr = a_nwk_rx_data.orgpkt.data + a_nwk_rx_data.nwkOffset;
    if (NWK_IS_CMD(*ptr)) {//currently don't handle CMD packets. Discard.���������֡��·���й�
      //DEBUG_STRING(DBG_INFO,"NWK: Received NWK CMD packet, discarding.\n");
      //MAC resource already free; need to free the MEM resource
      MemFree(a_nwk_rx_data.orgpkt.data);
      nwkRxState = NWK_RXSTATE_IDLE;
      break;
    }
    //this is a data packet. do more parsing.
    nwkParseHdr(ptr);	//������������ͷ��
    //see if this is for us.
    if ((a_nwk_rx_data.dstSADDR == LRWPAN_BCAST_SADDR) ||//���Ŀ�ĵ�ַ�ǹ㲥��ַ
        (a_nwk_rx_data.dstSADDR == LRWPAN_SADDR_USE_LADDR) ||//����Ŀ�ĵ�ַ�ǳ���ַ
          (a_nwk_rx_data.dstSADDR == macGetShortAddr())) {//����Ŀ�ĵ�ַ(�̵�ַ)ƥ��
            //hand this off to the APS layer
            nwkRxState = NWK_RXSTATE_APS_HANDOFF;
          } else {//have to route this packet
            nwkRxState = NWK_RXSTATE_DOROUTE;
          }
    goto nwkRxFSM_start;//��ת������״̬����ʼλ
  case NWK_RXSTATE_APS_HANDOFF:
    if (apsRxBusy()) break;    //apsRX is still busy
    //handoff the current packet
    apsRxHandoff();
    //we are finished with this packet.
    //we don't need to do anything to free this resource other
    // than to change state
    nwkRxState = NWK_RXSTATE_IDLE;
    break;
  case NWK_RXSTATE_DOROUTE:
#ifdef LRWPAN_RFD//����Ǿ������豸��û��·�ɹ���
    //RFD somehow got a data packet not intended for it.
    //should never happen, but put code here anyway to discard it.
    //DEBUG_STRING(DBG_INFO,"NWK: RFD received spurious datapacket, discarding.\n");
    MemFree(a_nwk_rx_data.orgpkt.data);
    nwkRxState = NWK_RXSTATE_IDLE;
#else
    //first, check the radius, if zero, then discard.
    if (!(*(ptr+6))) {
      //DEBUG_STRING(DBG_INFO,"NWK: Data packet is out of hops for dest: ");
      //DEBUG_UINT16(DBG_INFO,a_nwk_rx_data.dstSADDR);
      //DEBUG_STRING(DBG_INFO,", discarding...\n");
      MemFree(a_nwk_rx_data.orgpkt.data);
      nwkRxState = NWK_RXSTATE_IDLE;
      break;
    }
    //DEBUG_STRING(DBG_INFO,"NWK: Routing NWK Packet to: ");
    //DEBUG_UINT16(DBG_INFO,a_nwk_rx_data.dstSADDR);
    //DEBUG_STRING(DBG_INFO,"\n");
    //this packet requires routing, not destined for us.
    if (nwkRxBuffFull()) {//������ջ�������
      //no more room. discard this packet
      //DEBUG_STRING(DBG_INFO,"NWK: FWD buffer full, discarding pkt.\n");
      //DEBUG_STRING(DBG_INFO,"NWK state: ");
      //DEBUG_UINT8(DBG_INFO,nwkState);
      //DEBUG_STRING(DBG_INFO,"MAC state: ");
      //DEBUG_UINT8(DBG_INFO,macState);
      //DEBUG_STRING(DBG_INFO,"\n");
      MemFree(a_nwk_rx_data.orgpkt.data);
      nwkRxState = NWK_RXSTATE_IDLE;
    } else {//ok, add this pkt to the buffer
      nwk_pib.rxHead++;
      if (nwk_pib.rxHead == NWK_RXBUFF_SIZE) nwk_pib.rxHead = 0;
      //save it.
      nwk_pib.rxBuff[nwk_pib.rxHead].data = a_nwk_rx_data.orgpkt.data;
      nwk_pib.rxBuff[nwk_pib.rxHead].nwkOffset = a_nwk_rx_data.nwkOffset;
      nwkRxState = NWK_RXSTATE_IDLE;
      //this packet will be retransmitted by nwkFSM
    }
#endif
    break;
  default:
    break;
  }
}



//Callback from MAC Layer
//Returns TRUE if nwk is still busy with last RX packet.
/*��������:�ж���������״̬������״̬����FALSE���ǿ���״̬����TRUE*/
BOOL nwkRxBusy(void){
	return(nwkRxState != NWK_RXSTATE_IDLE);	
}

//Callback from MAC Layer��mac����лص�
//Hands off parsed packet from MAC layer, frees MAC for parsing
//next packet.�ñ��Ĵ�mac�㴫�͵���������֮�����ͷ�mac���Դ�����������
/*��������:������MAC��������ݣ���Ҫ��һЩ��Ϣ�Ĵ���*/
void nwkRxHandoff(void){
	a_nwk_rx_data.orgpkt.data = a_mac_rx_data.orgpkt->data;	//�Ѹñ��Ĵ�mac�㴫�͵������
	a_nwk_rx_data.orgpkt.rssi = a_mac_rx_data.orgpkt->rssi;//�ź�ǿ��ָʾ�ֽ�
	a_nwk_rx_data.nwkOffset = a_mac_rx_data.pload_offset;	//ƫ����
	nwkRxState = NWK_RXSTATE_START;	//������ջ�״̬Ϊ��ʼ
}

/*��������:�������������ݰ���ͷ�����н���������Ŀ�ĵ�ַ��Դ��ַ*/
void nwkParseHdr(BYTE *ptr) {
	//ptr is pointing at nwk header. Get the SRC/DST nodes.
	ptr= ptr+2;
	//get Dst SADDR
	a_nwk_rx_data.dstSADDR = *ptr;
	ptr++;
	a_nwk_rx_data.dstSADDR += (((UINT16)*ptr) << 8);
	ptr++;

	//get Src SADDR
	a_nwk_rx_data.srcSADDR = *ptr;
	ptr++;
	a_nwk_rx_data.srcSADDR += (((UINT16)*ptr) << 8);
	ptr++;
}

#ifdef LRWPAN_FFD

//copies packet to forward from heap space to TXbuffer space
/*��������:����ת�����ݣ����ڴ浽���ͻ����������������֡��ͷ����������Ч�غɣ�
			ͬʱ��Ŀ�ĵ�ַ�͹㲥�뾶д��a_nwk_tx_data�ṹ��*/
void nwkCopyFwdPkt(void){
	BYTE *srcptr, len;
	NWK_FWD_PKT *pkt;

	phy_pib.currentTxFrm = &tmpTxBuff[LRWPAN_MAX_FRAME_SIZE];
	//get next PKT
	pkt = nwkGetRxPacket();//Ӧ�ü���Ƿ�ΪNULL����ʵ�ڷǿյ�����µ��õ�
	srcptr = pkt->data;  //points at original packet in heapspace
	//compute bytes to copy.����Ҫ���Ƶ��ֽ���
	//nwkoffset is the offset of the nwkheader in the original packet
	len = *(srcptr) - pkt->nwkOffset - PACKET_FOOTER_SIZE + 1;
	//point this one byte past the end of the packet
	srcptr = srcptr		//�Ѹ�ָ���Ƶ����ĵ�β��
		+ *(srcptr) //length of original packet, not including this byte
		+ 1         //add one for first byte which contains packet length
		- PACKET_FOOTER_SIZE; //subtract footer bytes, don't want to copy these.
	//save length
	phy_pib.currentTxFlen = len;//��ǰ���ͱ��ĵĳ���
	//copy from heap space to TXBuffer space
	do {
		srcptr--; phy_pib.currentTxFrm--;
		*phy_pib.currentTxFrm = *srcptr;
		len--;
	}while(len);
	nwkFreeRxPacket(TRUE);  //free this packet
	//some final steps
	//get the dstSADDR, needed for routing.
	a_nwk_tx_data.dstSADDR = *(phy_pib.currentTxFrm+2);
	a_nwk_tx_data.dstSADDR += (((UINT16)*(phy_pib.currentTxFrm+3)) << 8);
	//decrement the radius before sending it on.
	*(phy_pib.currentTxFrm+6)= *(phy_pib.currentTxFrm+6)- 1;
	a_nwk_tx_data.radius = *(phy_pib.currentTxFrm+6);

	
	//leave the SADDR unchanged as we want to know where this originated from!
#if 0
	//replace the SADDR with our SADDR
	*(phy_pib.currentTxFrm+4) = (BYTE) macGetShortAddr();
	*(phy_pib.currentTxFrm+5) = (BYTE) (macGetShortAddr() >>8);
#endif

}

/*��������:�ж��������ջ������Ƿ�Ϊ����������������true,������û������false*/
static BOOL nwkRxBuffFull(void){
	BYTE tmp;
	//if next write would go to where Tail is, then buffer is full
	tmp = nwk_pib.rxHead+1;
	if (tmp == NWK_RXBUFF_SIZE) tmp = 0;
	return(tmp == nwk_pib.rxTail);
}

/*��������:�ж��������ջ������Ƿ�Ϊ�գ��������շ���true,�������ǿշ���false*/
static BOOL nwkRxBuffEmpty(void){
	return(nwk_pib.rxTail == nwk_pib.rxHead);
}

//this does NOT remove the packet from the buffer
/*��������:���������ջ�������ȡһ��ָ��ת�����ݰ���ָ�룬��������Ϊ�գ�����NULL*/
static NWK_FWD_PKT *nwkGetRxPacket(void) {
	BYTE tmp;
	if (nwk_pib.rxTail == nwk_pib.rxHead) return(NULL);
	tmp = nwk_pib.rxTail+1;
	if (tmp == NWK_RXBUFF_SIZE) tmp = 0;
	return(&nwk_pib.rxBuff[tmp]);
}

//frees the first packet in the buffer.�ͷŽ��ջ�����
/*��������:�ͷ��������ջ�������һ��ָ��ָ���ת�����ݰ��ڴ�*/
static void nwkFreeRxPacket(BOOL freemem) {
	nwk_pib.rxTail++;
	if (nwk_pib.rxTail == NWK_RXBUFF_SIZE) nwk_pib.rxTail = 0;
	if (freemem) MemFree(nwk_pib.rxBuff[nwk_pib.rxTail].data);
}

#endif

/*��������:�������繦�ܣ�����ִ������ɨ��򱻶�ɨ�裬��ɺ�����ھӱ�
	��ɨ�践�ص���Ϣ��ϳ�����������*/
void nwkDiscoveryNetwork(void)
{
  BYTE i,j,k;
  NAYBORENTRY *nt_ptr;

//#ifdef LRWPAN_FFD
  //����ɨ��
  a_mac_service.cmd = LRWPAN_SVC_MAC_SCAN_REQ;
  a_mac_service.args.scan.request.ScanType = ACTIVE;
  a_mac_service.args.scan.request.ScanChannels = a_nwk_service.args.discovery_network.request.ScanChannels;
  a_mac_service.args.scan.request.ScanDuration = a_nwk_service.args.discovery_network.request.ScanDuration;
  macDoService();	
//#endif
  /*
#ifdef LRWPAN_RFD
  //����ɨ��
  a_mac_service.cmd = LRWPAN_SVC_MAC_SCAN_REQ;
  a_mac_service.args.scan.request.ScanType = PASSIVE;
  a_mac_service.args.scan.request.ScanChannels = a_nwk_service.args.discovery_network.request.ScanChannels;
  a_mac_service.args.scan.request.ScanDuration = a_nwk_service.args.discovery_network.request.ScanDuration;
  macDoService();	
#endif
  */
  if (a_mac_service.status==LRWPAN_STATUS_MAC_NO_BEACON) {
    //DEBUG_STRING(DBG_ERR, "Discovery network failed, active/passive scan is unsuccessful.\n");
  } else if (a_mac_service.status==LRWPAN_STATUS_SUCCESS) {
    nwkInitNetworkDescriptorList();
    mac_pib.macScanNodeCount = a_mac_service.args.scan.confirm.ResultListSize;
    k = 0;
    for (i=0;i<mac_pib.macScanNodeCount;i++) {
      //ִ������/����ɨ���Ժ󣬸��¸��豸���ھӱ����ҽ����ص�ɨ����Ϣ��ϳ�����������
      //�����������У��ھ��豸Ӧ���Ǹ��豸�����豸�������������н��и���
      //��Mesh�����У��ھ��豸Ӧ����PoS�ڵ��κ�һ������ͨ�ŵ��豸���ڷ���������������ӵ�����¸����ھӱ�
      nt_ptr = &mac_nbr_tbl[0];
      for (j=0;j<NTENTRIES;j++,nt_ptr++) {

        if (!nt_ptr->options.bits.used) {
          nt_ptr->options.bits.used = 1;
          nt_ptr->saddr = PANDescriptorList[i].ShortAddress;
          nt_ptr->laddr[0] = PANDescriptorList[i].ExtendedAddress.bytes[0];
          nt_ptr->laddr[1] = PANDescriptorList[i].ExtendedAddress.bytes[1];
          nt_ptr->laddr[2] = PANDescriptorList[i].ExtendedAddress.bytes[2];
          nt_ptr->laddr[3] = PANDescriptorList[i].ExtendedAddress.bytes[3];
          nt_ptr->laddr[4] = PANDescriptorList[i].ExtendedAddress.bytes[4];
          nt_ptr->laddr[5] = PANDescriptorList[i].ExtendedAddress.bytes[5];
          nt_ptr->laddr[6] = PANDescriptorList[i].ExtendedAddress.bytes[6];
          nt_ptr->laddr[7] = PANDescriptorList[i].ExtendedAddress.bytes[7];
          nt_ptr->capinfo = 0x00;
          //LRWPAN_SET_CAPINFO_ALTPAN(nt_ptr->capinfo);
          LRWPAN_SET_CAPINFO_DEVTYPE(nt_ptr->capinfo);
          //LRWPAN_SET_CAPINFO_PWRSRC(nt_ptr->capinfo);
          //LRWPAN_SET_CAPINFO_RONIDLE(nt_ptr->capinfo);
          if (PANDescriptorList[i].options.bits.SecurityUse)
            LRWPAN_SET_CAPINFO_SECURITY(nt_ptr->capinfo);
          //LRWPAN_SET_CAPINFO_ALLOCADDR(nt_ptr->capinfo);
          nt_ptr->PANID = PANDescriptorList[i].CoordPANId;
          if (PANDescriptorList[i].options.bits.PANCoordinator)
            nt_ptr->DeviceType = LRWPAN_DEVTYPE_COORDINATOR;
          else
            nt_ptr->DeviceType = LRWPAN_DEVTYPE_ROUTER;
          nt_ptr->Relationship = LRWPAN_DEVRELATION_SIBING;
          nt_ptr->Depth = PANDescriptorList[i].Depth;
          nt_ptr->BeaconOrder = ((BYTE)PANDescriptorList[i].SuperframeSpec)&0x0F;
          nt_ptr->LQI = PANDescriptorList[i].LinkQuality;
          nt_ptr->LogicalChannel = PANDescriptorList[i].LogicalChannel;
          if (PANDescriptorList[i].options.bits.AssociationPermit)
            nt_ptr->options.bits.PermitJoining = 1;
          break;
        }
      }

      if (j== NTENTRIES) {
        //DEBUG_STRING(DBG_ERR, "There is no room in neighbor table!\n");
      }

      if (!PANDescriptorList[i].options.bits.PANCoordinator)
        continue;

      NetworkDescriptorList[k].PANId = PANDescriptorList[i].CoordPANId;
      NetworkDescriptorList[k].CoordShortAddress = PANDescriptorList[i].ShortAddress;
      NetworkDescriptorList[k].CoordExtendedAddress.bytes[0] = PANDescriptorList[i].ExtendedAddress.bytes[0];
      NetworkDescriptorList[k].CoordExtendedAddress.bytes[1] = PANDescriptorList[i].ExtendedAddress.bytes[1];
      NetworkDescriptorList[k].CoordExtendedAddress.bytes[2] = PANDescriptorList[i].ExtendedAddress.bytes[2];
      NetworkDescriptorList[k].CoordExtendedAddress.bytes[3] = PANDescriptorList[i].ExtendedAddress.bytes[3];
      NetworkDescriptorList[k].CoordExtendedAddress.bytes[4] = PANDescriptorList[i].ExtendedAddress.bytes[4];
      NetworkDescriptorList[k].CoordExtendedAddress.bytes[5] = PANDescriptorList[i].ExtendedAddress.bytes[5];
      NetworkDescriptorList[k].CoordExtendedAddress.bytes[6] = PANDescriptorList[i].ExtendedAddress.bytes[6];
      NetworkDescriptorList[k].CoordExtendedAddress.bytes[7] = PANDescriptorList[i].ExtendedAddress.bytes[7];
      NetworkDescriptorList[k].LogicalChannel = PANDescriptorList[i].LogicalChannel;
      NetworkDescriptorList[k].LinkQuality = PANDescriptorList[i].LinkQuality;
      NetworkDescriptorList[k].StackProfile = PANDescriptorList[i].StackProfile;
      NetworkDescriptorList[k].ZigBeeVersion = PANDescriptorList[i].ZigBeeVersion;
      NetworkDescriptorList[k].BeaconOrder = (BYTE)(PANDescriptorList[i].SuperframeSpec&0x000F);
      NetworkDescriptorList[k].SuperframeOrder = (BYTE)((PANDescriptorList[i].SuperframeSpec>>4)&0x000F);
      NetworkDescriptorList[k].PermitJioning = (BOOL)(PANDescriptorList[i].SuperframeSpec>>15);
      NetworkDescriptorList[k].SecurityLevel = 0x00;//Ŀǰ�����ð�ȫ�����Ľ�
      k++;
    }
    nwk_pib.nwkNetworkCount = k;
    a_nwk_service.args.discovery_network.confirm.NetworkCount = k;
  }

  a_nwk_service.status = a_mac_service.status;
}


#ifndef LRWPAN_COORDINATOR
/*��������:��������*/
void nwkJoinNetwork(void)
{
  BYTE i,lqi;
  NAYBORENTRY *nt_ptr;

  if (a_nwk_service.args.join_network.options.bits.RejoinNetwork) {//���¼�������
    /*��������ʵ�巢��MLME_SCAN.requestԭ��µ�ɨ�衣���յ���Ӧ����MAC���������*/
    a_mac_service.cmd = LRWPAN_SVC_MAC_SCAN_REQ;
    a_mac_service.args.scan.request.ScanType = ORPHAN;
    a_mac_service.args.scan.request.ScanChannels = a_nwk_service.args.join_network.ScanChannels;
    a_mac_service.args.scan.request.ScanDuration = a_nwk_service.args.join_network.ScanDuration;
    macDoService();
    a_nwk_service.status = a_mac_service.status;
  } else {
    phyInit();
    macInit();
    /*��������ʵ�巢��MLME_ASSOCIATE.requestԭ������Ϣ���ھӱ��л�ȡ��
    ������ھӱ��в����ڷ����������豸��������㷵��״̬NOT_PERMITTED
    */
    mac_pib.flags.bits.macAssociationPermit = 0;//if trying to join, do not allow association
    mac_pib.flags.bits.macIsAssociated = 0;
    nt_ptr = NULL;
    lqi = 0;
    a_mac_service.cmd = LRWPAN_SVC_MAC_ASSOC_REQ;
    //���¼������磬��Ӧ���Ӳ�ȷ���ĸ����磬�����ȷ�������ĸ��ڵ�
    for (i=0;i<NTENTRIES;i++) {
      if (!mac_nbr_tbl[i].options.bits.used)
        continue;
      if (mac_nbr_tbl[i].PANID==a_nwk_service.args.join_network.PANID && mac_nbr_tbl[i].saddr!=LRWPAN_BCAST_SADDR) {
        if (mac_nbr_tbl[i].LQI > lqi) {
          lqi = mac_nbr_tbl[i].LQI;
          nt_ptr = &mac_nbr_tbl[i];
        }
      }
    }
    if (nt_ptr==NULL) {
      //DEBUG_STRING(DBG_ERR, "Network join failed, have no a right parent device!\n");
      nwkState = NWK_STATE_IDLE;
      a_nwk_service.status = LRWPAN_STATUS_NWK_JOIN_NOT_PERMITTED;
      return;
    }
    a_mac_service.args.associate.request.LogicalChannel = nt_ptr->LogicalChannel;
    a_mac_service.args.associate.request.CoordAddrMode = LRWPAN_ADDRMODE_SADDR;
    a_mac_service.args.associate.request.CoordPANID = a_nwk_service.args.join_network.PANID;
    a_mac_service.args.associate.request.CoordAddress.saddr = nt_ptr->saddr;
    a_mac_service.args.associate.request.CapabilityInformation = LRWPAN_ASSOC_CAPINFO_ALLOCADDR;//����̵�ַ
    if (a_nwk_service.args.join_network.options.bits.JoinAsRouter) {//��Ϊ·�����ͱ���Э����
      a_mac_service.args.associate.request.CapabilityInformation |= LRWPAN_ASSOC_CAPINFO_ALTPAN;
      a_mac_service.args.associate.request.CapabilityInformation |= LRWPAN_ASSOC_CAPINFO_DEVTYPE;
    }
    if (a_nwk_service.args.join_network.options.bits.PowerSource)//������Դ
      a_mac_service.args.associate.request.CapabilityInformation |= LRWPAN_ASSOC_CAPINFO_PWRSRC;
    if (a_nwk_service.args.join_network.options.bits.RxOnWhenIdle)//����ʱ���ջ���
      a_mac_service.args.associate.request.CapabilityInformation |= LRWPAN_ASSOC_CAPINFO_RONIDLE;
    if (a_nwk_service.args.join_network.options.bits.MACSecurity) {//ʹ�ð�ȫ
      a_mac_service.args.associate.request.CapabilityInformation |= LRWPAN_ASSOC_CAPINFO_SECURITY;
      a_mac_service.args.associate.request.SecurityEnable = 0;
    }

    //a_mac_service.args.associate.request.CapabilityInformation = 0x01;
    a_nwk_service.status = macInitRadio(a_mac_service.args.associate.request.LogicalChannel, a_mac_service.args.associate.request.CoordPANID);
    if (a_nwk_service.status != LRWPAN_STATUS_SUCCESS) {//��ʼ���ŵ�ʧ��
      //DEBUG_STRING(DBG_ERR, "Network join failed, phy radio initializtion is unsuccessful!\n");
      nwkState = NWK_STATE_IDLE;
      a_nwk_service.status = LRWPAN_STATUS_PHY_RADIO_INIT_FAILED;
      return;
    }

    macDoService();
    a_nwk_service.status = a_mac_service.status;
    if (a_nwk_service.status == LRWPAN_STATUS_SUCCESS) {
      mac_pib.flags.bits.macAssociationPermit = 1;//if trying to join, do not allow association
      mac_pib.flags.bits.macIsAssociated = 1;
      nt_ptr->Relationship = LRWPAN_DEVRELATION_PARENT;
      mac_pib.macDepth = nt_ptr->Depth + 1;
      mac_pib.macParentShortAddress = nt_ptr->saddr;
      halUtilMemCopy(&mac_pib.macParentExtendedAddress.bytes[0], &nt_ptr->laddr[0], 8);
#ifdef LRWPAN_FFD
      //��ʼ���ھ��豸��Ŀ�Ͷ̵�ַ����
      ntInitAddressAssignment();
#endif
      //DEBUG_STRING(DBG_INFO,"Network join successed, the AssocShortAddress is ");
      //DEBUG_UINT16(DBG_INFO,a_mac_service.args.associate.confirm.AssocShortAddress);
      //DEBUG_STRING(DBG_INFO,"  \n ");
    } else {
      //DEBUG_STRING(DBG_INFO,"Network join failed, the coordinator denies this associate request!\n");
    }
    nwkState = NWK_STATE_IDLE;
    /*���� ������������״̬���õ� ����Ķ̵�ַ���ڽ��յ�������Ӧ����MAC���������*/
  }
}
#endif

void nwkLeaveNetwork(void)
{
//FFD�豸�Ͽ�������������ʽ��1.��ĳ�豸���ɲ���DeviceAddressָ����ͬ����Ͽ���2.����������DeviceAddressΪNULL��ͬ����Ͽ�
#ifdef LRWPAN_FFD
  if (nwkCheckLaddrNull(&a_nwk_service.args.leave_network.DeviceAddress)){
    a_mac_service.args.disassoc_req.DisassociateReason = DEVICE_DISASSOC_WITH_NETWORK;
    nwkSetLaddrNull(&a_mac_service.args.disassoc_req.DeviceAddress.bytes[0]);
    a_mac_service.args.disassoc_req.SecurityEnable = FALSE;
    a_mac_service.cmd = LRWPAN_SVC_MAC_DISASSOC_REQ;
    macDoService();
    a_nwk_service.status = a_mac_service.status;

    if (a_nwk_service.status == LRWPAN_STATUS_SUCCESS) {
      //DEBUG_STRING(DBG_INFO, "Network leaved successed, I am disassociated!");
    } else {
      //DEBUG_STRING(DBG_INFO, "Network leaved failed, I am not disassociated!");
      return;
    }
    //���·�ɱ�,�ھӱ��͵�ַ��
    ntInitTable();
    mac_pib.flags.bits.macIsAssociated = 0;
    //MAC�㸴λ
    a_mac_service.cmd = LRWPAN_SVC_MAC_RESET_REQ;
    a_mac_service.args.reset.SetDefaultPIB = TRUE;
    macDoService();
    a_nwk_service.status = a_mac_service.status;		
  } else {
    if ((NAYBORENTRY *)ntFindByLADDR( &a_nwk_service.args.leave_network.DeviceAddress) == NULL){//���ھӱ����Ҳ������豸
      a_nwk_service.status = LRWPAN_STATUS_UNKNOWN_DEVICE;
      //DEBUG_STRING(DBG_ERR, "Force a device to leave network, the device is not existent in neighbortable!\n");
      return;
    }	
    a_mac_service.args.disassoc_req.DisassociateReason = FORCE_DEVICE_OUTOF_NETWORK;
    halUtilMemCopy(&a_mac_service.args.disassoc_req.DeviceAddress.bytes[0], &a_nwk_service.args.leave_network.DeviceAddress.bytes[0],8);
    a_mac_service.args.disassoc_req.SecurityEnable = FALSE;
    a_mac_service.cmd = LRWPAN_SVC_MAC_DISASSOC_REQ;
    ntDelNbrByLADDR(&a_nwk_service.args.leave_network.DeviceAddress);
    macDoService();
    if (a_nwk_service.status == LRWPAN_STATUS_SUCCESS) {
      //DEBUG_STRING(DBG_INFO, "Network leaved successed, this device is disassociated!");
    } else {
      //DEBUG_STRING(DBG_INFO, "Network leaved failed, this device is not disassociated!");
      return;
    }
    a_nwk_service.status = a_mac_service.status;			
  }

#endif
//RFD�豸ֻ�ܽ�����ͬ����Ͽ��������DeviceAddress����ΪNULL
#ifdef LRWPAN_RFD
  if ( !(nwkCheckLaddrNull(&a_nwk_service.args.leave_network.DeviceAddress))){
    a_nwk_service.status = LRWPAN_STATUS_INVALID_PARAMETER;
    //DEBUG_STRING(DBG_ERR, "RFD leaved the network,the parameter of DeviceAddress is invalid!\n");
    return;
  }
  a_mac_service.args.disassoc_req.DisassociateReason = DEVICE_DISASSOC_WITH_NETWORK;
  nwkSetLaddrNull(&a_mac_service.args.disassoc_req.DeviceAddress.bytes[0]);
  a_mac_service.args.disassoc_req.SecurityEnable = FALSE;
  macDoService();
  a_nwk_service.status = a_mac_service.status;

  if (a_nwk_service.status == LRWPAN_STATUS_SUCCESS) {
    //DEBUG_STRING(DBG_INFO, "Network leaved successed, I am disassociated!");
  } else {
    //DEBUG_STRING(DBG_INFO, "Network leaved failed, I am not disassociated!");
    return;
  }
  //���·�ɱ���ַ����Ϣ
  //ntInitAddressMap();//��ntDelAddressByIndex(0)������ͬ
  mac_pib.flags.bits.macIsAssociated = 0;
  //MAC�㸴λ
  a_mac_service.cmd = LRWPAN_SVC_MAC_RESET_REQ;
  a_mac_service.args.reset.SetDefaultPIB = TRUE;
  macDoService();
  a_nwk_service.status = a_mac_service.status;
#endif
}

#ifdef LRWPAN_COORDINATOR
/*��������:�γ����繦��*/
void nwkFormNetwork(void)
{
  BYTE minenergy;
  BYTE channel,i;
  UINT16 panid;
  UINT32 optionalchannel;	

  nwk_pib.flags.bits.nwkFormed = 0;	
  mac_pib.flags.bits.macIsAssociated = 0;
  mac_pib.flags.bits.macAssociationPermit = 0;
  //�������ɨ��
  //DEBUG_STRING(DBG_INFO,"Network formed, start to energy detect.\n");
  a_mac_service.cmd = LRWPAN_SVC_MAC_SCAN_REQ;
  a_mac_service.args.scan.request.ScanType = ENERGY_DETECT;
  a_mac_service.args.scan.request.ScanChannels = a_nwk_service.args.form_network.ScanChannels;
  a_mac_service.args.scan.request.ScanDuration = a_nwk_service.args.form_network.ScanDuration;
  macDoService();
  //�鿴��Щ�ŵ���ɨ�裬��һ������Щ�ŵ���������ɨ��
  optionalchannel = a_nwk_service.args.form_network.ScanChannels&(~a_mac_service.args.scan.confirm.UnscanChannels);
  /*
  //����ɨ��
  DEBUG_STRING(DBG_INFO,"Network formed, start to active detect.\n");
  a_mac_service.cmd = LRWPAN_SVC_MAC_SCAN_REQ;
  a_mac_service.args.scan.request.ScanType = ACTIVE;
  a_mac_service.args.scan.request.ScanChannels = a_nwk_service.args.form_network.ScanChannels;
  a_mac_service.args.scan.request.ScanDuration = a_nwk_service.args.form_network.ScanDuration;
  macDoService();
  //�鿴��Щ�ŵ���ɨ�裬׼������ѡ���ʵ����ŵ���PANID
  optionalchannel = optionalchannel&a_nwk_service.args.form_network.ScanChannels&(~a_mac_service.args.scan.confirm.UnscanChannels);
  */
  //ѡ���ŵ�
  //DEBUG_STRING(DBG_INFO,"Network formed, start to select a fine logicalchannel.\n");
  minenergy = 0xFF;

  for (i=11;i<27;i++) {
    if (optionalchannel&(((UINT32)1)<<i)) {
      if (EnergyDetectList[i-11]<minenergy) {
        minenergy = EnergyDetectList[i-11];
        channel = i;
      }
    }
    channel = 13;
  }

  if (minenergy==0xFF) {
    a_nwk_service.status = LRWPAN_STATUS_STARTUP_FAILURE;
    //DEBUG_STRING(DBG_ERR, "NWK Formation failed, have not a right logicalchannel!\n");
    nwkState = NWK_STATE_IDLE;
    return;
  }
  //ȷ��PANID
  //DEBUG_STRING(DBG_INFO,"Network formed, start to select PANID.\n");
  if (a_nwk_service.args.form_network.PANID==0x0000) {
    panid = halGetRandomByte();
    panid = panid<<8;
    panid = panid + halGetRandomByte();
    panid = panid&0x3fff;
    a_nwk_service.args.form_network.PANID = panid;
  }  else
    panid = a_nwk_service.args.form_network.PANID;

  //DEBUG_STRING(DBG_INFO,"Network formed, the current logicalchannel : ");
  //DEBUG_UINT8(DBG_INFO,channel);
  //DEBUG_STRING(DBG_INFO," ;  the current PANID : ");
  //DEBUG_UINT16(DBG_INFO,panid);
  //DEBUG_STRING(DBG_INFO," \n");

  phyInit();
  macInit();
  ntInitTable();  //init neighbor table	

  //��ʼ��RF
  a_nwk_service.status = macInitRadio(channel, panid);  //turns on the radio
  if (a_nwk_service.status != LRWPAN_STATUS_SUCCESS) {
    //DEBUG_STRING(DBG_ERR, "Network formed failed, initial RF failure!\n");
    return;
  }
  //������ز���PANID���ŵ����̵�ַ
  macSetPANID(panid);
  macSetChannel(channel);
  macSetShortAddr(0);
  macSetDepth(0);
  //��ʼ���̵�ַ���������������Э�����Ķ̵�ַ������ɺ���е�ַ����
#ifdef LRWPAN_FFD
  ntInitAddressAssignment();	
#endif
  //���ñ�־λ����ʾ�����Ѿ��γɣ��Լ��Ѿ��������磬mac����������
  nwk_pib.flags.bits.nwkFormed = 1;	
  mac_pib.flags.bits.macIsAssociated = 1;
  mac_pib.flags.bits.macAssociationPermit = 1;
  //��ʼ���µĳ�֡��֡����
  //DEBUG_STRING(DBG_INFO, "Network formed, start to upgrade superframe!\n");
  a_mac_service.cmd = LRWPAN_SVC_MAC_START_REQ;
  a_mac_service.args.start.PANID = panid;
  a_mac_service.args.start.LogicalChannel = channel;
  a_mac_service.args.start.BeaconOrder = a_nwk_service.args.form_network.BeaconOrder;
  a_mac_service.args.start.SuperframeOrder = a_nwk_service.args.form_network.SuperframeOrder;
  a_mac_service.args.start.options.bits.PANCoordinator = TRUE;
  a_mac_service.args.start.options.bits.BatteryLifeExtension = a_nwk_service.args.form_network.BatteryLifeExtension;
  a_mac_service.args.start.options.bits.CoordRealignment = TRUE;
  a_mac_service.args.start.options.bits.SecurityEnable = FALSE;
  macDoService();
  a_nwk_service.status = a_mac_service.status;
}
#endif
//given a router child SADDR, find the parent router SADDR
/*��������:����������һ��·�����̵�ַ������丸�豸�Ķ̵�ַ*/
UINT16 nwkFindParentSADDR(SADDR childSADDR) {
	UINT8 currentDepth;	//��ǰ���
	SADDR currentParent;	//��ǰ���ڵ�
	SADDR currentRouter;	//��ǰ·����	
	SADDR maxSADDR;		//����ַ
	UINT8 i;


	currentDepth = 1;	//��ǰ���Ϊ1
	currentParent = 0;	//��ǰ���ڵ�Ϊ0��Э����
	do {
		for (i=0; i<LRWPAN_MAX_ROUTERS_PER_PARENT; i++) {//�μ��̵�ַ�ķ������
			if (i==0) currentRouter = currentParent+1;
			else currentRouter += ntGetCskip(currentDepth);
			if (childSADDR == currentRouter) return(currentParent);//���İ�
			//if (childSADDR == currentRouter) return(currentRouter);//ԭ�棬�е�����
			maxSADDR = ntGetMaxSADDR(currentRouter,currentDepth+1);
			if ((childSADDR > currentRouter) && (childSADDR <= maxSADDR))
				break; //must go further down the tree
		}
		currentDepth++;
		currentParent = currentRouter;
	}
	while (currentDepth < LRWPAN_MAX_DEPTH-1);//<4
	//if we reach here, could not find an address. Return 0 as an error
	return(0);
}

UINT16 nwkGetHopsToDest(SADDR dstSADDR){	//����Ŀ�Ľڵ�ĵ�ַ���ȥĿ�Ľڵ������
				//�ö���ö�����216ҳ��ͼ����
	UINT16 numHops;
	SADDR currentParent, maxSADDR;
	UINT8 currentDepth;
	UINT8 i;
	SADDR currentRouter;

	numHops = 1;            //return a minimum value of 1

	currentDepth = 0;
	//first compute hops up the tree then down the tree
	if ( macGetShortAddr() == 0) goto nwkGetHopsToDest_down;  //this is the coordinator,the coordinator short address is 0x0000
	if (macGetShortAddr() == dstSADDR) return(1);  //to myself, should not happen, but return min value
	currentParent = mac_pib.macCoordShortAddress; //start with my parent address��ǰ���ڵ��ֵΪЭ�����Ķ̵�ַ
	currentDepth = mac_pib.macDepth - 1; //depth of my parent.��ǰ���Ϊ���ڵ�����
	do {	
		if (currentParent == dstSADDR) return(numHops);  //destination is one of my parent nodes.Ŀ�Ľڵ��ǵ�ǰ���ڵ㣬�Ҹ��ڵ㲻��Э�������򷵻�����
		if (currentParent == 0) break;         //at coordinator.�����ǰ���ڵ���Э���������˳�
		//compute the max SADDR address range of parent		//���㸸�ڵ㷶Χ������ַ

		maxSADDR = ntGetMaxSADDR(currentParent,currentDepth+1);  //depth of parent's children���㷨�е����⣬��Ҫ�����Ķ�Ӧ���е�����
		if ((dstSADDR > currentParent) &&  (dstSADDR <= maxSADDR)) {
			//this address is in this router's range, stop going up.�õ�ַ�����·�����ڵ㴦��ͬһ��ȵĽڵ㣬�����ٴ����ϲ�
			break;
		}
		//go up a level����һ��
		currentDepth--;//��ǰ��ȼ�1
		numHops++;	//������1	
		if (currentDepth == 0 )
			currentParent =0;//�����ǰ���Ϊ 0����ǰ���ڵ�ֵΪ0��Э����
		else { currentParent = nwkFindParentSADDR(currentParent);	//�����ҳ���ǰ���ڵ�ĸ��ڵ�ĵ�ַ(Ҳ���ǵ�ǰ�ڵ��үү�ĵ�ַ:)
		if (!currentParent) {//���үү�ڵ��Ѿ���Э�����Ļ������Ѿ��ҵ�����
			//could not find, set numHops to maximum and return�Ҳ�������������Ϊ���ֵ��������
			return(LRWPAN_MAX_DEPTH<<1);
		}
		}
	}while(1);

nwkGetHopsToDest_down:	//this is the coordinator
	currentDepth++; //increment depth, as this should reflect my current children��ǰ��ȼ�1��������ȣ�Ϊ�˷�Ӧ��ǰ�ӽڵ�
	//now search going down.	����Э����Ҫ������Ѱ
	do {
		//destination is in the current parent's range
		//see if it is one of the routers or children.
		//first see if it is one of the children of current parent
		//Ŀ�Ľڵ����ڵ�ǰ���ڵ�ķ�Χ�����Ƿ���·���������ն��豸�����ȿ��Ƿ��ǵ�ǰ���ڵ���ӽڵ�
		numHops++;	//������1
		maxSADDR = ntGetMaxSADDR(currentParent,currentDepth);	//�������ַ
		if (dstSADDR > (maxSADDR-LRWPAN_MAX_NON_ROUTER_CHILDREN) &&
			dstSADDR <= maxSADDR) break;  //it is one of the children nodes�����е�һ���ն��豸
		for (i=0; i<LRWPAN_MAX_ROUTERS_PER_PARENT; i++) {	
			if (i==0) currentRouter = currentParent+1;//��·�������ȿ϶��Ǵ�0Э������ʼ������1��������2��Щ���ǵ�ϵ·��ϵͳ�������Ķ�����֧
			else currentRouter += ntGetCskip(currentDepth);//���ⲽ�Ѿ��������·�����ĵ�ֵַ

			if (dstSADDR == currentRouter) return(currentRouter);
			maxSADDR = ntGetMaxSADDR(currentRouter,currentDepth+1);//�������Э������������ӽڵ�һ���ĵ�ַ��Χ
			if ((dstSADDR > currentRouter) && (dstSADDR <= maxSADDR))//���ǲ�������ڵ�һ��
				break; //must go further down the tree
		}
		if (i == LRWPAN_MAX_ROUTERS_PER_PARENT) {
			//must be one of my non-router children, increment hops, return
			return(numHops);
		}
		currentDepth++;		//��ǰ��ȼ�1����������ڵ�һ���������Ҫ�ټ�1
		currentParent = currentRouter;	//��ǰ·�������ǵ�ǰ���ڵ�

	}while(currentDepth < LRWPAN_MAX_DEPTH-1);

	if (numHops > LRWPAN_NWK_MAX_RADIUS) {
		//DEBUG_STRING(DBG_ERR,"nwkGetHopsToDest: Error in hop calculation: ");
		//DEBUG_UINT8(DBG_ERR,numHops);
		//DEBUG_STRING(DBG_ERR,"\n");
		numHops = LRWPAN_NWK_MAX_RADIUS-1;
	}
	return(numHops);
}








