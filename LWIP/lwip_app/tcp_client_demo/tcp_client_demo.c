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
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F407开发板
//TCP Client 测试代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2014/8/15
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//*******************************************************************************
//修改信息
//无
////////////////////////////////////////////////////////////////////////////////// 	   
 #define pi 3.1415
//TCP Client接收数据缓冲区
u8 tcp_client_recvbuf[TCP_CLIENT_RX_BUFSIZE];	
//TCP服务器发送数据内容
//const u8 *tcp_client_sendbuf="GD0530055001\r";
const u8 cmd[5][14]={"%ST\r","BM\r","QT\r","RS\r",
	"GD0480060000\r"};
int recv_len,cp,data_len=600-480;
u32 data[3000];	
u8 flag = 0;
	
//TCP Client 测试全局状态标记变量
//bit7:0,没有数据要发送;1,有数据要发送
//bit6:0,没有收到数据;1,收到数据了.
//bit5:0,没有连接上服务器;1,连接上服务器了.
//bit4~0:保留

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
//设置远端IP地址
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
	
	tbuf=mymalloc(SRAMIN,100);	//申请内存
	if(tbuf==NULL)return;
	//前三个IP保持和DHCP得到的IP一致
	lwipdev.remoteip[0]=lwipdev.ip[0];
	lwipdev.remoteip[1]=lwipdev.ip[1];
	lwipdev.remoteip[2]=lwipdev.ip[2]; 
	
	//sprintf((char*)tbuf,"Remote IP:%d.%d.%d.",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2]);//远端IP
	
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
			if(key==8||key==6)lwipdev.remoteip[3]++;//IP增加
			if(key==2||key==4)lwipdev.remoteip[3]--;//IP减少
			printf("Remote IP:%d.%d.%d.%d\n",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);
//			LCD_ShowxNum(xoff,1100,lwipdev.remoteip[3],3,16,0X80);//显示新IP
		}
	}
	//myfree(SRAMIN,tbuf); 
}
 

