#ifndef _USER_COMMUNICATION_H_
#define _USER_COMMUNICATION_H_

#include "platform.h"
extern u16 GetCrc16(const u8* pData, int nLength);
extern void LoginMsgPackage(void);//��¼��Ϣ��װ
extern void LBSPackage(void);//LBS��Ϣ��װ
extern void StatePackage(void);//�ϱ�״̬��Ϣ
extern void SeeSosCmdPackage(void);//�ϱ�sos������Ϣ
#endif
