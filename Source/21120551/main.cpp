#include <iostream>
#include "func.h"
using namespace std;

void main(int argc, char*argv[]) {

	//khởi tạo các biến cần thiết để đưa vào hàm
	char*check = NULL; //check dùng để kiểm tra file hợp lệ hay không

	HEADER header; //header file input
	DIB dib; //dib file input

	HEADER newheader; //header file output
	DIB newdib; //dib file output

	HEADER header8; //header file 8 bit (để lấy bảng màu)
	DIB dib8; //dib file 8 bit (để lấy bảng màu)

	char file8[] = "GetColorTable-8bit.bmp"; //file ảnh 8 bit dùng để lấy bảng màu

	char*temp = NULL; //temp dùng để lưu phần thừa của file nếu kích thước phần DIB>40
	unsigned char*data = NULL; //data dùng để lưu dữ liệu điểm ảnh file input

	char*colortableA = NULL; //colortableA dùng để lưu bảng màu của file ảnh 8 bit
	unsigned char*newdata = NULL; //newdata dùng để lưu dữ liệu điểm ảnh file output

	char*temp8 = NULL; //temp8 dùng để lưu phần thừa của file 8 bit
	unsigned char*data8 = NULL; //data8 dùng để lưu dữ liệu điểm ảnh cho file 8 bit


	if (argc < 4 || argc > 5) //nếu tham số dòng lệnh <4 hoặc >5 thì dừng chương trình
	{
		cout << "Tham so dong lenh khong hop le!" << endl;
		return;
	}
	if (strcmp(argv[1], "-conv")==0) //chuyển ảnh sang trắng đen nếu argv[1] là -conv
	{
		readfile(argv[2], header, dib, temp, data, check, colortableA); //đọc ảnh input
		if (check == NULL) { //nếu ảnh không hợp lệ thì giải phóng vùng nhớ rồi dừng chương trình
			delete[] temp;
			delete[] data;
			delete[] colortableA;
			return;
		}
		readfile(file8, header8, dib8, temp8, data8, check, colortableA); /*lấy bảng màu từ một ảnh 8 bit khác để 
																		  gán sang ảnh output*/
		convert8bit(header, dib, data, newheader, newdib, newdata, check); //chuyển ảnh sang trắng đen
		if (check == NULL) { //nếu ảnh input không hợp lệ (bpp không phải 24 hoặc 32) thì giải phóng vùng nhớ rồi dừng chương trình
			delete[] temp;
			delete[] newdata;
			delete[] data;
			delete[] colortableA;
			delete[] temp8;
			delete[] data8;
			return;
		}
		writefile(argv[3], newheader, newdib, temp, newdata, colortableA); //ghi ảnh output
	}
	else if (strcmp(argv[1], "-zoom")==0) { //thu nhỏ ảnh theo tỉ lệ S nếu argv[1] là -conv
		if (argc < 5) //nếu tham số dòng lệnh <5 thì dừng chương trình
		{
			cout << "Tham so dong lenh khong hop le!" << endl;
			return;
		}
		readfile(argv[2], header, dib, temp, data, check, colortableA); //đọc ảnh input
		if (check == NULL) { //nếu ảnh không hợp lệ thì giải phóng vùng nhớ rồi dừng chương trình
			delete[] temp;
			delete[] data;
			delete[] colortableA;
			return;
		}
		resize(header, dib, newheader, newdib, data, newdata, argv[4]); //thu nhỏ ảnh theo tỉ lệ S ở argv[4]
		writefile(argv[3], newheader, newdib, temp, newdata, colortableA); //ghi ảnh output

	}
	else { //thông báo nếu tham số dòng lệnh không hợp lệ và dừng chương trình
		cout << "Tham so dong lenh khong hop le!" << endl;
		return;
	}
		

	//giải phóng vùng nhớ
	delete[] temp;
	delete[] newdata;
	delete[] data;
	delete[] colortableA;
	delete[] temp8;
	delete[] data8;
}