#include "StdAfx.h"
//up

#include "Plan_Path_VFH.h"
#include "vfh_algorithm.h"
//#include "processSpeech.h"
//#include "MotorTest.h"
//#include "Map.h"
#include <math.h>
#include <stdlib.h>

#include "Comm_data_motor3.h"  //电机

//#include "GlobalParameter.h"
//#include "Comm_data_sonar.h"
//#include "Comm_data_motor.h"
//#include "RobotClientUserDlg.h"

#include <time.h>

#include <mmsystem.h>
#pragma comment( lib, "Winmm.lib" )
#pragma comment(lib,"Strmiids.lib") 
#pragma comment(lib,"ws2_32.lib")

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
#define PIf 3.1415926
//MAP map_world;

//int speed_high = speed_stated;
//int speed_low = speed_stated*0.5;
extern CMotor motor;
extern struct robotinfo Info_robot;

///////////////
struct object
{
	int objectnum;
	float objectnum_x;
	float objectnum_y;
	float direct;
	float time;
	int mode;


	char* contect1;
	char* contect2;
	char* contect3;
};
bool isbegio=true;
extern int objectnowpos;
extern struct object zhanpin[100];
extern int objectnums[100];
extern int objectnow;
////////////
CPlan_Path_VFH::CPlan_Path_VFH(void)
{ 
	pi=3.141592654;
	Drobot = 400;
	zuolunfangxiang = 0;
	youlunfangxiang = 0;
	zuolunjuli = 0;
	youlunjuli = 0;
	pianzhuan = 0;//pi/2,
	//#define Start_Pose_x 1388
	//#define Start_Pose_y 1787
	pointrox = 10000;
	pointroy = 10000;
	pian = 0.00001;

	BiZhang_JuLi=(int)200*(LENGTH_x/Scenario_Width);  //我们首先考虑避障距离为400mm的情况下,
	b_constant=2.0;
	C_certainty_value=15.0;
	alpha=5.0;
	prevAngle=0.0;

	dangerdistance0 = 300; //转向距离
	dangerdistance1 = 500; //减速距离
	dangerdistance2 = 700; //安全距离
	//DangerGrade[12] = {0}; //判断结果
	 Desired_Angle_abs=0.0; 

	 Desired_Angle_ob=0.0;//展品转向

}


CPlan_Path_VFH::~CPlan_Path_VFH(void)
{

}

void CPlan_Path_VFH::Init()
{
	//	file=fopen("pp_pos.txt","a+");
	SetupVFHPLUS();
	Grid_Scale_x=(double)Scenario_Width/LENGTH_x;
	Grid_Scale_y=(double)Scenario_Height/LENGTH_y;

	//  target_x=LENGTH_x/2;  //先以为基本单位 cm
	//  target_z=LENGTH_y/2;  //
	speed_high = speed_stated;
	speed_low = speed_stated*0.5;
	speed_l = 0;
	speed_r = 0;
	socktime = 0;
	zuo1=0;
	you1=0;
	direct=TARGET;
	distance = 0;
	current_x=LENGTH_x/2;
	current_z=LENGTH_y/2;
	current_angle=m_angle;

	m_Scenario_Width=Scenario_Width;
	m_Scenario_Height=Scenario_Height;
	target_num = 0;

	Desired_Angle = 0.0;
	speed_line = 0.0;
	speed_angle = 0.0;

	//for(int i=0;i<LENGTH_x;i++)
	//	for(int j=0;j<LENGTH_y;j++)
	//	{
	//		mapp[i][j]=0;     //进行初始化,说明初始地图上没有障碍
	//		mcount[i][j]=0;
	//	}
//	SetupVFHPLUS();

	//  //fprintf(file,"Init: z:%d    x:%d  %f  %f  \n",target_z,target_x,Grid_Scale_x,Grid_Scale_y);
}


int CPlan_Path_VFH::SetupVFHPLUS()
{
	//	player_bbox_t geom;
	this->active_goal = false;
	this->turninginplace = false;

	float  robot_radius=205;//单位mm
	speed=0;
	turnrate=0;

	// FIXME
	// Allocate and intialize VFH算法初始化
	vfh_Algorithm.Init();

	vfh_Algorithm.SetRobotRadius( robot_radius );

	return 0;
}


void CPlan_Path_VFH::TransformToGrid(float x,float z,int *grid_x,int *grid_z)
{
	(*grid_x)=int((x)/(Grid_Scale_x+0.5));
	(*grid_z)=int((z)/Grid_Scale_y+0.5);


	/*(*grid_x)=int((x)/0.1+0.5);
	(*grid_z)=int((z)/0.1+0.5);*/

	//	*grid_x=LENGTH_x-(*grid_x);
	//	*grid_x=(*grid_x);
	//	*grid_z=LENGTH_y-(*grid_z);
	//	*grid_z=(*grid_z);


	if(*grid_x<0)  *grid_x=0;
	if(*grid_x>LENGTH_x-1) *grid_x=LENGTH_x-1;
	if(*grid_z<0)  *grid_z=0;

	if(*grid_z>LENGTH_y-1) *grid_z=LENGTH_y-1;

	////fprintf(file,"real_TARGET: z %f  x%f  z_grid:%d    x_grid:%d  \n",z,x,*grid_z,*grid_x);

}

void CPlan_Path_VFH::PixelTransformToGrid(float x,float z,int *grid_x,int *grid_z,double RealPositionToPixel_x,double RealPositionToPixel_y)
{
	(*grid_x)=(int)(x)*RealPositionToPixel_x/(Grid_Scale_x);
	(*grid_z)=(int)(z)*RealPositionToPixel_y/(Grid_Scale_y);


	////fprintf(file," newtest   x:%f    z:%f  gridx:%d  gridz%d %f  %f   \n",x,z,*grid_x,*grid_z,RealPositionToPixel_x,Grid_Scale_x);

	//	*grid_x=(*grid_x);
	//	*grid_z=LENGTH_y-(*grid_z);

	if(*grid_x<0)  *grid_x=0;
	if(*grid_x>LENGTH_x-1) *grid_x=LENGTH_x-1;
	if(*grid_z<0)  *grid_z=0;

	if(*grid_z>LENGTH_y-1) *grid_z=LENGTH_y-1;


}


void CPlan_Path_VFH::SetCurrentZuobiao(float x,float z,float angle)  //cm
{
	TransformToGrid(x,z,&current_x,&current_z);//坐标值栅格化
	//PixelTransformToGrid(x,z,&current_x,&current_z,RealPositionToPixel_x,RealPositionToPixel_y);//转换到网格
	current_angle=angle;
	if(mapp[current_x][current_z]!=3 && mapp[current_x][current_z]!=1)//该处栅格既不是障碍物(3)也不是目标点(1)，显示机器人位置(2)
		mapp[current_x][current_z]=2;//
	//  //fprintf(file,"SetCurrentRobotZuobiao: %f  %f  %f  x:%d   z:%d  \n",x,z,angle,current_x,current_z);
	/////////////////////////////////////////////
	//for (int i=0;i<LENGTH_x;i++)
	//{
	// for (int j=0;j<LENGTH_y;j++)
	// {
	//  //fprintf(file,"%d ",mapp[i][j]);
	// }
	//}
	/////////////////////////////////////////////
}


void CPlan_Path_VFH::ProcessCommand( player_position2d_cmd_pos_t &cmd)
{
	int x,y,t;

	x = (int)rint(cmd.pos.px );
	y = (int)rint(cmd.pos.py );
	t = (int)rint(RTOD(cmd.pos.pa));

	this->cmd_type = 1;
	this->cmd_state = cmd.state;

	if((x != target_x) || (y != target_z) || (t != target_t))
	{
		this->active_goal = true;//目标处于活动
		this->turninginplace = false;//转变位置为假
		// target_x = x;
		//target_z = y;
		//target_t = t;//定义目标点的位姿（由服务器提供）
	}
	//  //fprintf(file,"ProcessCommand %d  %d  %d  \n",x,y,t);
}

void CPlan_Path_VFH::ProcessOdom( player_position2d_data_t &data)
{

	// Cache the new odometric pose, velocity, and stall info
	// NOTE: this->odom_pose is in (mm,mm,deg), as doubles
	this->odom_pose[0] = data.pos.px;
	this->odom_pose[1] = data.pos.py;
	this->odom_pose[2] = RTOD(data.pos.pa);//弧度转角度函数
	this->odom_vel[0] = data.vel.px;
	this->odom_vel[1] = data.vel.py;
	this->odom_vel[2] = RTOD(data.vel.pa);
	this->odom_stall = data.stall;

	// Also change this info out for use by others
	// //fprintf(file,"ProcessOdom %f  %f  %f %f %f %f  \n",this->odom_pose[0],this->odom_pose[1],this->odom_pose[2],this->odom_vel[0],this->odom_vel[1],this->odom_vel[2]);

	// Also change this info out for use by others

}

