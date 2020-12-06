#include <vkcl/vkcl.h>
#include <vector>
#include <GL/glut.h>
#include <GL/glu.h>
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdarg.h>

#define GRAV_CONST 4.30091E-3
#define SOFTENING 1.0E-5

typedef float vec2[2];
typedef float vec3[3];
typedef float vec4[4];

typedef struct body {
	vec4 pos;
	vec4 vel;
//	float mass;
} body_t;

void attract_2body(body_t *a, body_t *b, float deltatime)
{
	vec3 diff = {0, 0, 0};
	vec3 dist = {0, 0, 0};
	float distance = 0.f;

	for (size_t i = 0; i < 3; i++) {
		diff[i] = b->pos[i] - a->pos[i];
		dist[i] = diff[i]*diff[i];
		distance += dist[i];
	}

	distance = sqrtf(distance + SOFTENING);
	distance = distance * distance * distance;

//	float force = GRAV_CONST * a->mass / distance;
	float force = GRAV_CONST * 1000.0 / distance;

	for (size_t i = 0; i < 3; i++) {
		a->vel[i] += force * diff[i] * deltatime;
	}
}

void attract_nbody(body_t *body, size_t n, float deltatime)
{
	for (size_t i = 0; i < n; i++) {
		for (size_t j = 0; j < n; j++) {
			if (i == j)
				continue;

			attract_2body(&body[i], &body[j], deltatime);
		}
	}

	for (size_t i = 0; i < n; i++) {
		for (size_t j = 0; j < 3; j++) {
			body[i].pos[j] += body[i].vel[j] * deltatime;
		}
	}

}

size_t n = 32768;
#define MAX 50
#define WORKGROUPS 8
float deltatime = 0.5f;
float last = 0.0f;
body_t *body;

std::vector<vkcl::Device> devices;
vkcl::Buffer buffer;
vkcl::Buffer metabuffer;
vkcl::Shader shader;

// g++ nbody.cpp -Iinclude -L. -lvkcl -lvulkan-1 -lfreeglut -lOpengl32 -lglu32 -o nbody.exe
// glslangValidator.exe attraction.comp -V -o attraction.spv

float random_range(float min, float max)
{
	return min + (rand() / (float)RAND_MAX) * (max - min);
}

body_t *body_gen(size_t num, vec3 min, vec3 max, float mass)
{
	body_t *body = (body_t *)malloc(sizeof(body_t) * num);
	if (!body)
		return NULL;

	for (size_t i = 0; i < num; i++) {
//		body[i].mass = mass;
		
		for (size_t j = 0; j < 3; j++) {
			body[i].pos[j] = random_range(min[j], max[j]);
			body[i].vel[j] = 0.0000001;
		}
	}

	return body;
}

void renderstr(float x, float y, float z, const char *fmt, ...)
{
	char str[256];
	va_list ap;
	va_start(ap, fmt);
	vsprintf(str, fmt, ap);
	va_end(ap);

	glColor3f(1.0, 1.0, 1.0);
	glPushMatrix();
//	glScalef(1, 1, 1);
	glTranslatef(x, y, z);
	glScalef(MAX * 0.001, MAX * 0.001, MAX * 0.001); // 10

	for (char *c = str; *c != 0; c++) {
		glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
	}

	glPopMatrix();
}


void idle()
{
	float s = (float)glutGet(GLUT_ELAPSED_TIME) * 0.001;
	deltatime = s - last;
	last = s;

	glutPostRedisplay();
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -MAX * 3.25);

	renderstr(-MAX * 2, MAX * 1.35, MAX, "fps: %f", 1/deltatime);

	static float r = 0.0;
	r += 5.f * deltatime;
	glRotatef(r, 0.0, 1.0, 0.0);

	shader.Run((uint32_t)ceil(n / WORKGROUPS), (uint32_t)ceil(n / WORKGROUPS), 1);
	body = (body_t *)buffer.GetData();

	glColor4f(1.0, 0.0, 0.0, 0.10);
	glBegin(GL_POINTS);
	#pragma omp parallel for num_threads(8)
	for (size_t i = 0; i < n; i++) {
		glVertex3fv(body[i].pos);
	}
	glEnd();
	buffer.ReleaseData();

	renderstr(-MAX, -MAX * 1.15, MAX, "0pc");
	renderstr(MAX, -MAX * 1.15, MAX, "%dpc", MAX);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glScalef(MAX, MAX, MAX);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_QUADS);
	glVertex3f(-1.f, 1.f, 1.f);
	glVertex3f(1.f, 1.f, 1.f);
	glVertex3f(1.f, 1.f, -1.f);
	glVertex3f(-1.f, 1.f, -1.f);

	glVertex3f(-1.f, -1.f, 1.f);
	glVertex3f(1.f, -1.f, 1.f);
	glVertex3f(1.f, 1.f, 1.f);
	glVertex3f(-1.f, 1.f, 1.f);
	
	glVertex3f(1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	
	glVertex3f(1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	
	glEnd();

	glFlush();

}

void reshape(int w, int h)
{
	float range = w/2;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(70.0f, (GLdouble)w/(GLdouble)h, 0.1, 0.0);
}

int main (int argc, char **argv)
{
	srand(time(NULL));
	omp_set_num_threads(8);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(640, 480);
	glutCreateWindow("NBODY");

	devices = vkcl::QueryAllDevices();
	buffer.Load(devices[0], sizeof(body_t) * n);
	shader.Load(devices[0], "./attraction.spv", 1);

	vec3 min = { -MAX, -MAX, -MAX };
	vec3 max = {  MAX,  MAX,  MAX };
	body = body_gen(n, min, max, 1000.0f);
	buffer.UploadData((void *)body);
	shader.BindBuffers(&buffer);
	free(body);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glPointSize(4.0f);	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glutReshapeFunc(reshape);
	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutMainLoop();

	free(body);

	return 0;
}

