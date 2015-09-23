#include <mutex>
#include <thread>
#include "glut.h"

struct dir_v {
	int x_vel;
	int y_vel;
};

class Ball{
public:
	dir_v direction;
	int x_pos;
	int y_pos;
	int r; //radius

	bool prev_collide = false;
	bool blocked = false;

	int block_tick = 0;
	int arr_pos;
	int arr_rg; //array range
	Ball ** arr;

};

int x_bound = 1000;
int y_bound = 1000;
std::mutex mtx;
int move(Ball * b){

	while (true){
		//collision
		if (b->block_tick != 0)
			continue;
		for (int i = 0; i < b->arr_rg; ++i){
			if (i != b->arr_pos){		//not self;
				int x = abs(b->x_pos - b->arr[i]->x_pos);				//calculate collision
				int y = abs(b->y_pos - b->arr[i]->y_pos);
				int r = abs(b->r + b->arr[i]->r);
				if (x*x + y*y < r*r){		//if collide
					if ((b->x_pos > b->arr[i]->x_pos) //largest x value;
						||
						(b->x_pos == b->arr[i]->x_pos && b->y_pos > b->arr[i]->y_pos) //tied x value, largest y value;
						){
						//sync
						mtx.try_lock();
						//recal x, y
						//get tan of angle using x, y
						b->direction.x_vel = 0;
						b->direction.y_vel = 0;

						b->arr[i]->direction.x_vel = 0;
						b->arr[i]->direction.y_vel = 0;
						b->arr[i]->block_tick = 0;
						mtx.unlock();
					}
					else {					//do nothing and wait for unblock;
						b->block_tick++;
						continue;
					}
				}
			}
		}

		//move
		//check blocktick;
		b->x_pos += b->direction.x_vel + b->direction.x_vel * b->block_tick;
		b->y_pos += b->direction.y_vel + b->direction.y_vel * b->block_tick;
		
		//wall check;
		//top/bot
		if (b->x_pos > x_bound || b->x_pos < 0)
		{
			if (b->x_pos > x_bound - b->r){
				b->x_pos = x_bound - b->r;
			}
			if (b->x_pos - b->r < 0){
				b->x_pos = b->r;
			}
			b->direction.x_vel *= -1;
		}
		//left/right;
		if (b->y_pos > y_bound || b->y_pos < 0)
		{
			if (b->y_pos > y_bound - b->r){
				b->y_pos = y_bound - b->r;
			}
			if (b->y_pos - b->r < 0){
				b->y_pos = b->r;
			}
			b->direction.y_vel *= -1;
		}
	}
}

void display() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color to black and opaque
	glClear(GL_COLOR_BUFFER_BIT);         // Clear the color buffer (background)

	// Draw a Red 1x1 Square centered at origin
	glBegin(GL_QUADS);              // Each set of 4 vertices form a quad
	glColor3f(1.0f, 0.0f, 0.0f); // Red
	glVertex2f(-0.5f, -0.5f);    // x, y
	glVertex2f(0.5f, -0.5f);
	glVertex2f(0.5f, 0.5f);
	glVertex2f(-0.5f, 0.5f);
	glEnd();
	glFlush();  // Render now
}

int main(int argc, char **argv){

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(250, 250);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("hello");
	glutMainLoop();

	int ARR_SIZE = 10;
	Ball** b_arr;
	b_arr = new Ball *[ARR_SIZE];

	//create array;
	for (int i = 0; i < ARR_SIZE; ++i) {
		b_arr[i] = new Ball();

		b_arr[i]->arr_pos = i;        //multithread;
		b_arr[i]->arr = b_arr;

		b_arr[i]->x_pos = 10;         //ball characteristics
		b_arr[i]->y_pos = 10;
		b_arr[i]->r = 10; // radius;

		dir_v vector;                //ball speed vector
		vector.x_vel = 10;
		vector.y_vel = 10;
		b_arr[i]->direction = vector;
	};

	//start thread;
	for (int i = 0; i < ARR_SIZE; ++i) {
		std::thread t(move, b_arr[i]);
		t.join();
	}
	while (true) {
		/*
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glColor3f(1.0, 1.0, 1.0);
		glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
		glBegin(GL_POLYGON);
		glVertex3f(0.25, 0.25, 0.0);
		glVertex3f(0.75, 0.25, 0.0);
		glVertex3f(0.75, 0.75, 0.0);
		glVertex3f(0.25, 0.75, 0.0);
		glEnd();
		glFlush();
		*/
		//during pause
		for (int i = 0; i < ARR_SIZE; ++i) {
		}
	};
}
