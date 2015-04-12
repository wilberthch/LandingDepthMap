#include "main.h"
#include "SDL.h"
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <Windows.h>
#include <Ole2.h>

#include <NuiApi.h>
#include <NuiImageCamera.h>
#include <NuiSensor.h>

using namespace std;

struct vertice{
	float x;
	float y;
	float z;

};
// OpenGL Variables
GLuint textureId;
GLubyte data[width*height*4];
int tamanoReduccion = 2;
GLfloat arregloProfundidad[width*height];
int meshGenerados;
GLfloat arregloProfundidad2[width*height*4];
GLfloat mesh[height*4];


int const muestrasY = 60;
int const muestrasX = 80;
int const tamanoIndexBuffer =muestrasX*muestrasY*6;

int indexBuffer [tamanoIndexBuffer];
vertice vertices[(muestrasY+1)*(muestrasX+1)];

// Kinect variables
HANDLE depthStream;
INuiSensor* sensor;

bool initKinect() {
    // Get a working kinect sensor
    int numSensors;
    if (NuiGetSensorCount(&numSensors) < 0 || numSensors < 1) return false;
    if (NuiCreateSensorByIndex(0, &sensor) < 0) return false;

    // Initialize sensor
    sensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH | NUI_INITIALIZE_FLAG_USES_COLOR);
    sensor->NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH, // Depth camera or rgb camera?
        NUI_IMAGE_RESOLUTION_640x480,                // Image resolution
        0,         // Image stream flags, e.g. near mode
        2,        // Number of frames to buffer
        NULL,     // Event handle
        &depthStream);
    return sensor;
}

void getKinectData(GLubyte* dest,GLfloat * prof,GLfloat *prof2, GLfloat* meshAGenerar) {
    NUI_IMAGE_FRAME imageFrame;
    NUI_LOCKED_RECT LockedRect;
    if (sensor->NuiImageStreamGetNextFrame(depthStream, 0, &imageFrame) < 0) return;
    INuiFrameTexture* texture = imageFrame.pFrameTexture;
    texture->LockRect(0, &LockedRect, NULL, 0);
	const USHORT* curr = (const USHORT*) LockedRect.pBits;
        
	for (int i = 0; i< width * height; i++){
		*(prof + i) =  NuiDepthPixelToDepth(*curr++)/4000.0;
	}


	for (int i = 0; i< width * height; i++){
		*(prof2 + (i*4) ) =   prof[i] ;
		*(prof2 + (i*4) +1 )=  prof[i];
		*(prof2 + (i*4) +2) =  prof[i];
		*(prof2 + (i*4) +3) =  1.0;

	}
	
	int totalVerticesMesh= 0;
	
	
	int saltoMuestrasX= width / muestrasX;
	int saltoMuestrasY= height/ muestrasY;
	
	//Se extrae los vertices que se utilizaran en la graficacion
	for (int i = 0; i <= muestrasY; i++){
		int offsetY= -1;
		if (i ==0){
			offsetY=0;
		}
		for(int j = 0; j <= muestrasX; j++){
			int offsetX= -1;
			if (j ==0){
				offsetX=0;
			}
			vertices[(i*(muestrasX+1))+j].x = j*saltoMuestrasX+offsetX;
			vertices[(i*(muestrasX+1))+j].y = i*saltoMuestrasY+offsetY;
			vertices[(i*(muestrasX+1))+j].z = prof[((saltoMuestrasY+offsetY)*width*i)+ (j*saltoMuestrasX+offsetX)]*300;
			totalVerticesMesh++;
		}
		
	}


	//Se extraen los indices de los vertices para generar los triangulos 
	/*
	for (int i = 0; i < muestrasY; i++){
		for (int j = 0; j<muestrasX;j++){
			indexBuffer[(i*muestrasX*6)+(j*6)  ] = (i*(muestrasX+1))+j+1;
			indexBuffer[(i*muestrasX*6)+(j*6)+1] = (i*(muestrasX+1))+j+2;
			indexBuffer[(i*muestrasX*6)+(j*6)+2] = (i*(muestrasX+1))+1+j+muestrasX+1;
			indexBuffer[(i*muestrasX*6)+(j*6)+3] = (i*(muestrasX+1))+j+2;
			indexBuffer[(i*muestrasX*6)+(j*6)+4] = (i*(muestrasX+1))+j+1+muestrasX+2;
			indexBuffer[(i*muestrasX*6)+(j*6)+5] = (i*(muestrasX+1))+j+1+muestrasX+1;
		}
	}*/

	for (int i = 0; i < muestrasY; i++){
		for (int j = 0; j<muestrasX;j++){
			indexBuffer[(i*muestrasX*6)+(j*6)  ] = (i*(muestrasX+1))+j+1;
			indexBuffer[(i*muestrasX*6)+(j*6)+1] = (i*(muestrasX+1))+1+j+muestrasX+1;
			indexBuffer[(i*muestrasX*6)+(j*6)+2] = (i*(muestrasX+1))+j+2;
			indexBuffer[(i*muestrasX*6)+(j*6)+3] = (i*(muestrasX+1))+j+2;
			indexBuffer[(i*muestrasX*6)+(j*6)+4] = (i*(muestrasX+1))+j+1+muestrasX+1;
			indexBuffer[(i*muestrasX*6)+(j*6)+5] = (i*(muestrasX+1))+j+1+muestrasX+2;
		}
	}

    if (LockedRect.Pitch != 0) {
        const USHORT* curr = (const USHORT*) LockedRect.pBits;
        const USHORT* dataEnd = curr + (width*height);
		int pos = 0;
        while (curr < dataEnd) {
            // Get depth in millimeters
            USHORT depth = NuiDepthPixelToDepth(*curr++);
			
            // Draw a grayscale image of the depth:
            // B,G,R are all set to depth%256, alpha set to 1.
            for (int i = 0; i < 3; ++i)
				*dest++ = (BYTE) (prof[pos]*256);
            *dest++ = 0xff;
			pos++;
        }
    }
    texture->UnlockRect(0);
    sensor->NuiImageStreamReleaseFrame(depthStream, &imageFrame);
}

