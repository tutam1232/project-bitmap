#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string.h>
#include "func.h"
using namespace std;

//hàm đọc file bmp
void readfile(char*filepath, HEADER&header, DIB&dib, char*&temp, unsigned char*&data, char*&check, char*&colortableA) {

	//mở file bmp và kiểm tra file có mở được không
	ifstream f(filepath, ios::binary);
	
	if (!f)
	{
		cout << "Khong mo duoc file!" << endl;
		return;
	}

	//đọc header và kiểm tra file được mở có phải file bmp không?
	f.read((char*)&header, 14);
	check = strstr(header.signature, "BM");
	if (check==NULL)
	{
		cout << "Day khong phai mot file bmp!";
		return;
	}

	//đọc dib và kiểm tra liệu kích thước dib có phải 40 byte hay không? nếu không thì lưu phần còn lại vào temp
	f.read((char*)&dib, 40);
	if (dib.dibsize != 40)
	{
		f.seekg(54, ios::beg);
		temp = new char[dib.dibsize - 40];
		f.read(temp, dib.dibsize - 40);
	}

	//đọc bảng màu (đối với ảnh 8 bit)
	if (dib.bpp == 8)
	{
		int somau = 4 * pow(2, 8);
		colortableA = new char[somau];
		f.seekg(14 + dib.dibsize, ios::beg);
		f.read(colortableA, somau);
	}
	//đưa con trỏ đến vị trí lưu dữ liệu điểm ảnh và đọc dữ liệu điểm ảnh
	f.seekg(header.offset, ios::beg);
	data = new unsigned char[header.filesize - header.offset];
	f.read((char*)data, header.filesize - header.offset);
	
	// đóng file
	f.close();
}

//hàm ghi file bmp
void writefile(char*filepath, HEADER&header, DIB&dib, char*temp, unsigned char*data,char*colortableA) {

	//mở file để lưu ảnh và kiểm tra file có mở được hay không
	ofstream f(filepath, ios::binary);
	if (!f)
	{
		cout << "File khong ton tai!" << endl;
		return;
	}

	//ghi header
	f.write((char*)&header, 14);

	//ghi dib
	f.write((char*)&dib, 40);

	//ghi phần dư nếu kích thước dib không phải 40
	if (dib.dibsize!=40)
	{
		f.write(temp, dib.dibsize - 40);
	}
	//ghi bảng màu (đối với ảnh 8 bit)
	if (dib.bpp == 8)
	{
		f.write(colortableA, 4 * pow(2, dib.bpp));
	}

	//ghi dữ liệu điểm ảnh
	f.write((char*)data, header.filesize - header.offset);

	f.close();
}

