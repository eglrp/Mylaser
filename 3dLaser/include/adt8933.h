#ifndef _CTRL_CARD_H_
#define _CTRL_CARD_H_
int _stdcall adt8933_initial(void);
int _stdcall set_pulse_mode(int cardno,int axis,int value,int logic,int dir_logic);
int _stdcall set_limit_mode(int cardno,int axis,int v1,int v2,int logic);
int _stdcall set_stop0_mode(int cardno,int axis,int v,int logic);
int _stdcall set_stop1_mode(int cardno,int axis,int v,int logic);
int _stdcall get_status(int cardno,int axis,int *v);
int _stdcall get_inp_status(int cardno,int *v);

int _stdcall set_acc(int cardno, int axis,long add);
int _stdcall set_startv(int cardno, int axis,long speed);
int _stdcall set_speed(int cardno, int axis,long speed);
int _stdcall set_command_pos(int cardno, int axis,long pos);
int _stdcall set_actual_pos(int cardno, int axis,long pos);
int _stdcall get_command_pos(int cardno,int axis,long *pos);
int _stdcall get_actual_pos(int cardno,int axis,long *pos);
int _stdcall get_speed(int cardno,int axis,long *speed);
int _stdcall pmove(int cardno,int axis,long pos);
int _stdcall dec_stop(int cardno,int axis);
int _stdcall sudden_stop(int cardno,int axis);
int _stdcall inp_move2(int cardno,int axis1,int axis2,long pulse1,long pulse2);
int _stdcall inp_move3(int cardno,long pulse1,long pulse2,long pulse3);

int _stdcall read_bit(int cardno,int number);
int _stdcall write_bit(int cardno,int number,int value);

int _stdcall set_daout(int cardno,int ch,int value);//ch 1-3
int _stdcall set_pwm(int cardno,long freq,float value); 
int _stdcall set_delay_time(int cardno,int value);//��ʱʱ�� ��λΪ 1/16uS
int _stdcall get_delay_status(int cardno,int *v);//
int _stdcall get_lib_version();

//-----------------------------------------------------------//
//               Ӳ�������ຯ�� 2008.9.10                    //
//-----------------------------------------------------------//

int _stdcall get_hardware_version(int cardno);
/******************************��ȡӲ���汾��******************************
cardno	����
	����ֵ	 Ӳ���汾�ţ�0x33XX��
******************************************************************/

int _stdcall read_fifo(int cardno,int *value);
/******************************��ȡ����ָ����******************************
	cardno	����
value     ��ǰ����ָ����ָ��
	����ֵ	 0����ȷ   1������
************************************************************************/

int _stdcall reset_fifo(int cardno);
/******************************���û���******************************
	cardno	����
	����ֵ	 0����ȷ   1������
******************************************************************/

int _stdcall get_fifo_status(int cardno,int *status);
/******************************��ȡ����״̬******************************
cardno	����
value     ��ǰ����ָ����ָ��
	����ֵ	 0����ȷ   1������
******************************************************************/

int _stdcall fifo_set_pwm_freq1(int cardno,int value);
/******************************����PWMƵ�����*****************************
cardno	����
	value     PWM�������ʱ�䣨ʱ�䵥λΪ1/8us��
Ƶ�� = 1(s) / ����ʱ��
******************************************************************/

int _stdcall fifo_set_pwm_freq2(int cardno,int value);
/******************************����PWMռ�ձ����*****************************
cardno	����
	value     PWM�����Ч��ƽʱ�䣨ʱ�䵥λΪ1/8us��
  ռ�ձ� = ��Ч��ƽʱ�� / ����ʱ��
******************************************************************/

int _stdcall fifo_set_delay_time1(int cardno,int value);
/******************************����Ӳ����ʱ*****************************
cardno	����
	value     Ӳ����ʱʱ�䣨ʱ�䵥λΪ1/8us��
  ע��ÿ��������һ��fifo_set_daout�Զ˿�1�Ͷ˿�2�������ʱ�˺������õ���ʱʱ��
******************************************************************/

int _stdcall fifo_set_delay_time2(int cardno,int value);
/******************************����Ӳ����ʱ*****************************
cardno	����
	value     Ӳ����ʱʱ�䣨ʱ�䵥λΪ1/8us��
  ע��ÿ��������һ��fifo_set_daout�Զ˿�1�Ͷ˿�2�������ʱ�˺������õ���ʱʱ��
******************************************************************/

int _stdcall fifo_set_daout(int cardno,int ch,int value);
/******************************����DA���*****************************
cardno	����
	ch        �˿ں�(1,4)
	value     DA���ֵ(-32768,32767)
ע��valueΪ-32768���Ϊ-5V��valueΪ0���Ϊ0V��valueΪ32767���Ϊ5V�������������ơ�
******************************************************************/

int _stdcall fifo_set_delay_time3(int cardno,int value);

int _stdcall fifo_set_delay_time4(int cardno,int value);

int _stdcall fifo_set_laser_control(int cardno,int value);
/******************************���漤�⿪��*****************************
cardno	����
value 	1  ����     0 ����
******************************************************************/
int _stdcall fifo_set_pwm_freq3(int cardno,int value);
int _stdcall fifo_set_pwm_freq4(int cardno,int value);
int _stdcall fifo_set_pwm_freq5(int cardno,int value);
int _stdcall fifo_set_pwm_freq6(int cardno,int value);
int _stdcall fifo_set_pwm_trans(int cardno,int value);
#endif