#define rint(R) (((R)>0?((R)+0.5):((R)-0.5)))

int temp1 = 2000;
int dangernum_l = 0;
int dangernum_r = 0;


//SERVEMODE为送餐模式,1-漫游模式;2-指定目标模式;3-到达目标等待取餐;
void CPlan_Path_VFH::PlanPath_vfh(float m_x,float m_y,float m_angle)
{
//	FILE *allout;
//	allout = fopen("allout5.txt","a+");
	int temp = 850;  

	int dangernum = 0;
	int dangernum1 = 0;


	dangernum = 0;
	dangernum1 = 0;
	//return m_nDangerDir;
	if (SERVEMODE == 1)
	{
		Decision();//运动决策(随机漫游)
		return;
	}
	else if(SERVEMODE == 2)
	{
		//Desired_Angle;
		dangernum_l = 0;
		dangernum_r = 0;

		double CENTER_X=pointrox;//以机器人的中心点为中心，定义其所在的栅格点的坐标
		double CENTER_Y=pointroy;

		double x=target_x*Grid_Scale_x;//目标所在的位姿值（非栅格值）
		double y=target_z*Grid_Scale_y;

		

		//basedVFHPLUS();
		//return;
		if (x < CENTER_X)
		{
			if (y < CENTER_Y)///////////////////////////第三象限
			{
				Desired_Angle_abs = atan((float)(CENTER_Y - y)/(float)(CENTER_X - x) );//逆时针为正
			//	Desired_Angle_abs *= (360 / 6.28);
				Desired_Angle_abs *= (180/pi);
				//	Desired_Angle_abs = 270 + Desired_Angle_abs;//转化成以Y轴正方向为角度的起始角（即机器人正后方）
				Desired_Angle_abs = 270+Desired_Angle_abs; 

			} 
			else if (y == CENTER_Y) 
			{
				Desired_Angle_abs = 270.0;//机器人正后方为180度
			} 
			else if (y > CENTER_Y) /////////////////第二象限
			{
				Desired_Angle_abs= atan((float)(y - CENTER_Y) /(float)(CENTER_X - x));
				Desired_Angle_abs *= (180/pi);
				Desired_Angle_abs = 270-Desired_Angle_abs;
			}
		} 
		else if (x == CENTER_X)
		{
			if (y < CENTER_Y)
			{
				Desired_Angle_abs =0;
			} 
			else if (y == CENTER_Y) 
			{
				Desired_Angle_abs = 0;
			}
			else if (y > CENTER_Y)
			{
				Desired_Angle_abs =180;
			}
		} 
		else if (x > CENTER_X) 
		{
			if (y < CENTER_Y)////////////////////////第四象限
			{
				Desired_Angle_abs = atan( (float)(CENTER_Y - y)/ (float)(x - CENTER_X));
				Desired_Angle_abs *= (180/pi);
				Desired_Angle_abs = 90-Desired_Angle_abs;
			} 
			else if (y == CENTER_Y)
			{
				Desired_Angle_abs = 90;
			} 
			else if (y > CENTER_Y)////////////////////第一象限

			{
				Desired_Angle_abs = atan( (float)(y - CENTER_Y)/(float)(x - CENTER_X));
				Desired_Angle_abs *= (180 / pi);
				Desired_Angle_abs =90+Desired_Angle_abs;

			}
		}


		 //if (x < CENTER_X)
			//{
			//	if (y < CENTER_Y)///////////////////////////第VI象限
			//	{
			//		Desired_Angle_abs = atan((float)(CENTER_X - x)/(float)(CENTER_Y - y) );//逆时针为正
			//			Desired_Angle_abs *= (360.0 / 6.28);
			//			Desired_Angle_abs =360-Desired_Angle_abs;//			
			//	} 
			//	else if (y == CENTER_Y) 
			//	{
			//	
			//		Desired_Angle_abs =270.0;//机器人后方为180度
			//	} 
			//	else if (y > CENTER_Y) /////////////////第II象限
			//	{
			//		Desired_Angle_abs= atan((float)(CENTER_X - x) /(float)(y - CENTER_Y));
			//		Desired_Angle_abs *= (360.0 / 6.28);
			//		Desired_Angle_abs = 180+Desired_Angle_abs;
			//	}
			//} 
			//else if (x == CENTER_X)
			//{
			//	if (y < CENTER_Y)
			//	{
			//		Desired_Angle_abs = 0;
			//	} 
			//	else if (y == CENTER_Y) 
			//	{
			//		Desired_Angle_abs= 0;
			//	}
			//	else if (y > CENTER_Y)
			//	{
			//		Desired_Angle_abs = 180;
			//	}
			//} 
			//else if (x > CENTER_X) 
			//{
			//	if (y < CENTER_Y)////////////////////////第I象限
			//	{
			//		Desired_Angle_abs = atan( (float)(x - CENTER_X)/ (float)(CENTER_Y - y));
			//		Desired_Angle_abs *= (360.0 / 6.28);
			//		Desired_Angle_abs = Desired_Angle_abs;
			//	} 
			//	else if (y == CENTER_Y)
			//	{
			//		Desired_Angle_abs= 90;
			//	} 
			//	else if (y > CENTER_Y)////////////////////第II象限

			//	{
			//		Desired_Angle_abs= atan( (float)(x - CENTER_X)/(float)(y - CENTER_Y));
			//		Desired_Angle_abs *=(360.0 / 6.28);
			//		Desired_Angle_abs =180-Desired_Angle_abs;

			//	}
			//}

		
		Desired_Angle = Desired_Angle_abs-(float)pianzhuan*180/pi;//目标点相对于机器人的期望位姿
	//	Desired_Angle = Desired_Angle_abs;//目标点相对于机器人的期望位姿
//	Desired_Angle -= 90;//////////////////////////zcs
		FILE *alloutXXX;
	alloutXXX = fopen("alloutxxx.txt","a+");
fprintf(alloutXXX ,"case3 -1111  %f  %f  %f %f  %f %f  %f \n",Desired_Angle,Desired_Angle_abs,(float)pianzhuan*180/pi,(float)x,(float)y,(float)CENTER_X,(float)CENTER_Y);
fclose(alloutXXX );
		while (Desired_Angle > 360.0)
			Desired_Angle -= 360.0;
		while (Desired_Angle < 0)
			Desired_Angle += 360.0;//将期望角度转换至0~360度

		/*	TCHAR c[10] = {NULL};
		WriteConsole(hdlWrite, "Desired_Angle: ", 15, NULL, NULL);
		itoa(Desired_Angle,c,sizeof(c));
		WriteConsole(hdlWrite, c, sizeof(c), NULL, NULL);
		WriteConsole(hdlWrite, "\n", 2, NULL, NULL);

		WriteConsole(hdlWrite, "Desired_Angle_abs: ", 18, NULL, NULL);
		itoa(Desired_Angle_abs,c,sizeof(c));
		WriteConsole(hdlWrite, c, sizeof(c), NULL, NULL);
		WriteConsole(hdlWrite, "\n", 2, NULL, NULL);*/


		/*	if (Desired_Angle<0)
		{
		Desired_Angle += 360;
		}*/
		//speed_stated = speed_high+15;
		//	speed_l = speed_stated;
		//	speed_r = speed_stated;

		double dist = sqrt(pow((x - this->odom_pose[0]),2) +
		pow((y- this->odom_pose[1]),2));
//			fprintf(allout,"%f    %f   %f   %f    %f %f %f %f \n",pointrox,pointroy,pianzhuan*180/pi,x,y,Desired_Angle,Desired_Angle_abs,dist);
		

		double mm_x=pointrox;//将位置的毫米单位转换成厘米
		double mm_y=pointroy;
		double mm_angle=pianzhuan/*-PI/2*/;

		//////////////////////////////////////////////////////////////////////////hhh
		//令偏转角弧度始终保持在一个周期内（0～2pi）；
		while(mm_angle>=pi*2)
			mm_angle -= pi*2;
		while(mm_angle<0)
			mm_angle+= pi*2;
		//////////////////////////////////////////////////////////////////////////
	//	SetCurrentZuobiao(mm_x,mm_y,mm_angle*180/PI);	// 设置机器人的位置轨迹


		//使用vfh算法


		/*TCHAR c[10] = {NULL};
		itoa(target_x,c,sizeof(c));
		WriteConsole(hdlWrite, "target_X:", sizeof(c), NULL, NULL);
		WriteConsole(hdlWrite, c, sizeof(c), NULL, NULL);
		WriteConsole(hdlWrite, "\n", 2, NULL, NULL);
		WriteConsole(hdlWrite, "target_Z:", sizeof(c), NULL, NULL);
		itoa(target_z,c,sizeof(c));
		WriteConsole(hdlWrite, c, sizeof(c), NULL, NULL);
		WriteConsole(hdlWrite, "\n",2, NULL, NULL);*/
		//	CtrlMode = 2;
		basedVFHPLUS();



	}
	else if (SERVEMODE == 3) //呼叫模式
	{
		;
	}
	else if (SERVEMODE == 4) //等待
	{
		//Sleep(100);
		speed_l = 0;
		speed_r = 0;
		speed_line = 0;
		speed_angle = 0;
	}
	else
	{
		//Sleep(100);
	}
//	fclose(allout);
}

