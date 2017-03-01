#include "tcp_client_demo.h" 
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "malloc.h"
#include "stdio.h"
#include "string.h" 
#include "math.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//TCP Client ���Դ���	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/8/15
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//*******************************************************************************
//�޸���Ϣ
//��
////////////////////////////////////////////////////////////////////////////////// 	   
 #define pi 3.1415
//TCP Client�������ݻ�����
u8 tcp_client_recvbuf[TCP_CLIENT_RX_BUFSIZE];	
//TCP������������������
//const u8 *tcp_client_sendbuf="GD0530055001\r";
const u8 cmd[5][14]={"%ST\r","BM\r","QT\r","RS\r",
	"GD0480060000\r"};
int recv_len,cp,data_len=600-480;
u32 data[3000];	
u8 flag = 0;
	
//TCP Client ����ȫ��״̬��Ǳ���
//bit7:0,û������Ҫ����;1,������Ҫ����
//bit6:0,û���յ�����;1,�յ�������.
//bit5:0,û�������Ϸ�����;1,�����Ϸ�������.
//bit4~0:����

u8 tcp_client_flag;	 
u32 convert(u8* data,int len){
	u32 ans=0,i=0;
	while (i<len){
		ans<<=6;
		ans+=data[i]-48;
		i++;
	}
	return ans;
}
void decode(u8 *databuf,int len){
	int i=0,p=0;
	u8 ptr=0;
	u8* tmp;
	tmp=mymalloc(SRAMIN,3);
	while (databuf[i]!=0x0a) i++;
	i+=1;
	while (databuf[i]!=0x0a) i++;
	i+=1;
	data[p++]=convert(databuf+i,4);
	i+=6;
	while (i<len){
		if (databuf[i+1]==0x0a)
			i+=2;
		memcpy((u8*)tmp+ptr,(u8*)&databuf[i],1);
		ptr++;i++;
		if (ptr==3) {
			data[p++]=convert(tmp,3);
			memset(tmp,0,3);
			ptr=0;
		}
	}
	myfree(SRAMIN,tmp);
}

float ToAngle(int l,int r,float alfa){
	float len,ans;
	len = l*l+r*r-2*l*r*cos(alfa);
	ans = l*l/len*powf(sin(alfa),2);
	ans = sqrtf(ans);
	return asin(ans);
}

float linear(const u32* data,int len){
	float Ax = 0,Ay = 0,ansX = 0,ansY = 0;
	float alfa,tmp;
	int i;
//	for (i=1;i<=len+1;i++){
//		alfa = (90-data_len/2*0.25+(i-1)*0.25)*pi/180;
//		Ax += data[i]*cos(alfa);
//		Ay += data[i]*sin(alfa);
//	}
//	Ax /= data_len+1;
//	Ay /= data_len+1;
//	for (i=1;i<=len+1;i++){
//		alfa = (90-data_len/2*0.25+(i-1)*0.25)*pi/180;
//		tmp = data[i]*cos(alfa)-Ax;
//		ansY += tmp*(data[i]*sin(alfa)-Ay);
//		ansX += tmp*tmp;
//	}
//	tmp = atan2(ansY,ansX);
	
	for (i=1;i<=len+1;i++){
		alfa = (90-len/2*0.25+(i-1)*0.25)*pi/180;
		tmp = data[i]*cos(alfa);
		printf("%d,",data[i]);
		Ax += tmp;
		Ay += data[i]*sin(alfa);
		ansY += data[i]*sin(alfa)*tmp;
		ansX += tmp*tmp;
	}
	printf("\n");
	tmp = atan2(ansY-Ax*Ay/(len+1),ansX-Ax*Ax/(len+1));
	
	return tmp;
}
//����Զ��IP��ַ
void tcp_client_set_remoteip(void)
{
	u8 *tbuf;
	u16 xoff;
	u8 key;
//	LCD_Clear(WHITE);
//	POINT_COLOR=RED;
	printf("TCP Client Test\n");
	printf("Remote IP Set\n");
	
//	LCD_ShowString(30,30,200,16,16,"TCP Client Test");
//	LCD_ShowString(30,50,200,16,16,"Remote IP Set");  
//	LCD_ShowString(30,70,200,16,16,"KEY0:+  KEY2:-");  
//	LCD_ShowString(30,90,200,16,16,"KEY_UP:OK");  
	
	tbuf=mymalloc(SRAMIN,100);	//�����ڴ�
	if(tbuf==NULL)return;
	//ǰ����IP���ֺ�DHCP�õ���IPһ��
	lwipdev.remoteip[0]=lwipdev.ip[0];
	lwipdev.remoteip[1]=lwipdev.ip[1];
	lwipdev.remoteip[2]=lwipdev.ip[2]; 
	
	//sprintf((char*)tbuf,"Remote IP:%d.%d.%d.",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2]);//Զ��IP
	
	printf("Remote IP:%d.%d.%d.%d\n",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);
	
	//LCD_ShowString(30,110,210,16,16,tbuf); 
	
//	POINT_COLOR=BLUE;
//	xoff=strlen((char*)tbuf)*8+30;
//	LCD_ShowxNum(xoff,110,lwipdev.remoteip[3],3,16,0); 

	while(1)
	{
//		key=KEY_Scan(0);
//		if(key==WKUP_PRES)break;
		key = USART_RX_BUF[0];
		if (key == 5) break;
		else if(key>0&&key<9)
		{
			if(key==8||key==6)lwipdev.remoteip[3]++;//IP����
			if(key==2||key==4)lwipdev.remoteip[3]--;//IP����
			printf("Remote IP:%d.%d.%d.%d\n",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);
//			LCD_ShowxNum(xoff,1100,lwipdev.remoteip[3],3,16,0X80);//��ʾ��IP
		}
	}
	//myfree(SRAMIN,tbuf); 
}
 