//hàm chuyển ảnh 24 hoặc 32 bit sang ảnh 8 bit
void convert8bit(HEADER header, DIB dib, unsigned char*&data,HEADER&newheader,DIB&newdib, unsigned char*&newdata, char*&check) {

	//gán header và dib của ảnh input sang ảnh output
	newheader = header;
	newdib = dib;

	//thay đổi bpp của ảnh output thành 8 và điều chỉnh offset (vì ảnh 8 bit có bảng màu)
	newdib.bpp = 8;
	newheader.offset = 14 + newdib.dibsize + 1024;

	//tính số padding byte ảnh input và output, từ đó tính lại kích thước dữ liệu ảnh và kích thước ảnh của ảnh output
	int oldpadding = (4 - (((dib.bpp / 8)*dib.width) % 4)) % 4; //số padding byte ảnh input
	int padding = (4 - (((newdib.bpp / 8)*newdib.width) % 4)) % 4; //số padding byte ảnh output
	//tính kích thước dữ liệu điểm ảnh của ảnh output
	newdib.imgsize = newdib.width*newdib.height*(newdib.bpp / 8) + newdib.height*padding; 
	newheader.filesize = newheader.offset + newdib.imgsize; //tính kích thước ảnh của ảnh output

	//tạo mảng động chứa dữ liệu điểm ảnh cho file output
	newdata = new unsigned char[newdib.imgsize];
	unsigned int average = 0;

	//chuyển ảnh input 24 bit sang một ảnh output 8 bit
	if (dib.bpp == 24)
	{
		int j = 0;
		int temp_j = 0;

		for (int i = 0; i < dib.imgsize; i++)
		{
			//tính trung bình cộng của các giá trị Blue, Green, Red của từng pixel từ ảnh input rồi gán vào ảnh output
			average = (data[i] + data[++i] + data[++i]) / 3;
			newdata[j] = average;
			j++;
			temp_j++;

			/* khi temp_j = chiều ngang của ảnh output => đã xử lý xong một dòng dữ liệu điểm ảnh => ta chuyển
			sang chèn padding byte cho ảnh output rồi xử lý dòng tiếp theo của dữ liệu điểm ảnh */
			if (temp_j == newdib.width)
			{
				for (int k = 0; k < padding; k++)
				{
					newdata[j] = NULL;
					j++;
				}
				i = i + oldpadding;
				temp_j = 0; /*gán lại temp_j=0 để trở lại vòng lặp và so sánh nếu temp_j=newdib.width thì tiếp tục 
				chèn padding byte cho dòng tiếp theo trong mảng 2 chiều của dữ liệu điểm ảnh*/

			}
		}
	}

	//chuyển ảnh input 32 bit sang một ảnh output 8 bit
	else if (dib.bpp == 32)
	{
		int j = 0;
		int temp_j = 0;
		for (int i = 0; i < dib.imgsize; i+=2)
		{
			/*tính trung bình cộng của các giá trị Blue, Green, Red từ ảnh input rồi gán vào ảnh output,
			bỏ qua giá trị Alpha trong ABRG*/
			
			average = (data[i] + data[++i] + data[++i]) / 3;
			newdata[j] = average;
			j++;
			temp_j++;

			/* khi temp_j = chiều ngang của ảnh output => đã xử lý xong một dòng dữ liệu điểm ảnh => ta chuyển
			sang chèn padding byte cho ảnh output rồi xử lý dòng tiếp theo của dữ liệu điểm ảnh */
			if (temp_j == newdib.width)
			{
				for (int k = 0; k < padding; k++)
				{
					newdata[j] = NULL;
					j++;
				}
				i = i + oldpadding;
				temp_j = 0; /*gán lại temp_j=0 để trở lại vòng lặp và so sánh nếu temp_j=newdib.width thì tiếp tục
				chèn padding byte cho dòng tiếp theo trong mảng 2 chiều của dữ liệu điểm ảnh*/

			}
			
		}
	}
	//nếu ảnh input không phải 24 bit hoặc 32 bit thì ảnh không hợp lệ
	else
	{
		cout << "Anh dau vao phai la anh 24 bit hoac 32 bit!" << endl;
		check = NULL; //gán check về NULL, xem như ảnh input không hợp lệ
		return;
	}
	
}