void drawKinectData() {
    glBindTexture(GL_TEXTURE_2D, textureId);
    getKinectData(data,arregloProfundidad, arregloProfundidad2, mesh);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_FLOAT, (GLvoid*)arregloProfundidad2);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(0, 0, 0);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(width, 0, 0);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(width, height, 0.0f);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(0, height, 0.0f);
    glEnd();
	ofstream salida;
	ostringstream o;
	o << std::to_string(meshGenerados) << ".txt";
	std::cout << o.str();
	salida.open(o.str());

	//Genera TXT de los vertices
	
	/* Version Original
	
	for(int i=0; i<(muestrasX+1)*(muestrasY+1);i++){
		salida <<  vertices[i].x << ";"<< vertices[i].y << ";"<< vertices[i].z ;
		if (i <(muestrasX+1)*(muestrasY+1)- 1){
			salida << ";";
		}
	}
	
	*/


	//Genera TXT de los vertices
	for(int i=0; i<(muestrasX+1)*(muestrasY+1);i++){
		salida << "[" <<  vertices[i].x << ","<< vertices[i].y << ","<< vertices[i].z << "]"  ;
		if (i <(muestrasX+1)*(muestrasY+1)- 1){
			salida << ",";
		}
	}



	salida.close();


	ofstream salidaIndices;
	ostringstream b;
	b << std::to_string(meshGenerados) << "-index.txt";
	std::cout << b.str();
	salidaIndices.open(b.str());

	//Genera TXT de los indices
	/* INDICES ORIGINAL
	
	for(int i=0; i<tamanoIndexBuffer;i++){
		salidaIndices << indexBuffer[i] ;
		if (i < (tamanoIndexBuffer- 1)){
			salidaIndices << ";";
		}
	}*/


	//Genera TXT de los vertices
	for(int i=0; i<tamanoIndexBuffer;i+=3){
		salidaIndices << "[" <<  indexBuffer[i] << ","<< indexBuffer[i+1] << ","<< indexBuffer[i+2] << "]"  ;
		if (i <tamanoIndexBuffer - 4){
			salidaIndices << ",";
		}
	}

	salidaIndices.close();


	meshGenerados++;
}

int main(int argc, char* argv[]) {
    if (!init(argc, argv)) return 1;
    if (!initKinect()) return 1;
	meshGenerados = 0;
    // Initialize textures
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, (GLvoid*) arregloProfundidad2);
    glBindTexture(GL_TEXTURE_2D, 0);

    // OpenGL setup
    glClearColor(0,0,0,0);
    glClearDepth(1.0f);
    glEnable(GL_TEXTURE_2D);

    // Camera setup
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, width, height, 0, 1, -1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Main loop
    execute();
    return 0;
}
