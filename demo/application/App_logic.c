
#include "App_include.h"
extern 	void GsmInitCallback(eat_bool result);
void Ask_ModuleLBSmsg(void);
void User0MsgCb_SMS_Ready(u8 *pMsgStr, u16 len);
eat_bool DecodeSeverData(void);
void  AppMainRunOnce(void);
void  AppKeyFunction(void);
void Call_ATH(void);
void Call_ATA(void);
void Call_DailNumber(u8 SosSN);


void AppLogicHandle(void)
{
u32 temp=0;
u8 counter=0;
  if(flag.CLCCin)//ע��:��δ�����ڴ˺�������ǰ������ִ��
  	{
  	if(1==Call.dir)//����
  		{
	  	if((0==Call.isSOS)&&(!flag.sosIsNull))// ����������룬����������벻Ϊ��
	  		{
	  		TRACE_DEBUG("NO sos number, sos is not NULL");
			Call_ATH();		//�Ҷ�����
			flag.CLCCin=0;
	  		}
  		}
  	}

  if(flag.timer1up)
  	{
  	counter++;
  	}
  if(counter>=200)
  	{
	TRACE_DEBUG("count=%d",counter);
  	counter=0;
  	}
  if(flag.keycome)
	{
	flag.keycome=0;
	AppKeyFunction();
	}

  switch(StateIndex)
	{
	case PowerOnState:
		TRACE_DEBUG("+++++++PowerOnState++++++");
		break;
	case StandbyState:
		if(!flag.GSMisOk)
			{
			if(!flag.StandbyState_fst)
				{
				if(flag.sms_ready)
					{
					flag.StandbyState_fst=1;
					simcom_gsm_init("1234",GsmInitCallback);//��ʼ��GSM
					eat_timer_start(EAT_TIMER_4,60*1000);//���������Gsm��ʼ�����ɹ���30���ʼ��һ�Ρ�
					}
				}
			else
				{
				if(flag.timer41up)
					{
					flag.timer41up=0;
					flag.StandbyState_fst=0;//60���ӳ�ʼ��һ��
					TRACE_DEBUG("++start to init gsm logic.c line41++\n");
					simcom_gsm_init("1234",GsmInitCallback);//��ʼ��GSM
					}
				}
			}
		else//�ɹ�������ת�����ӷ�����״̬
			{
			flag.LoginOK=0;
			StateIndex=SeverConnectingState;
			flag.StandbyState_fst=0;
			flag.SeverConnecting_fst=0;
			g_reConnectSeverCounter=0;
			eat_timer_stop(EAT_TIMER_4);
			TRACE_DEBUG("stop EAT_TIMER_4 GSMisOk\n");
			AppMainRunOnce();
			TRACE_DEBUG("+StandbyState+to+ConnectingState++++\n");
			}
		break;
	case SeverConnectingState:	//�������ӷ�����
	flag.LoginOK=0;
	flag.SendLoginMsg_fst=0;
		if(flag.sms_ready)
			{
			if(!flag.SeverConnecting_fst)
				{
				g_reConnectSeverCounter=0;
				flag.SeverConnecting_fst=1;
				flag.SeverConnected=0;
				App_connect_server();
				eat_timer_start(EAT_TIMER_6,60*1000);//��������1��������һ��
				}
			else
				{
				if(flag.reConnectSever)
					{
					flag.reConnectSever=0;
					flag.SeverConnected=0;
					eat_timer_start(EAT_TIMER_6,60*1000);//��������1��������һ��
					App_connect_server();
					if(g_reConnectSeverCounter==3)
						{
						TRACE_DEBUG("+SeverConnectingState timer to reboot 20minute\n");
						eat_timer_start(EAT_TIMER_7,20*60*1000);//�������������20����δ����������
						}
					g_reConnectSeverCounter++;
					TRACE_DEBUG("+SeverConnectingState reconnect times=%d\n",g_reConnectSeverCounter);
					if(g_reConnectSeverCounter>200)//��ֹ���
						{
						g_reConnectSeverCounter=4;
						}
					}
				else if(flag.SeverConnected)
					{
					StateIndex=SeverconnectedState;
					eat_timer_stop(EAT_TIMER_7);//�ر�������ʱ��
					eat_timer_stop(EAT_TIMER_6);//�ر�������ʱ��
					AppMainRunOnce();
					TRACE_DEBUG("go to SeverconnectedState\n");
					}
				}
			}
		else
			{
			TRACE_DEBUG("+SeverConnectingState  flag.sms_ready=0\n");
			}
		break;
	case SeverconnectedState://���ӳɹ���׼����¼

		if(!flag.LoginOK)//�����û�е�¼�ɹ�
		{
		if(!flag.SendLoginMsg_fst)//����ǵ�һ�ε�¼
			{
			//g_SendDatAbnormalCnt=0;
			TRACE_DEBUG("+++++LoginMsgPackage to send logomsg++++++\n");
			flag.SendLoginMsg_fst=1;
			LoginMsgPackage();//��¼��Ϣ��װ
			//simcom_send_to_server(socket_id,logoinInfo.logoinArry, sizeof(logoinInfo.logoinArry));
			simcom_send_to_server(socket_id,g_sendDataArry, g_sendDataArryLen);
			eat_timer_start(EAT_TIMER_4,20*1000);//����10s�������ݼ�ʱ
			}
		else
			{
			if(flag.timer4up)//��ʱ�Ͽ����ӣ��������Ӳ�������¼���ݰ�
				{
				flag.timer4up=0;
				if(!flag.connectAbnormal)
					{
					flag.connectAbnormal=1;//���ò��������ӱ�־λ
					g_SendDatAbnormalCnt=1;
					}
				else
					{
					g_SendDatAbnormalCnt++;
					if(g_SendDatAbnormalCnt==3)
						{
						TRACE_DEBUG("+SeverconnectedState EAT_TIMER_8 start!!!!\n");
						eat_timer_start(EAT_TIMER_8,10*60*1000);//�������������10����δ����������
						}
					TRACE_DEBUG("+SeverconnectedState abnormal times=%d\n",g_SendDatAbnormalCnt);
					if(g_SendDatAbnormalCnt>200)//��ֹ���
						{
						g_SendDatAbnormalCnt=5;
						}
					}
				StateIndex=SeverConnectingState;
				TRACE_DEBUG("+++SeverconnectedState is fail go to severconnectingstate one more time+++");
				flag.SeverConnecting_fst=0;
				flag.LoginOK=0;
				flag.SendLoginMsg_fst=0;
				AppMainRunOnce();//û����Ļ��������𣬵ȴ���һ�������ź�
				}
			}
		}
		else	//��¼�ɹ�����ת����¼�ɹ�״̬
		{
		flag.reportLBSmsg_fst=0;
		if(flag.connectAbnormal)
			{
			flag.connectAbnormal=0;
			g_SendDatAbnormalCnt=0;	
			eat_timer_stop(EAT_TIMER_8);//�ر�10����������ʱ��
			TRACE_DEBUG("Login ok ,stop EAT_TIMER_8");
			}
		eat_timer_stop(EAT_TIMER_4);
		StateIndex=reportLBSmsgState;
		TRACE_DEBUG("+++go reportlbsmsgstate+++");
		//flag.LoginOK=0;//++++++
		//flag.SendLoginMsg_fst=0;//+++++
		TRACE_DEBUG("Login ok ,stop EAT_TIMER_4");//+++++
		//Delayms(1000);//+++++
		AppMainRunOnce();
		}
		break;
	case reportLBSmsgState:
		if(!flag.reportLBSmsgOK)
		{
			if(!flag.reportLBSmsg_fst)
			{
			//g_SendDatAbnormalCnt=0;
			flag.reportLBSmsg_fst=1;
			LBSPackage();
			simcom_send_to_server(socket_id,g_sendDataArry, g_sendDataArryLen);
			eat_timer_start(EAT_TIMER_4,20*1000);//����10s�������ݼ�ʱ
			}
		else
			{
			if(flag.timer4up)//��ʱ�Ͽ����ӣ��������Ӳ�������¼���ݰ�
				{
				flag.timer4up=0;
				if(!flag.connectAbnormal)
					{
					flag.connectAbnormal=1;//���ò��������ӱ�־λ
					}
				else
					{
					if(g_SendDatAbnormalCnt==3)
						{
						//g_SendDatAbnormalCnt=0;
						eat_timer_start(EAT_TIMER_8,10*60*1000);//�������������10����δ����������
						}
					g_SendDatAbnormalCnt++;
					if(g_SendDatAbnormalCnt>200)//��ֹ���
						{
						g_SendDatAbnormalCnt=5;
						}
					}
				StateIndex=SeverConnectingState;
				flag.SeverConnecting_fst=0;
				flag.reportLBSmsg_fst=0;
				AppMainRunOnce();//��main����һ��
				}
			}
		}
		else
		{
		if(flag.connectAbnormal)
			{
			flag.connectAbnormal=0;
			g_SendDatAbnormalCnt=0;	
			eat_timer_stop(EAT_TIMER_8);//�ر�10����������ʱ��
			TRACE_DEBUG("report LBS ok stop EAT_TIMER_8");
			}
		flag.reportLBSmsg_fst=0;
		eat_timer_stop(EAT_TIMER_4);
		StateIndex=reportStatemsgState;
		TRACE_DEBUG("report LBS ok ,stop EAT_TIMER_4");
		AppMainRunOnce();
		}
		TRACE_DEBUG("+++reportLBSmsgState+++");
		
		break;
	case reportStatemsgState:
		if(!flag.reportStatemsgOK)
		{
			if(!flag.reportStatemsg_fst)
			{
			flag.reportStatemsg_fst=1;
			StatePackage();
			simcom_send_to_server(socket_id,g_sendDataArry, g_sendDataArryLen);
			eat_timer_start(EAT_TIMER_4,20*1000);//����10s�������ݼ�ʱ
			}
		else
			{
			if(flag.timer4up)//��ʱ�Ͽ����ӣ��������Ӳ�������¼���ݰ�
				{
				flag.timer4up=0;
				if(!flag.connectAbnormal)
					{
					flag.connectAbnormal=1;//���ò��������ӱ�־λ
					}
				else
					{
					g_SendDatAbnormalCnt++;
					}
				StateIndex=SeverConnectingState;
				flag.SeverConnecting_fst=0;
				flag.reportStatemsg_fst=0;
				AppMainRunOnce();//��main����һ��
				}
			}
		}
		else
		{
		if(flag.connectAbnormal)
		{
		flag.connectAbnormal=0;
		g_SendDatAbnormalCnt=0;			
		}
		flag.reportStatemsg_fst=0;
		eat_timer_stop(EAT_TIMER_4);
		StateIndex=SeverLogoinState;
		flag.reportStatemsgOK=0;
		flag.reportStatemsg_fst=0;
		flag.enterLogoinedState_fst=0;
		TRACE_DEBUG("stop EAT_TIMER_4");
		AppMainRunOnce();
		}
		TRACE_DEBUG("+++reportStatemsgState+++");
		
		break;
	case SeverLogoinState:
		if(!flag.enterLogoinedState_fst)
			{
			//TRACE_DEBUG("+++SeverLogoinState+++");
			flag.enterLogoinedState_fst=1;
			temp=2*60;//60s����һ������
			eat_timer_start(EAT_TIMER_5,temp*1000);
			TRACE_DEBUG("SeverLogoinState+++eat_timer_start=%ds",temp);
			AppMainRunOnce();
			}
		else
			{
			if(flag.timer5up)
				{
				flag.enterLogoinedState_fst=0;
				flag.timer5up=0;
				StateIndex=reportLBSmsgState;
				flag.reportLBSmsgOK=0;
				flag.reportLBSmsg_fst=0;
				TRACE_DEBUG("report LBS and state \n");
				AppMainRunOnce();
				}
			else if(flag.GPS_TIMER_UP)
				{
				flag.GPS_TIMER_UP=0;
				if(flag.GPSisOk)
					{
					flag.GPSisOk=0;
					GPSPackage();
					simcom_send_to_server(socket_id,g_sendDataArry, g_sendDataArryLen);
					TRACE_DEBUG("report GPS MSG \n");
					}
				}
				{
				//eat_trace("module will go to sleep\n");
				//eat_sleep(1000);
				//eat_sleep_enable(TRUE);
				}
			}
		
		break;
	default:
		break;
  	}
  if(flag.recvSeverData)
  	{
  	flag.recvSeverData=0;
	DecodeSeverData();
  	}
  
	if (flag.reBootTP_10)
	{
	flag.reBootTP_10=0;
	TRACE_DEBUG("\n\n\n\n\n\n reboot! 10 minute\n\n\n");
	eat_sleep(100);
	//eat_reset_module();
	}
	if (flag.reBootTP_20)
	{
	flag.reBootTP_20=0;
	TRACE_DEBUG("\n\n\n\n\n\n  reboot! 20 minute\n\n\n");
	eat_sleep(100);
	//eat_reset_module();
	}
#if 1
	if(flag.ServerCmdSeeSOS)
		{
		flag.ServerCmdSeeSOS =0;
		SeeSosCmdPackage();//�ϱ�sos������Ϣ
		simcom_send_to_server(socket_id,g_sendDataArry, g_sendDataArryLen);
		TRACE_DEBUG("report SOS Number MSG \n");
		}
#endif

}