void CPlan_Path_VFH::basedVFHPLUS(void)
{
	////////////////////////////////////////////////////////////////////////////////
	// Main function for device thread 设备线程的主函数

	FILE *allout;
	allout = fopen("allout24.txt","a+");

	float dist;//距离
	double angdiff=0.0;//角度误差
	struct timeval startescape, curr;//结构函数，开始时间和当前时间
	bool escaping = false;//不逃跑
	double timediff;
	int escape_turnrate_deg;//逃跑时的旋转角度
	player_position2d_cmd_pos_t cmd;//机器人状态
	player_position2d_data_t data;//机器人电机状态
	float x,y;
	float CENTER_X,CENTER_Y;
//	float Desired_Angle_abs;
	dist_eps=0.1;
	//CtrlMode = 2;
	// bookkeeping to implement hysteresis when rotating at the goal
	int rotatedir = 1;//旋转方向

	// bookkeeping to implement smarter escape policy实现逃离停止点的聪明策略，逃离方向
	int escapedir;

	escapedir = 1;
	cmd.pos.px=pointrox;
	cmd.pos.py=pointroy;
	cmd.pos.pa=pianzhuan;
	cmd.vel.px=speed*cos(pianzhuan);
	cmd.vel.py=speed*sin(pianzhuan);
	cmd.vel.pa=turnrate;
	cmd.state=1;

	this->odom_pose[0]=pointrox;
	this->odom_pose[1]=pointroy;
	this->odom_pose[2]=pianzhuan;

	ProcessCommand(cmd);//处理命令
	////////////////////////////////////
	data.pos.px=pointrox;
	data.pos.py=pointroy;
	data.pos.pa=pianzhuan;
	data.vel.px=speed*cos(pianzhuan);
	data.vel.py=speed*sin(pianzhuan);
	data.vel.pa=turnrate;
	data.stall=0;
	/////////////////////////////////////






	////////////////////
	ProcessOdom(data);//处理里程计数据

	dist = sqrt(pow((target_x*Grid_Scale_x - this->odom_pose[0]),2) +
		pow((target_z*Grid_Scale_y- this->odom_pose[1]),2));
	   angdiff = this->angle_diff((double)target_t,this->odom_pose[2]);

	   ////////////////////////////////////////xhy引导点
		if(dist < (10.0*dist_eps * 1e3)&&isbegio&&zhanpin[objectnow].mode==3)
		{

			objectnowpos=objectnowpos+1;
			int nums_oba=0;
			for(;nums_oba<100;++nums_oba)
			{
				if(zhanpin[nums_oba].objectnum==objectnums[objectnowpos])
				{
					break;
				}
			}
			target_x=zhanpin[nums_oba].objectnum_x;
			target_z=zhanpin[nums_oba].objectnum_y;
			Desired_Angle_ob=zhanpin[nums_oba].direct;
			objectnow=nums_oba;
			isbegio=true;

		}

///////////////////////////////////////




	 if((dist > (4.0*dist_eps * 1e3))&&isbegio)  
	   // CASE 3: The robot is too far from the goal position, so invoke VFH to
		//         get there.条件三：机器人没有进入目标点容许范围内，调用VFH到达目标点
	{
		int icount_try=0;
		MyFtestDanger();

		if ((((Desired_Angle)>180)&& ((Desired_Angle)<270)))
		{
			if(( DangerGrade[4] <=1 ||DangerGrade[5] <=1 || DangerGrade[6]<=1 || DangerGrade[7] <=1))
			{
			speed=-(int)rint(vfh_Algorithm.GetCurrentMaxSpeed() / 10.0);
	
			turnrate=2.0*vfh_Algorithm.GetMaxTurnrate(speed);
			}
			else 
			{
			speed=0.0;
	
			turnrate=2.0*vfh_Algorithm.GetMaxTurnrate(speed);
			}
			fprintf(allout,"case3 -1111  %f  %f  %f %f  \n",speed,  turnrate,dist,Desired_Angle);

		}
		else if ((((Desired_Angle)>=270)&& ((Desired_Angle)<360)))
		{
			if(( DangerGrade[4] <=1 ||DangerGrade[5] <=1 || DangerGrade[6]<=1 || DangerGrade[7] <=1))
			{
			speed=-(int)rint(vfh_Algorithm.GetCurrentMaxSpeed() / 10.0);
	
			turnrate=-2.0*vfh_Algorithm.GetMaxTurnrate(speed);
			}
			else 
			{
			speed=0.0;
	
			turnrate=-2.0*vfh_Algorithm.GetMaxTurnrate(speed);
			}
			
		
			fprintf(allout,"case3 -1111  %f  %f  %f %f  \n",speed,  turnrate,dist,Desired_Angle);

		}

		else 
		{

			if(( DangerGrade[4] <=1 ||DangerGrade[5] <=1 || DangerGrade[6]<=1 || DangerGrade[7] <=1))
			{
				icount_try++;
				if (icount_try<5)
				{
				speed=-(int)rint(vfh_Algorithm.GetCurrentMaxSpeed() / 10.0);
				}
				else 
				{
					turnrate=3.0*vfh_Algorithm.GetMaxTurnrate(speed);
				
				}

				fprintf(allout,"icount_try= %d speed= %f turnrate= %f  \n",icount_try,speed,turnrate);
			}
			else 
			{
				vfh_Algorithm.Update_VFH(  speed,Desired_Angle, dist, dist_eps * 1e3,speed,turnrate );//VFH更新函数
			}
			///////////////speed为bbb原来的值，根据VFH改变
			float tempturnrate=0;		

	
			{
					if ((DangerGrade[0]==0 | DangerGrade[1]==0| DangerGrade[2]==0))
					{
						if (turnrate<0)
						{
						turnrate=3.0*vfh_Algorithm.GetMaxTurnrate(speed);
						speed=(int)rint(vfh_Algorithm.GetCurrentMaxSpeed() / 10.0);
						fprintf(allout,"右下角有障碍，而机器人右转时，反转避开障碍物 \n");
						}
						else 
						{
							turnrate=2*vfh_Algorithm.GetMaxTurnrate(speed);
						speed=(int)rint(vfh_Algorithm.GetCurrentMaxSpeed() / 10.0);
						fprintf(allout,"右下角有障碍，而机器人左转时，快速避开障碍物 \n");
						}
					}
					else 
					{

					}
				


				if ((DangerGrade[11]==0 |DangerGrade[10]==0| DangerGrade[9]==0) )
				{
					if (turnrate<0)
					{
					turnrate=-3.0*vfh_Algorithm.GetMaxTurnrate(speed);
					speed=(int)rint(vfh_Algorithm.GetCurrentMaxSpeed() / 10.0);
					}
					else 
					{
					turnrate=-3.0*vfh_Algorithm.GetMaxTurnrate(speed);
					speed=(int)rint(vfh_Algorithm.GetCurrentMaxSpeed() / 10.0);
					}
			//		fprintf(allout,"hhhhhhhhhhhhhhhh \n");
				}
				

			}
			fprintf(allout,"case3 -666  %f  %f  %f %f  \n",speed,  turnrate,dist,Desired_Angle);

			

		/*	if(DangerGrade[3]==1 || DangerGrade[4] == 1 || DangerGrade[5] == 1 || DangerGrade[6] == 1 || DangerGrade[7] == 1 || DangerGrade[8] == 1)
			{
				speed=0.0;
				turnrate=-vfh_Algorithm.GetMaxTurnrate(speed);
			}*/
		}
		
		
		
		fprintf(allout,"case3——1 %f  %f  %f   %f %d  %d %d  %d %d %d %d  %d %d  %d %d %d \n",speed,  turnrate,dist,Desired_Angle,DangerGrade[11],DangerGrade[10],DangerGrade[9],DangerGrade[8],DangerGrade[7],DangerGrade[6],DangerGrade[5],DangerGrade[4],DangerGrade[3],DangerGrade[2],DangerGrade[1],DangerGrade[0]);
	

		PutCommand( speed, turnrate );//3333向回发布命令，让机器人按照该命令运动
		

	}
	 else if ((dist > ( 2.0*dist_eps * 1e3))&& (dist <= ( 4.0*dist_eps * 1e3))&&isbegio)
	  // CASE 4: The robot is at the goal position, but still needs to turn
		//         in place to reach the desired orientation.条件4：如果机器人处在目标位置，但依然需要到达莫个地方以接近预期方向
	{



			if (abs(Desired_Angle-90.0)>=15.0)
			{
				// At goal, stop在目标点，停止
		
		//	vfh_Algorithm.Update_VFH(  speed,Desired_Angle, dist, dist_eps * 1e3,speed,turnrate );//VFH更新函数
		
			//speed = (int)rint(vfh_Algorithm.GetCurrentMaxSpeed() / 4.0);
				speed=0.0;
			// Threshold to make sure we don't send arbitrarily small turn speeds
				// (which may not cause the robot to actually move).//阈值可以确保我们不发送任意小的转向速度（不能引起机器人实际移动），
				if (Desired_Angle<90.0)
				{
						turnrate = -1.6*vfh_Algorithm.GetMaxTurnrate(speed);
						fprintf(allout,"case4 -3333  %f  %f  %f %f  \n",speed,  turnrate,dist,Desired_Angle);
				}
				else if (Desired_Angle<180.0)
				{
					turnrate =1.6*vfh_Algorithm.GetMaxTurnrate(speed);
					fprintf(allout,"case4 -4444  %f  %f  %f %f  \n",speed,  turnrate,dist,Desired_Angle);
				}
				else if (Desired_Angle<270.0)
				{
					turnrate =2*vfh_Algorithm.GetMaxTurnrate(speed);
					fprintf(allout,"case4 -5555  %f  %f  %f %f  \n",speed,  turnrate,dist,Desired_Angle);
				}
				else 
				{

					turnrate =-2*vfh_Algorithm.GetMaxTurnrate(speed);
					fprintf(allout,"case4 -6666  %f  %f  %f %f  \n",speed,  turnrate,dist,Desired_Angle);
				}
			

				fprintf(allout,"case4 -1111  %f  %f  %f %f  \n",speed,  turnrate,dist,Desired_Angle);

			}
			else 
			{
				turnrate=0.0;
				speed = (int)rint(vfh_Algorithm.GetCurrentMaxSpeed() / 3.0);
				fprintf(allout,"case4-2222 %f  %f  %f %f  \n",speed,  turnrate,dist,Desired_Angle);
			}
		
		
		
		PutCommand( speed, turnrate );
		fprintf(allout,"case4 %f  %f  %f %f  \n",speed,  turnrate,dist,Desired_Angle);



	}
	else  // CASE 2: The robot is at the goal, within user-specified tolerances, so
		//         stop.条件2：机器人在目标点容许的范围内，停止

	{
		isbegio=false;
		//speed_stated = speed_stated*0.5;***********************************************************

//		 speed=(int)rint(vfh_Algorithm.GetCurrentMaxSpeed() / 10.0);
		///////////////////////////////xhy
		speed=0.0;

		//if(pianzhuan<-1)
		//{
		//	pianzhuan+=360;
		//}
		//else if(pianzhuan>361)
		//{
		//	pianzhuan-=360;
		//}

		if(abs(pianzhuan*180/PIf-Desired_Angle_ob)>=30.0)
		{
			if((Desired_Angle_ob-pianzhuan*180/PIf<180&&Desired_Angle_ob-pianzhuan*180/PIf>0)||Desired_Angle_ob-pianzhuan*180/PIf<-180)
			{
				turnrate =30*vfh_Algorithm.GetMaxTurnrate(speed);
			}
			else
			{

				turnrate =-30*vfh_Algorithm.GetMaxTurnrate(speed);
			}

		}
		else if(abs(pianzhuan*180/PIf-Desired_Angle_ob)>=10.0)
		{
			if(Desired_Angle_ob-pianzhuan*180/PIf<180&&Desired_Angle_ob-pianzhuan*180/PIf>0)
			{
				turnrate =15*vfh_Algorithm.GetMaxTurnrate(speed);
			}
			else
			{

				turnrate =-15*vfh_Algorithm.GetMaxTurnrate(speed);
			}

		}
		else
		{
			turnrate=0;
		}




		//////////////////////////////
				fprintf(allout,"case2 %f  %f  %f  \n",pianzhuan*180/PIf,  Desired_Angle_ob,turnrate);

//			turnrate=0.0;
		

		PutCommand(speed,  turnrate);////////线速度，角速度



	}

	
	
	fclose(allout);
}


