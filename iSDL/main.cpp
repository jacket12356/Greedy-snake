#include "data.h"
using namespace std;
//还是用链表比较方便，用链表表示蛇身上的每一块的坐标就行,这样蛇的移动问题就解决了

Uint32 callback(Uint32 interval, void *param);			//callback

void snakeDied(void);

void mouseButtonEvent(const SDL_Event *pEvent);			//失败选择界面的鼠标单击事件

class Interface
{
public:

	void backgroundPaint(void)        //有SDL_RenderPresent
	{
		SDL_RenderCopy(pren,ptex,NULL,NULL);
		//SDL_RenderPresent(pren);
	}

	Interface():pwin(NULL),pren(NULL),ptex(NULL),wall(NULL),food(NULL)
	{
		SDL_Init(SDL_INIT_EVERYTHING);
		pwin=SDL_CreateWindow("iSDL",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,1000,600,SDL_WINDOW_SHOWN);
		pren=SDL_CreateRenderer(pwin,-1,NULL);
		ptex=IMG_LoadTexture(pren,"../image/background.jpg");		//这是背景
		wall=IMG_LoadTexture(pren,"../image/wall.png");
		food=IMG_LoadTexture(pren,"../image/food.png");
		for(int i=0;i<12;i++)					//获取原矩阵副本
		{
			for(int j=0;j<20;j++)	Map[i][j]=map[i][j];
		}
	}

	~Interface()
	{
		SDL_DestroyTexture(food);
		SDL_DestroyTexture(wall);
		SDL_DestroyTexture(ptex);
		SDL_DestroyRenderer(pren);
		SDL_DestroyWindow(pwin);
	}
protected:
	SDL_Window *pwin;
	SDL_Renderer *pren;
	SDL_Texture *ptex;			//背景图片
	SDL_Texture *wall;
	SDL_Texture *food;
	int Map[12][20];			//data.h中的矩阵副本，作绘制每一帧用
};

class Snake:public Interface
{
public:
	block *initBlock()
	{
		block *tail;
		tail=new block;
		tail->next=NULL;
		tail->x=1;	tail->y=1;			//0，0 是墙

		return tail;			//尾巴就是第一块
	}

	block *createBody(block *tail,int len)       //传进来的值是3，那么蛇的初始长度就是4了
	{
		block *head=tail;							//用到了尾插法，传进来的是尾，输出去的是头。
		while(len>0)
		{
			head->next=new block;
			head->next->x=head->x;	head->next->y=head->y+1;	//逐个构造蛇身坐标
			head=head->next;
			head->next=NULL;
			len--;
		}
		return head;		//返回头
	}

	void laySnake(block * tail)		//将整条蛇的位置放到地图中去
	{
		block *body=tail;
		while(body!=NULL)
		{
			Map[body->x][body->y]=BLOCK;
			body=body->next;
		}
	}

	void layFood(void)			//在地图中随机放置食物
	{
		int x,y;

		srand((unsigned int)time(0));		//播种
		x=rand()%10+1;	y=rand()%18+1;
		fpos.x=x;	fpos.y=y;
		while(Map[x][y]==BLOCK)
		{
			x=rand()%10+1;	y=rand()%18+1;
			fpos.x=x;	fpos.y=y;
		}
		Map[x][y]=FOOD;
	}

	int bodyFound(block *tail)
	{
		block *p=tail;
		while(p!=Head)
		{
			if(p->x==Head->x&&p->y==Head->y)
			{
				return 1;
			}
			p=p->next;
		}
		return 0;
	}

	void eat(block* &head,direction dir)		//由move函数调用
	{
		block *p=NULL;

		p=new block;
		if(dir==UP)
		{
			p->x=head->x-1;	p->y=head->y;	p->next=NULL;
			head->next=p;	head=head->next;
			Map[p->x][p->y]=SPACE;				//原来是食物的地方置空
		}
		if(dir==DOWN)				
		{
			p->x=head->x+1;	p->y=head->y;	p->next=NULL;
			head->next=p;	head=head->next;
			Map[p->x][p->y]=SPACE;
		}
		if(dir==LEFT)							
		{
			p->x=head->x;	p->y=head->y-1;	p->next=NULL;
			head->next=p;	head=head->next;
			Map[p->x][p->y]=SPACE;
		}
		if(dir==RIGHT)
		{
			p->x=head->x;	p->y=head->y+1;	p->next=NULL;
			head->next=p;	head=head->next;
			Map[p->x][p->y]=SPACE;
		}
		length++;
	}