#define ServerToTerminalCMDTypeMaxNum 3

u8 *Server_To_Terminal_CMD_Type[]={
		"SEESOS#",	// 0
		"GPSON",     // 1  GPSON,60# ( web���͵�������������)
		"SOS,A"
	};

eat_bool DecodeSeverData(void)
	{
 	eat_bool ret_val = EAT_FALSE;
	u8 i;
	u16 msgLen=0;			//��Ϣ�ĳ���
	u16 CmdLen=0;			//����ĳ���
	u8 serverMark4byte[4]={0,0,0,0};
	u8 CmdText[100]={0,0,0,0,0,   0,0,0,0,0};
	u8 CmdType = 0xFF;
	//u16 msgStartMark=0x0000;
	u16 crc16=0,datacrc16=0;
	u16 datacrc1=0,datacrc2=0;
	u8 ProtocolNumber=0;
	u8	*pCmdText = CmdText;

	u8 para1Num;
	u8 para[20]={0,0,0,0,0, 0,0,0,0,0 ,0,0,0,0,0, 0,0,0,0,0};
	#if 0
	for(i=0;i<g_RecvServerDataLen;i++)
		{
		//Read from Server.TODO something
		eat_trace("R222 data 0x=%x",g_RecvServerData[i]);
		}
	#endif
	if(g_RecvServerData[0]==0x78&&g_RecvServerData[0]==0x78)
		{
		;//TRACE_DEBUG("msgStartMark==0x7878\n");
		}
	else
		{
		TRACE_DEBUG("ERRO msgStartMark==0x%x,0x%x\n",g_RecvServerData[0],g_RecvServerData[1]);
		return ret_val;
		}
	for(i=0;i<g_RecvServerDataLen;i++)
	{
	//Read from Server.TODO something
	eat_trace("R 333 default data 0x=%x",g_RecvServerData[i]);
	}

 	msgLen=g_RecvServerData[2];
	datacrc1=g_RecvServerData[g_RecvServerDataLen-4];
	datacrc2=g_RecvServerData[g_RecvServerDataLen-3];
	datacrc16=(datacrc1<<8)|datacrc2;
	
	crc16=GetCrc16((const u8*)g_RecvServerData+2,msgLen-1);
	if (datacrc16==crc16)
		{
		;//TRACE_DEBUG("CRC ok\n");
		}
	else
		{
		TRACE_DEBUG("CRC NG\n");
		return ret_val;
		}
	#if DebugMsgOnOff
	TRACE_DEBUG("GetCrc16 crc16=0x%x,0x%x",datacrc16,crc16);
	TRACE_DEBUG("data lenth is msgLen=%x",msgLen);
	#endif

	
	ProtocolNumber=g_RecvServerData[3];	//��Э��Ÿ���ʱ����

	switch(ProtocolNumber)
	{
		case LoginMsgNUM :
			flag.LoginOK=1;//��ɳɹ�
			//eat_send_msg_to_user(EAT_USER_0, EAT_USER_0, EAT_FALSE, 5, "NULL", EAT_NULL);//�������������߳�ִ��һ��
			AppMainRunOnce();
             #if DebugMsgOnOff
             TRACE_DEBUG("++++register server ok!!!++++\n");
             #endif
			break;
		case LBSMsgNUM:
			//flag.LBSmsgReady=1;
			flag.reportLBSmsgOK=1;
           #if DebugMsgOnOff
           	TRACE_DEBUG("++++LBSMsg report msg ok!!!++++\n");
           #endif
			AppMainRunOnce();
			break;
		case StateMsgNUM:
			//flag.LBSmsgReady=1;
			flag.reportStatemsgOK=1;
            #if DebugMsgOnOff
            TRACE_DEBUG("++++StateMsg report msg ok!!!++++\n");
            #endif
			AppMainRunOnce();
			break;
		case ServerToTerminalCMD:
         #if DebugMsgOnOff
         TRACE_DEBUG("+++get ServerToTerminalCMD+++\n");
         #endif
		 if(msgLen<=4)
		 	{
		 	TRACE_DEBUG("msgLen<=4\n");
			for(i=0;i<g_RecvServerDataLen;i++)
			{
			//Read from Server.TODO something
			eat_trace("msgLen<=4 R333 0x=%x",g_RecvServerData[i]);
			}
			break;
		 	}
		 CmdLen=g_RecvServerData[4];	//0x80 �����ָ���
		 serverMark4byte[0]=g_RecvServerData[5];
		 serverMark4byte[1]=g_RecvServerData[6];
		 serverMark4byte[2]=g_RecvServerData[7];
		 serverMark4byte[3]=g_RecvServerData[8];

		 for(i=0;i<4;i++)
		 	{
		 	g_serverMark4byte[i]=serverMark4byte[i];
		 	TRACE_DEBUG("g_serverMark4byte[%d]=%d\n",i,g_serverMark4byte[i]);
		 	}
		 for(i=0;i<CmdLen-4;i++)
		 	{
				CmdText[i]=g_RecvServerData[9+i];
				TRACE_DEBUG("CmdText[%d]=0x%x,%c\n",i,CmdText[i],CmdText[i]);
		 	}
		 for(i=0;i<ServerToTerminalCMDTypeMaxNum;i++)
		 	{
				if (0==strncmp(Server_To_Terminal_CMD_Type[i], CmdText, sizeof(Server_To_Terminal_CMD_Type[i]))){
				 	 CmdType=i;
					 break;
			 	}
		 	}

		 switch(CmdType)
		 	{
		 	case 0:				
				flag.ServerCmdSeeSOS = 1;
				TRACE_DEBUG("SEE sosCMD\n");
				break;
			case 1:
				flag.ServerCmdGPSON= 1;
				pCmdText = (u8*)strchr(pCmdText,',');
				++pCmdText;
				g_ServerCmdGPSONTimer=atoi(pCmdText);
				TRACE_DEBUG("GPSON CMD g_ServerCmdGPSONTimer=%d\n",g_ServerCmdGPSONTimer);
			
				break;
			case 2:
				pCmdText=strchr(pCmdText,',')+1;
				if((*pCmdText=='A')||(*pCmdText=='a'))
					{
					for(i=0;i<4;i++)
					{
					pCmdText=strchr(pCmdText,',')+1;//��Ѱ��1������
					AppMemSet(para,0x00,20);//���para
					sscanf(pCmdText,"%15[^,|^#]",para);//��������','��ֹͣ���
					//�ж�para�����ֵĸ���
					para1Num=JudgeNumber(para);
					TRACE_DEBUG("i=%d,para1Num=%d,num1=%s",i,para1Num,para);
					if(para1Num>0)//�������λ������0��洢��λ��1
						{
						strcpy(SosNumber[i],para);	//2016 -5-24 add ͬ������sos�ڴ����
						app_nvram_save(SOS_NUM_1_ADDR+i, para,13);
						ret_val=TRUE;
						}
					else
						{
						AppMemSet(SosNumber[para[0]-'0'],0x00,11);//2016 -5-24 add ͬ������sos�ڴ���룬�������
						}
					}
					if(ret_val)
						{
						TRACE_DEBUG("SOS ADD OK");
						}
					else
						{
						TRACE_DEBUG("SOS ADD ERROR");
						}
					}						
				TRACE_DEBUG("SOS ADD CMD\n");
				break;
			default:
				TRACE_DEBUG("Server_To_Terminal_CMD_Type error cmd\n");
				break;
		 	}
		 #if 0
		 if (0==CmdType)
		 	{
		 	flag.ServerCmdSeeSOS = 1;
		 	TRACE_DEBUG("SEE sosCMD\n");
		 	}
		 else if(1==CmdType)
		 	{
		 	flag.ServerCmdGPSON= 1;
			pCmdText = (u8*)strchr(pCmdText,',');
			++pCmdText;
			g_ServerCmdGPSONTimer=atoi(pCmdText);
			TRACE_DEBUG("GPSON CMD g_ServerCmdGPSONTimer=%d\n",g_ServerCmdGPSONTimer);
		 	}
		 else
		 	{
		 	TRACE_DEBUG("Server_To_Terminal_CMD_Type error cmd\n");
		 	}
				
			break;
			#endif
		default:
		for(i=0;i<g_RecvServerDataLen;i++)
		{
		//Read from Server.TODO something
		eat_trace("R222 default data 0x=%x",g_RecvServerData[i]);
		}
			break;
	}
	
	ret_val=EAT_TRUE;
	return ret_val;
 	}