void CPlan_Path_VFH::PutCommand(float cmd_speed, float cmd_turnrate )
{
	///////////////////////////////////
FILE *alloutfile9;
	
	alloutfile9 = fopen("allout69.txt","a+");
	
	player_position2d_cmd_vel_t cmd;
	this->con_vel[0] = (double)cmd_speed*cos(pianzhuan);
	this->con_vel[1] = (double)cmd_speed*sin(pianzhuan);
	this->con_vel[2] = (double)cmd_turnrate;
	memset(&cmd, 0, sizeof(cmd));

	// Stop the robot (locks the motors) if the motor state is set to
	// disabled.  The P2OS driver does not respect the motor state.若马达设定为不能使用，则机器人停止（锁定马达）
	//if (this->cmd_state == 0)
	//{
	//	cmd.vel.px = 0;
	//	cmd.vel.py = 0;
	//	cmd.vel.pa = 0;
	//}
	//// Position mode
	//else//否则，


	//xhy注销
	{
		if((this->con_vel[2]) >(double)vfh_Algorithm.GetMaxTurnrate(cmd_speed))//角速度大于平移速度时，
			//机器人旋转太快，停止旋转
		{

			this->con_vel[2] = 3*(double)vfh_Algorithm.GetMaxTurnrate(cmd_speed);
		}
		else if ((this->con_vel[2]) <  -(double)vfh_Algorithm.GetMaxTurnrate(cmd_speed))
		{
			this->con_vel[2] =  -3*(double)vfh_Algorithm.GetMaxTurnrate(cmd_speed);
		}

		cmd.vel.px =  this->con_vel[0];//单位转变
		cmd.vel.py =  this->con_vel[1];
		cmd.vel.pa =  DTOR(this->con_vel[2]);
		//	cmd.vel.pa =  this->con_vel[2];

	}


	////////此处调用motor类中的运动函数（既有前进速度，也有转向角速度时）！！！！！
	//motor.Velocity_control(cmd.vel.px, cmd.vel.pa);
	speed_line = cmd_speed;
	speed_angle = cmd.vel.pa;
//	motor.VectorMove(speed_line*80,speed_angle,Info_robot.pianzhuan,Info_robot.pointrox,Info_robot.pointroy);
	fprintf(alloutfile9,"%f %f %d %f %f %f \n",speed_line*80,speed_angle,vfh_Algorithm.GetMaxTurnrate(cmd_speed),Info_robot.pianzhuan,Info_robot.pointrox,Info_robot.pointroy);
	fclose(alloutfile9);

}


