//�û������ļ�
#ifndef LRWPAN_CONFIG_H
#define LRWPAN_CONFIG_H

#define LRWPAN_VERSION  "0.2.2"

//�洢����̬����ĺ�����С
#ifndef LRWPAN_HEAPSIZE
#define LRWPAN_HEAPSIZE  1024
#endif

//add by weimin for ��ȫ��
#define SECURITY_GLOBALS


//���ڽ�֧��2.4GHz
#define LRWPAN_DEFAULT_FREQUENCY PHY_FREQ_2405M
#define LRWPAN_SYMBOLS_PER_SECOND   62500     //һ��16΢��ķ���,(1000000)/16=62500


#define LRWPAN_DEFAULT_START_CHANNEL  20   //2.4GHz.(2450MHz)��Ч�ŵ���11��26

//0��ʾ�ŵ�������,0xFFFFFFFF�������е�16���ŵ�
//2.4GHz �ŵ���11-26λ��
#define LRWPAN_DEFAULT_CHANNEL_MASK 0xFFFFFFFF

//��������PANID
#define LRWPAN_USE_STATIC_PANID      //������������,Ĭ�ϵ�PANID�ܻᱻʹ��
#define LRWPAN_DEFAULT_PANID 0x1347
#define LRWPAN_DEFAULT_CHANNEL_SCAN_DURATION 3
//#define LRWPAN_DEFAULT_CHANNEL_SCAN_DURATION 7

//MAC�����(RX)����������Ļ���
#define LRWPAN_MAX_MAC_RX_PKTS   4

//�ȴ���ת���������ڵ�����������ݰ����������
//ֻΪFFDs�����
#define LRWPAN_MAX_NWK_RX_PKTS   4


//��������Ĵ�����ļ�����ݰ�
//Э����ʹ��
#define LRWPAN_MAX_INDIRECT_RX_PKTS 2


/*
���LRWPAN_ENABLE_SLOW_TIMER�Ѿ�����,��ôHAL��Ҳ
��ͨ��ʹ��SLOWTICKS_PER_SECOND��ֵ����һ�������ж϶�ʱ��
,hal�㽫����ÿ���жϷ���ʱ����usr�ĺ���usrSlowTimerInt().

���slow timer��Ч,��ôEVB���ؽ�����������ʽ��в���

��halStack.c ��Щ��ʱ����Դ���õ����
���붨ʱ����Դʹ��macTimer�ǲ�һ����

�����ǲ���ʹ����,����������

*/



//�����ϣ��ASSOC_RESPONSE, ASSOC_REQUEST��802.15.4���ݲ�Ҫע�͵���һ��
#define IEEE_802_COMPLY



//���û���豸�����
//�������õ�ַ�ķ���
//���������¶�����һλ,����λҲҪ���¶���
#ifndef aExtendedAddress_B7
#define aExtendedAddress_B7 0x00
#define aExtendedAddress_B6 0x00
#define aExtendedAddress_B5 0x00
#define aExtendedAddress_B4 0x00
#define aExtendedAddress_B3 0x00
#define aExtendedAddress_B2 0x00
#define aExtendedAddress_B1 0x00
#define aExtendedAddress_B0 0x00
#endif

//uncomment this if you want to force association to a particular target
//#ifdef LRWPAN_FORCE_ASSOCIATION_TARGET
//set the following to the long address of the parent to associate with
//if using forced association.
//if you use forced association, then you must NOT define IEEE_802_COMPLY
//as forced association depends upon our special associate request/response
#define parent_addr_B0 0x00
#define parent_addr_B1 0x00
#define parent_addr_B2 0x00
#define parent_addr_B3 0x00
#define parent_addr_B4 0x00
#define parent_addr_B5 0x00
#define parent_addr_B6 0x00
#define parent_addr_B7 0x00



//MAC Capability Info

//if either router or coordinator, then one of these must be defined
//#define LRWPAN_COORDINATOR
//#define LRWPAN_ROUTER




#if (defined (LRWPAN_COORDINATOR) || defined (LRWPAN_ROUTER) )
#define LRWPAN_FFD
#define LRWPAN_ROUTING_CAPABLE
#endif
#if !defined (LRWPAN_FFD)
#define LRWPAN_RFD
#endif

//define this if ACMAIN POWERED
#define LRWPAN_ACMAIN_POWERED
//define this if Receiver on when idle
#define LRWPAN_RCVR_ON_WHEN_IDLE
//define this if capable of RX/TX secure frames
//#define LRWPAN_SECURITY_CAPABLE



//comment this if you want the phy to call the EVBPOLL function
//do this if you want to poll EVB inputs during the stack idle
//time
//#define LRWPAN_PHY_CALL_EVBPOLL

#define LRWPAN_ZIGBEE_PROTOCOL_ID   0
#define LRWPAN_ZIGBEE_PROTOCOL_VER  0
#define LRWPAN_STACK_PROFILE  0         //indicates this is a closed network.
#define LRWPAN_APP_PROFILE    0xFFFF    //filter data packets by this profile number
#define LRWPAN_APP_CLUSTER    0x2A    //default cluster, random value for debugging

//define this if you want the beacon payload to comply with the Zigbee standard
#define LRWPAN_ZIGBEE_BEACON_COMPLY

//Network parameters



