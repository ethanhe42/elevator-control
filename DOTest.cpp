#include <windows.h>
#include <iostream>
#include <conio.h>
#include "winio.h"
#include "time.h"

#pragma comment(lib,"winio.lib")
using namespace std;

unsigned short BASE_ADDRESS = 0xE880;
int OPort=16;
int iPort=16;
int DO_data;
int DO[6]={0};
int tmp[6]={0};

int i;
int DI_data;
int DI[6]={0};

int creat_DO(int (&DO_bit)[6])
{
	int temp=0;
  	int i=0;
  	for(i=5;i>0;i--)
   	{
    	temp=(temp+DO_bit[i])*2;
   	}
  	return temp+DO_bit[0];
  }

int creat_DI(int (&DI_bit)[6], int num)
{
  	int i=0;
  	for(i=0;i<6;i++)
   	DI_bit[i]=(num>>i)&0x0001;
  	return 0;
}
void show(){
		DO_data=creat_DO(DO);
		_outp(BASE_ADDRESS + OPort, DO_data);
}
int uptime = 5000;
float timecnt;
void close(){
	DO[2] = 0;
	DO[3] = 1;
	show();
}
void open(){
	DO[2] = 1;
	DO[3] = 0;
	show();
}
void up() {
	DO[0] = 0;
	DO[1] = 0;
	close();
	DO[4] = 1;
	DO[5] = 0;
	show();
	Sleep(uptime);
}
void down() {
	DO[0] = 0;
	DO[1] = 0;
	close();
	DO[4] = 0;
	DO[5] = 1;
	show();
	Sleep(uptime);
}
void stable() {
	DO[4] = 0;	
	DO[5] = 0;
}
void first(){
	DO[0] = 1;
	DO[1] = 0;
	stable();
	show();
}
void second(){
	DO[0] = 0;
	DO[1] = 1;
	stable();
	show();
}

enum Stat {
	close0, close1, open0, open1, wait0, wait1
} status;

enum Trans{
	call0, call1, ok0 , ok1, closedoor, opendoor
} transcode;
int translation[6] = {0, 0, 1, 0, 0, 0};

int getTrans(Trans t){
	return translation[t];
}

void s_close0(){
	translation[closedoor] = 0;
	cout<<"close0"<<endl;
	first();
	close();
	if (getTrans(opendoor)){
		status = open0;
	}
	if (getTrans(call0)){
		timecnt=clock();
		translation[call0] = 0;
	}
	if (getTrans(call1)){
		cout<<"up"<<endl;
		up();
		status = wait1;
		translation[ok0] = 0;
		translation[ok1] = 0;
		translation[call1] = 0;
	}
}
void s_close1(){
	translation[closedoor] = 0;
	cout<<"close1"<<endl;
	second();
	close();
	if (getTrans(opendoor)){
		status = open1;
	}
	if (getTrans(call1)){
		status = open1;
		translation[call1] = 0;
	}
	if (getTrans(call0)){
		cout<<"down"<<endl;
		down();
		status = wait0;
		translation[ok0] = 0;
		translation[ok1] = 0;
		translation[call0] = 0;
	}
}
void s_open0(){
	translation[opendoor] = 0;
	cout<<"open0"<<endl;
	first();
	open();
	if (getTrans(closedoor)){
		status = close0;
	}
	if ((clock() - timecnt) / CLK_TCK >= 5)
		status = close0;
}
void s_open1(){
	translation[opendoor] = 0;
	cout<<"open1"<<endl;
	second();
	open();
	if (getTrans(closedoor)){
		status = close1;
	}
	if ((clock() - timecnt) / CLK_TCK >= 5)
		status = close1;
}
void s_wait0(){
	cout<<"wait0"<<endl;
	translation[opendoor] = 0;
	translation[closedoor] = 0;
	first();
	close();
	if (getTrans(ok0)){
		status = close0;
	}
}
void s_wait1(){
	cout<<"wait1"<<endl;
	translation[opendoor] = 0;
	translation[closedoor] = 0;
	second();
	close();
	if (getTrans(ok1)){
		status = close1;
	}
}




void main(void)

{

// 初始化WinIO 
	if (!InitializeWinIo())
	{
		cout<<"Error In InitializeWinIo!"<<endl;
		exit(1);
	}
//数字量输出

	status = close0;	

   while(1){
		DI_data = _inp(BASE_ADDRESS + iPort);
		creat_DI(DI,DI_data);
		Sleep(100);
		int i;

		for (i = 0;i<6;i++){
			if (translation[i] == 0 && DI[i] == 0)
					translation[i] = 1;
		}
		for (i=0;i<6;i++)
			cout << translation[i] << ' ';

		if (status == close0) {
			timecnt=clock();
			s_close0();
		}
		else if (status == close1) {
			timecnt=clock();
			s_close1();

		}
		else if (status == open0) {
			s_open0();
		}
		else if (status == open1) {
			s_open1();
		}
		else if (status == wait0) {
			s_wait0();
		}
		else if (status == wait1) {
			s_wait1();
		}
   }
	_outp(BASE_ADDRESS + OPort, 0);
    ShutdownWinIo();													//关闭WinIO
}