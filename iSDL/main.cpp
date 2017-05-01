#include "data.h"
using namespace std;
//����������ȽϷ��㣬�������ʾ�����ϵ�ÿһ����������,�����ߵ��ƶ�����ͽ����

Uint32 callback(Uint32 interval, void *param);			//callback

void snakeDied(void);

void mouseButtonEvent(const SDL_Event *pEvent);			//ʧ��ѡ��������굥���¼�

class Interface
{
public:

	void backgroundPaint(void)        //��SDL_RenderPresent
	{
		SDL_RenderCopy(pren,ptex,NULL,NULL);
		//SDL_RenderPresent(pren);
	}

	Interface():pwin(NULL),pren(NULL),ptex(NULL),wall(NULL),food(NULL)
	{
		SDL_Init(SDL_INIT_EVERYTHING);
		pwin=SDL_CreateWindow("iSDL",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,1000,600,SDL_WINDOW_SHOWN);
		pren=SDL_CreateRenderer(pwin,-1,NULL);
		ptex=IMG_LoadTexture(pren,"../image/background.jpg");		//���Ǳ���
		wall=IMG_LoadTexture(pren,"../image/wall.png");
		food=IMG_LoadTexture(pren,"../image/food.png");
		for(int i=0;i<12;i++)					//��ȡԭ���󸱱�
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
	SDL_Texture *ptex;			//����ͼƬ
	SDL_Texture *wall;
	SDL_Texture *food;
	int Map[12][20];			//data.h�еľ��󸱱���������ÿһ֡��
};

class Snake:public Interface
{
public:
	block *initBlock()
	{
		block *tail;
		tail=new block;
		tail->next=NULL;
		tail->x=1;	tail->y=1;			//0��0 ��ǽ

		return tail;			//β�;��ǵ�һ��
	}

	block *createBody(block *tail,int len)       //��������ֵ��3����ô�ߵĳ�ʼ���Ⱦ���4��
	{
		block *head=tail;							//�õ���β�巨������������β�����ȥ����ͷ��
		while(len>0)
		{
			head->next=new block;
			head->next->x=head->x;	head->next->y=head->y+1;	//���������������
			head=head->next;
			head->next=NULL;
			len--;
		}
		return head;		//����ͷ
	}

	void laySnake(block * tail)		//�������ߵ�λ�÷ŵ���ͼ��ȥ
	{
		block *body=tail;
		while(body!=NULL)
		{
			Map[body->x][body->y]=BLOCK;
			body=body->next;
		}
	}

	void layFood(void)			//�ڵ�ͼ���������ʳ��
	{
		int x,y;

		srand((unsigned int)time(0));		//����
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

	void eat(block* &head,direction dir)		//��move��������
	{
		block *p=NULL;

		p=new block;
		if(dir==UP)
		{
			p->x=head->x-1;	p->y=head->y;	p->next=NULL;
			head->next=p;	head=head->next;
			Map[p->x][p->y]=SPACE;				//ԭ����ʳ��ĵط��ÿ�
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

	void move(block* &head,block* &tail,direction dir)		//��callback��������
	{//move()�����л��������߳�ʳ�ϵͳ���·���ʳ���������Ϣȷ��������ߵȹ���,�ܵ���˵�������������ȷ�������е���Ϣ
		block *base=NULL;     //��ת

		for(int i=0;i<12;i++)					//ˢ�¾���,�ǵñ���ԭ��ʳ���λ��
		{
			for(int j=0;j<20;j++)	Map[i][j]=map[i][j];
		}
		SDL_RenderClear(pren);					//��Ⱦ��ҲҪ����ˢ��
		Map[fpos.x][fpos.y]=FOOD;

		if(dir==UP)
		{
			if(Map[head->x-1][head->y]==FOOD)	//�������ǰ����ʳ��͵���eat(),�߳������ڷ�һ��ʳ�����layFood()
			{eat(head,dir);		layFood();}
			else if(Map[head->x][head->y]==WALL)
			{
				snakeDied();
			}
			else
			{
				base=tail->next;										//��β�ͷŵ�ͷǰ��
				tail->x=head->x-1;	tail->y=head->y;	tail->next=NULL;
				head->next=tail;	head=head->next;
				tail=base;
			}
		}
		if(dir==DOWN)							//ע�⣺������Y�����˶�ʱ����ֻ��ѡ��һ��X�᷽��
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
		if(dir==LEFT)									//ע�⣺������X�����˶�ʱ����ֻ��ѡ��һ��Y�᷽��
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

		if(bodyFound(tail))		//������ǲ����Ѿ��Ե����Լ�
		{
			snakeDied();
		}
	}

	SDL_Rect *rectGen(int x,int y)		//��������ֵ���ɾ���
	{
		dst.x=y*BLOCK_W;	dst.y=x*BLOCK_H;
		dst.h=BLOCK_H;		dst.w=BLOCK_W;

		return &dst;
	}

	void interpret(void)					//��Map�е���Ϣ��ȡ�������Ӷ����Ƴ�ÿһ֡
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

	//void judge(int x,int y)		//�ж����ƶ�����һλ��ᷢ��ʲô���������ȡ��Ӧ�Ĳ���
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
		//����һ����(����)
		length=LEN;
		Dir=RIGHT;		//��ʼ��������
		Tail=initBlock();
		Head=createBody(Tail,length-1);
		//��ͼƬ
		snake=IMG_LoadTexture(pren,"../image/block.png");
		//��������Ϣ�ŵ�������
		laySnake(Tail);
		layFood();			//���ǵ�һ��ʳ��
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
	SDL_Texture *snake;		  //��ʵ�������ϵ�һ��ͼƬ
	block *Tail,*Head;        //�ߵ�ͷ��β
	direction Dir;
	SDL_Rect dst;
	Food fpos;				  //ʳ�����걣��
	int d;					  //������־
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
						 cout<<"�˳���Ϸ��"<<endl;
						 quit=1;
						 break;
					 case SDL_KEYDOWN:
						keyPress(events.key.keysym.sym);		//ͨ�����·�������ı䷽��
						if(id==2)	id=SDL_AddTimer(delay, callback, NULL);		//���°�������������ʼ
						break;
					case SDL_MOUSEBUTTONDOWN:		//������¼�
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
	SDL_TimerID id;		//��ʱ��id,��ֹ���������ʱ��
	int delay;			//��ÿ���ƶ����ӳ٣�������Ϸ�Ѷ���
	int quit;
};

Event *e=new Event;			//ȫ�ֱ���

void start(void)
{
	cout<<"��Ϸ��ʼ��"<<endl<<endl;
	cout<<"˵����WASD����������ߵ��ƶ���"<<endl;
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
	cout<<"You are died��"<<endl;
	SDL_RemoveTimer((*e).id);
	(*e).d=1;
}

void mouseButtonEvent(const SDL_Event *pEvent)
{
	int i;

	if(pEvent->button.button!=SDL_BUTTON_LEFT
		||pEvent->button.type!=SDL_MOUSEBUTTONDOWN)		//��������λ���,������������
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
