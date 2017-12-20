#ifndef __FUNC_H
	#define __FUNC_H


#define	FUNCRXDATASIZE	8
#define	FUNCTXDATASIZE	8


typedef enum{
 FUNC_ACK =(byte)0x00,
 FUNC_SWITCH_SET =(byte)0x01,
 FUNC_BYPASS2_SET =(byte)0x02,
 FUNC_BYPASS3_SET =(byte)0x03,
 FUNC_BYPASS4_SET =(byte)0x04
}OrderFuncTypeDef;

byte FuncTalk_Trans(byte order);
bool FuncTalk_Echo(void);

#endif