	void move(block* &head,block* &tail,direction dir)		//由callback函数调用
	{//move()函数中还包含了蛇吃食物、系统重新放置食物、链表中信息确定后放置蛇等过程,总的来说这个函数就是在确定矩阵中的信息
		block *base=NULL;     //中转

		for(int i=0;i<12;i++)					//刷新矩阵,记得保留原有食物的位置
		{
			for(int j=0;j<20;j++)	Map[i][j]=map[i][j];
		}
		SDL_RenderClear(pren);					//渲染器也要跟着刷新
		Map[fpos.x][fpos.y]=FOOD;

		if(dir==UP)
		{
			if(Map[head->x-1][head->y]==FOOD)	//如果蛇面前的是食物，就调用eat(),蛇吃完后就在放一块食物，调用layFood()
			{eat(head,dir);		layFood();}
			else if(Map[head->x][head->y]==WALL)
			{
				snakeDied();
			}
			else
			{
				base=tail->next;										//把尾巴放到头前面
				tail->x=head->x-1;	tail->y=head->y;	tail->next=NULL;
				head->next=tail;	head=head->next;
				tail=base;
			}
		}
		if(dir==DOWN)							//注意：当蛇在Y轴上运动时，它只能选择一个X轴方向
		{
			if(Map[head->x+1][head->y]==FOOD)
			{eat(head,dir);		layFood();}
			else if(Map[head->x][head->y]==WALL)
			{
				snakeDied();
			}
			else
			{
				base=tail->next;
				tail->x=head->x+1;	tail->y=head->y;	tail->next=NULL;
				head->next=tail;	head=head->next;
				tail=base;
			}
		}
		if(dir==LEFT)									//注意：当蛇在X轴上运动时，它只能选择一个Y轴方向
		{
			if(Map[head->x][head->y-1]==FOOD)
			{eat(head,dir);		layFood();}
			else if(Map[head->x][head->y]==WALL)
			{
				snakeDied();
			}
			else
			{
				base=tail->next;
				tail->x=head->x;	tail->y=head->y-1;	tail->next=NULL;
				head->next=tail;	head=head->next;
				tail=base;
			}
		}
		if(dir==RIGHT)
		{
			if(Map[head->x][head->y+1]==FOOD)
			{eat(head,dir);		layFood();}
			else if(Map[head->x][head->y]==WALL)
			{
				snakeDied();
			}
			else
			{
				base=tail->next;
				tail->x=head->x;	tail->y=head->y+1;	tail->next=NULL;
				head->next=tail;	head=head->next;
				tail=base;
			}
		}
		laySnake(tail);

		if(bodyFound(tail))		//检查蛇是不是已经吃到了自己
		{
			snakeDied();
		}
	}

	SDL_Rect *rectGen(int x,int y)		//根据坐标值生成矩阵
	{
		dst.x=y*BLOCK_W;	dst.y=x*BLOCK_H;
		dst.h=BLOCK_H;		dst.w=BLOCK_W;

		return &dst;
	}

	void interpret(void)					//将Map中的信息提取出来，从而绘制出每一帧
	{
		backgroundPaint();
		if(d==0)
		{
		for(int i=0;i<12;i++)
		{
			for(int j=0;j<20;j++)	
			{
				switch(Map[i][j])
				{
				case WALL:
					SDL_RenderCopy(pren,wall,NULL,rectGen(i,j));
					break;
				case BLOCK:
					SDL_RenderCopy(pren,snake,NULL,rectGen(i,j));
					break;
				case FOOD:
					SDL_RenderCopy(pren,food,NULL,rectGen(i,j));
					break;
				default:
					break;
				}
			}
		}
		SDL_RenderPresent(pren);
		}
		else
		{
			SDL_Texture *Died=NULL;
			Died=IMG_LoadTexture(pren,"../image/died.jpg");
			SDL_RenderClear(pren);
			SDL_RenderCopy(pren,Died,NULL,NULL);
			SDL_RenderPresent(pren);
		}
	}

	//void judge(int x,int y)		//判断蛇移动到下一位后会发生什么情况，并采取相应的操作
	//{
	//	switch(Map[x][y])
	//	{
	//	case FOOD:
	//		eat(Head,Dir);		
	//		layFood();
	//	case WALL:
	//	case BLOCK:
	//	case SPACE:
	//		break;
	//	}
	//}