void resize(HEADER header, DIB dib, HEADER&newheader, DIB&newdib, unsigned char*data, unsigned char*&newdata, char*S) {

	int s = atoi(S);
	//gán header và dib của ảnh input sang ảnh output
	newheader = header;
	newdib = dib;
	/*x để lưu thêm 1 pixel cho chiều ngang ảnh output nếu chia dư chiều ngang ảnh input cho S có kết quả khác 0,
	gán giá trị mặc định là 0*/
	int x = 0; 
	/*y để lưu thêm 1 pixel cho chiều dọc ảnh output nếu chia dư chiều dọc ảnh input cho S có kết quả khác 0,
	gán giá trị mặc định là 0*/
	int y = 0;

	int oldpadding = (4 - (((dib.bpp / 8)*dib.width) % 4)) % 4; //số padding byte ảnh input

	int temp_width = dib.width % s; //temp_width để tính số pixel dư theo chiều ngang nếu chia chiều ngang ảnh cho S
	int temp_height = dib.height % s; //temp_height để tính số pixel dư theo chiều dọc nếu chia chiều dọc ảnh cho S

	newdib.width = (dib.width - temp_width) / s; //chiều ngang ảnh output (tạm thời)
	if (temp_width != 0) {/*nếu kết quả của chiều ngang ảnh input chia dư cho S khác 0, tăng chiều ngang của ảnh 
							output lên 1 để lưu phần dư đó, gán x=1*/
		x = 1;
		newdib.width++;
	}

	newdib.height = (dib.height - temp_height) / s; //chiều dọc ảnh output (tạm thời)
	if (temp_height != 0) { /*nếu kết quả của chiều dọc ảnh input chia dư cho S khác 0, tăng chiều dọc của ảnh 
							output lên 1 để lưu phần dư đó, gán y=1*/
		y = 1;
		newdib.height++;
	}
	//tính kích thước dữ liệu điểm ảnh output (chưa có padding byte)
	int temp_newimgsize = newdib.width*newdib.height*(newdib.bpp / 8); 

	int padding = (4 - (((newdib.bpp / 8)*newdib.width) % 4)) % 4; //số padding byte ảnh output

	//tính kích thước dữ liệu điểm ảnh output (đã có padding byte)
	newdib.imgsize = temp_newimgsize + padding * newdib.height; 

	//tính filesize ảnh output
	newheader.filesize = newheader.offset + newdib.imgsize;

	//newdata để lưu phần dữ liệu điểm ảnh của ảnh output
	newdata = new unsigned char[newdib.imgsize];



	int dem = 0; //biến dem để xác định vị trí dữ liệu điểm ảnh của ảnh output trong mảng newdata
	int average = 0; //biến average để tính giá trị trung bình của các byte trong ô SxS
	int temp = 0; //biến temp để xác định vị trí của các byte trong dữu liệu điểm ảnh input
	int temp_dem = 0; /*biến temp_dem để xác định vị trí của temp là đang ở padding byte, từ đó ta sẽ chèn
					  padding byte vào dữ liệu điểm ảnh của ảnh output*/

	// thu nhỏ ảnh 8 bit
	if (newdib.bpp == 8) {
		int i = 0; // i để đếm chiều dọc theo pixel của ảnh output (chưa tính y)
		for (i; i < newdib.height - y; i++)
		{
			for (int j = 0; j < newdib.width - x; j++) // j để đếm chiều ngang theo pixel của ảnh output (chưa tính x)
			{
				//tính tổng giá trị của S*S byte 
				for (int k = 0; k < s; k++)
				{
					for (int e = 0; e < s; e++)
					{
						average += data[temp];
						temp++;
					}
					temp += dib.width + oldpadding - s;
				}
				//tính trung bình cộng giá trị của S*S byte và gán vào giá trị điểm ảnh của ảnh output
				newdata[dem] = average / (s*s); 
				average = 0; // gán trở lại average = 0 để tiếp tục tính trung bình cộng byte tiếp theo
				//tăng biến dem và temp_dem lên một đơn vị
				dem++; 
				temp_dem++; 
				/*khi temp_dem = newdib.width - x có nghĩa dem đã đi đến padding byte hoặc byte đã chia dư của 
				ảnh output*/
				if (temp_dem == newdib.width - x)
				{
					/*nếu x khác 0 nghĩa là có byte dư trên mỗi dòng (cột cuối trong ảnh input không có kích thước S*S), 
					lúc này ta sẽ tính giá trị trung bình của các byte trên cột này theo kích thước S*temp_width
					lần lượt cho đến hết chiều cao được tính theo newdib.height - y và gán vào dữ liệu điểm ảnh
					của ảnh output*/
					if (x != 0) 
					{
						temp = s * i * (dib.width) + s * i * oldpadding + (dib.width - temp_width); // đưa con trỏ đến đầu byte dư
						/*tính trung bình cộng các byte dư theo kích thước S*temp_width và gán vào giá trị điểm ảnh của
						ảnh output*/
						for (int i = 0; i < s; i++)
						{
							for (int j = 0; j < temp_width; j++) {
								average += data[temp];
								temp++;
							}
							temp += dib.width + oldpadding - temp_width;
						}
						newdata[dem] = average / (s*temp_width);
						dem++;
						average = 0;
					}
					//gán padding byte cho ảnh output
					dem += padding;
					temp_dem = 0;
				}
				temp = s * i * dib.width + s * i * oldpadding + s * (j + 1); //	đưa temp đến đầu ô S*S tiếp theo trong ảnh input

			}
			temp = s * (i + 1) * dib.width + s * (i + 1) * oldpadding; /*đưa temp đến dòng tiếp theo trong ảnh input cách
			dòng trước đó một khoảng S dòng*/
			average = 0;

		}
		/*khi đã tính và ghi hết dữ liệu điểm ảnh từ ảnh input vào ảnh output, nếu y khác 0 nghĩa là còn byte dư trong
		ảnh output (dòng cuối cùng trong ảnh input không có kích thước S*S), ta sẽ lần lượt tính trung bình cộng các
		byte của dòng này theo kích thước (newdib.width – x)*temp_height lần lượt cho đến hết theo chiều ngang
		newdib.width - x và gán vào kích thước dữ liệu điểm ảnh trong ảnh output*/
		if (y != 0)
		{
			temp = s * (i + 1) * dib.width + s * (i + 1) * oldpadding; //đưa temp đến đầu phần byte dư này
			//tính trung bình cộng của các byte dư này và gán vào ảnh output
			for (int z = 0; z < newdib.width - x; z++) {
				for (int j = 0; j < temp_height; j++)
				{
					for (int k = 0; k < s; k++) {
						average += data[temp];
						temp++;
					}
					temp += dib.width + oldpadding - s;

				}
				newdata[dem] = average / (s*temp_height);
				dem++;
				average = 0;
				temp = s * i * dib.width + s * i * oldpadding + s * (z + 1);
			}
			/*nếu x khác 0 nghĩa là có một cột không phải kích thước S*S, ta sẽ tính trung bình cộng các pixel cuối
			bên phải trong ảnh bitmap và gán vào dữ liệu điểm ảnh của ảnh output*/
			if (x != 0) {
				for (int z = 0; z < temp_height; z++)
				{
					for (int j = 0; j < temp_width; j++)
					{
						average += data[temp];
						temp++;
					}
					temp += dib.width + oldpadding - temp_width;//ok
				}
				newdata[dem] = average / (temp_width*temp_height);
				dem++;
				average = 0;
			}
		}
	}

	//thu nhỏ ảnh 24 bit hoặc 32 bit
	if (newdib.bpp == 24 || newdib.bpp==32) {

		int dem_pixeldata = 0;
		int dem_newwidth = 0;
		int next_pixel = 1;
		int i = 0;

		for (i; i < newdib.height - y; i++) //i dùng để đếm pixel theo chiều dọc (chưa tính y)
		{
			for (int j = 0; j < (newdib.width-x) * (newdib.bpp / 8); j++) //j dùng để đếm pixel theo chiều ngang (chưa tính x)
			{
				for (int k = 0; k < s; k++) //k để đếm số pixel s cần thu nhỏ theo chiều dọc
				{
					for (int e = 0; e < s; e++) //e để đếm số pixel s cần thu nhỏ theo chiều ngang
					{
						average += data[temp]; //cộng giá trị màu B (hoặc G, hoặc R) vào biến temp 
						temp += (newdib.bpp / 8); //temp đưa đến giá trị màu B (hoặc G, hoặc R) của pixel tiếp theo
					}
					//temp đưa đến giá trị màu B (hoặc G, hoặc R) của dòng tiếp theo trong ô SxS
					temp += dib.width*(newdib.bpp / 8) + oldpadding - s * (newdib.bpp / 8);
				}
				newdata[dem] = average / (s*s); /*tính trung bình cộng giá trị màu B (hoặc G, hoặc R) và gán vào
												mảng dữ liệu điểm ảnh của ảnh output*/
				dem_pixeldata++; //đếm số byte đã đi qua
				dem++; //tăng biến dem để giá trị điểm ảnh của ảnh output sẽ được gán vào byte tiếp theo ở lượt sau
				temp_dem++; //tăng biến temp_dem
				average = 0; //gán lại average = 0 để tiếp tục tính trung bình cộng các giá trị màu ở lượt sau

				/*khi dem_pixeldata = (newdib.bpp / 8) nghĩa là ta đã gán hết các giá trị ABRG hoặc BRG của một ô
				SxS của ảnh input vào một pixel của ảnh output*/
				if (dem_pixeldata == (newdib.bpp / 8)) {
					dem_newwidth++; //tăng giá trị của dem_newwidth lên 1 đơn vị (dùng để tính vị trí tiếp theo của temp)
					dem_pixeldata = 0; /*gán lại giá trị của dem_pixeldata thành 0*/
				}
				//đưa temp đến đầu ô SxS tiếp theo
				temp = s * i*dib.width*(newdib.bpp / 8) + s * i*oldpadding + s * dem_newwidth * (newdib.bpp / 8);

				/*khi dem_pixeldata khác 0 nghĩa là vẫn chưa gán hết giá trị màu ABGR hoặc BGR của một ô SxS
				vào 1 pixel trong phần dữ liệu điểm ảnh của ảnh output*/
				if (dem_pixeldata != 0)
				{
					temp += next_pixel; //đưa temp đến byte màu tiếp theo trong ảnh input
					next_pixel++;
				}
				else
					next_pixel = 1; /*nếu dem_pixeldata=0 nghĩa là ta đã gán hết giá trị màu ABGR hoặc BGR của một ô SxS
									vào 1 pixel trong phần dữ liệu điểm ảnh của ảnh output, ta gán lại next_pixel=1 để 
									trong vòng lặp sau sẽ đưa temp đến đúng vị trị của byte màu tiếp theo trong ô SxS*/

				/*nếu temp_dem = ((newdib.width - x) * (newdib.bpp / 8)) nghĩa là temp đã gán hết các giá trị màu
				của ảnh input vào ảnh output (chưa tính x)*/
				if (temp_dem == ((newdib.width-x) * (newdib.bpp / 8)))
				{
					//nếu x khác 0, ta gán tiếp các giá trị màu trong ô Sxtemp_width vào ảnh output
					if (x != 0)
					{
						temp = s * i * (dib.width) *(newdib.bpp / 8) + s * i * oldpadding + (dib.width - temp_width)*(newdib.bpp / 8); 
						int next_pixel2 = 1;

						for (int k = 0; k < (newdib.bpp / 8); k++) {
							for (int i = 0; i < s; i++)
							{
								for (int j = 0; j < temp_width; j++) {
									average += data[temp];
									temp += (newdib.bpp / 8);
								}
								temp += dib.width*(newdib.bpp / 8) + oldpadding - temp_width * (newdib.bpp / 8);
							}
							newdata[dem] = average / (s*temp_width);
							dem++;
							average = 0;
							temp = s * i*dib.width*(newdib.bpp / 8) + s * i*oldpadding + (newdib.bpp / 8) * (dib.width-temp_width)+ next_pixel2;
							next_pixel2++;
						}

					}
					dem += padding;
					temp_dem = 0;
				}  

			}
			temp = s * (i + 1) * dib.width * (newdib.bpp / 8) + s * (i + 1) * oldpadding;
			average = 0;
			dem_newwidth = 0;

		}
		/*đến đây, chương trình đã gán hết các giá trị màu của ảnh input vào ảnh output, trừ phần dư 
		(newdib.width - x)*(dib.bpp / 8) x temp_height ở các dòng cuối trong phần dữ liệu điểm ảnh của ảnh input,
		nếu y khác 0 nghĩa là phần dư này tồn tại, ta tiếp tục gán phần dư này vào ảnh output*/
		if (y != 0)
		{
			dem_newwidth = 0;
			dem_pixeldata = 0;
			int next_pixel2=1;

			temp = s * i * dib.width * (newdib.bpp / 8) + s * i * oldpadding;
			for (int z = 0; z < (newdib.width - x)*(dib.bpp / 8); z++) {

					for (int j = 0; j < temp_height; j++)
					{
						for (int k = 0; k < s; k++) {
							average += data[temp];
							temp += (dib.bpp / 8);
						}
						temp += dib.width*(newdib.bpp / 8) + oldpadding - s * (newdib.bpp / 8);

					}
					newdata[dem] = average / (s*temp_height);
					dem_pixeldata++;
					dem++;
					average = 0;
					
					if (dem_pixeldata == (newdib.bpp / 8)) {
						dem_newwidth++;
						dem_pixeldata = 0;
					}
					temp = s * i*dib.width*(newdib.bpp / 8) + s * i*oldpadding + s * dem_newwidth * (newdib.bpp / 8);
					if (dem_pixeldata != 0)
					{
						temp += next_pixel2;
						next_pixel2++;
					}
					else
						next_pixel2 = 1;
			}
			/*đến đây, nếu x khác 0 nghĩa là còn ô cuối cùng trong giá trị điểm ảnh của ảnh input ta chưa gán vào ảnh
			output nên ta tiếp tục tính trung bình cộng các giá trị màu của ô này trong ảnh input rồi tiếp tục
			gán vào ảnh output*/
			if (x != 0) {
				next_pixel2 = 1;
				for (int k = 0; k < (newdib.bpp / 8); k++){
					for (int z = 0; z < temp_height; z++)
					{
						for (int j = 0; j < temp_width; j++)
						{
							average += data[temp];
							temp += (newdib.bpp / 8);
						}
						temp += dib.width * (newdib.bpp / 8) + oldpadding - temp_width * (newdib.bpp / 8);
					}
					newdata[dem] = average / (temp_width*temp_height);
					dem++;
					average = 0;
					temp = s * i*dib.width*(newdib.bpp / 8) + s * i*oldpadding + s * (newdib.bpp / 8) * (newdib.width - x) + next_pixel2;
					next_pixel2++;
				}
			}
		}
	}
}