void Ask_ModuleLBSmsg(void)
{	
	eat_modem_write("AT+CREG?\n",strlen("AT+CREG?\n"));
}
void SetCregValue(void)// �Զ��ϱ�CREG״̬
{
eat_modem_write("AT+CREG=2\n",strlen("AT+CREG=2\n"));
eat_sleep(50);
}
void SetCENGValue(void)//����ģʽ����
{
eat_modem_write("AT+CENG=3,0\n",strlen("AT+CENG=3,0\n"));
eat_sleep(50);
}
void User0MsgCb_SMS_Ready(u8 *pMsgStr, u16 len)
{
eat_trace("app_user0: recv msg~SMS Ready start to inin GSM\n");
simcom_gsm_init("1234",GsmInitCallback);
}

//YML ADD 2015-8-17
MsgEntityQueue s_event_user0_msgQueue=
	{
		{
			{"SMS Ready",User0MsgCb_SMS_Ready}, 
		},
		1// 4
	};
	//yml add 2015-8-17
#define MSG_DELAY "DELAY:"
#define MSG_END "\x0d\x0a"
#define MSG_CR  '\x0d'
#define MSG_LF  '\x0a'
#define MSG_CTRL_Z "\x1a"
#define USE_MSG_EVENT_MAX	20
u8 *USE_MSG_EVENT[]={
	"SMS Ready",	//0
    "SEVER_CONNECT",
    "REC_SEVER_DATA",
	"SEVER_CLOSE",
	"SEND_TO_SEVER_DATA", 
    "SOC_ACKED",
    "TIMER3 UP",
    "LBS ASK OK",
    "KEY1",		// 8
    "KEY2",		// 9
    "KEY3",		// 10
    "KEY4",		// 11
    "KEY5",		// 12
    "KEY6",		// 13
    "KEY7",		// 14
    "KEY8",		// 15
    "KEY9",		// 16
    "KEY10",	// 17
	"KEY11",	// 18
	"KEY12",	// 19
};