void CPlan_Path_VFH::Decision()
{
		FILE *allout;
	allout = fopen("allout14.txt","a+");
		fprintf(allout,"newstart++++++++++++++++++++++\n");

	//	MyFtestDanger();
	CtrlMode = 1;

	//停止前进情况

	//if (DangerGrade[0]==0 && DangerGrade[1] == 0 && DangerGrade[2] == 0 && DangerGrade[9] == 0 && DangerGrade[10] == 0 && DangerGrade[11] == 0)

	if(DangerGrade[3]==0 || DangerGrade[4] == 0 || DangerGrade[5] == 0 || DangerGrade[6] == 0 || DangerGrade[7] == 0 || DangerGrade[8] == 0)
	{
		//motor.stop();
		speed_l = -10;
		speed_r = -10;

	//	speed=-10;
	//   turnrate=0;
			fprintf(allout,"case 1:%d    %d   %d  %d  %d  %d  %d  %d  %d  %d  %d %d  %d  %d  %f  %f  \n",DangerGrade[0],DangerGrade[1],DangerGrade[2],DangerGrade[3],DangerGrade[4],DangerGrade[5],DangerGrade[6],DangerGrade[7],DangerGrade[8],DangerGrade[9],DangerGrade[10],DangerGrade[11],speed_l,speed_r,speeddamping_l,speeddamping_r);

	//	return;
	}

	if(DangerGrade[3]==1 || DangerGrade[4] == 1 || DangerGrade[5] == 1 || DangerGrade[6] == 1 || DangerGrade[7] == 1 || DangerGrade[8] == 1)
	{
		//motor.stop();
		speed_l = 0;
		speed_r = 0;
	//	motor.VectorMove(0,0,pianzhuan,pointrox,pointroy);
	//	speed=0;
	//	turnrate=0;
			fprintf(allout,"case2: %d    %d   %d  %d  %d  %d  %d  %d  %d  %d  %d %d  %d  %d  %f  %f  \n",DangerGrade[0],DangerGrade[1],DangerGrade[2],DangerGrade[3],DangerGrade[4],DangerGrade[5],DangerGrade[6],DangerGrade[7],DangerGrade[8],DangerGrade[9],DangerGrade[10],DangerGrade[11],speed_l,speed_r,speeddamping_l,speeddamping_r);

//		return;
	}

	//转动躲避
	for(int loop1 = 6;loop1<=10;loop1++)
	{
		switch(DangerGrade[loop1])
		{
		case 0:
			{
				speeddamping_r = 0;
			
				break;
			}
		case 1:
			{
				speeddamping_r = 0;
				//speeddamping_r+=1;
				break;
			}
		case 2:
			{
				speeddamping_r += 0.3;
				break;
			}
		case 3:
			{
				speeddamping_r += 0.2;
				break;
			}
		case 4:
			{
				speeddamping_r += 0.1;
				break;
			}
		default:
			break;
		}
	}
	for(int loop2 = 1;loop2<=5;loop2++)
	{
		switch(DangerGrade[loop2])
		{
		case 0:
			{
				speeddamping_l = 0;
				break;
			}
		case 1:
			{
				speeddamping_l = 0;
				//	speeddamping_l+=1;
				break;
			}
		case 2:
			{
				speeddamping_l += 0.3;
				break;
			}
		case 3:
			{
				speeddamping_l += 0.2;
				break;
			}
		case 4:
			{
				speeddamping_l +=0.1;
				break;
			}
		default:
			break;
		}
	}


	//安全转弯
	//if (DangerGrade[5] == 2 || DangerGrade[6] == 2)
	//{
	//	SpeedCompute(900,359,&speed_l,&speed_r,0);
	//	return;
	//}
	//if (DangerGrade[5] == 1 || DangerGrade[6] == 1)
	//{
	//	speed_r = -10;
	//	speed_l = 10;
	//	return;
	//}

	//排除特殊状态后最终指令发送
	//motor.gomotor((speed_stated+speeddamping_l),(speed_stated+speeddamping_r));	
	danger = false;
	speed_l = speed_stated * (1- speeddamping_l);
	speed_r = speed_stated * (1- speeddamping_r);

		fprintf(allout,"%d    %d   %d  %d  %d  %d  %d  %d  %d  %d  %d %d  %d  %d  %f  %f  \n",DangerGrade[0],DangerGrade[1],DangerGrade[2],DangerGrade[3],DangerGrade[4],DangerGrade[5],DangerGrade[6],DangerGrade[7],DangerGrade[8],DangerGrade[9],DangerGrade[10],DangerGrade[11],speed_l,speed_r,speeddamping_l,speeddamping_r);

	
	speeddamping_l = 0.00;
	speeddamping_r = 0.00;
	fclose(allout);

}

//void CPlan_Path_VFH::Decision()
//{
//	//	MyFtestDanger();
//	CtrlMode = 1;
//	int StopNum_l = 0;
//	int StopNum_r = 0;
//	//停止前进情况
//
//	//if (DangerGrade[0]==0 && DangerGrade[1] == 0 && DangerGrade[2] == 0 && DangerGrade[9] == 0 && DangerGrade[10] == 0 && DangerGrade[11] == 0)
//	if((DangerGrade[3]+DangerGrade[4]+ DangerGrade[5]+ DangerGrade[6]+DangerGrade[7]+DangerGrade[8])<6)
//	{
//		//motor.stop();
//		speed_l = 0;
//		speed_r = 0;
//		return;
//	}
//
//	if(DangerGrade[3]==0 || DangerGrade[4] == 0 || DangerGrade[5] == 0 || DangerGrade[6] == 0 || DangerGrade[7] == 0 || DangerGrade[8] == 0)
//	{
//		//motor.stop();
//		speed_l = 0;
//		speed_r = 0;
//		return;
//	}
//
//	//转动躲避
//	for(int loop1 = 1;loop1<=5;loop1++)
//	{
//		switch(DangerGrade[loop1])
//		{
//		case 0:
//			{
//				speeddamping_l-=3;
//				speeddamping_r+=1;
//				break;
//			}
//		case 1:
//			{
//				speeddamping_l-=3;
//				//speeddamping_r+=1;
//				break;
//			}
//		case 2:
//			{
//				speeddamping_l-=2;
//				break;
//			}
//		case 3:
//			{
//				speeddamping_l-=1;
//				break;
//			}
//		case 4:
//			{
//				speeddamping_l-=0;
//				break;
//			}
//		default:
//			break;
//		}
//	}
//	for(int loop2 = 6;loop2<=10;loop2++)
//	{
//		switch(DangerGrade[loop2])
//		{
//		case 0:
//			{
//				speeddamping_r-=3;
//				speeddamping_l+=1;
//				break;
//			}
//		case 1:
//			{
//				speeddamping_r-=3;
//				//	speeddamping_l+=1;
//				break;
//			}
//		case 2:
//			{
//				speeddamping_r-=2;
//				break;
//			}
//		case 3:
//			{
//				speeddamping_r-=1;
//				break;
//			}
//		case 4:
//			{
//				speeddamping_r-=0;
//				break;
//			}
//		default:
//			break;
//		}
//	}
//
//
//	//安全转弯
//	//if (DangerGrade[5] == 2 || DangerGrade[6] == 2)
//	//{
//	//	SpeedCompute(900,359,&speed_l,&speed_r,0);
//	//	return;
//	//}
//	//if (DangerGrade[5] == 1 || DangerGrade[6] == 1)
//	//{
//	//	speed_r = -10;
//	//	speed_l = 10;
//	//	return;
//	//}
//
//	//排除特殊状态后最终指令发送
//	//motor.gomotor((speed_stated+speeddamping_l),(speed_stated+speeddamping_r));	
//	danger = false;
//	speed_l = speed_stated+speeddamping_l;
//	speed_r = speed_stated+speeddamping_r;
//	speeddamping_l = 0;
//	speeddamping_r = 0;
//
//}

