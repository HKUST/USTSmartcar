/******************** (C) COPYRIGHT 2011 Ұ��Ƕ��ʽ���������� ********************
 * �ļ���       ��PIT.h
 * ����         �������жϼ�ʱ��ͷ�ļ�
 *
 * ʵ��ƽ̨     ��Ұ��kinetis������
 * ��汾       ��
 * Ƕ��ϵͳ     ��
 *
 * ����         ��
 * �Ա���       ��http://firestm32.taobao.com
 * ����֧����̳ ��http://www.ourdev.cn/bbs/bbs_list.jsp?bbs_id=1008
**********************************************************************************/	

#ifndef _PIT_H_
#define _PIT_H_



typedef enum PITn
{
  PIT0,
  PIT1,
  PIT2,
  PIT3
}PITn;


void      pit_init(PITn,u32 cnt);                                                //��ʼ��PITn�������ö�ʱʱ��(��λΪbusʱ������)
#define   pit_init_ms(PITn,ms)          pit_init(PITn,ms * bus_clk_khz);         //��ʼ��PITn�������ö�ʱʱ��(��λΪ ms)

//#define  pit_init_ms(PITn,ms)          pit_init(PITn,ms * core_clk_khz); 
#define   PIT_Flag_Clear(PITn)          PIT_TFLG(PITn)|=PIT_TFLG_TIF_MASK        //���жϱ�־

#endif  //_PIT_H_