//TCP Client 测试
void tcp_client_test(void)
{
 	struct tcp_pcb *tcppcb;  	//定义一个TCP服务器控制块
	struct ip_addr rmtipaddr;  	//远端ip地址
	float alfa,ans;
	u8 f_cnt = 0;
	u8 *tbuf;
 	u8 key;
	u8 res=0;		
	u8 t=0; 
	u8 connflag=0;		//连接标记
	recv_len = 0;cp=0;
	
	
	tcp_client_set_remoteip();//先选择IP
//	LCD_Clear(WHITE);	//清屏
//	POINT_COLOR=RED; 	//红色字体
	
//	LCD_ShowString(30,30,200,16,16,"TCP Client Test");
//	LCD_ShowString(30,50,200,16,16,"KEY0:Send data");  
//	LCD_ShowString(30,70,200,16,16,"KEY_UP:Quit");  
	
	printf("TCP Client Mode\n");
	
//	tbuf=mymalloc(SRAMIN,200);	//申请内存
//	if(tbuf==NULL)return ;		//内存申请失败了,直接退出
	
//	sprintf((char*)tbuf,"Local IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);//服务器IP
//	LCD_ShowString(30,90,210,16,16,tbuf);  
	
	printf("Local IP:%d.%d.%d.%d\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	
//	sprintf((char*)tbuf,"Remote IP:%d.%d.%d.%d",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);//远端IP
//	LCD_ShowString(30,110,210,16,16,tbuf);  
	
	printf("Remote IP:%d.%d.%d.%d\n",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);
	
//	sprintf((char*)tbuf,"Remote Port:%d",TCP_CLIENT_PORT);//客户端端口号
//	LCD_ShowString(30,130,210,16,16,tbuf);
	
	printf("Remote Port:%d\n",TCP_CLIENT_PORT);
	
//	POINT_COLOR=BLUE;
//	LCD_ShowString(30,150,210,16,16,"STATUS:Disconnected"); 

	printf("STATUS:Disconnected\n");

	tcppcb=tcp_new();	//创建一个新的pcb
	if(tcppcb)			//创建成功
	{
		IP4_ADDR(&rmtipaddr,lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]); 
		tcp_connect(tcppcb,&rmtipaddr,TCP_CLIENT_PORT,tcp_client_connected);  //连接到目的地址的指定端口上,当连接成功后回调tcp_client_connected()函数
 	}else res=1;
	while(res==0)
	{
		key=USART_RX_BUF[0];
		if(key==0)break;
		if(key==5)//5按下了,发送数据
		{
			tcp_client_flag|=1<<7;//标记要发送数据
			printf("sent");
		}
		if (key<0&&key<9){
			if((key==4||key==2)) cp--;
			else if (key==8||key==6) cp++;
			if (cp<0)cp+=5;
			else cp%=5;
		//LCD_Fill(30,250,lcddev.width-1,lcddev.height-1,WHITE);//清上一次数据
		//LCD_ShowNum(30,290,cp,1,16);
			printf("ready to send :%s\n",cmd[4]);
		}
		if(tcp_client_flag&1<<6)//是否收到数据?
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
//					LCD_Fill(30,190,lcddev.width-1,lcddev.height-1,WHITE);//清上一次数据
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
//				LCD_Fill(30,190,lcddev.width-1,lcddev.height-1,WHITE);//清上一次数据
//				LCD_ShowString_length(30,190,lcddev.width-35,lcddev.height-230,16,tcp_client_recvbuf,recv_len);//显示接收到的数据
				printf("%s\n",tcp_client_recvbuf);
			}
			recv_len=0;
			tcp_client_flag&=~(1<<6);//标记数据已经被处理了.
		}
		if(tcp_client_flag&1<<5)//是否连接上?
		{
			if(connflag==0)
			{ 
//				LCD_ShowString(30,150,lcddev.width-30,lcddev.height-190,16,"STATUS:Connected   ");//提示消息		
//				POINT_COLOR=RED;
//				LCD_ShowString(30,170,lcddev.width-30,lcddev.height-190,16,"Receive Data:");//提示消息		
//				POINT_COLOR=BLUE;//蓝色字体
				printf("STATUS:Connected\n");
				printf("Receive Data:");
				connflag=1;//标记连接了
			} 
		}else if(connflag)
		{
// 			LCD_ShowString(30,150,190,16,16,"STATUS:Disconnected");
//			LCD_Fill(30,210,lcddev.width-1,lcddev.height-1,WHITE);//清屏
			printf("STATUS:Disconnected\n");
			connflag=0;	//标记连接断开了
		} 
		lwip_periodic_handle();
		delay_ms(2);
		t++;
		if(t==200)
		{
			if(connflag==0&&(tcp_client_flag&1<<5)==0)//未连接上,则尝试重连
			{ 
				tcp_client_connection_close(tcppcb,0);//关闭连接
				tcppcb=tcp_new();	//创建一个新的pcb
				if(tcppcb)			//创建成功
				{ 
					tcp_connect(tcppcb,&rmtipaddr,TCP_CLIENT_PORT,tcp_client_connected);//连接到目的地址的指定端口上,当连接成功后回调tcp_client_connected()函数
				}
			}
			t=0;
			LED0=!LED0;
		}		
	}
	tcp_client_connection_close(tcppcb,0);//关闭TCP Client连接
	myfree(SRAMIN,tbuf);
} 
//lwIP TCP连接建立后调用回调函数
err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
	struct tcp_client_struct *es=NULL;  
	if(err==ERR_OK)   
	{
		es=(struct tcp_client_struct*)mem_malloc(sizeof(struct tcp_client_struct));  //申请内存
		if(es) //内存申请成功
		{
 			es->state=ES_TCPCLIENT_CONNECTED;//状态为连接成功
			es->pcb=tpcb;  
			es->p=NULL; 
			tcp_arg(tpcb,es);        			//使用es更新tpcb的callback_arg
			tcp_recv(tpcb,tcp_client_recv);  	//初始化LwIP的tcp_recv回调功能   
			tcp_err(tpcb,tcp_client_error); 	//初始化tcp_err()回调函数
			tcp_sent(tpcb,tcp_client_sent);		//初始化LwIP的tcp_sent回调功能
			tcp_poll(tpcb,tcp_client_poll,1); 	//初始化LwIP的tcp_poll回调功能 
 			tcp_client_flag|=1<<5; 				//标记连接到服务器了
			err=ERR_OK;
		}else
		{ 
			tcp_client_connection_close(tpcb,es);//关闭连接
			err=ERR_MEM;	//返回内存分配错误
		}
	}else
	{
		tcp_client_connection_close(tpcb,0);//关闭连接
	}
	return err;
}
//lwIP tcp_recv()函数的回调函数
err_t tcp_client_recv(void *arg,struct tcp_pcb *tpcb,struct pbuf *p,err_t err)
{ 
	u32 data_len=0;
	struct pbuf *q;
	struct tcp_client_struct *es;
	err_t ret_err; 
	LWIP_ASSERT("arg != NULL",arg != NULL);
	es=(struct tcp_client_struct *)arg; 
	if(p==NULL)//如果从服务器接收到空的数据帧就关闭连接
	{
		es->state=ES_TCPCLIENT_CLOSING;//需要关闭TCP 连接了 
 		es->p=p; 
		ret_err=ERR_OK;
	}else if(err!= ERR_OK)//当接收到一个非空的数据帧,但是err!=ERR_OK
	{ 
		if(p)pbuf_free(p);//释放接收pbuf
		ret_err=err;
	}else if(es->state==ES_TCPCLIENT_CONNECTED)	//当处于连接状态时
	{
		if(p!=NULL)//当处于连接状态并且接收到的数据不为空时
		{
			memset(tcp_client_recvbuf,0,TCP_CLIENT_RX_BUFSIZE);  //数据接收缓冲区清零
			for(q=p;q!=NULL;q=q->next)  //遍历完整个pbuf链表
			{
				//判断要拷贝到TCP_CLIENT_RX_BUFSIZE中的数据是否大于TCP_CLIENT_RX_BUFSIZE的剩余空间，如果大于
				//的话就只拷贝TCP_CLIENT_RX_BUFSIZE中剩余长度的数据，否则的话就拷贝所有的数据
				if(q->len > (TCP_CLIENT_RX_BUFSIZE-data_len)) memcpy(tcp_client_recvbuf+data_len,q->payload,(TCP_CLIENT_RX_BUFSIZE-data_len));//拷贝数据
				else memcpy(tcp_client_recvbuf+data_len,q->payload,q->len);
				data_len += q->len;  	
				if(data_len > TCP_CLIENT_RX_BUFSIZE) break; //超出TCP客户端接收数组,跳出	
			}
			tcp_client_flag|=1<<6;		//标记接收到数据了
 			tcp_recved(tpcb,p->tot_len);//用于获取接收数据,通知LWIP可以获取更多数据
			pbuf_free(p);  	//释放内存
			ret_err=ERR_OK;
			recv_len+=p->tot_len;
		}
	}else  //接收到数据但是连接已经关闭,
	{ 
		tcp_recved(tpcb,p->tot_len);//用于获取接收数据,通知LWIP可以获取更多数据
		es->p=NULL;
		pbuf_free(p); //释放内存
		ret_err=ERR_OK;
	}
	return ret_err;
} 
//lwIP tcp_err函数的回调函数
void tcp_client_error(void *arg,err_t err)
{  
	//这里我们不做任何处理
} 
//lwIP tcp_poll的回调函数
err_t tcp_client_poll(void *arg, struct tcp_pcb *tpcb)
{
	err_t ret_err;
	struct tcp_client_struct *es; 
	es=(struct tcp_client_struct*)arg;
	if(es!=NULL)  //连接处于空闲可以发送数据
	{
		if(tcp_client_flag&(1<<7))	//判断是否有数据要发送 
		{
//			es->p=pbuf_alloc(PBUF_TRANSPORT, strlen((char*)tcp_client_sendbuf),PBUF_POOL);	//申请内存 
//			pbuf_take(es->p,(char*)tcp_client_sendbuf,strlen((char*)tcp_client_sendbuf));	//将tcp_client_sentbuf[]中的数据拷贝到es->p_tx中	
			es->p=pbuf_alloc(PBUF_TRANSPORT, strlen((char*)cmd[cp]),PBUF_POOL);	//申请内存 
			pbuf_take(es->p,(char*)cmd[cp],strlen((char*)cmd[cp]));	//将tcp_client_sentbuf[]中的数据拷贝到es->p_tx中
			tcp_client_senddata(tpcb,es);//将tcp_client_sentbuf[]里面复制给pbuf的数据发送出去
			tcp_client_flag&=~(1<<7);	//清除数据发送标志
			if(es->p)pbuf_free(es->p);	//释放内存
		}else if(es->state==ES_TCPCLIENT_CLOSING)
		{ 
 			tcp_client_connection_close(tpcb,es);//关闭TCP连接
		} 
		ret_err=ERR_OK;
	}else
	{ 
		tcp_abort(tpcb);//终止连接,删除pcb控制块
		ret_err=ERR_ABRT;
	}
	return ret_err;
} 
//lwIP tcp_sent的回调函数(当从远端主机接收到ACK信号后发送数据)
err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
	struct tcp_client_struct *es;
	LWIP_UNUSED_ARG(len);
	es=(struct tcp_client_struct*)arg;
	if(es->p)tcp_client_senddata(tpcb,es);//发送数据
	return ERR_OK;
}
//此函数用来发送数据
void tcp_client_senddata(struct tcp_pcb *tpcb, struct tcp_client_struct * es)
{
	struct pbuf *ptr; 
 	err_t wr_err=ERR_OK;
	while((wr_err==ERR_OK)&&es->p&&(es->p->len<=tcp_sndbuf(tpcb))) //将要发送的数据加入到发送缓冲队列中
	{
		ptr=es->p;
		wr_err=tcp_write(tpcb,ptr->payload,ptr->len,1);
		if(wr_err==ERR_OK)
		{  
			es->p=ptr->next;			//指向下一个pbuf
			if(es->p)pbuf_ref(es->p);	//pbuf的ref加一
			pbuf_free(ptr);				//释放ptr
		}else if(wr_err==ERR_MEM)es->p=ptr;
		tcp_output(tpcb);		//将发送缓冲队列中的数据立即发送出去
	} 
} 
//关闭与服务器的连接
void tcp_client_connection_close(struct tcp_pcb *tpcb, struct tcp_client_struct * es)
{
	//移除回调
	tcp_abort(tpcb);//终止连接,删除pcb控制块
	tcp_arg(tpcb,NULL);  
	tcp_recv(tpcb,NULL);
	tcp_sent(tpcb,NULL);
	tcp_err(tpcb,NULL);
	tcp_poll(tpcb,NULL,0);  
	if(es)mem_free(es); 
	tcp_client_flag&=~(1<<5);//标记连接断开了
}






