void CPlan_Path_VFH::MyFtestDanger(void)
{
	int xhyceshi=2;
	//自右向左将180°分为0-11共十二个扇区,分别判定每个扇区的危险情况
	//每个扇区有1-4个危险等级,1级为绝对危险,4级为绝对安全
	int danger0 = 100/xhyceshi; //每个扇区绝对停止距离
	int danger1 = 125/xhyceshi;
	int danger2 = 300/xhyceshi;
	int danger3 = 350/xhyceshi;
	int danger4 = 400/xhyceshi;
	int danger5 = 450/xhyceshi;
	int danger6 = 450/xhyceshi;
	int danger7 = 400/xhyceshi;
	int danger8 = 350/xhyceshi;
	int danger9 = 300/xhyceshi;
	int danger10 = 250/xhyceshi;
	int danger11 = 200/xhyceshi;

	//int danger0 = 360; //每个扇区绝对停止距离
	//int danger1 = 360;
	//int danger2 = 360;
	//int danger3 = 360;
	//int danger4 = 360;
	//int danger5 = 360;
	//int danger6 = 360;
	//int danger7 = 360;
	//int danger8 = 360;
	//int danger9 = 360;
	//int danger10 = 360;
	//int danger11 = 360;

	int i = 0;
	int icount=0;
	int laser_data_temp = 0;

	//0扇区判断
	laser_data_temp = 0;
	for(i = 129;i<172;i++)
	{
		laser_data_temp += m_laser_data_postpro[i];
		icount++;
	}
	laser_data_temp = laser_data_temp/icount;
	
	if (laser_position)
	{
		distance_min[11] = laser_data_temp;
		
	}
	else if (!laser_position)
	{
		distance_min[0] = laser_data_temp;
		
	}
	if (laser_data_temp<=danger0)
	{
		if (laser_position)
		{
			DangerGrade[11] = 0;
		}
		else if (!laser_position)
		{
			DangerGrade[0] = 0;
		}
	}
	else if (laser_data_temp>danger0 && laser_data_temp<=danger0+dangerdistance0)
	{
		if (laser_position)
		{
			DangerGrade[11] = 1;
		}
		else if (!laser_position)
		{
			DangerGrade[0] = 1;
		}
	}
	else if (laser_data_temp>danger0+dangerdistance0 && laser_data_temp<=danger0+dangerdistance1)
	{
		if (laser_position)
		{
			DangerGrade[11] = 2;
		}
		else if (!laser_position)
		{
			DangerGrade[0] = 2;
		};
	}
	else if (laser_data_temp>danger0+dangerdistance1 && laser_data_temp<=danger0+dangerdistance2)
	{
		if (laser_position)
		{
			DangerGrade[11] = 3;
		}
		else if (!laser_position)
		{
			DangerGrade[0] = 3;
		}
	}
	else if (laser_data_temp>danger0+dangerdistance2 && laser_data_temp<= 1800)
	{
		if (laser_position)
		{
			DangerGrade[11] = 4;
		}
		else if (!laser_position)
		{
			DangerGrade[0] = 4;
		}
	}
	else if (laser_data_temp>1800)
	{
		if (laser_position)
		{
			DangerGrade[11] = 5;
		}
		else if (!laser_position)
		{
			DangerGrade[0] = 5;
		}
	}

	//1扇区判断
	laser_data_temp = 0;
	icount=0;
	for(i = 172;i<214;i++)
	{
		laser_data_temp += m_laser_data_postpro[i];
		icount++;
	}
	laser_data_temp = laser_data_temp/icount;
	if (laser_position)
	{
		distance_min[10] = laser_data_temp;
	}
	else if (!laser_position)
	{
		distance_min[1] = laser_data_temp;
	}
	if (laser_data_temp<=danger1)
	{
		if (laser_position)
		{
			DangerGrade[10] = 0;
		}
		else if (!laser_position)
		{
			DangerGrade[1] = 0;
		}
	}
	else if (laser_data_temp>danger1 && laser_data_temp<=danger1+dangerdistance0)
	{
		if (laser_position)
		{
			DangerGrade[10] = 1;
		}
		else if (!laser_position)
		{
			DangerGrade[1] = 1;
		}
	}
	else if (laser_data_temp>danger1+dangerdistance0 && laser_data_temp<=danger1+dangerdistance1)
	{
		if (laser_position)
		{
			DangerGrade[10] = 2;
		}
		else if (!laser_position)
		{
			DangerGrade[1] = 2;
		}
	}
	else if (laser_data_temp>danger1+dangerdistance1 && laser_data_temp<=danger1+dangerdistance2)
	{
		if (laser_position)
		{
			DangerGrade[10] = 3;
		}
		else if (!laser_position)
		{
			DangerGrade[1] = 3;
		}
	}
	else if (laser_data_temp>danger1+dangerdistance2 && laser_data_temp<= 1800)
	{
		if (laser_position)
		{
			DangerGrade[10] = 4;
		}
		else if (!laser_position)
		{
			DangerGrade[1] = 4;
		}
	}
	else if (laser_data_temp>1800)
	{
		if (laser_position)
		{
			DangerGrade[10] = 5;
		}
		else if (!laser_position)
		{
			DangerGrade[1] = 5;
		}
	}
	//2扇区判断
	laser_data_temp = 0;
	icount=0;
	for(i = 214;i<257;i++)
	{
		laser_data_temp += m_laser_data_postpro[i];
		icount++;
	}
	laser_data_temp = laser_data_temp/icount;
	if (laser_position)
	{
		distance_min[9] = laser_data_temp;
	}
	else if (!laser_position)
	{
		distance_min[2] = laser_data_temp;
	}
	if (laser_data_temp<=danger2)
	{
		if (laser_position)
		{
			DangerGrade[9] = 0;
		}
		else if (!laser_position)
		{
			DangerGrade[2] = 0;
		}
	}
	else if (laser_data_temp>danger2 && laser_data_temp<=danger2+dangerdistance0)
	{
		if (laser_position)
		{
			DangerGrade[9] = 1;
		}
		else if (!laser_position)
		{
			DangerGrade[2] = 1;
		}
	}
	else if (laser_data_temp>danger2+dangerdistance0 && laser_data_temp<=danger2+dangerdistance1)
	{
		if (laser_position)
		{
			DangerGrade[9] = 2;
		}
		else if (!laser_position)
		{
			DangerGrade[2] = 2;
		}
	}
	else if (laser_data_temp>danger2+dangerdistance1 && laser_data_temp<=danger2+dangerdistance2)
	{
		if (laser_position)
		{
			DangerGrade[9] = 3;
		}
		else if (!laser_position)
		{
			DangerGrade[2] = 3;
		}
	}
	else if (laser_data_temp>danger2+dangerdistance2 && laser_data_temp<= 1800)
	{
		if (laser_position)
		{
			DangerGrade[9] = 4;
		}
		else if (!laser_position)
		{
			DangerGrade[2] = 4;
		}
	}
	else if (laser_data_temp>1800)
	{
		if (laser_position)
		{
			DangerGrade[9] = 5;
		}
		else if (!laser_position)
		{
			DangerGrade[2] = 5;
		}
	}

	//3扇区判断
	laser_data_temp = 0;
	icount=0;
	for(i = 257;i<299;i++)
	{
		laser_data_temp += m_laser_data_postpro[i];
		icount++;
	}
	laser_data_temp = laser_data_temp/icount;
	if (laser_position)
	{
		distance_min[8] = laser_data_temp;
	}
	else if (!laser_position)
	{
		distance_min[3] = laser_data_temp;
	}
	if (laser_data_temp<=danger3)
	{
		if (laser_position)
		{
			DangerGrade[8] = 0;
		}
		else if (!laser_position)
		{
			DangerGrade[3] = 0;
		}
	}
	else if (laser_data_temp>danger3 && laser_data_temp<=danger3+dangerdistance0)
	{
		if (laser_position)
		{
			DangerGrade[8] = 1;
		}
		else if (!laser_position)
		{
			DangerGrade[3] = 1;
		}
	}
	else if (laser_data_temp>danger3+dangerdistance0 && laser_data_temp<=danger3+dangerdistance1)
	{
		if (laser_position)
		{
			DangerGrade[8] = 2;
		}
		else if (!laser_position)
		{
			DangerGrade[3] = 2;
		}
	}
	else if (laser_data_temp>danger3+dangerdistance1 && laser_data_temp<=danger3+dangerdistance2)
	{
		if (laser_position)
		{
			DangerGrade[8] = 3;
		}
		else if (!laser_position)
		{
			DangerGrade[3] = 3;
		}
	}
	else if (laser_data_temp>danger3+dangerdistance2 && laser_data_temp<= 1800)
	{
		if (laser_position)
		{
			DangerGrade[8] = 4;
		}
		else if (!laser_position)
		{
			DangerGrade[3] = 4;
		}
	}
	else if (laser_data_temp>1800)
	{
		if (laser_position)
		{
			DangerGrade[8] = 5;
		}
		else if (!laser_position)
		{
			DangerGrade[3] = 5;
		}
	}

	//4扇区判断
	laser_data_temp = 0;
	icount=0;
	for(i = 299;i<342;i++)
	{
		laser_data_temp += m_laser_data_postpro[i];
		icount++;
	}
	laser_data_temp = laser_data_temp/icount;
	if (laser_position)
	{
		distance_min[7] = laser_data_temp;
	}
	else if (!laser_position)
	{
		distance_min[4] = laser_data_temp;
	}
	if (laser_data_temp<=danger4)
	{
		if (laser_position)
		{
			DangerGrade[7] = 0;
		}
		else if (!laser_position)
		{
			DangerGrade[4] = 0;
		}
	}
	else if (laser_data_temp>danger4 && laser_data_temp<=danger4+dangerdistance0)
	{
		if (laser_position)
		{
			DangerGrade[7] = 1;
		}
		else if (!laser_position)
		{
			DangerGrade[4] = 1;
		}
	}
	else if (laser_data_temp>danger4+dangerdistance0 && laser_data_temp<=danger4+dangerdistance1)
	{
		if (laser_position)
		{
			DangerGrade[7] = 2;
		}
		else if (!laser_position)
		{
			DangerGrade[4] = 2;
		}
	}
	else if (laser_data_temp>danger4+dangerdistance1 && laser_data_temp<=danger4+dangerdistance2)
	{
		if (laser_position)
		{
			DangerGrade[7] = 3;
		}
		else if (!laser_position)
		{
			DangerGrade[4] = 3;
		}
	}
	else if (laser_data_temp>danger4+dangerdistance2 && laser_data_temp<= 1800)
	{
		if (laser_position)
		{
			DangerGrade[7] = 4;
		}
		else if (!laser_position)
		{
			DangerGrade[4] = 4;
		}
	}
	else if (laser_data_temp>1800)
	{
		if (laser_position)
		{
			DangerGrade[7] = 5;
		}
		else if (!laser_position)
		{
			DangerGrade[4] = 5;
		}
	}

	//5扇区判断
	laser_data_temp = 0;
	icount=0;
	for(i = 342;i<384;i++)
	{
		laser_data_temp += m_laser_data_postpro[i];
		icount++;
	}
	laser_data_temp = laser_data_temp/icount;
	if (laser_position)
	{
		distance_min[6] = laser_data_temp;
	}
	else if (!laser_position)
	{
		distance_min[5] = laser_data_temp;
	}
	if (laser_data_temp<=danger5)
	{
		if (laser_position)
		{
			DangerGrade[6] = 0;
		}
		else if (!laser_position)
		{
			DangerGrade[5] = 0;
		}
	}
	else if (laser_data_temp>danger5 && laser_data_temp<=danger5+dangerdistance0)
	{
		if (laser_position)
		{
			DangerGrade[6] = 1;
		}
		else if (!laser_position)
		{
			DangerGrade[5] = 1;
		}
	}
	else if (laser_data_temp>danger5+dangerdistance0 && laser_data_temp<=danger5+dangerdistance1)
	{
		if (laser_position)
		{
			DangerGrade[6] = 2;
		}
		else if (!laser_position)
		{
			DangerGrade[5] = 2;
		}
	}
	else if (laser_data_temp>danger5+dangerdistance1 && laser_data_temp<=danger5+dangerdistance2)
	{
		if (laser_position)
		{
			DangerGrade[6] = 3;
		}
		else if (!laser_position)
		{
			DangerGrade[5] = 3;
		}
	}
	else if (laser_data_temp>danger5+dangerdistance2 && laser_data_temp<= 1800)
	{
		if (laser_position)
		{
			DangerGrade[6] = 4;
		}
		else if (!laser_position)
		{
			DangerGrade[5] = 4;
		}
	}
	else if (laser_data_temp>1800)
	{
		if (laser_position)
		{
			DangerGrade[6] = 5;
		}
		else if (!laser_position)
		{
			DangerGrade[5] = 5;
		}
	}
	//6扇区判断
	laser_data_temp = 0;
	icount=0;
	for(i = 384;i<426;i++)
	{
		laser_data_temp += m_laser_data_postpro[i];
		icount++;
	}
	laser_data_temp = laser_data_temp/icount;
	if (laser_position)
	{
		distance_min[5] = laser_data_temp;
	}
	else if (!laser_position)
	{
		distance_min[6] = laser_data_temp;
	}
	if (laser_data_temp<=danger6)
	{
		if (laser_position)
		{
			DangerGrade[5] = 0;
		}
		else if (!laser_position)
		{
			DangerGrade[6] = 0;
		}
	}
	else if (laser_data_temp>danger6 && laser_data_temp<=danger5+dangerdistance0)
	{
		if (laser_position)
		{
			DangerGrade[5] = 1;
		}
		else if (!laser_position)
		{
			DangerGrade[6] = 1;
		}
	}
	else if (laser_data_temp>danger6+dangerdistance0 && laser_data_temp<=danger6+dangerdistance1)
	{
		if (laser_position)
		{
			DangerGrade[5] = 2;
		}
		else if (!laser_position)
		{
			DangerGrade[6] = 2;
		}
	}
	else if (laser_data_temp>danger6+dangerdistance1 && laser_data_temp<=danger6+dangerdistance2)
	{
		if (laser_position)
		{
			DangerGrade[5] = 3;
		}
		else if (!laser_position)
		{
			DangerGrade[6] = 3;
		}
	}
	else if (laser_data_temp>danger6+dangerdistance2 && laser_data_temp<= 1800)
	{
		if (laser_position)
		{
			DangerGrade[5] = 4;
		}
		else if (!laser_position)
		{
			DangerGrade[6] = 4;
		}
	}
	else if (laser_data_temp>1800)
	{
		if (laser_position)
		{
			DangerGrade[5] = 5;
		}
		else if (!laser_position)
		{
			DangerGrade[6] = 5;
		}
	}

	//7扇区判断
	laser_data_temp = 0;
	icount=0;
	for(i = 426;i<469;i++)
	{
		laser_data_temp += m_laser_data_postpro[i];
		icount++;
	}
	laser_data_temp = laser_data_temp/icount;
	if (laser_position)
	{
		distance_min[4] = laser_data_temp;
	}
	else if (!laser_position)
	{
		distance_min[7] = laser_data_temp;
	}
	if (laser_data_temp<=danger7)
	{
		if (laser_position)
		{
			DangerGrade[4] = 0;
		}
		else if (!laser_position)
		{
			DangerGrade[7] = 0;
		}
	}
	else if (laser_data_temp>danger7 && laser_data_temp<=danger5+dangerdistance0)
	{
		if (laser_position)
		{
			DangerGrade[4] = 1;
		}
		else if (!laser_position)
		{
			DangerGrade[7] = 1;
		}
	}
	else if (laser_data_temp>danger7+dangerdistance0 && laser_data_temp<=danger7+dangerdistance1)
	{
		if (laser_position)
		{
			DangerGrade[4] = 2;
		}
		else if (!laser_position)
		{
			DangerGrade[7] = 2;
		}
	}
	else if (laser_data_temp>danger7+dangerdistance1 && laser_data_temp<=danger7+dangerdistance2)
	{
		if (laser_position)
		{
			DangerGrade[4] = 3;
		}
		else if (!laser_position)
		{
			DangerGrade[7] = 3;
		}
	}
	else if (laser_data_temp>danger7+dangerdistance2 && laser_data_temp<= 1800)
	{
		if (laser_position)
		{
			DangerGrade[4] = 4;
		}
		else if (!laser_position)
		{
			DangerGrade[7] = 4;
		}
	}
	else if (laser_data_temp>1800)
	{
		if (laser_position)
		{
			DangerGrade[4] = 5;
		}
		else if (!laser_position)
		{
			DangerGrade[7] = 5;
		}
	}

	//8扇区判断
	laser_data_temp = 0;
	icount=0;
	for(i = 469;i<511;i++)
	{
		laser_data_temp += m_laser_data_postpro[i];
		icount++;
	}
	laser_data_temp = laser_data_temp/icount;
	if (laser_position)
	{
		distance_min[3] = laser_data_temp;
	}
	else if (!laser_position)
	{
		distance_min[8] = laser_data_temp;
	}
	if (laser_data_temp<=danger8)
	{
		if (laser_position)
		{
			DangerGrade[3] = 0;
		}
		else if (!laser_position)
		{
			DangerGrade[8] = 0;
		}
	}
	else if (laser_data_temp>danger8 && laser_data_temp<=danger5+dangerdistance0)
	{
		if (laser_position)
		{
			DangerGrade[3] = 1;
		}
		else if (!laser_position)
		{
			DangerGrade[8] = 1;
		}
	}
	else if (laser_data_temp>danger8+dangerdistance0 && laser_data_temp<=danger8+dangerdistance1)
	{
		if (laser_position)
		{
			DangerGrade[3] = 2;
		}
		else if (!laser_position)
		{
			DangerGrade[8] = 2;
		}
	}
	else if (laser_data_temp>danger8+dangerdistance1 && laser_data_temp<=danger8+dangerdistance2)
	{
		if (laser_position)
		{
			DangerGrade[3] = 3;
		}
		else if (!laser_position)
		{
			DangerGrade[8] = 3;
		}
	}
	else if (laser_data_temp>danger8+dangerdistance2 && laser_data_temp<= 1800)
	{
		if (laser_position)
		{
			DangerGrade[3] = 4;
		}
		else if (!laser_position)
		{
			DangerGrade[8] = 4;
		}
	}
	else if (laser_data_temp>1800)
	{
		if (laser_position)
		{
			DangerGrade[3] = 5;
		}
		else if (!laser_position)
		{
			DangerGrade[8] = 5;
		}
	}

	//9扇区判断
	laser_data_temp = 0;
	icount=0;
	for(i = 511;i<554;i++)
	{
		laser_data_temp += m_laser_data_postpro[i];
		icount++;
	}	
	laser_data_temp = laser_data_temp/icount;
	if (laser_position)
	{
		distance_min[2] = laser_data_temp;
	}
	else if (!laser_position)
	{
		distance_min[9] = laser_data_temp;
	}
	if (laser_data_temp<=danger9)
	{
		if (laser_position)
		{
			DangerGrade[2] = 0;
		}
		else if (!laser_position)
		{
			DangerGrade[9] = 0;
		}
	}
	else if (laser_data_temp>danger9 && laser_data_temp<=danger5+dangerdistance0)
	{
		if (laser_position)
		{
			DangerGrade[2] = 1;
		}
		else if (!laser_position)
		{
			DangerGrade[9] = 1;
		}
	}
	else if (laser_data_temp>danger9+dangerdistance0 && laser_data_temp<=danger9+dangerdistance1)
	{
		if (laser_position)
		{
			DangerGrade[2] = 2;
		}
		else if (!laser_position)
		{
			DangerGrade[9] = 2;
		}
	}
	else if (laser_data_temp>danger9+dangerdistance1 && laser_data_temp<=danger9+dangerdistance2)
	{
		if (laser_position)
		{
			DangerGrade[2] = 3;
		}
		else if (!laser_position)
		{
			DangerGrade[9] = 3;
		}
	}
	else if (laser_data_temp>danger9+dangerdistance2 && laser_data_temp<= 1800)
	{
		if (laser_position)
		{
			DangerGrade[2] = 4;
		}
		else if (!laser_position)
		{
			DangerGrade[9] = 4;
		}
	}
	else if (laser_data_temp>1800)
	{
		if (laser_position)
		{
			DangerGrade[2] = 5;
		}
		else if (!laser_position)
		{
			DangerGrade[9] = 5;
		}
	}

	//10扇区判断
	laser_data_temp = 0;
	icount=0;
	for(i = 554;i<596;i++)
	{
		laser_data_temp += m_laser_data_postpro[i];
		icount++;
	}
	laser_data_temp = laser_data_temp/icount;
	if (laser_position)
	{
		distance_min[1] = laser_data_temp;
	}
	else if (!laser_position)
	{
		distance_min[10] = laser_data_temp;
	}
	if (laser_data_temp<=danger10)
	{
		if (laser_position)
		{
			DangerGrade[1] = 0;
		}
		else if (!laser_position)
		{
			DangerGrade[10] = 0;
		}
	}
	else if (laser_data_temp>danger10 && laser_data_temp<=danger5+dangerdistance0)
	{
		if (laser_position)
		{
			DangerGrade[1] = 1;
		}
		else if (!laser_position)
		{
			DangerGrade[10] = 1;
		}
	}
	else if (laser_data_temp>danger10+dangerdistance0 && laser_data_temp<=danger10+dangerdistance1)
	{
		if (laser_position)
		{
			DangerGrade[1] = 2;
		}
		else if (!laser_position)
		{
			DangerGrade[10] = 2;
		}
	}
	else if (laser_data_temp>danger10+dangerdistance1 && laser_data_temp<=danger10+dangerdistance2)
	{
		if (laser_position)
		{
			DangerGrade[1] = 3;
		}
		else if (!laser_position)
		{
			DangerGrade[10] = 3;
		}
	}
	else if (laser_data_temp>danger10+dangerdistance2 && laser_data_temp<= 1800)
	{
		if (laser_position)
		{
			DangerGrade[1] = 4;
		}
		else if (!laser_position)
		{
			DangerGrade[10] = 4;
		}
	}
	else if (laser_data_temp>1800)
	{
		if (laser_position)
		{
			DangerGrade[1] = 5;
		}
		else if (!laser_position)
		{
			DangerGrade[10] = 5;
		}
	}

	//11扇区判断
	laser_data_temp = 0;
	icount=0;
	for(i = 596;i<639;i++)
	{
		laser_data_temp += m_laser_data_postpro[i];
		icount++;
	}
	laser_data_temp = laser_data_temp/icount;
	if (laser_position)
	{
		distance_min[0] = laser_data_temp;
	}
	else if (!laser_position)
	{
		distance_min[11] = laser_data_temp;
	}
	if (laser_data_temp<=danger11)
	{
		if (laser_position)
		{
			DangerGrade[0] = 0;
		}
		else if (!laser_position)
		{
			DangerGrade[11] = 0;
		}
	}
	else if (laser_data_temp>danger11 && laser_data_temp<=danger5+dangerdistance0)
	{
		if (laser_position)
		{
			DangerGrade[0] = 1;
		}
		else if (!laser_position)
		{
			DangerGrade[11] = 1;
		}
	}
	else if (laser_data_temp>danger11+dangerdistance0 && laser_data_temp<=danger11+dangerdistance1)
	{
		if (laser_position)
		{
			DangerGrade[0] = 2;
		}
		else if (!laser_position)
		{
			DangerGrade[11] = 2;
		}
	}
	else if (laser_data_temp>danger11+dangerdistance1 && laser_data_temp<=danger11+dangerdistance2)
	{
		if (laser_position)
		{
			DangerGrade[0] = 3;
		}
		else if (!laser_position)
		{
			DangerGrade[11] = 3;
		}
	}
	else if (laser_data_temp>danger11+dangerdistance2 && laser_data_temp<= 1800)
	{
		if (laser_position)
		{
			DangerGrade[0] = 4;
		}
		else if (!laser_position)
		{
			DangerGrade[11] = 4;
		}
	}
	else if (laser_data_temp>1800)
	{
		if (laser_position)
		{
			DangerGrade[0] = 5;
		}
		else if (!laser_position)
		{
			DangerGrade[11] = 5;
		}
	}
}



