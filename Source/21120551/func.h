#pragma once
#pragma pack(1)

struct HEADER
{
	char signature[2];
	unsigned int filesize;
	unsigned int reserved;
	unsigned int offset;
};
struct DIB
{
	unsigned int dibsize;
	unsigned int width;
	unsigned int height;
	unsigned short planes;
	unsigned short bpp;
	unsigned int compress;
	unsigned int imgsize;
	unsigned int x_pizelperM;
	unsigned int y_pizelperM;
	unsigned int colors_used;
	unsigned int important_color;
};

void readfile(char*filepath, HEADER&header, DIB&dib, char*&temp, unsigned char*&data, char*&check, char*&colortableA);
void writefile(char*filepath, HEADER&header, DIB&dib, char*temp, unsigned char*data, char*colortableA);
void convert8bit(HEADER header, DIB dib, unsigned char*&data, HEADER&newheader, DIB&newdib, unsigned char*&newdata, char*&check);
void resize(HEADER header, DIB dib, HEADER&newheader, DIB&newdib, unsigned char*data, unsigned char*&newdata, char*S);