//���յ�msg��Ϣ���д���������˽���logicHandle���������߼�����
void eat_event_user0_msg_handler(u8* pMsgStr, u16 len)
{
	u8	i = 0;
	u8	*p = pMsgStr;

#if 1
s16  msgType = -1;
for (i = 0; i < USE_MSG_EVENT_MAX; i++)
{
	if (0==strncmp(USE_MSG_EVENT[i], p, len)){
		msgType=i;
		break;
	}
}
switch(msgType)
{
case 0:		//sms ready ��ʼ���ӷ�����

	//StateIndex=SeverConnectingState;
	//User0MsgCb_SMS_Ready(pMsgStr,len);
	flag.sms_ready=1;
	
	break;
case 1:	//�Ѿ�����
	flag.SeverConnected=1;
	//StateIndex=SeverconnectedState;
//	LoginMsgPackage();//��¼��Ϣ��װ
//	simcom_send_to_server(socket_id,logoinInfo.logoinArry, sizeof(logoinInfo.logoinArry)); 
	break;
case 2://�յ�������������
	flag.recvSeverData=1;
	
	break;
case 3:	//
	flag.reConnectSever=0;
	flag.connectAbnormal=1;
	flag.SeverConnected=0;
	flag.SeverConnecting_fst=0;
	StateIndex=SeverConnectingState;
	break;
case 6://	TIMER3 UP
	if(!flag.LBSmsgReady)
		{
		flag.askLBSmsg=1;
		}
	if(flag.sms_ready)
		{		
		TRACE_DEBUG("yml++send atcom +++\n");
		SendAtComHandle();
		}
	break;

case 7:
	flag.LBSmsgReady=1;//����ѯ��LBS�ɹ�
	break;
case 8: 	// KEY1
case 9:		// KEY2
case 10:	
case 11:
case 12:
case 13:
case 14:
case 15:
case 16:
case 17:
case 18:
case 19:	// KEY12
	flag.keycome=1;
	g_KeyValue=msgType-7;
	TRACE_DEBUG("main app rev��USE_MSG_EVENT[%d] g_KeyValue =%d",msgType,g_KeyValue);
	break;

default:
	break;
}


#else


		while(p) {
			/* ignore \r \n */
			while ( MSG_CR == *p || MSG_LF == *p)
			{
				p++;
			}
	
			for (i = 0; i < s_event_user0_msgQueue.count; i++)
			{
				if (!strncmp(s_event_user0_msgQueue.msgEntityArray[i].p_msgStr, p, strlen(s_event_user0_msgQueue.msgEntityArray[i].p_msgStr))) {
					if(s_event_user0_msgQueue.msgEntityArray[i].p_msgCallBack)
						s_event_user0_msgQueue.msgEntityArray[i].p_msgCallBack(p,len);
				}
			}
	
			p = (u8*)memchr(p,0x0a,0);
		}
		
#endif
	}