double CPlan_Path_VFH::angle_diff(double a, double b)
{
	double d1, d2;
	a = NORMALIZE(a);
	b = NORMALIZE(b);
	d1 = a - b;
	d2 = 2 * M_PI - fabs(d1);
	if (d1 > 0.0) d2 *= -1.0;
	if (fabs(d1) < fabs(d2)) return d1;
	return d2;
}

bool CPlan_Path_VFH::Range_to_go(int target_x,int target_y,int robot_x,int robot_y) //判断是否到达目标点
{
	distance = (sqrt(((robot_x-Scene_scale_x*target_x)*(robot_x-Scene_scale_x*target_x))+((robot_y-Scene_scale_y*target_y)*(robot_y-Scene_scale_y*target_y))));
	FILE *alloutXXX;
	alloutXXX = fopen("alloutfff.txt","a+");
	fprintf(alloutXXX ," %d  \n",distance);
	fclose(alloutXXX );
	if (distance<500)
	{
		temp1 = 2000;
		return true;
		//speed_stated = speed_low;
		//	WriteConsole(hdlWrite, "speed-low\n",12, NULL, NULL);
	}
	else if(distance>=500)
	{
		speed_stated = speed_high;
		//	WriteConsole(hdlWrite, "speed-hight\n",12, NULL, NULL);
		//return false;
	}

	if (distance<=0.80*dist_eps * 1e3)
	{
		temp1 = 2000;
		return true;
	}
	else if (distance<=1.5*dist_eps * 1e3)
	{
		temp1 = 200;
		return false;
	}
	else
	{
		return false;
	}

	
}



//根据转弯半径计算左右轮速
void CPlan_Path_VFH::SpeedCompute(int raidus,int wheel_gauge,int *speed_l,int *speed_r,int direction)  
{
	*speed_r = (*speed_l*(raidus-wheel_gauge)/wheel_gauge) / (1+(raidus-wheel_gauge)/wheel_gauge);
}