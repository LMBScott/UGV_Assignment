#include <zmq.hpp>
#include <Windows.h>

#include "SMStructs.h"
#include "SMFcn.h"
#include "SMObject.h"
#include "Camera.hpp"

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <turbojpeg.h>

void display();
void idle();

GLuint tex;

SMObject* PMObj;
ProcessManagement* PMData;

__int64 Frequency, Counter, prevCounter, PMDownCycles;

//ZMQ settings
zmq::context_t context(1);
zmq::socket_t subscriber(context, ZMQ_SUB);

int main(int argc, char** argv)
{
	//Define window size
	const int WINDOW_WIDTH = 800;
	const int WINDOW_HEIGHT = 600;

	//GL Window setup
	glutInit(&argc, (char**)(argv));
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("MTRN3500 - Camera");

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glGenTextures(1, &tex);

	//Socket to talk to server
	subscriber.connect("tcp://192.168.1.200:26000");
	subscriber.setsockopt(ZMQ_SUBSCRIBE, "", 0);

	PMObj = new SMObject(TEXT("ProcessManagement"), sizeof(ProcessManagement));

	PMObj->SMAccess();

	PMData = (ProcessManagement*)PMObj->pData;

	QueryPerformanceFrequency((LARGE_INTEGER*)&Frequency);
	QueryPerformanceCounter((LARGE_INTEGER*)&Counter);

	prevCounter = Counter;

	PMDownCycles = 0;

	glutMainLoop();

	return 1;
}


void display()
{
	//Set camera as gl texture
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);

	//Map Camera to window
	glBegin(GL_QUADS);
	glTexCoord2f(0, 1); glVertex2f(-1, -1);
	glTexCoord2f(1, 1); glVertex2f(1, -1);
	glTexCoord2f(1, 0); glVertex2f(1, 1);
	glTexCoord2f(0, 0); glVertex2f(-1, 1);
	glEnd();
	glutSwapBuffers();
}
void idle()
{
	prevCounter = Counter;
	QueryPerformanceCounter((LARGE_INTEGER*)&Counter);

	if (PMData->Heartbeat.Flags.Camera) {
		// Get process management down time in seconds
		__int64 PMLifeTime = PMDownCycles / Frequency;

		if (PMLifeTime >= MAX_PM_WAIT) { // Check if proc. man. has been unresponsive for too long
			exit(0);
		}

		PMDownCycles += Counter - prevCounter;
	}
	else {
		PMData->Heartbeat.Flags.Camera = 1;
		PMDownCycles = 0;
	}

	if (PMData->Shutdown.Flags.Camera) {
		exit(0);
	}

	//receive from zmq
	zmq::message_t update;
	if (subscriber.recv(&update, ZMQ_NOBLOCK))
	{
		//Receive camera data
		long unsigned int _jpegSize = update.size();
		std::cout << "received " << _jpegSize << " bytes of data\n";
		unsigned char* _compressedImage = static_cast<unsigned char*>(update.data());
		int jpegSubsamp = 0, width = 0, height = 0;

		//JPEG Decompression
		tjhandle _jpegDecompressor = tjInitDecompress();
		tjDecompressHeader2(_jpegDecompressor, _compressedImage, _jpegSize, &width, &height, &jpegSubsamp);
		unsigned char* buffer = new unsigned char[width * height * 3]; //!< will contain the decompressed image
		printf("Dimensions:  %d   %d\n", height, width);
		tjDecompress2(_jpegDecompressor, _compressedImage, _jpegSize, buffer, width, 0/*pitch*/, height, TJPF_RGB, TJFLAG_FASTDCT);
		tjDestroy(_jpegDecompressor);

		//load texture
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, buffer);
		delete[] buffer;
	}

	display();
}