void AppMainRunOnce(void)
{
eat_send_msg_to_user(EAT_USER_0, EAT_USER_0, EAT_FALSE, 5, "NULL", EAT_NULL);//�������������߳�ִ��һ��
}
void AppKeyFunction(void)
{
	TRACE_DEBUG("app key function g_keyValue=%d",g_KeyValue);
	switch(StateIndex)
	{
	case PowerOnState:
	case StandbyState:
		break;
	case SeverConnectingState:	//�������ӷ�����
	case SeverconnectedState://���ӳɹ���׼����¼
	case reportLBSmsgState:
	case reportStatemsgState:
	case SeverLogoinState:
	if(flag.CLCCin)//1.������ȥ��״̬��Ϣ�ϱ�
		{TRACE_DEBUG("flag.sosIsNull  0000\n");
			if(1==Call.dir)
				{
				if(flag.sosIsNull)
					{
					TRACE_DEBUG("flag.sosIsNull  9999\n");
					}
				else
					{
					TRACE_DEBUG("flag.sosIs no Null  888\n");
					}
				if(Call.isSOS>0||flag.sosIsNull)//����������������������Ϊ�գ�����Ӧ
					{					
					TRACE_DEBUG("is sos SN=%d,Number=%s",Call.isSOS,Call.phoneNum);
					switch(g_KeyValue)
						{
						case 1:
						case 2:
						case 3:
						flag.CLCCin=0;
						Call_ATA();
						break;
						case 4:
						break;
						case 16:
						flag.CLCCin=0;
						Call_ATH();
						break;
						default:
						break;
						}
					}
				//else ���������logic����ѭ����ȥ����
				}
				
			}
			if(g_KeyValue==16)//����ǹҶϼ�,������ʲô����¶�����һ�ιҶ����
				{
				flag.CLCCin=0;
				Call_ATH();
				}
			if(Call.stat>=6)
			{
			switch(g_KeyValue)
					{
					TRACE_DEBUG("dailing call!");
					case 7:	
					case 8:
					case 9:
					case 10:
						clear_CallArry();
						Call_DailNumber(g_KeyValue-7);						
						break;
					default:
						break;
					}
			}
		break;
	default:
		break;
	}
}
void Call_ATH(void)
{	
	eat_modem_write("ATH\r\n",strlen("ATH\r\n"));
	eat_modem_set_poweron_urc_dir(EAT_USER_1);//һ��Ĭ�϶���user0	���ں�core����Ϣ�ظ���EAT_USER_0
}
void Call_ATA(void)
{	
	eat_modem_write("ATA\r\n",strlen("ATA\r\n"));
	eat_modem_set_poweron_urc_dir(EAT_USER_1);//һ��Ĭ�϶���user0	���ں�core����Ϣ�ظ���EAT_USER_0
}
void Call_DailNumber(u8 SosSN)
{
    u8 pAtcom[16] = {0};
	AppMemSet(pAtcom,0x00,16);
    sprintf(pAtcom,"ATD%s;\r\n",SosNumber[SosSN]);
	eat_modem_write(pAtcom,strlen(pAtcom));
	eat_modem_set_poweron_urc_dir(EAT_USER_1);//һ��Ĭ�϶���user0	���ں�core����Ϣ�ظ���EAT_USER_0
}