//TCP Client ����
void tcp_client_test(void)
{
 	struct tcp_pcb *tcppcb;  	//����һ��TCP���������ƿ�
	struct ip_addr rmtipaddr;  	//Զ��ip��ַ
	float alfa,ans;
	u8 f_cnt = 0;
	u8 *tbuf;
 	u8 key;
	u8 res=0;		
	u8 t=0; 
	u8 connflag=0;		//���ӱ��
	recv_len = 0;cp=0;
	
	
	tcp_client_set_remoteip();//��ѡ��IP
//	LCD_Clear(WHITE);	//����
//	POINT_COLOR=RED; 	//��ɫ����
	
//	LCD_ShowString(30,30,200,16,16,"TCP Client Test");
//	LCD_ShowString(30,50,200,16,16,"KEY0:Send data");  
//	LCD_ShowString(30,70,200,16,16,"KEY_UP:Quit");  
	
	printf("TCP Client Mode\n");
	
//	tbuf=mymalloc(SRAMIN,200);	//�����ڴ�
//	if(tbuf==NULL)return ;		//�ڴ�����ʧ����,ֱ���˳�
	
//	sprintf((char*)tbuf,"Local IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);//������IP
//	LCD_ShowString(30,90,210,16,16,tbuf);  
	
	printf("Local IP:%d.%d.%d.%d\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	
//	sprintf((char*)tbuf,"Remote IP:%d.%d.%d.%d",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);//Զ��IP
//	LCD_ShowString(30,110,210,16,16,tbuf);  
	
	printf("Remote IP:%d.%d.%d.%d\n",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);
	
//	sprintf((char*)tbuf,"Remote Port:%d",TCP_CLIENT_PORT);//�ͻ��˶˿ں�
//	LCD_ShowString(30,130,210,16,16,tbuf);
	
	printf("Remote Port:%d\n",TCP_CLIENT_PORT);
	
//	POINT_COLOR=BLUE;
//	LCD_ShowString(30,150,210,16,16,"STATUS:Disconnected"); 

	printf("STATUS:Disconnected\n");

	tcppcb=tcp_new();	//����һ���µ�pcb
	if(tcppcb)			//�����ɹ�
	{
		IP4_ADDR(&rmtipaddr,lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]); 
		tcp_connect(tcppcb,&rmtipaddr,TCP_CLIENT_PORT,tcp_client_connected);  //���ӵ�Ŀ�ĵ�ַ��ָ���˿���,�����ӳɹ���ص�tcp_client_connected()����
 	}else res=1;
	while(res==0)
	{
		key=USART_RX_BUF[0];
		if(key==0)break;
		if(key==5)//5������,��������
		{
			tcp_client_flag|=1<<7;//���Ҫ��������
			printf("sent");
		}
		if (key<0&&key<9){
			if((key==4||key==2)) cp--;
			else if (key==8||key==6) cp++;
			if (cp<0)cp+=5;
			else cp%=5;
		//LCD_Fill(30,250,lcddev.width-1,lcddev.height-1,WHITE);//����һ������
		//LCD_ShowNum(30,290,cp,1,16);
			printf("ready to send :%s\n",cmd[4]);
		}
		if(tcp_client_flag&1<<6)//�Ƿ��յ�����?
		{
			if (memcmp(tcp_client_recvbuf,cmd[4],12)==0){
				decode(tcp_client_recvbuf,recv_len);
//				printf("\r\nTimeStamp:%d\r\n",data[0]);
//				for (recv_len=1;recv_len<=data_len+1;recv_len++){
//					printf("%d ",data[recv_len]);
//					if (recv_len%5==0) printf("\r\n");
//				}
//				printf("\r\n");
				
//				for (recv_len=1;recv_len<=data_len/2;recv_len++){
//					alfa = ToAngle(data[recv_len],data[data_len/2+recv_len+1],(data_len/2+1)*0.25*pi/180);
//					alfa = (90-recv_len*0.25)*pi/180-alfa;
//					ans += alfa*180/pi;
//					printf("%f ",alfa*180/pi);
//					if (recv_len%5==0)printf("\r\n");}
//				ans /= data_len/2;
				
				
//				alfa = ToAngle(data[1],data[data_len+1],data_len*0.25*pi/180);
//				alfa = (90-data_len/2*0.25)*pi/180-alfa;
//				ans += alfa*180/pi;
				
//				alfa = linear(data,data_len);
//				ans += alfa/pi*180;
//				
//				if (f_cnt==9){	
//					ans /= 10;
//					f_cnt = 0;
//					LCD_Fill(30,190,lcddev.width-1,lcddev.height-1,WHITE);//����һ������
//					//printf("%f\r\n",ans);
//					sprintf((char*)tbuf,"%f degree",ans);
//					LCD_ShowString(30,190,200,16,16,tbuf);
//					ans = 0; 
//				}else{
//					//printf("%f\r\n",alfa/pi*180);
//					tcp_client_flag|=1<<7;
//					f_cnt++;
//				}
				printf("Recieve Ok\n");
			}
			else{
//				LCD_Fill(30,190,lcddev.width-1,lcddev.height-1,WHITE);//����һ������
//				LCD_ShowString_length(30,190,lcddev.width-35,lcddev.height-230,16,tcp_client_recvbuf,recv_len);//��ʾ���յ�������
				printf("%s\n",tcp_client_recvbuf);
			}
			recv_len=0;
			tcp_client_flag&=~(1<<6);//��������Ѿ���������.
		}
		if(tcp_client_flag&1<<5)//�Ƿ�������?
		{
			if(connflag==0)
			{ 
//				LCD_ShowString(30,150,lcddev.width-30,lcddev.height-190,16,"STATUS:Connected   ");//��ʾ��Ϣ		
//				POINT_COLOR=RED;
//				LCD_ShowString(30,170,lcddev.width-30,lcddev.height-190,16,"Receive Data:");//��ʾ��Ϣ		
//				POINT_COLOR=BLUE;//��ɫ����
				printf("STATUS:Connected\n");
				printf("Receive Data:");
				connflag=1;//���������
			} 
		}else if(connflag)
		{
// 			LCD_ShowString(30,150,190,16,16,"STATUS:Disconnected");
//			LCD_Fill(30,210,lcddev.width-1,lcddev.height-1,WHITE);//����
			printf("STATUS:Disconnected\n");
			connflag=0;	//������ӶϿ���
		} 
		lwip_periodic_handle();
		delay_ms(2);
		t++;
		if(t==200)
		{
			if(connflag==0&&(tcp_client_flag&1<<5)==0)//δ������,��������
			{ 
				tcp_client_connection_close(tcppcb,0);//�ر�����
				tcppcb=tcp_new();	//����һ���µ�pcb
				if(tcppcb)			//�����ɹ�
				{ 
					tcp_connect(tcppcb,&rmtipaddr,TCP_CLIENT_PORT,tcp_client_connected);//���ӵ�Ŀ�ĵ�ַ��ָ���˿���,�����ӳɹ���ص�tcp_client_connected()����
				}
			}
			t=0;
			LED0=!LED0;
		}		
	}
	tcp_client_connection_close(tcppcb,0);//�ر�TCP Client����
	myfree(SRAMIN,tbuf);
} 
//lwIP TCP���ӽ�������ûص�����
err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
	struct tcp_client_struct *es=NULL;  
	if(err==ERR_OK)   
	{
		es=(struct tcp_client_struct*)mem_malloc(sizeof(struct tcp_client_struct));  //�����ڴ�
		if(es) //�ڴ�����ɹ�
		{
 			es->state=ES_TCPCLIENT_CONNECTED;//״̬Ϊ���ӳɹ�
			es->pcb=tpcb;  
			es->p=NULL; 
			tcp_arg(tpcb,es);        			//ʹ��es����tpcb��callback_arg
			tcp_recv(tpcb,tcp_client_recv);  	//��ʼ��LwIP��tcp_recv�ص�����   
			tcp_err(tpcb,tcp_client_error); 	//��ʼ��tcp_err()�ص�����
			tcp_sent(tpcb,tcp_client_sent);		//��ʼ��LwIP��tcp_sent�ص�����
			tcp_poll(tpcb,tcp_client_poll,1); 	//��ʼ��LwIP��tcp_poll�ص����� 
 			tcp_client_flag|=1<<5; 				//������ӵ���������
			err=ERR_OK;
		}else
		{ 
			tcp_client_connection_close(tpcb,es);//�ر�����
			err=ERR_MEM;	//�����ڴ�������
		}
	}else
	{
		tcp_client_connection_close(tpcb,0);//�ر�����
	}
	return err;
}
//lwIP tcp_recv()�����Ļص�����
err_t tcp_client_recv(void *arg,struct tcp_pcb *tpcb,struct pbuf *p,err_t err)
{ 
	u32 data_len=0;
	struct pbuf *q;
	struct tcp_client_struct *es;
	err_t ret_err; 
	LWIP_ASSERT("arg != NULL",arg != NULL);
	es=(struct tcp_client_struct *)arg; 
	if(p==NULL)//����ӷ��������յ��յ�����֡�͹ر�����
	{
		es->state=ES_TCPCLIENT_CLOSING;//��Ҫ�ر�TCP ������ 
 		es->p=p; 
		ret_err=ERR_OK;
	}else if(err!= ERR_OK)//�����յ�һ���ǿյ�����֡,����err!=ERR_OK
	{ 
		if(p)pbuf_free(p);//�ͷŽ���pbuf
		ret_err=err;
	}else if(es->state==ES_TCPCLIENT_CONNECTED)	//����������״̬ʱ
	{
		if(p!=NULL)//����������״̬���ҽ��յ������ݲ�Ϊ��ʱ
		{
			memset(tcp_client_recvbuf,0,TCP_CLIENT_RX_BUFSIZE);  //���ݽ��ջ���������
			for(q=p;q!=NULL;q=q->next)  //����������pbuf����
			{
				//�ж�Ҫ������TCP_CLIENT_RX_BUFSIZE�е������Ƿ����TCP_CLIENT_RX_BUFSIZE��ʣ��ռ䣬�������
				//�Ļ���ֻ����TCP_CLIENT_RX_BUFSIZE��ʣ�೤�ȵ����ݣ�����Ļ��Ϳ������е�����
				if(q->len > (TCP_CLIENT_RX_BUFSIZE-data_len)) memcpy(tcp_client_recvbuf+data_len,q->payload,(TCP_CLIENT_RX_BUFSIZE-data_len));//��������
				else memcpy(tcp_client_recvbuf+data_len,q->payload,q->len);
				data_len += q->len;  	
				if(data_len > TCP_CLIENT_RX_BUFSIZE) break; //����TCP�ͻ��˽�������,����	
			}
			tcp_client_flag|=1<<6;		//��ǽ��յ�������
 			tcp_recved(tpcb,p->tot_len);//���ڻ�ȡ��������,֪ͨLWIP���Ի�ȡ��������
			pbuf_free(p);  	//�ͷ��ڴ�
			ret_err=ERR_OK;
			recv_len+=p->tot_len;
		}
	}else  //���յ����ݵ��������Ѿ��ر�,
	{ 
		tcp_recved(tpcb,p->tot_len);//���ڻ�ȡ��������,֪ͨLWIP���Ի�ȡ��������
		es->p=NULL;
		pbuf_free(p); //�ͷ��ڴ�
		ret_err=ERR_OK;
	}
	return ret_err;
} 
//lwIP tcp_err�����Ļص�����
void tcp_client_error(void *arg,err_t err)
{  
	//�������ǲ����κδ���
} 
//lwIP tcp_poll�Ļص�����
err_t tcp_client_poll(void *arg, struct tcp_pcb *tpcb)
{
	err_t ret_err;
	struct tcp_client_struct *es; 
	es=(struct tcp_client_struct*)arg;
	if(es!=NULL)  //���Ӵ��ڿ��п��Է�������
	{
		if(tcp_client_flag&(1<<7))	//�ж��Ƿ�������Ҫ���� 
		{
//			es->p=pbuf_alloc(PBUF_TRANSPORT, strlen((char*)tcp_client_sendbuf),PBUF_POOL);	//�����ڴ� 
//			pbuf_take(es->p,(char*)tcp_client_sendbuf,strlen((char*)tcp_client_sendbuf));	//��tcp_client_sentbuf[]�е����ݿ�����es->p_tx��	
			es->p=pbuf_alloc(PBUF_TRANSPORT, strlen((char*)cmd[cp]),PBUF_POOL);	//�����ڴ� 
			pbuf_take(es->p,(char*)cmd[cp],strlen((char*)cmd[cp]));	//��tcp_client_sentbuf[]�е����ݿ�����es->p_tx��
			tcp_client_senddata(tpcb,es);//��tcp_client_sentbuf[]���渴�Ƹ�pbuf�����ݷ��ͳ�ȥ
			tcp_client_flag&=~(1<<7);	//������ݷ��ͱ�־
			if(es->p)pbuf_free(es->p);	//�ͷ��ڴ�
		}else if(es->state==ES_TCPCLIENT_CLOSING)
		{ 
 			tcp_client_connection_close(tpcb,es);//�ر�TCP����
		} 
		ret_err=ERR_OK;
	}else
	{ 
		tcp_abort(tpcb);//��ֹ����,ɾ��pcb���ƿ�
		ret_err=ERR_ABRT;
	}
	return ret_err;
} 
//lwIP tcp_sent�Ļص�����(����Զ���������յ�ACK�źź�������)
err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
	struct tcp_client_struct *es;
	LWIP_UNUSED_ARG(len);
	es=(struct tcp_client_struct*)arg;
	if(es->p)tcp_client_senddata(tpcb,es);//��������
	return ERR_OK;
}
//�˺���������������
void tcp_client_senddata(struct tcp_pcb *tpcb, struct tcp_client_struct * es)
{
	struct pbuf *ptr; 
 	err_t wr_err=ERR_OK;
	while((wr_err==ERR_OK)&&es->p&&(es->p->len<=tcp_sndbuf(tpcb))) //��Ҫ���͵����ݼ��뵽���ͻ��������
	{
		ptr=es->p;
		wr_err=tcp_write(tpcb,ptr->payload,ptr->len,1);
		if(wr_err==ERR_OK)
		{  
			es->p=ptr->next;			//ָ����һ��pbuf
			if(es->p)pbuf_ref(es->p);	//pbuf��ref��һ
			pbuf_free(ptr);				//�ͷ�ptr
		}else if(wr_err==ERR_MEM)es->p=ptr;
		tcp_output(tpcb);		//�����ͻ�������е������������ͳ�ȥ
	} 
} 
//�ر��������������
void tcp_client_connection_close(struct tcp_pcb *tpcb, struct tcp_client_struct * es)
{
	//�Ƴ��ص�
	tcp_abort(tpcb);//��ֹ����,ɾ��pcb���ƿ�
	tcp_arg(tpcb,NULL);  
	tcp_recv(tpcb,NULL);
	tcp_sent(tpcb,NULL);
	tcp_err(tpcb,NULL);
	tcp_poll(tpcb,NULL,0);  
	if(es)mem_free(es); 
	tcp_client_flag&=~(1<<5);//������ӶϿ���
}






