//this is a magic number exchanged with nodes wishing to join our
//network. If they do not match this number, then they are rejected.
//Sent in beacon payload
#define LRWPAN_NWK_MAGICNUM_B0 0x0AA
#define LRWPAN_NWK_MAGICNUM_B1 0x055
#define LRWPAN_NWK_MAGICNUM_B2 0x0C3
#define LRWPAN_NWK_MAGICNUM_B3 0x03C



/*
These numbers determine affect the size of the neighbor
table, and the maximum number of nodes in the network,
and how short addresses are assigned to nodes.

*/
#define LRWPAN_MAX_DEPTH                   5
#define LRWPAN_MAX_ROUTERS_PER_PARENT      4
//these are total children, includes routers
#define LRWPAN_MAX_CHILDREN_PER_PARENT    17
#define LRWPAN_MAX_NON_ROUTER_CHILDREN    (LRWPAN_MAX_CHILDREN_PER_PARENT-LRWPAN_MAX_ROUTERS_PER_PARENT)//17��4



//if using Indirect addressing, then this number determines the
//maximum size of the address table map used by the coordinator
//that matches long addresses with short addresses.
//You should set this value to the maximum number of RFDs that
//use indirect addressing. The value below is just chosen for testing.
//Its minimum value must be the maximum number of neighbors (RFDs+Routers+1), as this
//is also used in the neighbor table construction.
#ifdef LRWPAN_COORDINATOR
#define LRWPAN_MAX_ADDRESS_MAP_ENTRIES   (LRWPAN_MAX_CHILDREN_PER_PARENT*2)//17*2=34
#endif


#ifndef LRWPAN_MAX_ADDRESS_MAP_ENTRIES
//this is the minimum value for this, minimum value used by routers
#ifdef LRWPAN_ROUTER
#define LRWPAN_MAX_ADDRESS_MAP_ENTRIES (LRWPAN_MAX_CHILDREN_PER_PARENT+1)//17+1=18
#endif
#ifdef LRWPAN_RFD
#define LRWPAN_MAX_ADDRESS_MAP_ENTRIES 1
#endif
#endif

#ifdef LRWPAN_FFD
#if (LRWPAN_MAX_ADDRESS_MAP_ENTRIES < (LRWPAN_MAX_CHILDREN_PER_PARENT+1))
#error "In lrwpan_config.h, LRWPAN_MAX_ADDRESS_MAP_ENTRIES too small!"
#endif
#endif



//these precalculated based upon MAX_DEPTH, MAX_ROUTERS, MAX_CHILDREN
//Coord at depth 0, only endpoints are at depth MAX_DEPTH
//LRWPAN_CSKIP_(MAX_DEPTH) must be a value of 0.
//this hardcoding supports a max depth of 10, should be PLENTY
//Use the spreadsheet supplied with the distribution to calculate these
#define LRWPAN_CSKIP_1     1446
#define LRWPAN_CSKIP_2      358
#define LRWPAN_CSKIP_3       86
#define LRWPAN_CSKIP_4       18
#define LRWPAN_CSKIP_5       0
#define LRWPAN_CSKIP_6       0
#define LRWPAN_CSKIP_7       0
#define LRWPAN_CSKIP_8       0
#define LRWPAN_CSKIP_9       0
#define LRWPAN_CSKIP_10      0


#define LRWPAN_NWK_MAX_RADIUS  LRWPAN_MAX_DEPTH*2//5*2=10

//Binding
//if the following is defined, then the EVB binding functions use
//the binding resolution functions defined in stack/staticbind.c
//#define LRWPAN_USE_DEMO_STATIC_BIND

//Define this if you want to use the binding functions
//in pcbind.c/h that store the binding table on a PC client
//using the bindingdemo application
//#define LRWPAN_USE_PC_BIND
//PC_BIND_CACHE_SIZE only needed if USE_PC_BIND is defined
//number of bindings cached by the PC bind code
#define LRWPAN_PC_BIND_CACHE_SIZE  4

//these are defaults, can be changed by user
#define LRWPAN_APS_ACK_WAIT_DURATION 200   //in milliseconds, for depth=1
#define LRWPAN_NWK_JOIN_WAIT_DURATION 200  //in milliseconds



#define LRWPAN_APS_MAX_FRAME_RETRIES 3  //for acknowledge frames
#define LRWPAN_MAC_MAX_FRAME_RETRIES 3  //for MAC ack requests

//maximum number of endpoints, controls size of endpoint data structure
//in APS.h
#define LRWPAN_MAX_ENDPOINTS    6

//data for node descriptor, not currently used
#define LRWPAN_MAX_USER_PAYLOAD   93      //currently 93 bytes
#define LRWPAN_MANUFACTURER_CODE  0x0000  //assigned by Zigbee Alliance



//unsupported at this time
//#define LRWPAN_ALT_COORDINATOR
//#define LRWPAN_SECURITY_ENABLED

//HAL Stuff

#define LRWPAN_ENABLE_SLOW_TIMER

#define SLOWTICKS_PER_SECOND 10
#define LRWPAN_DEFAULT_BAUDRATE 57600
//#define LRWPAN_DEFAULT_BAUDRATE 9600   //9600
#define LRWPAN_ASYNC_RX_BUFSIZE   32

#define LRWPAN_ASYNC_INTIO

#if (defined(LRWPAN_USE_PC_BIND)&&defined(LRWPAN_COORDINATOR)&&!defined(LRWPAN_ASYNC_INTIO))
//ASYNC RX interrupt IO *must* be used with coordinator if using PC Binding application
//so that serial input from the PC client is not missed.
#define LRWPAN_ASYNC_INTIO

#endif


#endif