	Snake():snake(NULL),Tail(NULL),Head(NULL),d(0)
	{
		//创建一条蛇(链表)
		length=LEN;
		Dir=RIGHT;		//初始方向向右
		Tail=initBlock();
		Head=createBody(Tail,length-1);
		//蛇图片
		snake=IMG_LoadTexture(pren,"../image/block.png");
		//将链表信息放到矩阵中
		laySnake(Tail);
		layFood();			//这是第一块食物
		interpret();
	}

	~Snake(){
		SDL_DestroyTexture(snake);
		while(Tail!=NULL)
		{
			block *t=Tail;
			Tail=Tail->next;
			delete t;
		}
	}
protected:
	SDL_Texture *snake;		  //其实是蛇身上的一块图片
	block *Tail,*Head;        //蛇的头、尾
	direction Dir;
	SDL_Rect dst;
	Food fpos;				  //食物坐标保留
	int d;					  //死亡标志
	int length;
};

class Event:public Snake
{
public:
	Event():quit(0),id(2),delay(350){};

	void keyPress(SDL_Keycode key)
	{
		if(Dir==UP || Dir==DOWN)
		{
			switch(key)
			{
			case SDLK_LEFT:
				Dir=LEFT;	
				break;
			case SDLK_RIGHT:
				Dir=RIGHT;
				break;
			case SDLK_a:
				Dir=LEFT;
				break;
			case SDLK_d:
				Dir=RIGHT;
				break;
			default:
				break;
			}
		}
		if(Dir==LEFT || Dir==RIGHT)
		{
			switch(key)
			{
			case SDLK_UP:
				Dir=UP;
				break;
			case SDLK_DOWN:
				Dir=DOWN;
				break;
			case SDLK_w:
				Dir=UP;
				break;
			case SDLK_s:
				Dir=DOWN;
				break;
			default:
				break;
			}
		}
	}

	void eHandle(void)
	{
		while(quit==0)
		{
			while(SDL_PollEvent(&events))
			{
				 switch(events.type)
				 {
					 case SDL_QUIT:
						 cout<<"退出游戏！"<<endl;
						 quit=1;
						 break;
					 case SDL_KEYDOWN:
						keyPress(events.key.keysym.sym);		//通过按下方向键来改变方向
						if(id==2)	id=SDL_AddTimer(delay, callback, NULL);		//按下按键才能真正开始
						break;
					case SDL_MOUSEBUTTONDOWN:		//鼠标点击事件
						mouseButtonEvent(&events);
						break;
					 default:
						 break;
				 }
			}
		}
	}

	friend Uint32 callback(Uint32 interval, void *param);

	friend void snakeDied(void);

	friend void mouseButtonEvent(const SDL_Event *pEvent);	
	
private:
	SDL_Event events;
	SDL_TimerID id;		//计时器id,防止产生多个计时器
	int delay;			//蛇每次移动的延迟，控制游戏难度用
	int quit;
};

Event *e=new Event;			//全局变量

void start(void)
{
	cout<<"游戏开始！"<<endl<<endl;
	cout<<"说明：WASD或方向键控制蛇的移动！"<<endl;
	(*e).eHandle();
}

int SDL_main(int argc, char* argv[])
{
	
	start();

	SDL_Quit();

	return 0;
}

Uint32 callback(Uint32 interval, void *param) 
{  
	(*e).move((*e).Head,(*e).Tail,(*e).Dir);
	(*e).interpret();
	
    return interval;  
}	 

void snakeDied(void)
{
	cout<<"You are died！"<<endl;
	SDL_RemoveTimer((*e).id);
	(*e).d=1;
}

void mouseButtonEvent(const SDL_Event *pEvent)
{
	int i;

	if(pEvent->button.button!=SDL_BUTTON_LEFT
		||pEvent->button.type!=SDL_MOUSEBUTTONDOWN)		//处理鼠标键位点击,必须用左键点击
	{return;}

	if((pEvent->button.x>=347)&&(pEvent->button.x<=585)&&	//Retry
		(pEvent->button.y>=272)&&(pEvent->button.y<=346))
	{
		(*e).quit=1;
		delete e;
		SDL_Quit();
		e=new Event;
		start();
	}
	if((pEvent->button.x>=363)&&(pEvent->button.x<=552)&&	//Quit
		(pEvent->button.y>=390)&&(pEvent->button.y<=462))
	{
		(*e).quit=1;
	}
}
