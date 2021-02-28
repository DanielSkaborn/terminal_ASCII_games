#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define GAMES 100000

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>
#include "joystick.c"

pthread_t jsr;

int sx, sy;
int	gx, gy;
int	spx, spy;
int dx[10], dy[10];
int score;

volatile int moveg;

void spelPlan(int x, int y);
void gotoxy(int x,int y);
void ritaSpelPlan(void);

void gubbeOnDiamond(void) {
	int i;
	int lx, ly;
	
	for (i=0;i<10;i++) {
		for (lx=0;lx<3;lx++) {
			for (ly=0;ly<3;ly++) {
				if (((gx+lx)==dx[i])&&((gy+ly)==dy[i])) {
					dx[i] = 105;
					dy[i] = 3+2*i;
					score += 10;
					ritaSpelPlan();
				}
			}
		}
	}
	return;
}

void gubbe(void) {
	static int s=0;
	
	static int lpx, lpy;
	
	switch (moveg) {
	case 0:
		gy--;
		break;
	case 1:
		gx++;
		break;
	case 2:
		gy++;
		break;
	case 3:
		gx--;
		break;
	}
	if (gx<0)	gx = 0;
	if (gx>97)	gx = 97;
	if (gy<1)	gy = 1;
	if (gy>48)	gy = 48;
	
	if ((lpx!=gx) || (lpy!=gy)) {

		gubbeOnDiamond();
		
		gotoxy(lpx,lpy);
		printf("   ");
		gotoxy(lpx,lpy+1);
		printf("   ");
		gotoxy(lpx,lpy+2);
		printf("   ");
		
		gotoxy(gx,gy);
		printf("/O\\");
		gotoxy(gx,gy+1);
		printf("/|\\");
		gotoxy(gx,gy+2);
		if (s==1) {
			printf(" /\\");
			s = 0;
		} else {
			printf("/| ");
			s = 1;
		}
		lpx = gx;
		lpy = gy;
	}
}

void gotoxy(int x,int y) {
    printf("%c[%d;%df",0x1B,y,x);
}

void moveGhost(int move) {
	static int lpx, lpy;
	if (move == 0)
		return;
	
	switch (move){
		case 4:
			spx--;
			break;
		case 6:
			spx++;
			break;
			
		case 8:
			spy--;
			break;
			
		case 2:
			spy++;
			break;
	}
	
	if (spx<0) spx = 0;
	if (spx>99) spx = 99;
	if (spy<0) spy = 0;
	if (spy>49) spy = 49;
	
	spelPlan(spx,spy);
	spelPlan(lpx,lpy);
	lpx = spx;
	lpy = spy;
}

void spelPlan(int x, int y) {
	int p=0, i;
	gotoxy(x,y);
	
	if ((x==spx) && (y==spy)) {
		printf("S");
		p = 1;
	}
	else {
		for(i=0;i<10;i++) {
			if ((x==dx[i]) && (y==dy[i])) {
				printf("#");
				p = 1;
			}
		}
	}
	if (p==0)
		printf(" ");
	return;
}

void ritaSpelPlan(void) {
	int x,y,i;
	int p = 0;
	printf("\e[1;1H\e[2J");
	printf("\e[?25l");
	printf("%c\n",7);
	gotoxy(spx,spy);
	printf("S");
	
	for(i=0;i<10;i++) {
		gotoxy(dx[i],dy[i]);
		printf("#");
	}
	
	return;
}

int slumpa(void) {
	int slumptal;
	float slumptalf;
	
	slumptal = rand()&0xFF;
	slumptalf = (float)(slumptal)/2.55;
	slumptal = slumptalf;
	return slumptal;
}

void *jsreader(void *arg) {
	const char *device;
    int js;
    struct js_event event;
    struct axis_state axes[3] = {0};
    size_t axis;

	device = "/dev/input/js0";
	
	while(1) {
		js = open(device, O_RDONLY);
		moveg=1000;	
		while (read_event(js, &event) == 0) {
			switch (event.type)
			{
				case JS_EVENT_BUTTON:
//					printf("Button %u %s\n", event.number, event.value ? "pressed" : "released");
					if (event.value == 1)
						moveg=event.number;	
					else
						moveg=1000;	
					break;
//				case JS_EVENT_AXIS:
//					axis = get_axis_state(&event, axes);
//					if (axis < 3)
//						printf("Axis %zu at (%6d, %6d)\n", axis, axes[axis].x, axes[axis].y);
//					break;
				default:
					// Ignore init events. 
					break;
			}
		}
		close(js);
		sleep(1);
	}
}

int main(void){
	
	int k,a;
	float x;
	int temp;
	
	sx = 100;
	sy = 50;

	gx = 19;
	gy = 30;
	
	spx = 99;
	spy = 30;
	for (k=0;k<10;k++) {
		dx[k] = slumpa();
		dy[k] = slumpa()/2;
	}
	
	pthread_create(&jsr, NULL, jsreader, NULL);
	
	while(1) {
		
		for (k=0;k<10;k++) {
			dx[k] = slumpa();
			dy[k] = slumpa()/2+1;
		}
		ritaSpelPlan();
		gubbe();
		
		for (k=0;k<500;k++) {
			temp = slumpa();
			if (temp<10)
				moveGhost(2);
			else if (temp <20)
				moveGhost(4);
			else if (temp <30)
				moveGhost(8);
			else if (temp <40)
				moveGhost(6);

			gubbe();
			gotoxy(2,52);
			printf("Time: %d  ",500-k);
			gotoxy(20,52);
			printf("Score: %d  ",score);

			fflush(stdout);	
		
			usleep(50000);
		}
	}
}
